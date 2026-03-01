/*
 * File:   teambuilder_main.cpp
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
#include "pokemonai/teambuilder.h"

namespace po = boost::program_options;


struct Config {
  PokedexDynamic::Config pokedex;
  TeamBuilder::Config teambuilder;
  Game::Config game;
  PkCU::Config engine;
  std::vector<std::string> teams;
  std::vector<std::string> evalTypes = {"simple"};
  std::vector<std::shared_ptr<Evaluator::Config> > evalConfigs;
  std::vector<std::string> plannerTypes = {"random", "maximin"};
  std::vector<std::shared_ptr<Planner::Config> > plannerConfigs;

  int verbosity = spdlog::level::info;
  int random_seed = -1;

  void updateEvalTypes() {
    for (auto& evalType: evalTypes) { evalConfigs.push_back(evaluators::config(evalType)); }
    for (auto& planType: plannerTypes) { plannerConfigs.push_back(planners::config(planType)); }
  }

  Config() {
    game.storeSubcomponents = false;
    game.maxMatches = 1;
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
    desc.add(teambuilder.options());
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

  TeamBuilder teambuilder{cfg.teambuilder};
  teambuilder.setEngine(PkCU{cfg.engine});
  teambuilder.setGame(Game{cfg.game});
  teambuilder.setStateEvaluator(EvaluatorSimple().setEngine(PkCU{cfg.engine}));
  for (size_t iPlan = 0; iPlan != cfg.plannerTypes.size(); ++iPlan) {
    teambuilder.addPlanner(planners::choose(cfg.plannerTypes[iPlan], *cfg.plannerConfigs[iPlan]));
  }
  for (size_t iEval = 0; iEval != cfg.evalTypes.size(); ++iEval) {
    teambuilder.addEvaluator(evaluators::choose(cfg.evalTypes[iEval], *cfg.evalConfigs[iEval]));
  }
  for (const auto& teamPath: cfg.teams) {
    teambuilder.addTeam(TeamNonVolatile::load(teamPath));
  }

  teambuilder.initialize();
  teambuilder.evolve();

  std::exit(EXIT_SUCCESS);
}
