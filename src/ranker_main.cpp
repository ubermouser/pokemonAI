/* 
 * File:   ranker_main.cpp
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:15 AM
 */
#include <string>
#include <boost/program_options.hpp>

#include <inc/ranker.h>
#include <inc/engine.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>
#include <inc/evaluators.h>
#include <inc/planners.h>

namespace po = boost::program_options;


struct Config {
  PokedexStatic::Config pokedex;
  PkCU::Config engine;
  Ranker::Config trainer;

  int verbosity = 1;
  int random_seed = -1;
};

int main(int argc, char** argv) {
  Config cfg;
  cfg.trainer.verbosity = 2;
  cfg.trainer.minGamesPerBattlegroup = 100;
  cfg.trainer.game.verbosity = 0;
  cfg.trainer.game.maxMatches = 1;

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);

  Ranker ranker(cfg.trainer);
  ranker.addPlanner(planners::choose("random", *planners::config("random"))->setEngine(PkCU()));
  ranker.addPlanner(planners::choose("maximin", *planners::config("maximin"))->setEngine(PkCU()));
  ranker.addPlanner(planners::choose("max", *planners::config("max"))->setEngine(PkCU()));
  ranker.addEvaluator(evaluators::choose("simple", *evaluators::config("simple")));

  ranker.initialize();
  ranker.rank();
  
  std::exit(EXIT_SUCCESS);
}
