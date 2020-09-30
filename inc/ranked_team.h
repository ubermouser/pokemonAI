#ifndef TEAM_RANKED_H
#define TEAM_RANKED_H

#include "pkai.h"

#include <stdint.h>
#include <array>
#include <vector>
#include <unordered_map>
#include <boost/property_tree/ptree_fwd.hpp>

#include "ranked.h"
#include "ranked_pokemon.h"
#include "team_nonvolatile.h"

using LeagueCount = std::array<size_t, 6>;

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

  struct Contribution {
    double synergy = 1.0;
    double pokemon = 1.0;

    Contribution(){};
  };

  RankedTeam(const TeamNonVolatile& cTeam, PokemonLeague& league, const Contribution& = Contribution{});

  virtual const std::string& getName() const override { return nv_.getName(); };

  const TeamNonVolatile& nv() const { return nv_; }
  TeamNonVolatile& nv() { return nv_; }

  std::vector<GroupContribution> contributions();
  TrueSkill& computeSkill();
  std::vector<RankedPokemonPtr>& teammates() { return ranked_components_; }

  virtual void update(const HeatResult& hResult, size_t iTeam) override;

  virtual void input(const boost::property_tree::ptree& ptree) override;
  virtual boost::property_tree::ptree output(bool printHeader = true) const override;
protected:
  std::vector<RankedPokemonPtr> findSubteams(PokemonLeague& league) const;

  /* generate the hash, and the pokemon subhashes too if true */
  virtual void identify() override;
  virtual Hash generateHash(bool generateSubHashes = true) override;
  std::string defineName() override { return nv_.defineName(); }

  /* team that this ranked_team represents */
  TeamNonVolatile nv_;

  std::vector<RankedPokemonPtr> ranked_components_;

  Contribution contribution_;

  TrueSkill synergy_;
};

using RankedTeamPtr = std::shared_ptr<RankedTeam>;

class TeamLeague: public std::unordered_map<RankedTeam::Hash, RankedTeamPtr> {
public:
  /* count the number of pokemon within each league */
  LeagueCount countTeamLeague() const;

  /* retrieve all teams in a specified league */
  std::vector<RankedTeamPtr> getLeague(size_t numPokemon) const;

  /* retrieve all teams */
  std::vector<RankedTeamPtr> getAll() const;
};

#endif /* TEAM_RANKED_H */
