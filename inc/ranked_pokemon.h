/* 
 * File:   ranked_pokemon.h
 * Author: drendleman
 *
 * Created on September 21, 2020, 8:18 PM
 */

#ifndef RANKED_POKEMON_H
#define RANKED_POKEMON_H

#include <memory>
#include <unordered_map>
#include <ostream>

#include "ranked.h"
#include "pokemon_nonvolatile.h"

struct PokemonRankedRecord : public RankedRecord {
  /* number of times a given move has been used */
  std::array<uint64_t, 5> numMoves;
};


class RankedPokemon : public Ranked {
public:
  const PokemonNonVolatile& get() const { return pokemon_; }

  const std::string& getName() const { return pokemon_.getName(); };

  std::ostream& print(std::ostream& os) const;
protected:
  void generateHash(bool generateSubHashes = true) = 0;
  void defineName();

  PokemonRankedRecord record_;

  PokemonNonVolatile pokemon_;
};


std::ostream& operator <<(std::ostream& os, const RankedPokemon& tR);

using RankedPokemonPtr = std::shared_ptr<RankedPokemon>;
using PokemonLeague = std::unordered_map<RankedPokemon::Hash, RankedPokemonPtr >;

#endif /* RANKED_POKEMON_H */

