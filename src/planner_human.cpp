//#define PKAI_IMPORT
#include "pokemonai/planner_human.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <iostream>
#include <sstream>
#include <string>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_possible.h"
#include "pokemonai/pkCU.h"

PlannerHuman::PlannerHuman(const Config& cfg) : PlannerHuman(cfg, std::cin) {};

PlyResult PlannerHuman::generateSolutionAtLeaf(
    const ConstEnvironmentPossible& origin) const {
  fmt::print("{}", fmt::streamed(origin));
  printActions(origin);

  PlyResult result;
  result.agentAction = actionSelect(origin);
  return result;
}

void PlannerHuman::printActions(const ConstEnvironmentPossible& env) const {
  const ConstTeamVolatile cTeam = env.getEnv().getTeam(agentTeam_);
  const ConstPokemonVolatile cPokemon = cTeam.getPKV();

  double currentFitness = 0.0;
  if (shouldEval()) {
    currentFitness = eval_->evaluate(env, agentTeam_).fitness.lowerBound();

    fmt::print("Current Evaluator Fitness: {:.4f}\n", currentFitness);
  }

  auto getFitnessDelta = [&](const Action& agentAction) {
    if (!shouldEval()) return 0.0;

    // TODO: generateSolutionAtDepth must be implemented if maxDepth > 0!
    EvalResult worst = recurse_beta(env, agentAction, 1);
    return worst.fitness.lowerBound() - currentFitness;
  };

  fmt::print("Active pokemon: \n");

  // if this is false, then the only move this pokemon may use is "thrash"
  for (const auto& action : cu_->getValidMoveActions(env, agentTeam_)) {
    double delta = getFitnessDelta(action);
    std::string deltaStr =
        (shouldEval()) ? fmt::format("\t({:+.2f})", delta) : "";

    if (action.isStruggle()) {
      fmt::print("\t{} \"Struggle\" -/-{}\n", fmt::streamed(action), deltaStr);
    } else if (action.isWait()) {
      fmt::print("\t{} \"Nothing\" -/-{}\n", fmt::streamed(action), deltaStr);
    } else {
      const ConstMoveVolatile cMove = cPokemon.getMV(action);
      fmt::print(
          "\t{} {}{}\n", fmt::streamed(action), fmt::streamed(cMove), deltaStr);
    }
  }
  if (cTeam.nv().getNumTeammates() > 1) {
    fmt::print("Or switch to a sidelined pokemon: \n");
    for (const auto& action : cu_->getValidSwapActions(env, agentTeam_)) {
      double delta = getFitnessDelta(action);
      std::string deltaStr =
          (shouldEval()) ? fmt::format("\t({:+.2f})", delta) : "";

      fmt::print(
          "\t{} {}{}\n",
          fmt::streamed(action),
          fmt::streamed(cTeam.teammate(action)),
          deltaStr);
    }
  }
}

Action PlannerHuman::actionSelect(const ConstEnvironmentVolatile& env) const {
  std::string input;
  Action action;

  do {
    fmt::print(
        "Please select the index of your desired action for Team {}:\n",
        (agentTeam_ == TEAM_A ? "A" : "B"));
    getline(in_.get(), input);
    std::stringstream inputResult(input);

    // determine if action is valid:
    if (!(inputResult >> action) || !cu_->isValidAction(env, action, agentTeam_)) {
      fmt::print("Invalid action \"{}\"!\n", input);

      continue;
    }

    break;
  } while (true);

  return action;
}; // endOf actionSelect
