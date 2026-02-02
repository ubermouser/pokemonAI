#include "gen1_scripts_internal.h"

namespace gen1 {

const Pokedex* dex;

const Move* struggle_t;

const Type* normal_t;
const Type* fighting_t;
const Type* flying_t;
const Type* poison_t;
const Type* ground_t;
const Type* rock_t;
const Type* bug_t;
const Type* ghost_t;
const Type* steel_t;
const Type* fire_t;
const Type* water_t;
const Type* grass_t;
const Type* electric_t;
const Type* psychic_t;
const Type* ice_t;
const Type* dragon_t;

void initializePointers(const Pokedex& pkAI) {
  // register needed types:
  dex = &pkAI;
  const Moves& moves = dex->getMoves();
  struggle_t = orphan::orphanCheck(moves, "struggle");

  const Types& types = dex->getTypes();
  normal_t = orphan::orphanCheck(types, "normal");
  fighting_t = orphan::orphanCheck(types, "fighting");
  flying_t = orphan::orphanCheck(types, "flying");
  poison_t = orphan::orphanCheck(types, "poison");
  ground_t = orphan::orphanCheck(types, "ground");
  rock_t = orphan::orphanCheck(types, "rock");
  bug_t = orphan::orphanCheck(types, "bug");
  ghost_t = orphan::orphanCheck(types, "ghost");
  steel_t = orphan::orphanCheck(types, "steel");
  fire_t = orphan::orphanCheck(types, "fire");
  water_t = orphan::orphanCheck(types, "water");
  grass_t = orphan::orphanCheck(types, "grass");
  electric_t = orphan::orphanCheck(types, "electric");
  psychic_t = orphan::orphanCheck(types, "psychic");
  ice_t = orphan::orphanCheck(types, "ice");
  dragon_t = orphan::orphanCheck(types, "dragon");
}

} // namespace gen1

extern "C" bool registerExtensions(
    const Pokedex& pkAI, std::vector<plugin>& extensions) {
  gen1::registerGen1Extensions(pkAI, extensions);
  return true;
}