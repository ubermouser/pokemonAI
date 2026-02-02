#include "gen4_scripts_internal.h"
#include "pokemonai/pkCU.h"

namespace gen4 {

int move_hiddenPower_calculate(
    PokemonNonVolatile& cPKNV, MoveNonVolatile& cMNV) {
  // formula from http://www.smogon.com/dp/moves/hidden_power

  // clang-format off
  uint16_t cType =
    ((
      (cPKNV.getIV(FV_HITPOINTS) % 2) * 1 +
      (cPKNV.getIV(FV_ATTACK) % 2)    * 2 +
      (cPKNV.getIV(FV_DEFENSE) % 2)   * 4 +
      (cPKNV.getIV(FV_SPEED) % 2)     * 8 +
      (cPKNV.getIV(FV_SPATTACK) % 2)  * 16 +
      (cPKNV.getIV(FV_SPDEFENSE) % 2) * 32
    ) * 15) / 63;

  uint16_t cPower =
    (((
      ((cPKNV.getIV(FV_HITPOINTS) % 4) > 1 ? 1 : 0 ) +
      ((cPKNV.getIV(FV_ATTACK) % 4)    > 1 ? 2 : 0 ) +
      ((cPKNV.getIV(FV_DEFENSE) % 4)   > 1 ? 4 : 0 ) +
      ((cPKNV.getIV(FV_SPEED) % 4)     > 1 ? 8 : 0 ) +
      ((cPKNV.getIV(FV_SPATTACK) % 4)  > 1 ? 16 : 0 ) +
      ((cPKNV.getIV(FV_SPDEFENSE) % 4) > 1 ? 32 : 0 )
    ) * 40) / 63) + 30;
  // clang-format on

  // pointer arithmetic
  switch (cType) {
  case 0:
    cType = (uint16_t)fighting_t->index_;
    break;
  case 1:
    cType = (uint16_t)flying_t->index_;
    break;
  case 2:
    cType = (uint16_t)poison_t->index_;
    break;
  case 3:
    cType = (uint16_t)ground_t->index_;
    break;
  case 4:
    cType = (uint16_t)rock_t->index_;
    break;
  case 5:
    cType = (uint16_t)bug_t->index_;
    break;
  case 6:
    cType = (uint16_t)ghost_t->index_;
    break;
  case 7:
    cType = (uint16_t)steel_t->index_;
    break;
  case 8:
    cType = (uint16_t)fire_t->index_;
    break;
  case 9:
    cType = (uint16_t)water_t->index_;
    break;
  case 10:
    cType = (uint16_t)grass_t->index_;
    break;
  case 11:
    cType = (uint16_t)electric_t->index_;
    break;
  case 12:
    cType = (uint16_t)psychic_t->index_;
    break;
  case 13:
    cType = (uint16_t)ice_t->index_;
    break;
  case 14:
    cType = (uint16_t)dragon_t->index_;
    break;
  default:
  case 15:
    cType = (uint16_t)dark_t->index_;
    break;
  };

  assert((cType < dex->getTypes().size()) && cPower <= 70);

  cMNV.setScriptVal_a(cType);
  cMNV.setScriptVal_b(cPower);

  return 2;
};

int move_hiddenPower_setPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    uint32_t& basePower) {
  if (&mV.getBase() != hiddenPower_t) { return 0; }

  basePower = mV.nv().getScriptVal_b();
  return 1;
}

int move_hiddenPower_setType(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    const Type*& cType) {
  if (&(mV.getBase()) != hiddenPower_t) { return 0; }

  cType = dex->getTypes().atByIndex(mV.nv().getScriptVal_a());
  return 1;
};

void register_move_hidden_power(const Pokedex& pkAI, std::vector<plugin>& extensions) {
  extensions.push_back(plugin(move, "hidden power", PLUGIN_ON_INIT, move_hiddenPower_calculate, 0, current_team));
  extensions.push_back(plugin(move, "hidden power", PLUGIN_ON_SETBASEPOWER, move_hiddenPower_setPower, 0, current_team));
  extensions.push_back(plugin(move, "hidden power", PLUGIN_ON_SETMOVETYPE, move_hiddenPower_setType, 0, current_team));
}

} // namespace gen4
