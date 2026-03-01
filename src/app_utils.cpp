#include "pokemonai/app_utils.h"

#include <spdlog/spdlog.h>

#include <cstdlib>
#include <ctime>
#include <fstream>

#include "pokemonai/logging.h"

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

void parse_command_line_and_config(
    int argc,
    char** argv,
    const po::options_description& desc,
    po::variables_map& vm,
    bool allowUnregistered) {
  auto parser = po::command_line_parser(argc, argv).options(desc);
  if (allowUnregistered) { parser.allow_unregistered(); }
  po::store(parser.run(), vm);
  po::notify(vm);

  if (vm.count("config")) {
    std::string config_file = vm["config"].as<std::string>();
    SPDLOG_WARN("Loading configuration at \"{}\"...", config_file);

    std::ifstream ifs(config_file.c_str());
    if (!ifs) {
      throw std::runtime_error("Could not open config file: " + config_file);
    }
    po::store(po::parse_config_file(ifs, desc, allowUnregistered), vm);
    po::notify(vm);
  }
}

} // namespace PokemonAIAppUtils
