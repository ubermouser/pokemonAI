#ifndef LEAGUE_HEAT_H
#define LEAGUE_HEAT_H

#include <stdint.h>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

#include "pkai.h"
#include "game.h"
#include "league.h"
#include "ranked_battlegroup.h"

struct GameHeat {
  BattlegroupPtr team_a;
  
  BattlegroupPtr team_b;

  double matchQuality;

  HeatResult heatResult;
};


struct LeagueHeat : public League {
  std::vector<GameHeat> games;

  uint64_t numTies = 0;

  uint64_t numDraws = 0;

  uint64_t numGames = 0;

  uint64_t totalPlies = 0;

  double elapsedTime = 0.;

  LeagueHeat(const League& league = League{}) : League(league) {}

  double pliesPerGame() const { return double(totalPlies) / numGames; }
  double gamesPerSecond() const { return double(numGames) / elapsedTime; }
  double drawRate() const { return double(numDraws) / games.size(); }
  double tieRate() const { return double(numTies) / games.size(); }

  void resetStats() {
    games.clear();
    numTies = 0;
    numDraws = 0;
    numGames = 0;
    totalPlies = 0;
    elapsedTime = 0;
    counts = LeagueStats{};
  }

  void calculateStats() {
    calculateCounts();
    calculateContribution();
  }

  void calculateCounts();
  void calculateContribution();

  struct StatEntry {
    uint64_t count = 0;
    double aggregateContribution = 0;
    double simpleContribution = 0;
    double participation = 0;
  };

  struct LeagueStats {
    std::unordered_map<std::string, StatEntry> pokemon;
    std::unordered_map<std::string, StatEntry> abilities;
    std::unordered_map<std::string, StatEntry> types;
    std::unordered_map<std::string, StatEntry> natures;
    std::unordered_map<std::string, StatEntry> items;
    std::unordered_map<std::string, StatEntry> moves;
  } counts;
};

#endif /* LEAGUE_HEAT_H */
