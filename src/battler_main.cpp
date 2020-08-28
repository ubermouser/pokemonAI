#include <inc/pkai.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <boost/program_options.hpp>

#include <inc/game.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>
#include <inc/evaluator_montecarlo.h>
#include <inc/evaluator_random.h>
#include <inc/evaluator_simple.h>
#include <inc/planner_random.h>
#include <inc/planner_human.h>
#include <inc/planner_max.h>
#include <inc/planner_maximin.h>
#include <inc/orphan.h>

namespace po = boost::program_options;
using orphan::lowerCase;

struct Config {
  PokedexStatic::Config pokedex;
  Game::Config game;
  PkCU::Config engine;
  PkCU::Config agentEngine;

  std::array<Planner::Config, 2> agent = {Planner::Config(), Planner::Config()};
  std::array<std::string, 2> planner = {"Maximin", "Max"};
  std::array<std::string, 2> evaluator = {"Simple", "MonteCarlo"};
  std::array<std::string, 2> team = {"teams/hexTeamA.txt", "teams/hexTeamD.txt"};

  int verbosity = 1;
  int random_seed = -1;

  static po::options_description options(Config& cfg) {
    Config defaults{};
    po::options_description desc;

    desc.add_options()
        ("help", "produce this help message")
        ("team-a",
        po::value<std::string>(&cfg.team[TEAM_A])->default_value(defaults.team[TEAM_A]),
        "filepath of the first team")
        ("team-b",
        po::value<std::string>(&cfg.team[TEAM_B])->default_value(defaults.team[TEAM_B]),
        "filepath of the second team")
        ("planner-a",
        po::value<std::string>(&cfg.planner[TEAM_A])->default_value(defaults.planner[TEAM_A]),
        "planner-id of the first team")
        ("planner-b",
        po::value<std::string>(&cfg.planner[TEAM_B])->default_value(defaults.planner[TEAM_B]),
        "planner-id of the second team")
        ("evaluator-a",
        po::value<std::string>(&cfg.evaluator[TEAM_A])->default_value(defaults.evaluator[TEAM_A]),
        "evaluator-id of the first team")
        ("evaluator-b",
        po::value<std::string>(&cfg.evaluator[TEAM_B])->default_value(defaults.evaluator[TEAM_B]),
        "evaluator-id of the second team")
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
    desc.add(cfg.agent[TEAM_A].options(cfg.agent[TEAM_A], "agent-a planner configuration", "a"));
    desc.add(cfg.agent[TEAM_B].options(cfg.agent[TEAM_B], "agent-b planner configuration", "b"));

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


std::shared_ptr<Planner> choosePlanner(const std::string& _type, const Planner::Config& cfg) {
  auto type = lowerCase(_type);
  if (type == "maximin") {
    return std::make_shared<PlannerMaxiMin>(cfg);
  } else if (type == "random") {
    return std::make_shared<PlannerRandom>(); // TODO(@drendleman) - support for other configs
  } else if (type == "max") {
    return std::make_shared<PlannerMax>(cfg);
  } else if (type == "human") {
    return std::make_shared<PlannerHuman>(cfg);
  } else {
    std::cerr << "unknown planner type \"" << _type << "\"!\n";
    throw std::invalid_argument("planner type");
  }
}


std::shared_ptr<Evaluator> chooseEvaluator(const std::string& _type) {
  auto type = lowerCase(_type);
  if (type == "simple") {
    return std::make_shared<EvaluatorSimple>();
  } else if (type == "random") {
    return std::make_shared<EvaluatorRandom>();
  } else if (type == "montecarlo") {
    return std::make_shared<EvaluatorMonteCarlo>();
  } else {
    std::cerr << "unknown evaluator type \"" << _type << "\"!\n";
    throw std::invalid_argument("evaluator type");
  }
}


std::shared_ptr<Planner> buildPlanner(const Config& cfg, size_t iTeam) {
  auto agentEngine = PkCU(cfg.agentEngine);

  auto planner = choosePlanner(cfg.planner[iTeam], cfg.agent[iTeam]);
  auto evaluator = chooseEvaluator(cfg.evaluator[iTeam]);

  planner->setEngine(agentEngine).setEvaluator(evaluator);
  return planner;
}


int main(int argc, char** argv) {
  auto cfg = parse_command_line(argc, argv);

  verbose = cfg.verbosity;
  srand((cfg.random_seed < 0)?time(NULL):cfg.random_seed);

  auto pokedex = PokedexStatic(cfg.pokedex);

  auto game = Game(cfg.game)
      .setEngine(PkCU(cfg.engine))
      .setTeam(0, TeamNonVolatile::loadFromFile(cfg.team[0]))
      .setTeam(1, TeamNonVolatile::loadFromFile(cfg.team[1]))
      .setPlanner(0, buildPlanner(cfg, 0))
      .setPlanner(1, buildPlanner(cfg, 1));

  game.rollout();
  std::exit(EXIT_SUCCESS);
}
