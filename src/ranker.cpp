#include "../inc/ranker.h"

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <omp.h>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;
namespace po = boost::program_options;


po::options_description Ranker::Config::options(const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "allow-same-planner").c_str(),
      po::value<bool>(&allowSamePlanner)->default_value(defaults.allowSamePlanner),
      "when true, a given planner may fight itself.")
      ((prefix + "allow-same-evaluator").c_str(),
      po::value<bool>(&allowSameEvaluator)->default_value(defaults.allowSameEvaluator),
      "when true, a given evaluator may fight itself.")
      ((prefix + "allow-same-team").c_str(),
      po::value<bool>(&allowSameTeam)->default_value(defaults.allowSameTeam),
      "when true, a given team may fight itself.")
      ((prefix + "enforce-same-league").c_str(),
      po::value<bool>(&allowSameTeam)->default_value(defaults.allowSameTeam),
      "when true, teams will only fight teams with the same number of pokemon.")
      ((prefix + "print-battlegroup-leaderboard").c_str(),
      po::value<bool>(&printBattlegroupLeaderboard)->default_value(defaults.printBattlegroupLeaderboard),
      "Prints a leaderboard of battlegroups.")
      ((prefix + "print-pokemon-leaderboard").c_str(),
      po::value<bool>(&printPokemonLeaderboard)->default_value(defaults.printPokemonLeaderboard),
      "Prints a leaderboard of pokemon.")
      ((prefix + "leaderboard-size").c_str(),
      po::value<size_t>(&leaderboardPrintCount)->default_value(defaults.leaderboardPrintCount),
      "number of entities displayed per leaderboard.")
      ((prefix + "games-per-battlegroup").c_str(),
      po::value<size_t>(&minGamesPerBattlegroup)->default_value(defaults.minGamesPerBattlegroup),
      "minimum number of games played per battlegroup per league.")
      ((prefix + "ranker-verbosity").c_str(),
      po::value<int>(&verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls intermediate rank printing.")
      ((prefix + "ranker-seed").c_str(),
      po::value<uint32_t>(&randomSeed)->default_value(defaults.randomSeed),
      "random number generator seed.")
      ((prefix + "num-threads").c_str(),
      po::value<size_t>(&numThreads)->default_value(defaults.numThreads),
      "number of threads to use when ranking teams")
      ((prefix + "team-path").c_str(),
      po::value<std::string>(&teamPath)->default_value(defaults.teamPath),
      "folder for loading / saving pokemon teams");
  desc.add(game.options());
  desc.add(engine.options());

  return desc;
}


Ranker::Ranker(const Config& cfg) : cfg_(cfg), out_(std::cout), rand_(cfg.randomSeed) {
  setGame(Game{cfg_.game});
  setEngine(PkCU{cfg_.engine});
  //setStateEvaluator(EvaluatorSimple()); // TODO(@drendleman) why does uncommenting this let the demons out?
}


void Ranker::initialize() {
  if (games_.empty()) {throw std::runtime_error("game undefined"); }
  if (initialLeague_.evaluators.empty()) { throw std::runtime_error("no evaluators defined"); }
  if (initialLeague_.planners.empty()) { throw std::runtime_error("no planners defined"); }

  if (!cfg_.teamPath.empty()) { loadTeamPopulation(); }
  if (initialLeague_.teams.empty()) { throw std::runtime_error("no teams defined"); }

  initialized_ = true;
}


void Ranker::testInitialized() const {
  if (!initialized_) { throw std::runtime_error("ranker not initialized"); }
}


LeagueHeat Ranker::rank() const {
  testInitialized();

  LeagueHeat league = constructLeague();
  runLeague(league);

  return league;
}


LeagueHeat Ranker::constructLeague() const {
  LeagueHeat league{initialLeague_};
  return league;
}


void Ranker::runLeague(LeagueHeat& league) const {
  testInitialized();
  // TODO(@drendleman) - multithread this loop?
  for (auto& bg: league.battlegroups) {
    gauntlet(bg.second, league);
  }

  if (cfg_.verbosity >= 1) { printLeagueStatistics(league); }
}


void Ranker::gauntlet(BattlegroupPtr& battlegroup, LeagueHeat& league) const {
  testInitialized();
  const auto& record = battlegroup->record();
  size_t maxGames = cfg_.minGamesPerBattlegroup - record.numGamesPlayed();
  for (size_t iGame=0; iGame < maxGames && record.numGamesPlayed() < cfg_.minGamesPerBattlegroup; ++iGame) {
    BattlegroupPtr adversary = findMatch(*battlegroup, league);

    GameHeat result = singleGame(battlegroup, adversary);
    digestGame(result, league);
    if (cfg_.verbosity >= 3) { printHeatResult(result); }
  }
}


GameHeat Ranker::singleGame(
    BattlegroupPtr& battlegroup_a,
    BattlegroupPtr& battlegroup_b) const {
  double matchQuality = gameFactory_.matchQuality(*battlegroup_a, *battlegroup_b);
  int iThread = omp_get_thread_num();
  // game reference:
  auto& game = games_.at(iThread);
  // engine pointer:
  auto& engine = engines_.at(iThread);
  // state evaluator pointer:
  //auto& stateEvaluator = stateEvaluators_.at(iThread);
  // environment pointer:
  auto environment = std::make_shared<EnvironmentNonvolatile>(
      battlegroup_a->team().nv(), battlegroup_b->team().nv(), true);

  auto game_setPlanner = [&](size_t iTeam, BattlegroupPtr& battlegroup){
    // clone a planner by pointer:
    std::shared_ptr<Planner> planner{battlegroup->planner().get().clone()};
    // clone an evaluator by pointer:
    std::shared_ptr<Evaluator> evaluator{battlegroup->evaluator().get().clone()};
    // clone an evaluator by reference:
    planner->setEvaluator(evaluator);
    // use this thread's engine:
    planner->setEngine(engine);

    // assign game planner / evaluator / engine combo:
    game.setPlanner(iTeam, planner);
  };

  // assign game environment, propagating to planners/evaluators/engine:
  game.clear();
  //game.setEvaluator(stateEvaluator);
  game.setEnvironment(environment);
  game.setEngine(engine);
  game_setPlanner(0, battlegroup_a);
  game_setPlanner(1, battlegroup_b);
  // initialize and run the game:
  HeatResult result = game.run();

  return GameHeat{battlegroup_a, battlegroup_b, matchQuality, result};
}


BattlegroupPtr Ranker::findMatch(const Battlegroup& cBG, const LeagueHeat& league) const {
  size_t maxMatches = std::min(league.battlegroups.size(), cfg_.maxMatchesToConsider);

  // subsample the number of opponents if there are too many:
  if (maxMatches < league.battlegroups.size()) {
    BattlegroupLeague opponents; opponents.reserve(maxMatches);
    std::sample(
        league.battlegroups.begin(),
        league.battlegroups.end(),
        std::inserter(opponents, opponents.end()),
        maxMatches,
        rand_);
    return findSubsampledMatch(cBG, opponents);
  } else {
    return findSubsampledMatch(cBG, league.battlegroups);
  }
}


BattlegroupPtr Ranker::findSubsampledMatch(const Battlegroup& cBG, const BattlegroupLeague& league) const {
  size_t numMatches = league.size();
  std::vector<double> matchQualities; matchQualities.reserve(numMatches);
  std::vector<Battlegroup::Hash> opponentHashes; opponentHashes.reserve(numMatches);

  // compute match quality between all pairs in the league:
  for (auto& bg : league) {
    matchQualities.push_back(gameFactory_.matchQuality(cBG, *bg.second));
    opponentHashes.push_back(bg.first);
  }

  auto filterByPredicate = [&](auto predicate){
    for (size_t iBg = 0; iBg < matchQualities.size(); ++iBg) {
      const BattlegroupPtr& oBG = league.at(opponentHashes[iBg]);
      if (!predicate(*oBG)) { continue; }
      matchQualities[iBg] = 0.;
      numMatches--;
    }
  };

  // filter matches that aren't in the same league:
  if (cfg_.enforceSameLeague) {
    filterByPredicate([&](const Battlegroup& oBG){
      return cBG.team().nv().getNumTeammates() != oBG.team().nv().getNumTeammates();
    });
  }

  // filter matches that feature the same elements
  if (!cfg_.allowSameTeam) {
    filterByPredicate([&](const Battlegroup& oBG){
      return cBG.team() == oBG.team();
    });
  }

  if (!cfg_.allowSameEvaluator) {
    filterByPredicate([&](const Battlegroup& oBG){
      return cBG.evaluator() == oBG.evaluator();
    });
  }

  if (!cfg_.allowSamePlanner) {
    filterByPredicate([&](const Battlegroup& oBG){
      return cBG.planner() == oBG.planner();
    });
  }

  // choose a random good match:
  std::discrete_distribution<size_t> probabilities{matchQualities.begin(), matchQualities.end()};
  return league.at(opponentHashes.at(probabilities(rand_)));
}


Ranker& Ranker::setGame(const Game& game) {
  games_.clear();
  games_.resize(std::max(cfg_.numThreads, size_t(1U)), game);
  return *this;
}


Ranker& Ranker::setEngine(const PkCU& cu) {
  engines_.clear();
  for (size_t iThread = 0; iThread < std::max(cfg_.numThreads, size_t(1U)); ++iThread) {
    engines_.push_back(std::make_shared<PkCU>(cu));
  }
  return *this;
}


/*Ranker& Ranker::setStateEvaluator(const Evaluator& eval) {
  stateEvaluators_.clear();
  for (size_t iThread = 0; iThread < std::max(cfg_.numThreads, size_t(1U)); ++iThread) {
    stateEvaluators_.push_back(std::shared_ptr<Evaluator>(eval.clone()));
  }
  return *this;
}*/


template<typename League, typename LeagueType, typename AddFn>
void addToLeague(const LeagueType& obj, const std::string& itemType, League& league, AddFn add_fn) {
  if (league.count(obj.hash()) > 0) {
    std::cerr << boost::format("Duplicate %s added to league \"%s\"!\n") % itemType % obj.getName();
  }

  add_fn(std::make_shared<LeagueType>(obj));
}


Ranker& Ranker::addTeam(const TeamNonVolatile& team) {
  addToLeague(
      RankedTeam{team, initialLeague_.pokemon},
      "team",
      initialLeague_.teams,
      [&](const auto& obj){initialLeague_.addTeam(obj);});
  return *this;
}


Ranker& Ranker::addEvaluator(const std::shared_ptr<Evaluator>& evaluator) {
  addToLeague(
      RankedEvaluator{evaluator},
      "evaluator",
      initialLeague_.evaluators,
      [&](const auto& obj){initialLeague_.addEvaluator(obj);});
  return *this;
}


Ranker& Ranker::addPlanner(const std::shared_ptr<Planner>& planner) {
  addToLeague(
      RankedPlanner{planner},
      "planner",
      initialLeague_.planners,
      [&](const auto& obj){initialLeague_.addPlanner(obj);});
  return *this;
}


template<class LeagueType>
void printMapLeaderboard(std::ostream& os, const LeagueType& league, size_t numToPrint) {
  std::vector<typename LeagueType::mapped_type> rankedLeague; rankedLeague.reserve(league.size());
  for (auto& pair : league) { rankedLeague.push_back(pair.second); }
  
  printLeaderboard(os, rankedLeague, numToPrint);
};

template<class VectorLeagueType>
void printLeaderboard(std::ostream& os, VectorLeagueType& league, size_t numToPrint) {
  // sort numToPrint order:
  numToPrint = std::min(numToPrint, league.size());
  std::partial_sort(
      league.begin(),
      league.begin() + numToPrint,
      league.end(),
      [](const auto& a, const auto& b) {
        return a->skill() > b->skill();
  });

  for (size_t iRanked = 0; iRanked < numToPrint; ++iRanked) {
    const auto& ranked = league[iRanked];
    os << boost::format(" %02d: %s\n") % (iRanked+1) % *ranked;
  }
}


void Ranker::printLeagueStatistics(LeagueHeat& league) const {
  std::ostringstream os;
  auto printHeader = [&](auto& subleague, auto& name) {
    os << boost::format("---- %s LEADERBOARD (top %d of %d)----\n")
        % name
        % std::min(cfg_.leaderboardPrintCount, subleague.size())
        % subleague.size();
  };
  os << boost::format("played %d games!\n") % league.games.size();
  if (league.evaluators.size() >= 2) {
    printHeader(league.evaluators, "EVALUATOR");
    printMapLeaderboard(os, league.evaluators, cfg_.leaderboardPrintCount);
  }
  if (league.planners.size() >= 2) {
    printHeader(league.planners, "PLANNER");
    printMapLeaderboard(os, league.planners, cfg_.leaderboardPrintCount);
  }
  if (league.pokemon.size() >= 2 && cfg_.printPokemonLeaderboard) {
    printHeader(league.pokemon, "POKEMON");
    printMapLeaderboard(os, league.pokemon, cfg_.leaderboardPrintCount);
  }
  if (league.teams.size() >= 2) {
    printHeader(league.teams, "TEAM");
    printMapLeaderboard(os, league.teams, cfg_.leaderboardPrintCount);
  }
  if (cfg_.printBattlegroupLeaderboard) {
    printHeader(league.battlegroups, "BATTLEGROUP");
    printMapLeaderboard(os, league.battlegroups, cfg_.leaderboardPrintCount);
  }
  out_.get() << os.str();
}


void Ranker::printHeatResult(const GameHeat& heat) const {
  auto endStatus = heat.heatResult.endStatus;
  std::string winDrawLoss =
      endStatus==MATCH_TEAM_A_WINS?">":
      endStatus==MATCH_TEAM_B_WINS?"<":
      endStatus==MATCH_TIE?"=":"~";
  out_.get() << boost::format("%34.34s %s %-34.34s\n")
      % heat.team_a->getName()
      % winDrawLoss
      % heat.team_b->getName();
}


void Ranker::digestGame(GameHeat& gameHeat, LeagueHeat& league) const {
  // update ranks of both teams
  gameFactory_.update(*gameHeat.team_a, *gameHeat.team_b, gameHeat.heatResult);
  // TODO(@drendleman) - too many adjacent ranks! Compute only when determining skill?
  //league.recomputeAdjacentRanks(gameHeat.team_a);
  //league.recomputeAdjacentRanks(gameHeat.team_b);
  // update statistics:
  gameHeat.team_a->update(gameHeat.heatResult, TEAM_A);
  gameHeat.team_b->update(gameHeat.heatResult, TEAM_B);
  league.games.push_back(gameHeat);
}


size_t Ranker::loadTeamPopulation() {
  bf::path teamPath{cfg_.teamPath};
  if (!bf::exists(teamPath) || !bf::is_directory(teamPath)) {
    std::cerr << 
        boost::format("Team population directory at \"%s\" is not a folder or does not exist!\n")
        % cfg_.teamPath;
    return 0;
  }

  size_t numLoaded = 0;
  for (auto& pathIt : bf::directory_iterator(teamPath)) {
    if (!bf::is_regular_file(pathIt.path())) { continue; }

    try {
      addTeam(TeamNonVolatile::loadFromFile(pathIt.path().string()));
    } catch(std::invalid_argument& e) {
      if (cfg_.verbosity >= 2) {
        std::cerr << boost::format("Failed to load team at \"%s\"\n") % pathIt.path();
      }
      continue;
    }
    
    numLoaded += 1;
  }

  return numLoaded;
}
