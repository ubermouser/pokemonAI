/* 
 * File:   ranker_main.cpp
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:15 AM
 */
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
  PkCU::Config engine;
  Trainer::Config trainer;

  int verbosity = 1;
  int random_seed = -1;
};

int main(int argc, char** argv) {
  Config cfg;
  cfg.trainer.maxGenerations = 100;
  cfg.trainer.verbosity = 2;
  cfg.trainer.minGamesPerBattlegroup = 10;
  cfg.trainer.game.verbosity = 0;
  cfg.trainer.game.maxMatches = 1;
  cfg.trainer.printBattlegroupLeaderboard = false;
  cfg.trainer.leaderboardPrintCount = 30;

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);

  Trainer trainer(cfg.trainer);
  trainer.addPlanner(planners::choose("random", *planners::config("random"))->setEngine(PkCU()));
  trainer.addPlanner(planners::choose("maximin", *planners::config("maximin"))->setEngine(PkCU()));
  //trainer.addPlanner(planners::choose("max", *planners::config("max"))->setEngine(PkCU()));
  trainer.addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));

  trainer.initialize();
  trainer.evolve();
  
  std::exit(EXIT_SUCCESS);
}
