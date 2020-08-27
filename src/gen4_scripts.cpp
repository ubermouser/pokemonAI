//#define PKAI_STATIC
#include "../inc/pkai.h"
//#undef PKAI_STATIC

#include "../inc/gen4_scripts.h"

#include <stdint.h>
#include <vector>
#include <algorithm>

//#define PKAI_IMPORT
#include "../inc/orphan.h"
#include "../inc/engine.h"
#include "../inc/pkCU.h"
//#undef PKAI_IMPORT

//#define PKAI_EXPORT
#include "../inc/plugin.h"
//#undef PKAI_EXPORT

const Pokedex* dex;

const Move* airCutter_t;
const Move* absorb_t;
const Move* aerialAce_t;
const Move* attackOrder_t;
const Move* aromatherapy_t;
const Move* auraSphere_t;
const Move* blazeKick_t;
const Move* braveBird_t;
const Move* crabHammer_t;
const Move* crossChop_t;
const Move* crossPoison_t;
const Move* doubleEdge_t;
const Move* drainPunch_t;
const Move* explosion_t;
const Move* faintAttack_t;
const Move* flareBlitz_t;
const Move* gigaDrain_t;
const Move* healBell_t;
const Move* healOrder_t;
const Move* hiddenPower_t;
const Move* leafBlade_t;
const Move* leechLife_t;
const Move* magnetBomb_t;
const Move* magicalLeaf_t;
const Move* megaDrain_t;
const Move* memento_t;
const Move* milkDrink_t;
const Move* nightShade_t;
const Move* nightSlash_t;
const Move* painSplit_t;
const Move* psychoCut_t;
const Move* rapidSpin_t;
const Move* razorLeaf_t;
const Move* recover_t;
const Move* roost_t;
const Move* shadowClaw_t;
const Move* shadowPunch_t;
const Move* shockWave_t;
const Move* seismicToss_t;
const Move* selfDestruct_t;
const Move* softBoiled_t;
const Move* slackOff_t;
const Move* slash_t;
const Move* spikes_t;
const Move* stealthRock_t;
const Move* stoneEdge_t;
const Move* struggle_t;
const Move* swift_t;
const Move* toxicSpikes_t;
const Move* voltTackle_t;
const Move* woodHammer_t;

const Item* choiceBand_t;
const Item* choiceScarf_t;
const Item* choiceSpecs_t;
const Item* leftovers_t;
const Item* lifeOrb_t;
const Item* lumBerry_t;

const Ability* levitate_t;
const Ability* naturalCure_t;
const Ability* noGuard_t;
const Ability* technician_t;
const Ability* sereneGrace_t;

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
const Type* dark_t;

int move_hiddenPower_calculate(
  PokemonNonVolatile& cPKNV,
  MoveNonVolatile& cMNV)
{
  // formula from http://www.smogon.com/dp/moves/hidden_power

  uint16_t cType = 
    ((
      (cPKNV.getIV(FV_HITPOINTS)%2) * 1 +
      (cPKNV.getIV(FV_ATTACK)%2) * 2 +
      (cPKNV.getIV(FV_DEFENSE)%2) * 4 +
      (cPKNV.getIV(FV_SPEED)%2) * 8 +
      (cPKNV.getIV(FV_SPATTACK)%2) * 16 +
      (cPKNV.getIV(FV_SPDEFENSE)%2) * 32
    ) * 15) / 63;

  uint16_t cPower = 
    (((
      ((cPKNV.getIV(FV_HITPOINTS)%4)>1? 1 : 0 ) +
      ((cPKNV.getIV(FV_ATTACK)%4)>1? 2 : 0 ) +
      ((cPKNV.getIV(FV_DEFENSE)%4)>1? 4 : 0 ) +
      ((cPKNV.getIV(FV_SPEED)%4)>1? 8 : 0 ) +
      ((cPKNV.getIV(FV_SPATTACK)%4)>1? 16 : 0 ) +
      ((cPKNV.getIV(FV_SPDEFENSE)%4)>1? 32 : 0 )
    ) * 40) / 63) + 30;

  // pointer arithmetic
  switch(cType)
  {
  case 0:
    cType = (uint16_t)fighting_t->index_; break;
  case 1:
    cType = (uint16_t)flying_t->index_; break;
  case 2:
    cType = (uint16_t)poison_t->index_; break;
  case 3:
    cType = (uint16_t)ground_t->index_; break;
  case 4:
    cType = (uint16_t)rock_t->index_; break;
  case 5:
    cType = (uint16_t)bug_t->index_; break;
  case 6:
    cType = (uint16_t)ghost_t->index_; break;
  case 7:
    cType = (uint16_t)steel_t->index_; break;
  case 8:
    cType = (uint16_t)fire_t->index_; break;
  case 9:
    cType = (uint16_t)water_t->index_; break;
  case 10:
    cType = (uint16_t)grass_t->index_; break;
  case 11:
    cType = (uint16_t)electric_t->index_; break;
  case 12:
    cType = (uint16_t)psychic_t->index_; break;
  case 13:
    cType = (uint16_t)ice_t->index_; break;
  case 14:
    cType = (uint16_t)dragon_t->index_; break;
  default:
  case 15:
    cType = (uint16_t)dark_t->index_; break;
  };

  assert((cType < dex->getTypes().size()) && cPower <= 70);

  cMNV.setScriptVal_a(cType);
  cMNV.setScriptVal_b(cPower);

  return 2;
};

int move_hiddenPower_setType(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  const Type*& cType)
{
  if (&(mV.getBase()) != hiddenPower_t) { return 0; }

  cType = dex->getTypes().atByIndex(mV.nv().getScriptVal_a());
  return 1;
};

int move_hiddenPower_setPower(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  uint32_t& basePower)
{
  if (&mV.getBase() != hiddenPower_t) { return 0; }

  basePower = mV.nv().getScriptVal_b();
  return 1;
}

int move_painSplit(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  if (&mV.getBase() != painSplit_t) { return 0; }

  // calculate how much health total pokemon both have; average the two, rounding down
  uint32_t newHP = (cPKV.getHP() + tPKV.getHP() + 1) / 2;

  cPKV.setHP(newHP);
  tPKV.setHP(newHP);

  return 1;
};

int move_stealthRock_set(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  if (&mV.getBase() != stealthRock_t) { return 0; }

  tPKV.status().nonvolatile.stealthRock = 1;

  return 1;
};

int move_spikes_set(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  if (&mV.getBase() != spikes_t) { return 0; }

  uint32_t initial_spikes = tPKV.status().nonvolatile.spikes;
  tPKV.status().nonvolatile.spikes = std::min(3U, initial_spikes + 1U);

  return 1;
};

int move_toxicSpikes_set(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  if (&mV.getBase() != toxicSpikes_t) { return 0; }

  uint32_t initial_toxic = tPKV.status().nonvolatile.toxicSpikes;
  tPKV.status().nonvolatile.toxicSpikes = std::min(2U, initial_toxic + 1U);

  return 1;
};

int move_stealthRock_switch(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  if (cPKV.status().nonvolatile.stealthRock > 0) {
    // deal damage:
    fpType damage = 
      -0.125 * // base damage
      rock_t->getModifier(cPKV.getBase().getType(0)) *  // resistance to rock
      rock_t->getModifier(cPKV.getBase().getType(1));

    cPKV.modPercentHP(damage);

    return 1;
  }

  return 0;
};

int move_spikes_switch(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  switch (cPKV.status().nonvolatile.spikes) {
  case 3: // deal damage based on tier:
    cPKV.modPercentHP(-0.25); return 1;
  case 2:
    cPKV.modPercentHP(-0.1875); return 1;
  case 1:
    cPKV.modPercentHP(-0.125); return 1;
  default:
  case 0:
    return 0;
  }
};

int move_toxicSpikes_switch(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  if (cPKV.getStatusAilment() != AIL_NV_NONE) { return 0; }
  switch (cPKV.status().nonvolatile.toxicSpikes) {
  case 2: // inflict a type of poison based on tier:
    cPKV.setStatusAilment(AIL_NV_POISON_TOXIC); return 1;
  case 1:
    cPKV.setStatusAilment(AIL_NV_POISON); return 1;
  default:
  case 0:
    return 0;
  }
};

int move_rapidSpin(
  PkCUEngine& cu,
  MoveVolatile mV, 
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  if (&mV.getBase() != rapidSpin_t) { return 0; }

  // clear trapped:
  if (cPKV.status().cTeammate.trap < 7) { cPKV.status().cTeammate.trap = 0; }
  // clear entry hazards:
  cPKV.status().nonvolatile.toxicSpikes = 0;
  cPKV.status().nonvolatile.spikes = 0;
  cPKV.status().nonvolatile.stealthRock = 0;

  return 1;
};

int move_cureNonVolatile_team(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  const Move* tMove = &mV.getBase();
  if (
    (tMove != aromatherapy_t) && 
    (tMove != healBell_t)) { return 0; }

  // clear nonvolatile:
  TeamVolatile cTMV = cu.getTV();
  switch(cTMV.nv().getNumTeammates()) {
    case 6:
        cTMV.teammate(5).clearStatusAilment();
    case 5:
        cTMV.teammate(4).clearStatusAilment();
    case 4:
        cTMV.teammate(3).clearStatusAilment();
    case 3:
        cTMV.teammate(2).clearStatusAilment();
    case 2:
        cTMV.teammate(1).clearStatusAilment();
    case 1:
    default:
        cTMV.teammate(0).clearStatusAilment();
  };

  // clear volatile confusion:
  cTMV.status().cTeammate.confused = 0;

  return 1;
};

int move_heal50(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  const Move* tMove = &mV.getBase();
  if (
    (tMove != recover_t) && 
    (tMove != milkDrink_t) &&
    (tMove != slackOff_t) && 
    (tMove != softBoiled_t) && 
    (tMove != healOrder_t) && 
    (tMove != roost_t)) { return 0; }

  cPKV.modPercentHP(0.50);

  return 1;
};

int move_lifeLeech50(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();
  if (
    (cMove != absorb_t) && 
    (cMove != leechLife_t) &&
    (cMove != gigaDrain_t) &&
    (cMove != megaDrain_t) && 
    (cMove != drainPunch_t)) { return 0; }

  // add to hitpoints:
  cPKV.modHP(cu.getDamageComponent().damage / 2);

  return 1;
};

int move_recoil33(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();
  if (
    (cMove != doubleEdge_t) && 
    (cMove != woodHammer_t) &&
    (cMove != flareBlitz_t) &&
    (cMove != braveBird_t) && 
    (cMove != voltTackle_t)) { return 0; }

  // subtract hitpoints:
  cPKV.modHP((int32_t)cu.getDamageComponent().damage / -3);

  return cPKV.isAlive()?1:2;
};

int move_leveledDamage(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  const Type* resistedType;
  const Move* tMove = &mV.getBase();
  if (tMove == seismicToss_t) { resistedType = ghost_t; }
  else if ( tMove == nightShade_t) { resistedType = normal_t; }
  else { return 0; }

  // no damage if pokemon's class is of the resited type:
  const PokemonBase& tPKB = tPKV.getBase();
  if ((&tPKB.getType(0) == resistedType) || (&tPKB.getType(1) == resistedType)) { return 1; }

  tPKV.modHP(-1 * (int)cPKV.nv().getLevel());

  return 1;
};

int move_highCrit
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& probabilityToCrit)
{
  const Move* tMove = &mV.getBase();
  if (
    (tMove != airCutter_t) && 
    (tMove != attackOrder_t) &&
    (tMove != blazeKick_t) && 
    (tMove != crabHammer_t) && 
    (tMove != crossChop_t) && 
    (tMove != crossPoison_t) && 
    (tMove != leafBlade_t) && 
    (tMove != nightSlash_t) && 
    (tMove != psychoCut_t) && 
    (tMove != razorLeaf_t) && 
    (tMove != shadowClaw_t) && 
    (tMove != slash_t) && 
    (tMove != stoneEdge_t)) { return 0; }

  // raise move's crit stage by 1:
  probabilityToCrit = cPKV.getAccuracy_boosted(FV_CRITICALHIT, 1);

  return 1;
}

int move_suicide_modLife(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // suicide occurs regardless of hit or miss. No hasHit check.

  const Move* cMove = &mV.getBase();
  if ((cMove != explosion_t) && (cMove != selfDestruct_t) && (cMove != memento_t)) { return 0; }

  // kill pokemon:
  cPKV.setHP(0);

  // always return 2, because we killed the pokemon
  return 2;
};

int move_suicide_modPower
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& modifier)
{
  if ((&mV.getBase() != explosion_t) && (&mV.getBase() != selfDestruct_t)) { return 0; }

  modifier *= 2.0;

  return 1;
};

int move_alwaysHits
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& probabilityToHit)
{
  const Move* cMove = &mV.getBase();
  if (
    (cMove != auraSphere_t) && 
    (cMove != shockWave_t) && 
    (cMove != magnetBomb_t) &&
    (cMove != shadowPunch_t) && 
    (cMove != magicalLeaf_t) && 
    (cMove != aerialAce_t) && 
    (cMove != faintAttack_t) && 
    (cMove != swift_t) &&
    (cMove != struggle_t)) { return 0; }

  probabilityToHit = 1.0;

  // do not allow anything to affect hit chance other than this if the move always hits:
  return 2;
}

int ability_noGuard
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& probabilityToHit)
{
  bool doNoGuard = false;
  if  (
      (cPKV.nv().abilityExists() && (&(cPKV.nv().getAbility()) == noGuard_t))
      ||
      (tPKV.nv().abilityExists() && (&(tPKV.nv().getAbility()) == noGuard_t))
    ) 
  { doNoGuard = true; }

  if (!doNoGuard) { return 0; }

  probabilityToHit = 1.0;

  // do not allow anything to affect hit chance other than this if no guard occurs;
  return 2;
};

int ability_levitate
  (PkCUEngine& cu,
  const Type& cType,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& typeModifier)
{
  if (!tPKV.nv().abilityExists() || (&(tPKV.nv().getAbility()) != levitate_t)) { return 0; }

    // no effect if attack type isn't ground
  if (&cType != ground_t) { return 0; }

  // make immune to ground type attack
  typeModifier *= 0.0;

  return 1;
};

int ability_levitate_switch(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{

  if (!cPKV.nv().abilityExists() || (&(cPKV.nv().getAbility()) != levitate_t)) { return 0; }
  
  // preempt scripts which deal damage on switchin
  return 2;
};

int ability_naturalCure(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  if (!cPKV.nv().abilityExists() || (&(cPKV.nv().getAbility()) != naturalCure_t)) { return 0; }

  // clear status ailment on switchout
  cPKV.clearStatusAilment();

  return 0;
};

int ability_technician
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& basePowerModifier)
{
  if (!cPKV.nv().abilityExists() || (&(cPKV.nv().getAbility()) != technician_t)) { return 0; }

  // no effect if base power above 60
  if (cu.getDamageComponent().damage > 60) { return 0; }

  // multiply base power by 1.5
  basePowerModifier *= 1.5;

  return 1;
};

int ability_sereneGrace
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& probabilityToSecondary)
{
  if (!cPKV.nv().abilityExists() || (&(cPKV.nv().getAbility()) != sereneGrace_t)) { return 0; }

  uint8_t dType = mV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // multiply secondary probability by 2
  probabilityToSecondary *= 2.0;

  return 1;
};

int item_leftovers(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  if (cPKV.hasItem() && (&cPKV.getItem() == leftovers_t))
  {
    cPKV.modPercentHP(0.0625);
    return 1;
  }

  return 0;
};

int item_lumBerry
  (PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  // TODO(@drendleman) - why does this affect target pokemon and not current pokemon?
  PokemonVolatile tPKV = cu.getTPKV();

  // only affect targeted pokemon that have a lum berry
  if (!tPKV.hasItem() || (&tPKV.getItem() != lumBerry_t)) { return 0; }

  // only affect living pokemon
  if (!tPKV.isAlive()) { return 0; }

  bool conditionCured = false;
  // volatile status condition confusion will be cured 
  if (tPKV.status().cTeammate.confused > 0)
  {
    tPKV.status().cTeammate.confused = 0;
    conditionCured = true;
  }
  // all nonvolatile status conditions will be cured:
  else if (tPKV.getStatusAilment() != AIL_NV_NONE)
  {
    // cure status condition immediately
    tPKV.clearStatusAilment();
    conditionCured = true;
  }

  if (conditionCured) { tPKV.setNoItem(); return 1; }
  return 0;
}

int item_lifeOrb_modPower
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& modifier)
{
  if (!cPKV.hasItem() || !(&cPKV.getItem() == lifeOrb_t)) { return 0; }

  modifier *= 1.3;

  return 1;
};

int item_lifeOrb_modLife(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // must have hit, must have life orb item
  if (!cu.getBase().hasHit(cu.getICTeam()) || !cPKV.hasItem() || !(&cPKV.getItem() == lifeOrb_t)) { return 0; }

  uint8_t dType = mV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // subtract hitpoints:
  cPKV.modPercentHP(-0.1);

  return cPKV.isAlive()?1:2;
};

int item_choiceScarf_modSpeed(
    PkCUEngine& cu,
    PokemonVolatile cPKV,
    uint32_t& speed) {
  if(!cPKV.hasItem() || !(&cPKV.getItem() == choiceScarf_t)) { return 0; }

  speed = (speed * 3) / 2; // increase speed by 50%
  return 1;
}

int item_choiceItem_modPower(
    PkCUEngine& cu,
    MoveVolatile mV,
    PokemonVolatile cPKV,
    PokemonVolatile tPKV,
    fpType& modifier) {
  if (!cPKV.hasItem()) { return 0; }

  const Item* cItem = &cPKV.getItem();
  if (
    (cItem != choiceBand_t) &&
    (cItem != choiceSpecs_t)) { return 0; }

  auto damageType = mV.getBase().getDamageType();
  if ((cItem == choiceBand_t && damageType == ATK_PHYSICAL) ||
      (cItem == choiceSpecs_t && damageType == ATK_SPECIAL)) {

    modifier *= 1.5;
  }

  return 1;
};

int item_choiceItem_lockMove(
    PkCUEngine& cu,
    PokemonVolatile cPKV) {
  if (!cPKV.hasItem()) { return 0; }

  const Item* cItem = &cPKV.getItem();
  if (
    (cItem != choiceBand_t) &&
    (cItem != choiceScarf_t) &&
    (cItem != choiceSpecs_t)) { return 0; }

  // action is guaranteed to be a move action:
  size_t action_idx = (cu.getICAction() - AT_MOVE_0) + 1;
  // the pokemon may not use another move until it switches out:
  cPKV.status().cTeammate.itemScratch = action_idx;

  return 1;
}

int item_choiceItem_testLockedMove(
    ConstPokemonVolatile cPKV,
    ConstMoveVolatile mV,
    const Action& action,
    bool& moveAllowed) {
  if (!cPKV.hasItem()) { return 0; }

  const Item* cItem = &cPKV.getItem();
  if (
    (cItem != choiceBand_t) &&
    (cItem != choiceScarf_t) &&
    (cItem != choiceSpecs_t)) { return 0; }

  size_t choice_item_idx = cPKV.status().cTeammate.itemScratch;

  // if the user has not used a move with their choice item yet:
  if (choice_item_idx == 0) { return 1; }

  // else, if the choice item has chosen a move, the only acceptable move is the choice move:
  size_t action_idx = (action - AT_MOVE_0) + 1;
  moveAllowed = choice_item_idx == action_idx;
  
  return 1;
}

int engine_typeResistingBerry
  (PkCUEngine& cu,
  const Type& cType,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& typeModifier)
{
  if (!tPKV.hasItem()) { return 0; }
  const Type* resistedType = &tPKV.getItem().getResistedType();

  // no effect if attack type isn't of the resisted type
  if (&cType != resistedType) { return 0; }

  // berry reduces the damage of the attack by 50%
  typeModifier *= 0.5;
  // berry consumed after use
  tPKV.setNoItem();

  return 1;
};

int engine_typeBoostingItem
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& basePowerModifier)
{
  if (!cPKV.hasItem()) { return 0; }

  const Type* boostedType = &cPKV.getItem().getBoostedType();
  const Type* moveType = cu.getDamageComponent().mType;

  // no effect if attack type isn't of the boosted type
  if (moveType != boostedType) { return 0; }

  // multiply base power by 1.2
  basePowerModifier *= 1.2;

  return 1;
};

int engine_move_struggle(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const Move* cMove = &mV.getBase();

  if (cMove != struggle_t) { return 0; }

  // subtract hitpoints:
  cPKV.modPercentHP(-0.25);

  return cPKV.isAlive()?1:2;
};

int engine_modifyAttackPower_burn
  (PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV,
  fpType& modifier)
{
  modifier *= ((cPKV.getStatusAilment() == AIL_NV_BURN) && (mV.getBase().getDamageType() == ATK_PHYSICAL))?0.5:1.0;

  return 1;
};

int engine_onModifySpeed_paralyze
  (PkCUEngine&,
  PokemonVolatile cPKV,
  uint32_t& speed)
{
  // divide by 4 if pokemon is paralyzed
  speed /= (cPKV.getStatusAilment()==AIL_NV_PARALYSIS)?4:1;

  return 1;
};

int engine_endRoundDamageEffect(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  // nonvolatile:
  uint32_t condition = cPKV.getStatusAilment();
  if (condition == AIL_NV_POISON || condition == AIL_NV_BURN)
  {
    // reduce HP of pokemon by (1/8) or .125
    cPKV.modPercentHP(-0.125);
  }
  else if (condition == AIL_NV_POISON_TOXIC)
  {
    uint32_t toxicTier = cPKV.status().cTeammate.toxicPoison_tier;
      
    // increment toxic tier, more added damage per round
    if (toxicTier < 15)
    {
      cPKV.status().cTeammate.toxicPoison_tier++;
    }
      
    cPKV.modPercentHP(-0.0625 * (fpType)(toxicTier + 1));
  }

  // volatile: 
  // flinch only lasts for the current round. Only a pokemon moving first can flinch the other pokemon
  cPKV.status().cTeammate.flinch = 0;

  return (cPKV.isAlive()?1:2);
};

int engine_beginTurnNonvolatileEffect(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  // Does this pokemon have a non-volatile condition?
  uint32_t cStatus = cPKV.getStatusAilment();
  switch(cStatus)
  {
    case AIL_NV_FREEZE:
    {
      // generate a new environment on the result array:
      std::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.8);

      // 80% chance for frozen status effect to prevent user from moving:
      {
        // modify the status environment:
        EnvironmentPossible statEnv = cu.getStack().at(iREnv[1]);
        statEnv.setBlocked(cu.getICTeam());
      }
      // 20% chance for pokemon to not be completely frozen:
      {
        if (&cPKV.getMV(cu.getICAction()).getBase().getType() == fire_t) {
          cu.getPKV(iREnv[0]).clearStatusAilment();
        }
      }
      break;
    }
    case AIL_NV_SLEEP_4T:
    case AIL_NV_SLEEP_3T:
    case AIL_NV_SLEEP_2T:
    case AIL_NV_SLEEP_1T:
    {
      static const std::array<fpType, 4> sleepStatusProb = {{ 0.5, 1.0/3.0, 0.25, 0.0 }};
      // decrement sleep counter (no effect until next turn)
      cPKV.setStatusAilment(cPKV.getStatusAilment() - 1);

      uint32_t iSleepProb = std::min((uint32_t)4 , (uint32_t)(cStatus - AIL_NV_SLEEP_0T)) - 1;
      std::array<size_t, 2> iREnv = { cu.getIBase(), SIZE_MAX };
      if (iSleepProb != 3)
      {
        // generate a new environment on the result array:
        cu.duplicateState(iREnv, sleepStatusProb[iSleepProb]);

        // variable % chance for the pokemon to move this turn:
        cu.getPKV(iREnv[1]).clearStatusAilment();
      }
      // pokemon has a chance to move this turn:
      cu.getStack().at(iREnv[0]).setBlocked(cu.getICTeam());
      break;
    }
    case AIL_NV_PARALYSIS:
    {
      // generate a new environment on the result array:
      std::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.25);
      // 25% chance to be paralyzed and not move
      cu.getStack().at(iREnv[1]).setBlocked(cu.getICTeam());
      break;
    }
    case AIL_NV_NONE:
    default:
      // don't do anything if no status condition
      break;
  } // endOf nonVolatile switch

  return 1;
} // endOf begin turn nonvolatile effect

int engine_beginTurnVolatileEffect(
  PkCUEngine& cu,
  PokemonVolatile cPKV)
{
  // Does this pokemon have a volatile condition?
  if (cPKV.status().cTeammate.flinch > 0) {
    // set user blocked 100% of the time
    cu.getBase().setBlocked(cu.getICTeam());
  }
  if (cPKV.status().cTeammate.confused > 0) {
    uint32_t iConfused = cPKV.status().cTeammate.confused;
    if (iConfused != AIL_V_CONFUSED_0T)
    {
      // 50% chance to move:
      std::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.5);

      PokemonVolatile cConfusedPKV = cu.getPKV(iREnv[1]);

      // 50% chance to not move:
      {
        cu.getStack().at(iREnv[1]).setBlocked(cu.getICTeam());
        cConfusedPKV.status().cTeammate.confused--;
        // TODO: actual damage calculation
        cConfusedPKV.modHP(-40);
      }
      // if pokemon did not kill its self with hurt confusion:
      if (cConfusedPKV.isAlive())
      {
        uint32_t numTotalEnv = std::min((unsigned)4 , iConfused - AIL_V_CONFUSED_0T);
        uint32_t numTerminalEnv = ((iConfused - AIL_V_CONFUSED_0T)>=5)?0:1;
        fpType terminalProbability = ((fpType) numTerminalEnv) / ((fpType)numTotalEnv);

        std::array<size_t, 2> iTEnv;
      
        if ((numTerminalEnv > 0) && (numTotalEnv > numTerminalEnv)) {
          cu.duplicateState(iTEnv, terminalProbability, iREnv[1]);

          // variable % chance for this env to be the last environment confused:
          cConfusedPKV.status().cTeammate.confused = 0;
        }
      }
    }
    else /* equals AIL_V_CONFUSED_0T */
    {
      // pokemon breaks out of confusion this round
      cPKV.status().cTeammate.confused = 0;
    }
  } // end of confused

  return 1;
} // endOf begin turn volatile effect

int engine_secondaryBoostEffect(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  const Move& cMove = mV.getBase();

  // apply buffs to the current pokemon, and debuffs to the other pokemon:
  for (size_t iBuff = 0; iBuff != 9; ++iBuff) {
    cPKV.modBoost(iBuff, cMove.getSelfBuff(iBuff));
  }

  // all other effects modify the target pokemon, and we don't want to modify a dead one
  //  (this will stop all other plugins from running as well)
  if (!tPKV.isAlive()) { return 2; }

  for (size_t iBuff = 0; iBuff != 9; ++iBuff)
  {
    tPKV.modBoost(iBuff, -1 * cMove.getTargetDebuff(iBuff));
  }

  return 1;
} //endOf apply buffs / debuffs

int engine_secondaryNonvolatileEffect(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  const Move& cMove = mV.getBase();
  
  if (tPKV.getStatusAilment() != AIL_NV_NONE) { return 0; }

  // apply status conditions to the other pokemon:
  switch(cMove.getTargetAilment())
  {
    case AIL_NV_SLEEP:
    case AIL_NV_POISON_TOXIC:
    case AIL_NV_FREEZE:
    case AIL_NV_BURN:
    case AIL_NV_PARALYSIS:
    case AIL_NV_POISON:
    default:
      // reset toxic tier:
      tPKV.status().cTeammate.toxicPoison_tier = 0;
      // apply generic status condition
      tPKV.setStatusAilment(cMove.getTargetAilment());
      // implicitly push back bEnv status condition (already on array)
      break;
    case AIL_NV_NONE:
      break; // do not apply a status condition, or push anything back
  } // end of targetAilment switch
  return 1;
} //endOf apply buffs / debuffs

int engine_secondaryVolatileEffect(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // apply volatile status conditions to the other pokemon:
  switch(mV.getBase().getTargetVolatileAilment())
  {
    case AIL_V_CONFUSED:
      // confused for (at most) 5 turns, and (at least) 2 turns:
      tPKV.status().cTeammate.confused = AIL_V_CONFUSED_5T;
      // implicitly push back bEnv
      break;
    case AIL_V_FLINCH:
      tPKV.status().cTeammate.flinch = 1;
    case AIL_V_INFATUATED:
    default:
    case AIL_V_NONE:
      break; // do not apply a status condition, or push anything back
  } //endOf targetVolatileAilment switch
  return 1;
} //endOf apply buffs / debuffs

int engine_decrementPP(
  PkCUEngine& cu,
  MoveVolatile mV,
  PokemonVolatile cPKV,
  PokemonVolatile tPKV)
{
  // don't decrement PP if this move is struggle_t or the move did not hit
  if (!cu.getBase().hasHit(cu.getICTeam()) || (&mV.getBase() == struggle_t)) { return 0; }

  mV.modPP(-1);

  return 1;
};

/*int engine_endRoundWeatherDecrementEffect(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  //if (cu.getICTeam() != TEAM_B) { return 0; } // no effect for first team
  // nonvolatile:
  nonvolatileStatus& cNV = cu.getBase().getEnv().getTeam(TEAM_A).getNonVolatile();
  uint32_t weatherCondition = cNV.weather_type;
  if (weatherCondition == WEATHER_NORMAL) { return 0; } // end early if no weather effect exists
  uint32_t weatherDuration = cTMV.getNonVolatile().weather_duration;

  // decrement weather effect:
  if (weatherDuration == 0) { weatherCondition = WEATHER_NORMAL; }
  if (weatherDuration <= 6) { --weatherDuration; }

  cNV.weather_duration = weatherDuration;
  cNV.weather_type = weatherCondition;

  return 1;
};

int engine_endRoundWeatherDamageEffect(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  // nonvolatile:
  nonvolatileStatus& cNV = cu.getBase().getEnv().getTeam(TEAM_A).getNonVolatile();
  uint32_t weatherCondition = cNV.weather_type;
  
  // nasty hack: if second team, perform end of round decrement effects
  if (cu.getICTeam() == TEAM_B) { engine_endRoundWeatherDecrementEffect(cu, cPKNV, cTMV, cPKV); }

  std::array<const type*, 2> cTypes = 
  {{ 
    &cPKNV.getBase().getType(0),
    &cPKNV.getBase().getType(1)
  }};

  switch(weatherCondition)
  {
  case WEATHER_HAIL:
    if (cTypes[0] == ice_t || cTypes[1] == ice_t) { return 0; }
    break;
  case WEATHER_SAND:
    if (cTypes[0] == rock_t || cTypes[1] == rock_t ||
      cTypes[0] == steel_t || cTypes[1] == steel_t ||
      cTypes[0] == ground_t || cTypes[1] == ground_t) { return 0; }
    break;
  default:
    return 0;
  }

  // damage pokemon:
  cTMV.cModPercentHP(cPKNV, -0.0625);

  return cPKV.isAlive()?1:2;
};*/





bool registerExtensions(const Pokedex& pkAI, std::vector<plugin>& extensions)
{
  // register needed types:
  dex = &pkAI;
  //moves:
  const Moves& moves = dex->getMoves();
  absorb_t = orphan::orphanCheck(moves, "absorb");
  aerialAce_t = orphan::orphanCheck(moves, "aerial ace");
  airCutter_t = orphan::orphanCheck(moves, "air cutter");
  aromatherapy_t = orphan::orphanCheck(moves, "aromatherapy");
  attackOrder_t = orphan::orphanCheck(moves, "attack order");
  auraSphere_t = orphan::orphanCheck(moves, "aura sphere");
  blazeKick_t = orphan::orphanCheck(moves, "blaze kick");
  braveBird_t = orphan::orphanCheck(moves, "brave bird");
  crabHammer_t = orphan::orphanCheck(moves, "crabhammer");
  crossChop_t = orphan::orphanCheck(moves, "cross chop");
  crossPoison_t = orphan::orphanCheck(moves, "cross poison");
  doubleEdge_t = orphan::orphanCheck(moves, "double-edge");
  drainPunch_t = orphan::orphanCheck(moves, "drain punch");
  explosion_t = orphan::orphanCheck(moves, "explosion");
  faintAttack_t = orphan::orphanCheck(moves, "faint attack");
  flareBlitz_t = orphan::orphanCheck(moves, "flare blitz");
  gigaDrain_t = orphan::orphanCheck(moves, "giga drain");
  healBell_t = orphan::orphanCheck(moves, "heal bell");
  healOrder_t = orphan::orphanCheck(moves, "heal order");
  hiddenPower_t = orphan::orphanCheck(moves, "hidden power");
  leafBlade_t = orphan::orphanCheck(moves, "leaf blade");
  leechLife_t = orphan::orphanCheck(moves, "leech life");
  magicalLeaf_t = orphan::orphanCheck(moves, "magical leaf");
  magnetBomb_t = orphan::orphanCheck(moves, "magnet bomb");
  megaDrain_t = orphan::orphanCheck(moves, "mega drain");
  memento_t = orphan::orphanCheck(moves, "memento");
  milkDrink_t = orphan::orphanCheck(moves, "milk drink");
  nightShade_t = orphan::orphanCheck(moves, "night shade");
  nightSlash_t = orphan::orphanCheck(moves, "night slash");
  painSplit_t = orphan::orphanCheck(moves, "pain split");
  psychoCut_t = orphan::orphanCheck(moves, "psycho cut");
  rapidSpin_t = orphan::orphanCheck(moves, "rapid spin");
  razorLeaf_t = orphan::orphanCheck(moves, "razor leaf");
  roost_t = orphan::orphanCheck(moves, "roost");
  seismicToss_t = orphan::orphanCheck(moves, "seismic toss");
  selfDestruct_t = orphan::orphanCheck(moves, "selfdestruct");
  shadowClaw_t = orphan::orphanCheck(moves, "shadow claw");
  shadowPunch_t = orphan::orphanCheck(moves, "shadow punch");
  shockWave_t = orphan::orphanCheck(moves, "shock wave");
  slackOff_t = orphan::orphanCheck(moves, "slack off");
  slash_t = orphan::orphanCheck(moves, "slash");
  spikes_t = orphan::orphanCheck(moves, "spikes");
  softBoiled_t = orphan::orphanCheck(moves, "softboiled");
  stealthRock_t = orphan::orphanCheck(moves, "stealth rock");
  stoneEdge_t = orphan::orphanCheck(moves, "stone edge");
  struggle_t = orphan::orphanCheck(moves, "struggle");
  swift_t = orphan::orphanCheck(moves, "swift");
  toxicSpikes_t = orphan::orphanCheck(moves, "toxic spikes");
  voltTackle_t = orphan::orphanCheck(moves, "volt tackle");
  woodHammer_t = orphan::orphanCheck(moves, "wood hammer");
  //items:
  const Items& items = dex->getItems();
  choiceBand_t = orphan::orphanCheck(items, "choice band");
  choiceScarf_t = orphan::orphanCheck(items, "choice scarf");
  choiceSpecs_t = orphan::orphanCheck(items, "choice specs");
  leftovers_t = orphan::orphanCheck(items, "leftovers");
  lifeOrb_t = orphan::orphanCheck(items, "life orb");
  lumBerry_t = orphan::orphanCheck(items, "lum berry");
  //abilities:
  const Abilities& abilities = dex->getAbilities();
  levitate_t = orphan::orphanCheck(abilities, "levitate");
  naturalCure_t = orphan::orphanCheck(abilities, "natural cure");
  noGuard_t = orphan::orphanCheck(abilities, "no guard");
  technician_t = orphan::orphanCheck(abilities, "technician");
  sereneGrace_t = orphan::orphanCheck(abilities, "serene grace");
  //types:
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
  dark_t = orphan::orphanCheck(types, "dark");

  // move effects:
  extensions.push_back(plugin(MOVE_PLUGIN, "absorb", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "aerial ace", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "air cutter", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "aromatherapy", PLUGIN_ON_EVALUATEMOVE, move_cureNonVolatile_team, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "attack order", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "aura sphere", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "blaze kick", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "brave bird", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "crabhammer", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "cross chop", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "cross poison", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "drain punch", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "double-edge", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "explosion", PLUGIN_ON_MODIFYATTACKPOWER, move_suicide_modPower, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "explosion", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "faint attack", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "flare blitz", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "giga drain", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "heal bell", PLUGIN_ON_EVALUATEMOVE, move_cureNonVolatile_team, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "heal order", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "hidden power", PLUGIN_ON_INIT, move_hiddenPower_calculate, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "hidden power", PLUGIN_ON_SETBASEPOWER, move_hiddenPower_setPower, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "hidden power", PLUGIN_ON_SETMOVETYPE, move_hiddenPower_setType, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "leaf blade", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "leech life", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "magical leaf", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "magnet bomb", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "mega drain", PLUGIN_ON_ENDOFMOVE, move_lifeLeech50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "memento", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "milk drink", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "night shade", PLUGIN_ON_EVALUATEMOVE, move_leveledDamage, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "night slash", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "pain split", PLUGIN_ON_EVALUATEMOVE, move_painSplit, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "psycho cut", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "rapid spin", PLUGIN_ON_ENDOFMOVE, move_rapidSpin, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "razor leaf", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "recover", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "roost", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "seismic toss", PLUGIN_ON_EVALUATEMOVE, move_leveledDamage, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "selfdestruct", PLUGIN_ON_MODIFYATTACKPOWER, move_suicide_modPower, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "selfdestruct", PLUGIN_ON_ENDOFMOVE, move_suicide_modLife, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "shadow claw", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "shadow punch", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "shock wave", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "slack off", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "slash", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "softboiled", PLUGIN_ON_EVALUATEMOVE, move_heal50, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "spikes", PLUGIN_ON_SWITCHIN, move_spikes_switch, 2, 2));
  extensions.push_back(plugin(MOVE_PLUGIN, "spikes", PLUGIN_ON_EVALUATEMOVE, move_spikes_set, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "stealth rock", PLUGIN_ON_SWITCHIN, move_stealthRock_switch, 0, 2));
  extensions.push_back(plugin(MOVE_PLUGIN, "stealth rock", PLUGIN_ON_EVALUATEMOVE, move_stealthRock_set, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "stone edge", PLUGIN_ON_MODIFYCRITPROBABILITY, move_highCrit, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "swift", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "toxic spikes", PLUGIN_ON_SWITCHIN, move_toxicSpikes_switch, 2, 2));
  extensions.push_back(plugin(MOVE_PLUGIN, "toxic spikes", PLUGIN_ON_EVALUATEMOVE, move_toxicSpikes_set, 0, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "wood hammer", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, 0));
  extensions.push_back(plugin(MOVE_PLUGIN, "volt tackle", PLUGIN_ON_ENDOFMOVE, move_recoil33, -1, 0));
  // item effects:
  extensions.push_back(plugin(ITEM_PLUGIN, "choice band", PLUGIN_ON_BEGINNINGOFTURN, item_choiceItem_lockMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice band", PLUGIN_ON_TESTMOVE, item_choiceItem_testLockedMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice band", PLUGIN_ON_MODIFYRAWDAMAGE, item_choiceItem_modPower, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice scarf", PLUGIN_ON_BEGINNINGOFTURN, item_choiceItem_lockMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice scarf", PLUGIN_ON_TESTMOVE, item_choiceItem_testLockedMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice scarf", PLUGIN_ON_MODIFYSPEED, item_choiceScarf_modSpeed, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice specs", PLUGIN_ON_BEGINNINGOFTURN, item_choiceItem_lockMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice specs", PLUGIN_ON_TESTMOVE, item_choiceItem_testLockedMove, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "choice specs", PLUGIN_ON_MODIFYRAWDAMAGE, item_choiceItem_modPower, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "leftovers", PLUGIN_ON_ENDOFROUND, item_leftovers, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "life orb", PLUGIN_ON_MODIFYRAWDAMAGE, item_lifeOrb_modPower, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "life orb", PLUGIN_ON_ENDOFMOVE, item_lifeOrb_modLife, 0, 0));
  extensions.push_back(plugin(ITEM_PLUGIN, "lum berry", PLUGIN_ON_ENDOFTURN, item_lumBerry, 0, 1));
  // ability effects:
  extensions.push_back(plugin(ABILITY_PLUGIN, "natural cure", PLUGIN_ON_SWITCHOUT, ability_naturalCure, 0, 0));
  extensions.push_back(plugin(ABILITY_PLUGIN, "no guard", PLUGIN_ON_MODIFYHITPROBABILITY, ability_noGuard, -2, 2));
  extensions.push_back(plugin(ABILITY_PLUGIN, "levitate", PLUGIN_ON_SETDEFENSETYPE, ability_levitate, -1, 1));
  extensions.push_back(plugin(ABILITY_PLUGIN, "levitate", PLUGIN_ON_SWITCHIN, ability_levitate_switch, 1, 0));
  extensions.push_back(plugin(ABILITY_PLUGIN, "technician", PLUGIN_ON_MODIFYBASEPOWER, ability_technician, -1, 0));
  extensions.push_back(plugin(ABILITY_PLUGIN, "serene grace", PLUGIN_ON_MODIFYSECONDARYPROBABILITY, ability_sereneGrace, -1, 0));

  // engine effects:
  extensions.push_back(plugin(ENGINE_PLUGIN, "type boosting item effect", PLUGIN_ON_MODIFYBASEPOWER, engine_typeBoostingItem, 0, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "type resisting berry effect", PLUGIN_ON_MODIFYITEMPOWER, engine_typeResistingBerry, 0, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "move PP decrement effect", PLUGIN_ON_ENDOFMOVE, engine_decrementPP, 0, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "struggle damage effect", PLUGIN_ON_ENDOFMOVE, engine_move_struggle, 0, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "struggle always hits effect", PLUGIN_ON_MODIFYHITPROBABILITY, move_alwaysHits, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile speed change", PLUGIN_ON_MODIFYSPEED, engine_onModifySpeed_paralyze, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile beginning-of-round damage", PLUGIN_ON_BEGINNINGOFTURN, engine_beginTurnNonvolatileEffect, -2, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "volatile beginning-of-round damage", PLUGIN_ON_BEGINNINGOFTURN, engine_beginTurnVolatileEffect, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect boosts", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryBoostEffect, -3, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect nonvolatile", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryNonvolatileEffect, -2, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect volatile", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryVolatileEffect, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile end-of-round damage", PLUGIN_ON_ENDOFROUND, engine_endRoundDamageEffect, -1, 2));

  return true;
}
