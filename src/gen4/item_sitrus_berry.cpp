#include "gen4_scripts_internal.h"

namespace gen4 {

namespace {

bool checkAndTriggerSitrusBerry(PokemonVolatile pkv) {
  if (pkv.isAlive() && pkv.hasItem() && (&pkv.getItem() == sitrusBerry_t)) {
    if (pkv.getHP() <= pkv.nv().getMaxHP() / 2) {
      pkv.modPercentHP(0.25);
      pkv.setNoItem();
      return true;
    }
  }
  return false;
}

} // namespace

int item_sitrusBerry_endOfMove(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV) {
  int result = 0;
  if (checkAndTriggerSitrusBerry(cPKV)) { result = 1; }
  if (checkAndTriggerSitrusBerry(tPKV)) { result = 1; }
  return result;
}

int item_sitrusBerry_onEndOfTurn(PkCUEngine& cu, PokemonVolatile cPKV) {
  return checkAndTriggerSitrusBerry(cPKV) ? 1 : 0;
}

int item_sitrusBerry_onEndOfRound(PkCUEngine& cu, PokemonVolatile cPKV) {
  return checkAndTriggerSitrusBerry(cPKV) ? 1 : 0;
}

int item_sitrusBerry_onSwitchIn(PkCUEngine& cu, PokemonVolatile cPKV) {
  return checkAndTriggerSitrusBerry(cPKV) ? 1 : 0;
}

void register_item_sitrus_berry(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(pluginOnEndOfMove(item, "sitrus berry", item_sitrusBerry_endOfMove, 0, all_teams));
  extensions.push_back(pluginOnEndOfTurn(item, "sitrus berry", item_sitrusBerry_onEndOfTurn, 0, all_teams));
  extensions.push_back(pluginOnEndOfRound(item, "sitrus berry", item_sitrusBerry_onEndOfRound, 0, all_teams));
  extensions.push_back(pluginOnSwitchIn(item, "sitrus berry", item_sitrusBerry_onSwitchIn, 0, all_teams));
}

} // namespace gen4
