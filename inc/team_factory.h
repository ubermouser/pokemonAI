/* 
 * File:   team_factory.h
 * Author: drendleman
 *
 * Created on September 18, 2020, 11:31 AM
 */

#ifndef TEAM_FACTORY_H
#define TEAM_FACTORY_H

#include "engine.h"

class TeamFactory {
public:

  TeamNonVolatile createRandom(size_t numPokemon) const;

  PokemonNonVolatile createRandom_single(const TeamNonVolatile& cTeam, size_t iReplace = SIZE_MAX) const;

  /* randomize a pokemon's species */
  void randomSpecies(const TeamNonVolatile& cTeam, PokemonNonVolatile& cPokemon, size_t iReplace = SIZE_MAX) const;

  void randomAbility(PokemonNonVolatile& cPokemon) const;

  void randomNature(PokemonNonVolatile& cPokemon) const;

  void randomItem(PokemonNonVolatile& cPokemon) const;

  /* randomize a number of the pokemon's IVs */
  void randomIV(PokemonNonVolatile& cPokemon, size_t numIVs = 1) const;

  void randomEV(PokemonNonVolatile& cPokemon) const;

  void randomGender(PokemonNonVolatile& cPokemon) const;

  /* randomize a number of the pokemon's moves */
  void randomMove(PokemonNonVolatile& cPokemon, size_t numMoves = 1) const;

  void mutate_single(TeamNonVolatile& cTeam, size_t iTeammate, size_t numMutations) const;

  PokemonNonVolatile crossover_single(
      const PokemonNonVolatile& parentA,
      const PokemonNonVolatile& parentB) const;

  TeamNonVolatile crossover(
      const TeamNonVolatile& parentA,
      const TeamNonVolatile& parentB) const;

protected:
};

#endif /* TEAM_FACTORY_H */

