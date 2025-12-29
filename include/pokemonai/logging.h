#ifndef POKEMONAI_LOGGING_H
#define POKEMONAI_LOGGING_H

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

void initialize_logger(int log_level = spdlog::level::info);

#endif // POKEMONAI_LOGGING_H
