#include "gen4_scripts_internal.h"

namespace gen4 {

int item_lifeOrb_modPower(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target,
    fpType& modifier) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  if (!cPKV.hasItem() || !(&cPKV.getItem() == lifeOrb_t)) { return 0; }

  modifier *= 1.3;

  return 1;
};

int item_lifeOrb_modLife(
    PkCUEngine& cu,
    const Actor& actor,
    const Action& action,
    const Actor& target) {
  PokemonVolatile cPKV = cu.getPKV(actor);
  MoveVolatile mV = cu.getMV(actor);
  // must have hit, must have life orb item
  if (!cu.getBase().flagsFor((TEAM)cu.getICTeam()).isHit() || !cPKV.hasItem() ||
      !(&cPKV.getItem() == lifeOrb_t)) {
    return 0;
  }

  uint8_t dType = mV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // subtract hitpoints:
  cPKV.modPercentHP(-0.1);

  return cPKV.isAlive() ? 1 : 2;
};

void register_item_life_orb(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnModifyRawDamage(item, "life orb", item_lifeOrb_modPower, 0, all_teams));
  extensions.push_back(pluginOnEndOfMove(item, "life orb", item_lifeOrb_modLife, 0, all_teams));
}

} // namespace gen4
