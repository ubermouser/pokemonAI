/* 
 * File:   team_nonvolatile.h
 * Author: Ubermouser
 *
 * Created on June 30, 2011, 6:52 PM
 */

#ifndef TEAM_NONVOLATILE_H
#define	TEAM_NONVOLATILE_H

#include <stdint.h>

#include <array>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/irange.hpp>
#include <iosfwd>
#include <string>
#include <vector>

#include "actor.h"
#include "name.h"
#include "pkai.h"
#include "pokemon_nonvolatile.h"
#include "serializable.h"
#include "signature.h"


class TeamVolatile;

#define TEAM_NONVOLATILE_DIGESTSIZE (POKEMON_NONVOLATILE_DIGESTSIZE * 6 + 1)

class PKAISHARED TeamNonVolatile
  : public Name,
    public Signature<TeamNonVolatile, TEAM_NONVOLATILE_DIGESTSIZE>,
    public Serializable<TeamNonVolatile> {
public:
  /* nonvolatile teammmates of this team */ 
  std::vector<PokemonNonVolatile> teammates_;

  TEAM team_;

  TeamNonVolatile();
  TeamNonVolatile(const TeamNonVolatile& orig) = default;
  ~TeamNonVolatile() = default;

  PokemonNonVolatile& teammate(size_t iTeammate);
  const PokemonNonVolatile& teammate(size_t iTeammate) const;

  const PokemonNonVolatile& getPKNV(const TeamVolatile& source) const;

  /* returns number of teammates current pokemon team has */
  size_t getNumTeammates() const { return teammates_.size(); };

  static size_t getMaxNumTeammates() { return 6; }

  size_t iTeam() const { return team_; }

  /* is this pokemon allowed to be on the given team according to the current ruleset? */
  bool isLegalAdd(const PokemonNonVolatile& cPokemon) const;

  bool isLegalSet(size_t iPosition, const PokemonNonVolatile& cBase) const;

  bool isLegalAdd(const PokemonBase& cPokemon) const;

  bool isLegalSet(size_t iPosition, const PokemonBase& cBase) const;

  /* add a pokemon to the array of pokemon in this team */
  TeamNonVolatile& addPokemon(const PokemonNonVolatile& cPokemon);

  /* remove a pokemon from the array of pokemon in this team */
  TeamNonVolatile& removePokemon(size_t iPokemon);

  /* sets pokemon to swappedPokemon */
  TeamNonVolatile& setPokemon(size_t iPokemon, const PokemonNonVolatile& swappedPokemon);

  /* sets the lead pokemon. The pokemon that was the lead pokemon takes the index of the switched pokemon */
  TeamNonVolatile& setLeadPokemon(size_t iPokemon);

  TeamNonVolatile& initialize();

  auto yieldActors(size_t iCurrent = 0) const {
    size_t numTeammates = getNumTeammates();
    TEAM team = team_;
    return boost::irange(iCurrent, iCurrent + numTeammates) |
           boost::adaptors::transformed([=](size_t iTeammate) {
             return Actor((size_t)team, iTeammate % numTeammates);
           });
  }

  auto yieldPokemon(size_t iCurrent = 0) const {
    return yieldActors(iCurrent) |
           boost::adaptors::transformed([&](const Actor& actor) {
             return std::make_tuple(actor, teammate(actor.iTeammate()));
           });
  }

  void uninitialize();

  const std::string& defineName();

  void printSummary(std::ostream& os, const std::string& prefix="") const;

  void input(const boost::property_tree::ptree& ptree);
  boost::property_tree::ptree output(bool printHeader = true) const;
  
  void createDigest_impl(std::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const;
};


std::ostream& operator <<(std::ostream& os, const TeamNonVolatile& team);

#endif	/* TEAM_NONVOLATILE_H */

