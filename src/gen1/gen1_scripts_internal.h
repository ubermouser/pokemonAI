#ifndef GEN1_SCRIPTS_INTERNAL_H
#define GEN1_SCRIPTS_INTERNAL_H

#include <stdint.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "pokemonai/engine.h"
#include "pokemonai/orphan.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/pkai.h"
#include "pokemonai/pluggable_types.h"
#include "pokemonai/plugin.h"


class Move;
class Item;
class Ability;
class Type;

namespace gen1 {

extern const Pokedex* dex;

extern const Move* struggle_t;

extern const Type* normal_t;
extern const Type* fighting_t;
extern const Type* flying_t;
extern const Type* poison_t;
extern const Type* ground_t;
extern const Type* rock_t;
extern const Type* bug_t;
extern const Type* ghost_t;
extern const Type* steel_t;
extern const Type* fire_t;
extern const Type* water_t;
extern const Type* grass_t;
extern const Type* electric_t;
extern const Type* psychic_t;
extern const Type* ice_t;
extern const Type* dragon_t;

// clang-format off
void initializePointers(const Pokedex& pkAI);

void register_move_struggle(const Pokedex& pkAI, std::vector<plugin>& extensions);
void register_engine_common(const Pokedex& pkAI, std::vector<plugin>& extensions);

void registerGen1Extensions(const Pokedex& pkAI, std::vector<plugin>& extensions);
// clang-format on

} // namespace gen1

#endif // GEN1_SCRIPTS_INTERNAL_H
