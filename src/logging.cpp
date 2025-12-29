
#include "pokemonai/logging.h"

void initialize_logger(int log_level) {
  spdlog::set_pattern("[%H:%M:%S.%e] [%L] [%s:%#] %v");
  spdlog::set_level(spdlog::level::level_enum(log_level));
}