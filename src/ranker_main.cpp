/* 
 * File:   ranker_main.cpp
 * Author: drendleman
 *
 * Created on September 17, 2020, 11:15 AM
 */

#include <cstdlib>
#include <string>
#include <boost/program_options.hpp>

#include <inc/ranker.h>
#include <inc/pkCU.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>

namespace po = boost::program_options;


struct Config {
  PokedexStatic::Config pokedex;
  Game::Config game;
  PkCU::Config engine;
  Ranker::Config trainer;

  int verbosity = 1;
  int random_seed = -1;
};

int main(int argc, char** argv) {

  return 0;
}
