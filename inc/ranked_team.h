#ifndef TEAM_RANKED_H
#define TEAM_RANKED_H

#include "pkai.h"

#include <stdint.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>

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
  static const std::string HEADER;

  RankedTeam(const TeamNonVolatile& cTeam, PokemonLeague& league);

  virtual const std::string& getName() const override { return nv_.getName(); };

  const TeamNonVolatile& nv() const { return nv_; }
  TeamNonVolatile& nv() { return nv_; }

  std::vector<RankedPokemonPtr>& teammates() { return ranked_components_; }

  virtual void update(const HeatResult& hResult, size_t iTeam) override;

  virtual void input(const boost::property_tree::ptree& ptree) override;
  virtual boost::property_tree::ptree output(bool printHeader = true) const override;
protected:
  std::vector<RankedPokemonPtr> findSubteams(PokemonLeague& league) const;

  /* generate the hash, and the pokemon subhashes too if true */
  virtual void identify() override;
  virtual Hash generateHash(bool generateSubHashes = true) override;
  std::string defineName() override;

  /* team that this ranked_team represents */
  TeamNonVolatile nv_;

  std::vector<RankedPokemonPtr> ranked_components_;
};

using RankedTeamPtr = std::shared_ptr<RankedTeam>;
using TeamLeague = std::unordered_map<RankedPokemon::Hash, RankedTeamPtr >;

#endif /* TEAM_RANKED_H */
