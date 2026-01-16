#ifndef POKEMON_AI_APP_UTILS_H
#define POKEMON_AI_APP_UTILS_H

#include "pokemonai/pokedex_dynamic.h"

namespace PokemonAIAppUtils {

/**
 * @brief Performs common initialization for PokemonAI applications.
 * 
 * This includes:
 * 1. Initializing the logger.
 * 2. Setting the spdlog level based on verbosity.
 * 3. Seeding the random number generator (srand).
 * 4. Initializing and returning a PokedexDynamic instance.
 * 
 * @param verbosity spdlog level to set.
 * @param randomSeed Seed for srand. If < 0, time(NULL) is used.
 * @param pokedexCfg Configuration for PokedexDynamic.
 * @return PokedexDynamic instance.
 */
PokedexDynamic bootstrap(
    int verbosity,
    int randomSeed,
    const PokedexDynamic::Config& pokedexCfg);

} // namespace PokemonAIAppUtils

#endif // POKEMON_AI_APP_UTILS_H
