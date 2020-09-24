#include "../inc/ranker.h"
#include "../inc/ranked_battlegroup.h"

#include <algorithm>
#include <unordered_map>
#include <random>
#include <omp.h>

void Ranker::initialize() {
  
}


void Ranker::runLeague(LeagueHeat& league) const {
  for (size_t iBG = 0; iBG < league.battlegroups.size(); ++iBG) {
    const BattlegroupPtr& cBG = league.battlegroups.at(iBG);

    gauntlet(cBG, league);
  }
}

void Ranker::gauntlet(BattlegroupPtr tsTeam, LeagueHeat& league) const {
  auto& record = tsTeam->record();
  while (record.numGamesPlayed() < cfg_.minGamesPerGeneration) {
    BattlegroupPtr adversary = findMatch(tsTeam, league);

    GameHeat result = singleGame(tsTeam, adversary);
    digestGame(result);
  }
}


GameHeat Ranker::singleGame(
    BattlegroupPtr& team_a,
    BattlegroupPtr& team_b) const {
  auto& game = games.at(omp_get_thread_num());

  auto setTSTeam = [&](size_t iTeam, BattlegroupPtr& tsTeam){
    // clone a planner by pointer:
    std::unique_ptr<Planner> planner{tsTeam->planner()->get().clone()};
    // clone an evaluator by reference:
    planner->setEvaluator(tsTeam->evaluator()->get());

    game->setPlanner(iTeam, *planner);
    game->setTeam(iTeam, tsTeam->team()->nv());
  };

  setTSTeam(0, team_a);
  setTSTeam(1, team_b);
  HeatResult result = game->run();

  return GameHeat{team_a, team_b, result};
}


BattlegroupPtr Ranker::findMatch(const BattlegroupPtr& cBG, const LeagueHeat& league) const {
  std::vector<double> matchQualities(0.0, league.battlegroups.size());
  size_t numMatches = league.battlegroups.size();

  // compute match quality between all pairs in the league:
  for (size_t iBG = 0; iBG < league.battlegroups.size(); ++iBG) {
    const BattlegroupPtr& oBG = league.battlegroups.at(iBG);
    matchQualities.push_back(gameFactory_->matchQuality(*cBG, *oBG));
  }

  auto filterByPredicate = [&](auto predicate){
    for (size_t iBG = 0; iBG < league.battlegroups.size() && numMatches > 0; ++iBG) {
      const BattlegroupPtr& oBG = league.battlegroups.at(iBG);
      if (!predicate(iBG, oBG)) { continue; }
      matchQualities[iBG] = 0.;
      numMatches--;
    }
  };

  // filter matches that aren't in the same league:
  if (cfg_.enforceSameLeague) {
    filterByPredicate([&](size_t iBG, const BattlegroupPtr& oBG){
      return cBG->team()->nv().getNumTeammates() != oBG->team()->nv().getNumTeammates();
    });
  }

  // filter matches that feature the same elements
  if (!cfg_.allowSameTeam) {
    filterByPredicate([&](size_t iBG, const BattlegroupPtr& oBG){
      return cBG->team() == oBG->team();
    });
  }

  if (!cfg_.allowSameEvaluator) {
    filterByPredicate([&](size_t iBG, const BattlegroupPtr& oBG){
      return cBG->evaluator() == oBG->evaluator();
    });
  }

  if (!cfg_.allowSamePlanner) {
    filterByPredicate([&](size_t iBG, const BattlegroupPtr& oBG){
      return cBG->planner() == oBG->planner();
    });
  }

  // choose a random good match:
  std::default_random_engine generator{rand()};
  std::discrete_distribution<size_t> probabilities{matchQualities.begin(), matchQualities.end()};
  return league.battlegroups.at(probabilities(generator));
}


void Ranker::digestGame(GameHeat& gameHeat) const {
  // update ranks of both teams
  
}