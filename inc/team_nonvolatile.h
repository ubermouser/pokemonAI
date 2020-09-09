/* 
 * File:   team_nonvolatile.h
 * Author: Ubermouser
 *
 * Created on June 30, 2011, 6:52 PM
 */

#ifndef TEAM_NONVOLATILE_H
#define	TEAM_NONVOLATILE_H

#include "../inc/pkai.h"

#include <array>
#include <ostream>
#include <stdint.h>
#include <string>
#include <vector>

#include "name.h"
#include "pokemon_nonvolatile.h"
#include "signature.h"


class TeamVolatile;

#define TEAM_NONVOLATILE_DIGESTSIZE (POKEMON_NONVOLATILE_DIGESTSIZE * 6 + 1)

class PKAISHARED TeamNonVolatile : public Name, public Signature<TeamNonVolatile, TEAM_NONVOLATILE_DIGESTSIZE> 
{
public:
  /* nonvolatile teammmates of this team */ 
  std::array<PokemonNonVolatile, 6> teammates;

  /* number of pokemon in this team */
  uint8_t numTeammates;
  
  TeamNonVolatile();
  TeamNonVolatile(const TeamNonVolatile& orig);
  ~TeamNonVolatile() { };

  PokemonNonVolatile& teammate(size_t iTeammate);

  const PokemonNonVolatile& getPKNV(const TeamVolatile& source) const;

  /* returns number of teammates current pokemon team has */
  size_t getNumTeammates() const
  {
    return (size_t) numTeammates;
  };

  static size_t getMaxNumTeammates()
  {
    return 6;
  }

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

  void uninitialize();
  
  const PokemonNonVolatile& teammate(size_t iTeammate) const;

  void printSummary(std::ostream& os, const std::string& prefix) const;
  void output(std::ostream& oFile, bool printHeader = true) const;

  bool input(const std::vector<std::string>& lines, size_t& iLine);
  
  void createDigest_impl(std::array<uint8_t, TEAM_NONVOLATILE_DIGESTSIZE>& digest) const;

  static TeamNonVolatile loadFromFile(const std::string& path);

};


std::ostream& operator <<(std::ostream& os, const TeamNonVolatile& team);

#endif	/* TEAM_NONVOLATILE_H */

