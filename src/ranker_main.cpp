/* 
 * File:   ranker_main.cpp
 * Author: drendleman
 *
 * Created on December 8, 2025, 5:47 PM
 */
#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>
#include <boost/format.hpp>

#include "../inc/engine.h"
#include "../inc/pkCU.h"
#include "../inc/pokedex_static.h"
#include "../inc/evaluators.h"
#include "../inc/evaluator_simple.h"
#include "../inc/planners.h"

#include "../inc/ranker.h"

namespace po = boost::program_options;


struct Config {
  PokedexStatic::Config pokedex;
  Ranker::Config ranker;
  Game::Config game;
  PkCU::Config engine;
  std::vector<std::string> teams;
  std::vector<std::string> evalTypes = {"simple"};
  std::vector<std::shared_ptr<Evaluator::Config> > evalConfigs;
  std::vector<std::string> plannerTypes = {"random", "maximin"};
  std::vector<std::shared_ptr<Planner::Config> > plannerConfigs;

  int verbosity = 1;
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
        ("random-seed",
        po::value<int>(&random_seed)->default_value(defaults.random_seed),
        "random number generator seed. -1 for TIME.")
        ("verbosity",
        po::value<int>(&verbosity)->default_value(defaults.verbosity),
        "static verbosity level.");
    desc.add(pokedex.options());
    desc.add(ranker.options());
    desc.add(game.options("game configuration", "game"));
    desc.add(engine.options());
    for (size_t iPlan = 0; iPlan != plannerConfigs.size(); ++iPlan) {
      desc.add(plannerConfigs[iPlan]->options(
          (boost::format("planner-%d %s configuration") % (iPlan+1) % plannerTypes[iPlan]).str(),
          (boost::format("p%d") % (iPlan+1)).str()));
    }
    for (size_t iEval = 0; iEval != evalConfigs.size(); ++iEval) {
      desc.add(evalConfigs[iEval]->options(
          (boost::format("evaluator-%d %s configuration") % (iEval+1) % evalTypes[iEval]).str(),
          (boost::format("e%d") % (iEval+1)).str()));
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
    po::store(
        po::command_line_parser(argc, argv).options(description).allow_unregistered().run(), vm);
    po::notify(vm);
  }

  cfg.updateEvalTypes();
  {
    po::variables_map vm;
    auto description = cfg.options();
    po::store(po::parse_command_line(argc, argv, description), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cerr << description << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }

  return cfg;
}

int main(int argc, char** argv) {
  auto cfg = parse_command_line(argc, argv);

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);

  Ranker ranker{cfg.ranker};
  ranker.setEngine(PkCU{cfg.engine});
  ranker.setGame(Game{cfg.game});
  ranker.setStateEvaluator(EvaluatorSimple().setEngine(PkCU{cfg.engine}));
  for (size_t iPlan = 0; iPlan != cfg.plannerTypes.size(); ++iPlan) {
    ranker.addPlanner(planners::choose(cfg.plannerTypes[iPlan], *cfg.plannerConfigs[iPlan]));
  }
  for (size_t iEval = 0; iEval != cfg.evalTypes.size(); ++iEval) {
    ranker.addEvaluator(evaluators::choose(cfg.evalTypes[iEval], *cfg.evalConfigs[iEval]));
  }
  for (const auto& teamPath: cfg.teams) {
    ranker.addTeam(TeamNonVolatile::load(teamPath));
  }

  ranker.initialize();
  ranker.rank();
  
  std::exit(EXIT_SUCCESS);
}
