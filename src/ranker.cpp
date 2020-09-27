#include "../inc/ranker.h"

#include <assert.h>
#include <algorithm>
#include <iostream>
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
      "folder for loading / saving pokemon teams");;

  return desc;
}


Ranker::Ranker(const Config& cfg) : cfg_(cfg), out_(std::cout), rand_(cfg.randomSeed) {
  setGame(Game{cfg_.game});
}


void Ranker::initialize() {
  if (games_.empty()) {throw std::runtime_error("game undefined"); }
  if (evaluators_.empty()) { throw std::runtime_error("no evaluators defined"); }
  if (planners_.empty()) { throw std::runtime_error("no planners defined"); }

  if (!cfg_.teamPath.empty()) {
    loadTeamPopulation();
  }
  if (teams_.empty()) { throw std::runtime_error("no teams defined"); }

  initialized_ = true;
}


void Ranker::testInitialized() const {
  if (!initialized_) { throw std::runtime_error("ranker not initialized"); }
}


LeagueHeat Ranker::rank() const {
  testInitialized();

  LeagueHeat league = constructLeague();
  runLeague(league);

  if (cfg_.verbosity >= 1) { printLeagueStatistics(league); }
  return league;
}


LeagueHeat Ranker::constructLeague() const {
  LeagueHeat league;
  league.planners = planners_;
  league.evaluators = evaluators_;
  league.teams = teams_;
  league.pokemon = pokemon_;
  for (auto& planner: league.planners) {
    for (auto& evaluator: league.evaluators) {
      for (auto& team: league.teams) {
        league.battlegroups.push_back(std::make_shared<Battlegroup>(
            team.second, evaluator.second, planner.second, cfg_.contributions));
      }
    }
  }

  assert(!league.battlegroups.empty());
  return league;
}


void Ranker::runLeague(LeagueHeat& league) const {
  testInitialized();
  for (size_t iBG = 0; iBG < league.battlegroups.size(); ++iBG) {
    BattlegroupPtr& cBG = league.battlegroups.at(iBG);

    gauntlet(cBG, league);
  }
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
  auto& game = games_.at(omp_get_thread_num());

  auto setTSTeam = [&](size_t iTeam, BattlegroupPtr& battlegroup){
    // clone a planner by pointer:
    std::unique_ptr<Planner> planner{battlegroup->planner().get().clone()};
    // clone an evaluator by reference:
    planner->setEvaluator(battlegroup->evaluator().get());

    game.setPlanner(iTeam, *planner);
    game.setTeam(iTeam, battlegroup->team().nv());
  };

  setTSTeam(0, battlegroup_a);
  setTSTeam(1, battlegroup_b);
  HeatResult result = game.run();

  return GameHeat{battlegroup_a, battlegroup_b, result};
}


BattlegroupPtr Ranker::findMatch(const Battlegroup& cBG, const LeagueHeat& league) const {
  size_t numMatches = league.battlegroups.size();
  std::vector<double> matchQualities; matchQualities.reserve(numMatches);

  // compute match quality between all pairs in the league:
  for (size_t iBG = 0; iBG < league.battlegroups.size(); ++iBG) {
    const BattlegroupPtr& oBG = league.battlegroups.at(iBG);
    matchQualities.push_back(gameFactory_.matchQuality(cBG, *oBG));
  }

  auto filterByPredicate = [&](auto predicate){
    for (size_t iBG = 0; iBG < league.battlegroups.size() && numMatches > 0; ++iBG) {
      const BattlegroupPtr& oBG = league.battlegroups.at(iBG);
      if (!predicate(iBG, *oBG)) { continue; }
      matchQualities[iBG] = 0.;
      numMatches--;
    }
  };

  // filter matches that aren't in the same league:
  if (cfg_.enforceSameLeague) {
    filterByPredicate([&](size_t iBG, const Battlegroup& oBG){
      return cBG.team().nv().getNumTeammates() != oBG.team().nv().getNumTeammates();
    });
  }

  // filter matches that feature the same elements
  if (!cfg_.allowSameTeam) {
    filterByPredicate([&](size_t iBG, const Battlegroup& oBG){
      return cBG.team() == oBG.team();
    });
  }

  if (!cfg_.allowSameEvaluator) {
    filterByPredicate([&](size_t iBG, const Battlegroup& oBG){
      return cBG.evaluator() == oBG.evaluator();
    });
  }

  if (!cfg_.allowSamePlanner) {
    filterByPredicate([&](size_t iBG, const Battlegroup& oBG){
      return cBG.planner() == oBG.planner();
    });
  }

  // choose a random good match:
  std::discrete_distribution<size_t> probabilities{matchQualities.begin(), matchQualities.end()};
  return league.battlegroups.at(probabilities(rand_));
}


Ranker& Ranker::setGame(const Game& game) {
  games_.clear();
  games_.resize(std::max(cfg_.numThreads, 1U), game);
  return *this;
}


template<typename League, typename LeagueType>
void addToLeague(const LeagueType& obj, League& league, const std::string& itemType) {
  if (league.count(obj.hash()) > 0) {
    std::cerr << boost::format("Duplicate %s added to league \"%s\"!\n") % itemType % obj.getName();
  }
  
  league[obj.hash()] = std::make_shared<LeagueType>(obj);
}


Ranker& Ranker::addTeam(const TeamNonVolatile& team) {
  addToLeague(RankedTeam{team, pokemon_}, teams_, "team");
  return *this;
}


Ranker& Ranker::addEvaluator(const std::shared_ptr<Evaluator>& evaluator) {
  addToLeague(RankedEvaluator{evaluator}, evaluators_ , "evaluator");
  return *this;
}


Ranker& Ranker::addPlanner(const std::shared_ptr<Planner>& planner) {
  addToLeague(RankedPlanner{planner}, planners_, "planner");
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
    const auto& record = ranked->record();

    os << boost::format(" %02d: %s\n") % (iRanked+1) % *ranked;
  }
}


void Ranker::printLeagueStatistics(LeagueHeat& league) const {
  out_.get() << boost::format("played %d games!\n") % league.games.size();
  if (league.evaluators.size() >= 2) {
    out_.get() << "---- EVALUATOR LEADERBOARD ----\n";
    printMapLeaderboard(out_, league.evaluators, cfg_.leaderboardPrintCount);
  }
  if (league.planners.size() >= 2) {
    out_.get() << "---- PLANNER LEADERBOARD ----\n";
    printMapLeaderboard(out_, league.planners, cfg_.leaderboardPrintCount);
  }
  if (league.pokemon.size() >= 2 && cfg_.printPokemonLeaderboard) {
    out_.get() << "---- POKEMON LEADERBOARD ----\n";
    printMapLeaderboard(out_, league.pokemon, cfg_.leaderboardPrintCount);
  }
  if (league.teams.size() >= 2) {
    out_.get() << "---- TEAM LEADERBOARD ----\n";
    printMapLeaderboard(out_, league.teams, cfg_.leaderboardPrintCount);
  }
  if (cfg_.printBattlegroupLeaderboard) {
    out_.get() << "---- BATTLEGROUP LEADERBOARD ----\n";
    printLeaderboard(out_, league.battlegroups, cfg_.leaderboardPrintCount);
  }
}


void Ranker::printHeatResult(const GameHeat& heat) const {
  auto endStatus = heat.heatResult.endStatus;
  std::string winDrawLoss =
      endStatus==MATCH_TEAM_A_WINS?">":
      endStatus==MATCH_TEAM_B_WINS?"<":
      endStatus==MATCH_TIE?"=":"~";
  out_.get() << boost::format("%32.32s %s %-32.32s\n")
      % heat.team_a->getName()
      % winDrawLoss
      % heat.team_b->getName();
}


void Ranker::digestGame(GameHeat& gameHeat, LeagueHeat& league) const {
  // update ranks of both teams
  // TODO(@drendleman) - this should update all battlegroups which reference the specified component
  gameFactory_.update(*gameHeat.team_a, *gameHeat.team_b, gameHeat.heatResult);
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
