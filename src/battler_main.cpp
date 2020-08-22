#include <inc/pkai.h>

#include <iostream>
#include <memory>
#include <string>
#include <boost/program_options.hpp>

#include <inc/game.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>
#include <inc/evaluator_simple.h>
#include <inc/evaluator_montecarlo.h>
#include <inc/planner_random.h>
#include <inc/planner_human.h>
#include <inc/planner_max.h>
#include <inc/planner_maximin.h>

namespace po = boost::program_options;


struct Config {
  PokedexStatic::Config pokedex;
  Game::Config game;
  PkCU::Config engine;
  PkCU::Config agentEngine;

  Planner::Config agent_a;
  Planner::Config agent_b;

  std::string team_a = "teams/hexTeamA.txt";
  std::string team_b = "teams/hexTeamD.txt";

  int verbosity = 1;
  int random_seed = -1;

  static po::options_description options(Config& cfg) {
    Config defaults{};
    po::options_description desc;

    desc.add_options()
        ("help", "produce this help message")
        ("team-a",
        po::value<std::string>(&cfg.team_a)->default_value(defaults.team_a),
        "filepath of the first team")
        ("team-b",
        po::value<std::string>(&cfg.team_b)->default_value(defaults.team_b),
        "filepath of the second team")
        ("random-seed",
        po::value<int>(&cfg.random_seed)->default_value(defaults.random_seed),
        "random number generator seed. -1 for TIME.")
        ("verbosity",
        po::value<int>(&cfg.verbosity)->default_value(defaults.verbosity),
        "static verbosity level.");
    desc.add(cfg.pokedex.options(cfg.pokedex));
    desc.add(cfg.game.options(cfg.game));
    desc.add(cfg.engine.options(cfg.engine, "engine configuration"));
    desc.add(cfg.engine.options(cfg.engine, "agent-engine configuration", "agent"));
    desc.add(cfg.agent_a.options(cfg.agent_a, "agent-a planner configuration", "a"));
    desc.add(cfg.agent_b.options(cfg.agent_b, "agent-b planner configuration", "b"));

    return desc;
  }
};


Config parse_command_line(int argc, char**argv) {
  Config cfg;
  auto description = Config::options(cfg);
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, Config::options(cfg)), vm);
  po::notify(vm);

  if (vm.count("help")) {
    std::cout << description << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return cfg;
}


int main(int argc, char** argv) {
  auto cfg = parse_command_line(argc, argv);

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);
  auto engine = PkCU(cfg.engine);
  auto agentEngine = PkCU(cfg.agentEngine);

  auto team_a = TeamNonVolatile::loadFromFile(cfg.team_a);
  auto team_b = TeamNonVolatile::loadFromFile(cfg.team_b);

  auto game = Game(cfg.game)
      .setEngine(engine)
      .setTeam(0, team_a)
      .setTeam(1, team_b)
      //.setPlanner(0, PlannerHuman().setEngine(engine))
      //.setPlanner(1, PlannerHuman().setEngine(engine));
      .setPlanner(0, PlannerMaxiMin(cfg.agent_a).setEngine(agentEngine).setEvaluator(EvaluatorMonteCarlo()))
      .setPlanner(1, PlannerMax(cfg.agent_b).setEngine(agentEngine).setEvaluator(EvaluatorSimple()));

  game.rollout();
  std::exit(EXIT_SUCCESS);
}
