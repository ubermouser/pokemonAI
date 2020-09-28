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
#include <iosfwd>

#include "ranked.h"
#include "pokemon_nonvolatile.h"

struct PokemonRankedRecord : public RankedRecord {
  /* number of times a pokemon is in play */
  uint64_t numPliesInPlay;

  /* number of times a given move has been used */
  std::array<uint64_t, 5> numMoves;
};


class RankedPokemon : public Ranked {
public:
  RankedPokemon(const PokemonNonVolatile& pk) : Ranked(pk.hash()), pokemon_(pk) { identify(); }

  const PokemonNonVolatile& get() const { return pokemon_; }

  virtual void update(const HeatResult& hResult, size_t iTeam, size_t iPokemon);
  virtual const std::string& getName() const override { return pokemon_.getName(); };

  virtual const PokemonRankedRecord& record() const override { return record_; }
  virtual PokemonRankedRecord& record() override { return record_; }
protected:
  virtual void identify() override;
  virtual Hash generateHash(bool generateSubHashes = true) override;
  virtual std::string defineName() override { return pokemon_.defineName(); }

  PokemonRankedRecord record_;

  PokemonNonVolatile pokemon_;
};

using RankedPokemonPtr = std::shared_ptr<RankedPokemon>;
using PokemonLeague = std::unordered_map<RankedPokemon::Hash, RankedPokemonPtr >;

#endif /* RANKED_POKEMON_H */

