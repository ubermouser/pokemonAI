//#define PKAI_IMPORT
#include "pokemonai/planner_human.h"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <sstream>
#include <string>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_possible.h"
#include "pokemonai/pkCU.h"

PlannerHuman::PlannerHuman(const Config& cfg) : PlannerHuman(cfg, std::cin) {};


PlyResult PlannerHuman::generateSolutionAtDepth(
    const ConstEnvironmentPossible& origin, size_t maxPly) const {
  fmt::print("{}", fmt::streamed(origin));
  printActions(origin);

  PlyResult result;
  result.agentAction = actionSelect(origin.getEnv());
  return result;
}


void PlannerHuman::printActions(const ConstEnvironmentPossible& env) const {
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);
  auto actors = cTeam.getActiveActors();

  double currentFitness = 0.0;
  if (shouldEval()) {
    currentFitness = eval_->evaluate(env, agentTeam_).fitness.lowerBound();

    fmt::print("Current Evaluator Fitness: {:.4f}\n", currentFitness);
  }

  auto getFitnessDelta = [&](const Actor& agent, const Action& agentAction) {
    if (!shouldEval()) return 0.0;

    ActionMap actionMap;
    for (const auto& actor : actors) {
      actionMap[actor] = (actor == agent) ? agentAction : Action::wait();
    }

    EvalResult worst = recurse_beta(env, actionMap, 1);
    return worst.fitness.lowerBound() - currentFitness;
  };

  fmt::print("Active pokemon: \n");

  // if this is false, then the only move this pokemon may use is "thrash"
  for (const auto& actor : actors) {
    for (const auto& action : cu_->getValidMoveActions(env.getEnv(), actor)) {
      double delta = getFitnessDelta(actor, action);
      std::string deltaStr =
          (shouldEval()) ? fmt::format("\t({:+.2f})", delta) : "";

      if (action.isStruggle()) {
        fmt::print(
            "\t{} \"Struggle\" -/-{}\n", fmt::streamed(action), deltaStr);
      } else if (action.isWait()) {
        fmt::print("\t{} \"Nothing\" -/-{}\n", fmt::streamed(action), deltaStr);
      } else {
        const ConstMoveVolatile cMove = env.teammate(actor).getMV(action);
        cMove.prettyPrint(
            fmt::format("\t{} ", fmt::streamed(action)), deltaStr);
      }
    }
  }

  if (cTeam.nv().getNumTeammates() > actors.size()) {
    fmt::print("Or switch to a sidelined pokemon: \n");
    for (const auto& actor : actors) {
      for (const auto& action : cu_->getValidSwapActions(env.getEnv(), actor)) {
        double delta = getFitnessDelta(actor, action);
        std::string deltaStr =
            (shouldEval()) ? fmt::format("\t({:+.2f})", delta) : "";

        const ConstPokemonVolatile cTeammate = cTeam.teammate(action);
        cTeammate.prettyPrint(
            fmt::format("\t{} ", fmt::streamed(action)), deltaStr);
      }
    }
  }
}


ActionMap PlannerHuman::actionSelect(
    const ConstEnvironmentVolatile& env) const {
  std::string input;
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);
  auto actors = cTeam.getActiveActors();
  ActionMap actionMap;

  for (const auto& actor : actors) {
    Action action;
    do {
      fmt::print(
          "Please select the index of your desired action for Actor {}:\n",
          fmt::streamed(actor));
      getline(in_.get(), input);
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

  return actionMap;
};  // endOf actionSelect
