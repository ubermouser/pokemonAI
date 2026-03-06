/*
 * File:   trainer_main.cpp
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:15 AM
 */
#include <fmt/format.h>

#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <string>

#include "pokemonai/app_utils.h"
#include "pokemonai/engine.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/evaluators.h"
#include "pokemonai/logging.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/planners.h"
#include "pokemonai/pokedex_dynamic.h"
#include "pokemonai/trainer.h"

namespace po = boost::program_options;


struct Config {
  PokedexDynamic::Config pokedex;
  Trainer::Config trainer;
  Game::Config game;
  PkCU::Config engine;
  std::vector<std::string> teams;
  std::vector<std::string> evalTypes = {"tnetwork16", "simple", "random"};
  std::vector<std::shared_ptr<Evaluator::Config>> evalConfigs;
  std::shared_ptr<Evaluator::Config> stateEvalConfig;
  std::vector<std::string> plannerTypes = {"softmax"};
  std::vector<std::shared_ptr<Planner::Config>> plannerConfigs;

  int verbosity = spdlog::level::info;
  int random_seed = -1;

  void updateEvalTypes() {
    evalConfigs.clear();
    for (auto& evalType : evalTypes) {
      evalConfigs.push_back(evaluators::config(evalType));
    }
    stateEvalConfig = evaluators::config(trainer.stateEvaluatorType);
    plannerConfigs.clear();
    for (auto& planType : plannerTypes) {
      plannerConfigs.push_back(planners::config(planType));
    }
  }

  Config() {
    game.storeSubcomponents = true;
    game.maxMatches = 1;
    stateEvalConfig = evaluators::config(trainer.stateEvaluatorType);
  }

  po::options_description options() {
    Config defaults{};
    po::options_description desc;

    // clang-format off
    desc.add_options()
        ("planners",
        po::value<std::vector<std::string>>(&plannerTypes)->multitoken(),
        "planner types to seed.")
        ("evaluators",
        po::value<std::vector<std::string>>(&evalTypes)->multitoken(),
        "evaluator types to seed.")
        ("teams",
        po::value<std::vector<std::string>>(&teams)->multitoken(),
        "teams to seed.")
        ("help", "produce this help message")
        ("config", po::value<std::string>(), "config file path")
        ("random-seed",
        po::value<int>(&random_seed)->default_value(defaults.random_seed),
        "random number generator seed. -1 for TIME.")
        ("verbosity",
        po::value<int>(&verbosity)->default_value(defaults.verbosity),
        "static verbosity level.");
    
    desc.add(pokedex.options());
    desc.add(trainer.options());
    desc.add(game.options("game configuration", "game"));
    desc.add(engine.options());
    for (size_t iPlan = 0; iPlan != plannerConfigs.size(); ++iPlan) {
      desc.add(plannerConfigs[iPlan]->options(
          fmt::format(
              "planner-{} {} configuration", iPlan + 1, plannerTypes[iPlan]),
          fmt::format("p{}", iPlan + 1)));
    }
    for (size_t iEval = 0; iEval != evalConfigs.size(); ++iEval) {
      desc.add(evalConfigs[iEval]->options(
          fmt::format(
              "evaluator-{} {} configuration", iEval + 1, evalTypes[iEval]),
          fmt::format("e{}", iEval + 1)));
    }
    if (stateEvalConfig) {
      desc.add(stateEvalConfig->options(
          fmt::format(
              "state-evaluator {} configuration", trainer.stateEvaluatorType),
          "se"));
    }

    return desc;
  }
};


Config parse_command_line(int argc, char**argv) {
  Config cfg{};
  // determine prototype values:
  {
    po::variables_map vm;
    auto description = cfg.options();
    PokemonAIAppUtils::parse_command_line_and_config(argc, argv, description, vm, true);
  }

  cfg.updateEvalTypes();
  {
    po::variables_map vm;
    auto description = cfg.options();
    PokemonAIAppUtils::parse_command_line_and_config(argc, argv, description, vm, false);

    if (vm.count("help")) {
      std::cerr << description << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  return cfg;
}


int main(int argc, char** argv) {
  auto cfg = parse_command_line(argc, argv);
  auto pokedex = PokemonAIAppUtils::bootstrap(cfg.verbosity, cfg.random_seed, cfg.pokedex);

  Trainer trainer{cfg.trainer};
  trainer.setEngine(PkCU{cfg.engine});
  trainer.setGame(Game{cfg.game});
  auto stateEvaluator =
      evaluators::choose(cfg.trainer.stateEvaluatorType, *cfg.stateEvalConfig);
  stateEvaluator->setEngine(std::make_shared<PkCU>(cfg.engine));
  trainer.setStateEvaluator(*stateEvaluator);
  for (size_t iPlan = 0; iPlan != cfg.plannerTypes.size(); ++iPlan) {
    trainer.addPlanner(planners::choose(cfg.plannerTypes[iPlan], *cfg.plannerConfigs[iPlan]));
  }
  for (size_t iEval = 0; iEval != cfg.evalTypes.size(); ++iEval) {
    trainer.addEvaluator(evaluators::choose(cfg.evalTypes[iEval], *cfg.evalConfigs[iEval]));
  }
  for (const auto& teamPath: cfg.teams) {
    trainer.addTeam(TeamNonVolatile::load(teamPath));
  }

  trainer.initialize();
  trainer.evolve();

  std::exit(EXIT_SUCCESS);
}
