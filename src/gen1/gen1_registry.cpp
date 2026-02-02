#include "gen1_scripts_internal.h"

namespace gen1 {

void registerGen1Extensions(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  initializePointers(pkAI);

  register_move_struggle(pkAI, extensions);

  register_engine_common(pkAI, extensions);
}

} // namespace gen1
