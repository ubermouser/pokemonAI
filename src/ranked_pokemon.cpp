#include "pokemonai/ranked_pokemon.h"

#include <sstream>
#include <string>
#include <boost/format.hpp>

void RankedPokemon::identify() {
  Hash newHash = get().hash();
  if (newHash != hash()) {
    generateHash(true);
    defineName();
  }
}


RankedPokemon::Hash RankedPokemon::generateHash(bool generateSubHashes) {
  hash_ = pokemon_.hash();
  return hash_;
}


void RankedPokemon::update(const HeatResult& hResult, size_t iTeam, size_t iPokemon) {
  Ranked::update(hResult, iTeam);

  const auto& cPokemon = hResult.teams[iTeam].pokemon[iPokemon];
  record().numPliesInPlay += uint64_t(cPokemon.participation * hResult.numPlies * hResult.matchesPlayed);
  for (size_t iMove = 0; iMove < 5; ++iMove) {
    record().numMoves[iMove] += uint64_t(cPokemon.moveUse[iMove] * hResult.numPlies * hResult.matchesPlayed);
  }
}
