#include "../inc/ranker.h"

#include <assert.h>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include <random>
#include <omp.h>
#include <boost/format.hpp>


Ranker::Ranker(const Config& cfg) : cfg_(cfg), out_(std::cout), rand_(cfg.randomSeed) {
  setGame(Game{cfg_.game});
}


void Ranker::initialize() {
  if (games_.empty()) {throw std::runtime_error("game undefined"); }
  if (teams_.empty()) { throw std::runtime_error("no teams defined"); }
  if (evaluators_.empty()) { throw std::runtime_error("no evaluators defined"); }
  if (planners_.empty()) { throw std::runtime_error("no planners defined"); }

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
  std::vector<double> matchQualities(0.0, league.battlegroups.size());
  size_t numMatches = league.battlegroups.size();

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
      [](const typename VectorLeagueType::value_type& a, const typename VectorLeagueType::value_type& b) {
        return a.get() > b.get();
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


void Ranker::digestGame(GameHeat& gameHeat, LeagueHeat& league) const {
  // update ranks of both teams
  gameFactory_.update(*gameHeat.team_a, *gameHeat.team_b, gameHeat.heatResult);
  // update statistics:
  gameHeat.team_a->update(gameHeat.heatResult, TEAM_A);
  gameHeat.team_b->update(gameHeat.heatResult, TEAM_B);
  league.games.push_back(gameHeat);
}
