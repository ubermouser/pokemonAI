#ifndef TEAM_RANKED_H
#define TEAM_RANKED_H

#include "pkai.h"

#include <stdint.h>
#include <array>
#include <vector>

#include "ranked.h"
#include "ranked_pokemon.h"
#include "team_nonvolatile.h"


struct TeamRankedRecord : public RankedRecord {
  /* number of plies a given pokemon has been active in */
  std::array<uint64_t, 6> numPokemonPlies;
  
  /* sum of the each pokemon's rank from game results */
  std::array<uint64_t, 6> rankPoints;

  void resetRecord() {
    RankedRecord::resetRecord();
    numPokemonPlies.fill(0);
    rankPoints.fill(0);
  };
};

class RankedTeam : public Ranked {
public:
  static const std::string header;

  RankedTeam(const TeamNonVolatile& cTeam = TeamNonVolatile(), const PokemonLeague& league, size_t generation = 0);
  RankedTeam(const RankedTeam& other) = default;

  const std::string& getName() const { return nv_.getName(); };

  const TeamNonVolatile& nv() const { return nv_; }
  TeamNonVolatile& nv() { return nv_; }

  void output(std::ostream& oFile, bool printHeader = true) const;

  std::ostream& print(std::ostream& os) const;

  bool input(const std::vector<std::string>& lines, size_t& firstLine);
protected:
  /* generate the hash, and the pokemon subhashes too if true */
  void generateHash(bool generateSubHashes = true);

  /* team that this ranked_team represents */
  TeamNonVolatile nv_;

  std::vector<RankedPokemonPtr> ranked_components_;
};

using RankedTeamPtr = std::shared_ptr<RankedTeam>;
using TeamLeague = std::unordered_map<RankedPokemon::Hash, RankedTeamPtr >;

std::ostream& operator <<(std::ostream& os, const RankedTeam& tR);

#endif /* TEAM_RANKED_H */
