#include "pokemonai/app_utils.h"
#include "pokemonai/logging.h"
#include <spdlog/spdlog.h>
#include <cstdlib>
#include <ctime>

namespace PokemonAIAppUtils {

PokedexDynamic bootstrap(
    int verbosity,
    int randomSeed,
    const PokedexDynamic::Config& pokedexCfg) {
  
  initialize_logger();
  spdlog::set_level(spdlog::level::level_enum(verbosity));
  srand((randomSeed < 0) ? std::time(nullptr) : randomSeed);
  
  return PokedexDynamic(pokedexCfg);
}

} // namespace PokemonAIAppUtils
