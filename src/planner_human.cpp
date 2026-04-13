//#define PKAI_IMPORT
#include "pokemonai/planner_human.h"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_possible.h"
#include "pokemonai/pkCU.h"

PlannerHuman::PlannerHuman(const Config& cfg) : PlannerHuman(cfg, std::cin) {};


PlannerHuman& PlannerHuman::setEngine(const std::shared_ptr<PkCU>& cu) {
  // copy, rather than share
  Planner::setEngine(std::make_shared<PkCU>(*cu));
  cu_->setAllowInvalidMoves(PkCU::ActionValidationMethod::WAIT_ONLY);
  return *this;
}


PlyResult PlannerHuman::generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const {
  fmt::print("{}", fmt::streamed(origin));
  printActions(origin);

  PlyResult result;
  result.agentAction = actionSelect(origin.getEnv());
  return result;
}


std::unordered_map<size_t, ActionVector> PlannerHuman::actionsPerMoveType(
    const ConstEnvironmentVolatile& env, const Actor& actor) const {
  std::unordered_map<size_t, ActionVector> actionsPerMoveType;
  for (const auto& action : cu_->getValidMoveActions(env, actor)) {
    actionsPerMoveType[action.type()].push_back(action);
  }
  return actionsPerMoveType;
}


void PlannerHuman::printActions(const ConstEnvironmentPossible& env) const {
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);
  auto actors = cTeam.getActiveActors();

  double currentFitness = 0.0;
  if (shouldEval()) {
    currentFitness = eval_->evaluate(env, agentTeam_).fitness.lowerBound();

    fmt::print("Current Evaluator Fitness: {:.4f}\n", currentFitness);
  }

  printActions_moves(env, cTeam, actors, currentFitness);
  printActions_swaps(env, cTeam, actors, currentFitness);
  printActions_activations(env, cTeam, currentFitness);
}


ActionMap PlannerHuman::actionSelect(
    const ConstEnvironmentVolatile& env) const {
  std::string input;
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);

  ActionMap actionMap;

  // 1. Handle currently active actors
  auto actors = cTeam.getActiveActors();
  for (const auto& actor : actors) {
    Action action;
    do {
      fmt::print(
          "Please select the index of your desired action for Actor {}:\n",
          fmt::streamed(actor));
      if (!getline(in_.get(), input)) {
        throw std::runtime_error(
            "PlannerHuman: Input stream exhausted while reading actor action");
      }
      std::stringstream inputResult(input);

      // determine if action is valid:
      if (!(inputResult >> action) || !cu_->isValidAction(env, actor, action)) {
        fmt::print("Invalid action \"{}\"!\n", input);
        continue;
      }

      break;
    } while (true);
    actionMap[actor] = action;
  }

  // 2. Handle empty slots (entries from bench)
  size_t numToActivate = cu_->numRequiredToActivate(cTeam);
  if (numToActivate > 0) {
    for (size_t i = 0; i < numToActivate; ++i) {
      Action action = Action::activate();
      Actor actor(agentTeam_, 0);
      do {
        fmt::print(
            "Please select the index of the pokemon you wish to activate "
            "({}/{}):\n",
            i + 1,
            numToActivate);
        if (!getline(in_.get(), input)) {
          throw std::runtime_error(
              "PlannerHuman: Input stream exhausted while reading activation");
        }
        std::stringstream inputResult(input);

        if (!(inputResult >> actor) ||
            !cu_->isValidAction(env, actor, action)) {
          fmt::print("Invalid activation \"{}\"!\n", input);
          continue;
        }

        if (actionMap.count(actor)) {
          fmt::print("Actor {} already selected!\n", fmt::streamed(actor));
          continue;
        }

        break;
      } while (true);
      actionMap[actor] = action;
    }
  }

  return actionMap;
};  // endOf actionSelect


void PlannerHuman::printActions_moves(
    const ConstEnvironmentPossible& env,
    const ConstTeamVolatile& cTeam,
    const std::vector<Actor>& actors,
    double currentFitness) const {
  for (const auto& actor : actors) {
    fmt::print(
        "Active pokemon {}: {}\n",
        fmt::streamed(actor),
        fmt::streamed(env.teammate(actor)));
    for (const auto& [moveType, actions] : actionsPerMoveType(env, actor)) {
      Action protoype{moveType, Action::FRIENDLY_NONE, Action::HOSTILE_NONE};
      if (protoype.isStruggle()) {
        fmt::print("\t\"Struggle\" -/-\n");
      } else if (protoype.isWait()) {
        fmt::print("\t\"Nothing\" -/-\n");
      } else {
        const ConstMoveVolatile cMove = env.teammate(actor).getMV(protoype);
        cMove.prettyPrint("\t");
      }

      std::vector<std::string> actionDeltas;
      for (const auto& action : actions) {
        std::string deltaStr =
            getFitnessDelta(env, cTeam, currentFitness, actor, action);
        actionDeltas.push_back(
            fmt::format("{}{}", fmt::streamed(action), deltaStr));
      }
      fmt::print("\t[{}]\n", fmt::join(actionDeltas, ", "));
    }
  }
}


void PlannerHuman::printActions_swaps(
    const ConstEnvironmentPossible& env,
    const ConstTeamVolatile& cTeam,
    const std::vector<Actor>& actors,
    double currentFitness) const {
  if (cTeam.nv().getNumTeammates() > actors.size()) {
    fmt::print("Or switch to a sidelined pokemon: \n");
    std::map<size_t, std::vector<std::pair<Actor, Action>>> sidelinedToActors;
    for (const auto& actor : actors) {
      for (const auto& action : cu_->getValidSwapActions(env.getEnv(), actor)) {
        sidelinedToActors[action.iFriendly()].push_back({actor, action});
      }
    }

    for (const auto& [teammateIdx, actorActions] : sidelinedToActors) {
      const ConstPokemonVolatile cTeammate = cTeam.teammate(teammateIdx);

      std::vector<std::string> swapDeltas;
      for (const auto& [actor, action] : actorActions) {
        std::string deltaStr =
            getFitnessDelta(env, cTeam, currentFitness, actor, action);
        swapDeltas.push_back(
            fmt::format("{}{}", fmt::streamed(actor), deltaStr));
      }

      cTeammate.prettyPrint(
          fmt::format("\t{}: ", fmt::streamed(Action::swap(teammateIdx))),
          fmt::format(" [{}]", fmt::join(swapDeltas, ", ")));
    }
  }
}


void PlannerHuman::printActions_activations(
    const ConstEnvironmentPossible& env,
    const ConstTeamVolatile& cTeam,
    double currentFitness) const {
  if (cu_->numRequiredToActivate(cTeam) > 0) {
    fmt::print("Or bring a new pokemon into the field: \n");
    for (const auto& [actor, action] : cu_->getValidEntryActions(cTeam)) {
      std::string deltaStr =
          getFitnessDelta(env, cTeam, currentFitness, actor, action);

      const ConstPokemonVolatile cTeammate = cTeam.teammate(actor);
      cTeammate.prettyPrint(
          fmt::format("\t{}-{} ", fmt::streamed(actor), fmt::streamed(action)),
          deltaStr);
    }
  }
}


std::string PlannerHuman::getFitnessDelta(
    const ConstEnvironmentPossible& env,
    const ConstTeamVolatile& cTeam,
    double currentFitness,
    const Actor& actor,
    const Action& action) const {
  if (!shouldEval()) return std::string();

  ActionMap actionMap{{actor, action}};
  fillRemainingActionsWithDefaults(cTeam, actionMap);

  EvalResult result = recurse_beta(env, actionMap, 1);
  double delta = result.fitness.lowerBound() - currentFitness;

  return fmt::format(" ({:+.2f})", delta);
}
