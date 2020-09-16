#include <inc/pkai.h>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <boost/program_options.hpp>

#include <inc/game.h>
#include <inc/pkCU.h>
#include <inc/pokedex_static.h>
#include <inc/evaluators.h>
#include <inc/planners.h>

namespace po = boost::program_options;

struct Config {
  PokedexStatic::Config pokedex;
  Game::Config game;
  PkCU::Config engine;
  PkCU::Config agentEngine;

  std::array<std::shared_ptr<Planner::Config>, 2> agent = {NULL, NULL};
  std::array<std::shared_ptr<Evaluator::Config>, 2> agenteval = {NULL, NULL};
  std::array<std::string, 2> planner = {"Maximin", "Max"};
  std::array<std::string, 2> evaluator = {"Simple", "MonteCarlo"};
  std::array<std::string, 2> team = {"teams/hexTeamA.txt", "teams/hexTeamD.txt"};

  int verbosity = 1;
  int random_seed = -1;

  static Config create(const Config& prototype) {
    Config result = prototype;
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      result.agent[iTeam] = planners::config(result.planner[iTeam]);
      result.agenteval[iTeam] = evaluators::config(result.evaluator[iTeam]);
    }

    return result;
  }

  po::options_description options() {
    Config defaults = Config::create(*this);
    po::options_description desc;

    desc.add_options()
        ("help", "produce this help message")
        ("team-a",
        po::value<std::string>(&team[TEAM_A])->default_value(defaults.team[TEAM_A]),
        "filepath of the first team")
        ("team-b",
        po::value<std::string>(&team[TEAM_B])->default_value(defaults.team[TEAM_B]),
        "filepath of the second team")
        ("planner-a",
        po::value<std::string>(&planner[TEAM_A])->default_value(defaults.planner[TEAM_A]),
        "planner-id of the first team")
        ("planner-b",
        po::value<std::string>(&planner[TEAM_B])->default_value(defaults.planner[TEAM_B]),
        "planner-id of the second team")
        ("evaluator-a",
        po::value<std::string>(&evaluator[TEAM_A])->default_value(defaults.evaluator[TEAM_A]),
        "evaluator-id of the first team")
        ("evaluator-b",
        po::value<std::string>(&evaluator[TEAM_B])->default_value(defaults.evaluator[TEAM_B]),
        "evaluator-id of the second team")
        ("random-seed",
        po::value<int>(&random_seed)->default_value(defaults.random_seed),
        "random number generator seed. -1 for TIME.")
        ("verbosity",
        po::value<int>(&verbosity)->default_value(defaults.verbosity),
        "static verbosity level.");
    desc.add(pokedex.options());
    desc.add(game.options());
    desc.add(engine.options("engine configuration"));
    desc.add(engine.options("agent-engine configuration", "agent"));

    // TODO(@drendleman) - Config should default to base-class, then be specified if valid
    desc.add(agent[TEAM_A]->options("agent-a planner configuration", "a"));
    desc.add(agent[TEAM_B]->options("agent-b planner configuration", "b"));

    desc.add(agenteval[TEAM_A]->options("agent-a evaluator configuration", "a"));
    desc.add(agenteval[TEAM_B]->options("agent-b evaluator configuration", "b"));

    return desc;
  }
};


Config parse_command_line(int argc, char**argv) {
  // determine prototype values:
  Config protocfg = Config::create(Config{});
  {
    po::variables_map vm;
    auto description = protocfg.options();
    po::store(
        po::command_line_parser(argc, argv).options(description).allow_unregistered().run(), vm);
    po::notify(vm);
  }
  
  Config cfg = Config::create(protocfg);
  {
    po::variables_map vm;
    auto description = cfg.options();
    po::store(po::parse_command_line(argc, argv, description), vm);
    po::notify(vm);

    if (vm.count("help")) {
      std::cout << description << std::endl;
      std::exit(EXIT_FAILURE);
    }
  }
  
  return cfg;
}


std::shared_ptr<Planner> buildPlanner(const Config& cfg, size_t iTeam) {
  auto agentEngine = PkCU(cfg.agentEngine);

  auto planner = planners::choose(cfg.planner[iTeam], *cfg.agent[iTeam]);
  auto evaluator = evaluators::choose(cfg.evaluator[iTeam], *cfg.agenteval[iTeam]);

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

  game.run();
  std::exit(EXIT_SUCCESS);
}
