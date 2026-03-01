#ifndef POKEMON_AI_APP_UTILS_H
#define POKEMON_AI_APP_UTILS_H

#include <boost/program_options.hpp>

#include "pokemonai/pokedex_dynamic.h"

namespace PokemonAIAppUtils {

namespace po = boost::program_options;

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

/**
 * @brief Parses command-line arguments and an optional config file.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @param desc Options description.
 * @param vm Variables map to store parsed options.
 * @param allowUnregistered Whether to allow unregistered options.
 */
void parse_command_line_and_config(
    int argc,
    char** argv,
    const po::options_description& desc,
    po::variables_map& vm,
    bool allowUnregistered = false);

} // namespace PokemonAIAppUtils

#endif // POKEMON_AI_APP_UTILS_H
