/* 
 * File:   ranker_main.cpp
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:15 AM
 */
#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>

#include <inc/ranker.h>
#include <inc/trainer.h>
#include <inc/engine.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>
#include <inc/evaluators.h>
#include <inc/planners.h>

#include "inc/trainer.h"

namespace po = boost::program_options;


struct Config {
  PokedexStatic::Config pokedex;
  Trainer::Config trainer;
  //std::vector<std::string> evalTypes = {"simple"};
  //std::vector<std::shared_ptr<Evaluator::Config> > evalConfigs;
  //std::vector<std::string> plannerTypes = {"random", "maximin"};
  //std::vector<std::shared_ptr<Planner::Config> > plannerConfigs;

  int verbosity = 1;
  int random_seed = -1;

  Config() {}

  po::options_description options() {
    Config defaults{};
    po::options_description desc;

    desc.add_options()
        ("help", "produce this help message")
        ("random-seed",
        po::value<int>(&random_seed)->default_value(defaults.random_seed),
        "random number generator seed. -1 for TIME.")
        ("verbosity",
        po::value<int>(&verbosity)->default_value(defaults.verbosity),
        "static verbosity level.");
    desc.add(pokedex.options());
    desc.add(trainer.options());

    return desc;
  }
};


Config parse_command_line(int argc, char**argv) {
  Config cfg{};

  po::variables_map vm;
  auto description = cfg.options();
  po::store(po::parse_command_line(argc, argv, description), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cerr << description << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return cfg;
}

int main(int argc, char** argv) {
  auto cfg = parse_command_line(argc, argv);

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);

  Trainer trainer{cfg.trainer};
  trainer.addPlanner(planners::choose("random", *planners::config("random")));
  trainer.addPlanner(planners::choose("maximin", *planners::config("maximin")));
  //trainer.addPlanner(planners::choose("max", *planners::config("max"))->setEngine(PkCU()));
  trainer.addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));

  trainer.initialize();
  trainer.evolve();
  
  std::exit(EXIT_SUCCESS);
}
