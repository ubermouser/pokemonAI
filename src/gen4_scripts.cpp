//#define PKAI_STATIC
#include "../inc/pkai.h"
//#undef PKAI_STATIC

#include <stdint.h>
#include <vector>
#include <algorithm>

//#define PKAI_IMPORT
#include "../inc/orphan.h"
#include "../inc/type.h"
#include "../inc/move.h"
#include "../inc/move_volatile.h"
#include "../inc/move_nonvolatile.h"
#include "../inc/pokemon_volatile.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_volatile.h"
#include "../inc/environment_possible.h"
#include "../inc/pkCU.h"
#include "../inc/pokedex.h"
//#undef PKAI_IMPORT

//#define PKAI_EXPORT
#include "../inc/plugin.h"
//#undef PKAI_EXPORT

#ifndef GEN4_SCRIPTS_STATIC
const pokedex* pkdex;
#endif

static const move* airCutter_t;
static const move* absorb_t;
static const move* aerialAce_t;
static const move* attackOrder_t;
static const move* aromatherapy_t;
static const move* auraSphere_t;
static const move* blazeKick_t;
static const move* braveBird_t;
static const move* crabHammer_t;
static const move* crossChop_t;
static const move* crossPoison_t;
static const move* doubleEdge_t;
static const move* drainPunch_t;
static const move* explosion_t;
static const move* faintAttack_t;
static const move* flareBlitz_t;
static const move* gigaDrain_t;
static const move* healBell_t;
static const move* healOrder_t;
static const move* hiddenPower_t;
static const move* leafBlade_t;
static const move* leechLife_t;
static const move* magnetBomb_t;
static const move* magicalLeaf_t;
static const move* megaDrain_t;
static const move* memento_t;
static const move* milkDrink_t;
static const move* nightShade_t;
static const move* nightSlash_t;
static const move* painSplit_t;
static const move* psychoCut_t;
static const move* rapidSpin_t;
static const move* razorLeaf_t;
static const move* recover_t;
static const move* roost_t;
static const move* shadowClaw_t;
static const move* shadowPunch_t;
static const move* shockWave_t;
static const move* seismicToss_t;
static const move* selfDestruct_t;
static const move* softBoiled_t;
static const move* slackOff_t;
static const move* slash_t;
static const move* spikes_t;
static const move* stealthRock_t;
static const move* stoneEdge_t;
static const move* struggle_t;
static const move* swift_t;
static const move* toxicSpikes_t;
static const move* voltTackle_t;
static const move* woodHammer_t;

static const item* leftovers_t;
static const item* lifeOrb_t;
static const item* lumBerry_t;

static const ability* levitate_t;
static const ability* naturalCure_t;
static const ability* noGuard_t;
static const ability* technician_t;
static const ability* sereneGrace_t;

static const type* normal_t;
static const type* fighting_t;
static const type* flying_t;
static const type* poison_t;
static const type* ground_t;
static const type* rock_t;
static const type* bug_t;
static const type* ghost_t;
static const type* steel_t;
static const type* fire_t;
static const type* water_t;
static const type* grass_t;
static const type* electric_t;
static const type* psychic_t;
static const type* ice_t;
static const type* dragon_t;
static const type* dark_t;

int move_hiddenPower_calculate(
  pokemon_nonvolatile& cPKNV,
  move_nonvolatile& cMNV)
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
  const type* front = &pkdex->getTypes().front();
  switch(cType)
  {
  case 0:
    cType = (uint16_t)(fighting_t - front); break;
  case 1:
    cType = (uint16_t)(flying_t - front); break;
  case 2:
    cType = (uint16_t)(poison_t - front); break;
  case 3:
    cType = (uint16_t)(ground_t - front); break;
  case 4:
    cType = (uint16_t)(rock_t - front); break;
  case 5:
    cType = (uint16_t)(bug_t - front); break;
  case 6:
    cType = (uint16_t)(ghost_t - front); break;
  case 7:
    cType = (uint16_t)(steel_t - front); break;
  case 8:
    cType = (uint16_t)(fire_t - front); break;
  case 9:
    cType = (uint16_t)(water_t - front); break;
  case 10:
    cType = (uint16_t)(grass_t - front); break;
  case 11:
    cType = (uint16_t)(electric_t - front); break;
  case 12:
    cType = (uint16_t)(psychic_t - front); break;
  case 13:
    cType = (uint16_t)(ice_t - front); break;
  case 14:
    cType = (uint16_t)(dragon_t - front); break;
  default:
  case 15:
    cType = (uint16_t)(dark_t - front); break;
  };

  assert((cType < pkdex->getTypes().size()) && cPower <= 70);

  cMNV.setScriptVal_a(cType);
  cMNV.setScriptVal_b(cPower);

  return 2;
};

int move_hiddenPower_setType(
  pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  const type*& cType)
{
  if (&mNV.getBase() != hiddenPower_t) { return 0; }

  // pointer arithmetic
  cType = &pkdex->getTypes().front() + mNV.getScriptVal_a();
  return 1;
};

int move_hiddenPower_setPower(
  pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  uint32_t& basePower)
{
  if (&mNV.getBase() != hiddenPower_t) { return 0; }

  basePower = mNV.getScriptVal_b();
  return 1;
}

int move_painSplit(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != painSplit_t) { return 0; }

  // calculate how much health total pokemon both have; average the two, rounding down
  uint32_t newHP = (cPKV.getHP() + tPKV.getHP() + 1) / 2;

  cTMV.cSetHP(cPKNV, newHP);
  tTMV.cSetHP(tPKNV, newHP);

  return 1;
};

int move_stealthRock_set(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != stealthRock_t) { return 0; }

  tTMV.getNonVolatile().stealthRock = 1;

  return 1;
};

int move_spikes_set(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != spikes_t) { return 0; }

  tTMV.getNonVolatile().spikes = std::min(3U, tTMV.getNonVolatile().spikes + 1U);

  return 1;
};

int move_toxicSpikes_set(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != toxicSpikes_t) { return 0; }

  tTMV.getNonVolatile().toxicSpikes = std::min(2U, tTMV.getNonVolatile().toxicSpikes + 1U);

  return 1;
};

int move_stealthRock_switch(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV, 
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  if (cTMV.getNonVolatile().stealthRock > 0)
  {
    // deal damage:
    fpType damage = 
      -0.125 * // base damage
      rock_t->getModifier(cPKNV.getBase().getType(0)) *  // resistance to rock
      rock_t->getModifier(cPKNV.getBase().getType(1));

    cTMV.cModPercentHP(cPKNV, damage);

    return 1;
  }

  return 0;
};

int move_spikes_switch(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV, 
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  switch (cTMV.getNonVolatile().spikes)
  {
  case 3: // deal damage based on tier:
    cTMV.cModPercentHP(cPKNV, -0.25); return 1;
  case 2:
    cTMV.cModPercentHP(cPKNV, -0.1875); return 1;
  case 1:
    cTMV.cModPercentHP(cPKNV, -0.125); return 1;
  default:
  case 0:
    return 0;
  }
};

int move_toxicSpikes_switch(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV, 
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  if (cPKV.getStatusAilment() != AIL_NV_NONE) { return 0; }
  switch (cTMV.getNonVolatile().toxicSpikes)
  {
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
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != rapidSpin_t) { return 0; }

  // clear trapped:
  if (cTMV.getVolatile().trap < 7) { cTMV.getVolatile().trap = 0; }
  // clear entry hazards:
  cTMV.getNonVolatile().toxicSpikes = 0;
  cTMV.getNonVolatile().spikes = 0;
  cTMV.getNonVolatile().stealthRock = 0;

  return 1;
};

int move_cureNonVolatile_team(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (&mNV.getBase() != aromatherapy_t) { return 0; }
  const move* tMove = &mNV.getBase();
  if (
    (tMove != aromatherapy_t) && 
    (tMove != healBell_t)) { return 0; }

  // clear nonvolatile:
  cTMV.teammate(0).clearStatusAilment();
  cTMV.teammate(1).clearStatusAilment();
  cTMV.teammate(2).clearStatusAilment();
  cTMV.teammate(3).clearStatusAilment();
  cTMV.teammate(4).clearStatusAilment();
  cTMV.teammate(5).clearStatusAilment();

  // clear volatile confusion:
  cTMV.getVolatile().confused = 0;

  return 1;
};

int move_heal50(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  const move* tMove = &mNV.getBase();
  if (
    (tMove != recover_t) && 
    (tMove != milkDrink_t) &&
    (tMove != slackOff_t) && 
    (tMove != softBoiled_t) && 
    (tMove != healOrder_t) && 
    (tMove != roost_t)) { return 0; }

  cTMV.cModPercentHP(cPKNV, 0.50);

  return 1;
};

int move_lifeLeech50(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const move* cMove = &mNV.getBase();
  if (
    (cMove != absorb_t) && 
    (cMove != leechLife_t) &&
    (cMove != gigaDrain_t) &&
    (cMove != megaDrain_t) && 
    (cMove != drainPunch_t)) { return 0; }

  // add to hitpoints:
  cTMV.cModHP(cPKNV, (cu.getDamageComponent().damage / 2));

  return 1;
};

int move_recoil33(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const move* cMove = &mNV.getBase();
  if (
    (cMove != doubleEdge_t) && 
    (cMove != woodHammer_t) &&
    (cMove != flareBlitz_t) &&
    (cMove != braveBird_t) && 
    (cMove != voltTackle_t)) { return 0; }

  // subtract hitpoints:
  cTMV.cModHP(cPKNV, ((int32_t)cu.getDamageComponent().damage / -3));

  return cPKV.isAlive()?1:2;
};

int move_leveledDamage(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  const type* resistedType;
  const move* tMove = &mNV.getBase();
  if (tMove == seismicToss_t) { resistedType = ghost_t; }
  else if ( tMove == nightShade_t) { resistedType = normal_t; }
  else { return 0; }

  // no damage if pokemon's class is of the resited type:
  const pokemon_base& tPKB = tPKNV.getBase();
  if ((&tPKB.getType(0) == resistedType) || (&tPKB.getType(1) == resistedType)) { return 1; }

  tTMV.cModHP(tPKNV, -1 * (int)cPKNV.getLevel());

  return 1;
};

int move_highCrit
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& probabilityToCrit)
{
  const move* tMove = &mNV.getBase();
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
  probabilityToCrit = cTMV.cGetAccuracy_boosted(FV_CRITICALHIT, 1);

  return 1;
}

int move_suicide_modLife(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  // occurs regardless of hit or miss:
  //if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const move* cMove = &mNV.getBase();

  if ((cMove != explosion_t) && (cMove != selfDestruct_t) && (cMove != memento_t)) { return 0; }

  // kill pokemon:
  cTMV.cSetHP(cPKNV, 0);

  // always return 2, because we killed the pokemon
  return 2;
};

int move_suicide_modPower
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& modifier)
{
  if ((&mNV.getBase() != explosion_t) && (&mNV.getBase() != selfDestruct_t)) { return 0; }

  modifier *= 2.0;

  return 1;
};

int move_alwaysHits
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& probabilityToHit)
{
  const move* cMove = &mNV.getBase();
  if (
    (cMove != auraSphere_t) && 
    (cMove != shockWave_t) && 
    (cMove != magnetBomb_t) &&
    (cMove != shadowPunch_t) && 
    (cMove != magicalLeaf_t) && 
    (cMove != aerialAce_t) && 
    (cMove != faintAttack_t) && 
    (cMove != swift_t)) { return 0; }

  probabilityToHit = 1.0;

  // do not allow anything to affect hit chance other than this if no guard occurs;
  return 2;
}

int ability_noGuard
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& probabilityToHit)
{
  bool doNoGuard = false;
  if  (
      (cPKNV.abilityExists() && (&cPKNV.getAbility() == noGuard_t))
      ||
      (tPKNV.abilityExists() && (&tPKNV.getAbility() == noGuard_t))
    ) 
  { doNoGuard = true; }

  if (!doNoGuard) { return 0; }

  probabilityToHit = 1.0;

  // do not allow anything to affect hit chance other than this if no guard occurs;
  return 2;
};

int ability_levitate
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  const type& cType,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& typeModifier)
{
  if (!tPKNV.abilityExists() || (&tPKNV.getAbility() != levitate_t)) { return 0; }

    // no effect if attack type isn't ground
  if (&cType != ground_t) { return 0; }

  // make immune to ground type attack
  typeModifier *= 0.0;

  return 1;
};

int ability_levitate_switch(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV, 
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{

  if (!cPKNV.abilityExists() || (&cPKNV.getAbility() != levitate_t)) { return 0; }
  
  // preempt scripts which deal damage on switchin
  return 2;
};

int ability_naturalCure(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV, 
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  if (!cPKNV.abilityExists() || (&cPKNV.getAbility() != naturalCure_t)) { return 0; }

  // clear status ailment on switchout
  cPKV.clearStatusAilment();

  return 0;
};

int ability_technician
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& basePowerModifier)
{
  if (!cPKNV.abilityExists() || (&cPKNV.getAbility() != technician_t)) { return 0; }

  // no effect if base power above 60
  if (cu.getDamageComponent().damage > 60) { return 0; }

  // multiply base power by 1.5
  basePowerModifier *= 1.5;

  return 1;
};

int ability_sereneGrace
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& probabilityToSecondary)
{
  if (!cPKNV.abilityExists() || (&cPKNV.getAbility() != sereneGrace_t)) { return 0; }

  uint8_t dType = mNV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // multiply secondary probability by 2
  probabilityToSecondary *= 2.0;

  return 1;
};

int item_leftovers(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  if (cPKV.hasItem(cPKNV) && (&cPKV.getItem(cPKNV) == leftovers_t))
  {
    cTMV.cModPercentHP(cPKNV, 0.0625);
    return 1;
  }

  return 0;
};

int item_lumBerry
  (pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{

  const pokemon_nonvolatile& tPKNV = cu.getTPKNV();
  team_volatile& tTMV = cu.getTMV();
  pokemon_volatile& tPKV = cu.getTPKV();

  // only affect targeted pokemon that have a lum berry
  if (!tPKV.hasItem(tPKNV) || (&tPKV.getItem(tPKNV) != lumBerry_t)) { return 0; }

  // only affect living pokemon
  if (!tPKV.isAlive()) { return 0; }

  bool conditionCured = false;
  // volatile status condition confusion will be cured 
  if (tTMV.getVolatile().confused > 0)
  {
    tTMV.getVolatile().confused = 0;
    conditionCured = true;
  }
  // all nonvolatile status conditions will be cured:
  else if (tPKV.getStatusAilment() != AIL_NV_NONE)
  {
    // cure status condition immediately
    tPKV.clearStatusAilment();
    conditionCured = true;
  }

  if (conditionCured) { tPKV.setNoItem(tPKNV); return 1; }
  return 0;
}

int item_lifeOrb_modPower
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& modifier)
{
  if (!cPKV.hasItem(cPKNV) || !(&cPKV.getItem(cPKNV) == lifeOrb_t)) { return 0; }

  modifier *= 1.3;

  return 1;
};

int item_lifeOrb_modLife(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  // must have hit, must have life orb item
  if (!cu.getBase().hasHit(cu.getICTeam()) || !cPKV.hasItem(cPKNV) || !(&cPKV.getItem(cPKNV) == lifeOrb_t)) { return 0; }

  uint8_t dType = mNV.getBase().getDamageType();
  // must have used a physical or special attack move
  if (!((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL))) { return 0; }

  // subtract hitpoints:
  cTMV.cModPercentHP(cPKNV, -0.1);

  return cPKV.isAlive()?1:2;
};

int engine_typeResistingBerry
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  const type& cType,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& typeModifier)
{
  if (!tPKV.hasItem(tPKNV)) { return 0; }
  const type* resistedType = &tPKV.getItem(tPKNV).getResistedType();

  // no effect if attack type isn't of the resisted type
  if (&cType != resistedType) { return 0; }

  // berry reduces the damage of the attack by 50%
  typeModifier *= 0.5;
  // berry consumed after use
  tPKV.setNoItem(tPKNV);

  return 1;
};

int engine_typeBoostingItem
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& basePowerModifier)
{
  if (!cPKV.hasItem(cPKNV)) { return 0; }

  const type* boostedType = &cPKV.getItem(cPKNV).getBoostedType();
  const type* moveType = cu.getDamageComponent().mType;

  // no effect if attack type isn't of the boosted type
  if (moveType != boostedType) { return 0; }

  // multiply base power by 1.2
  basePowerModifier *= 1.2;

  return 1;
};

int engine_move_struggle(
  pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  // this plugin_t only triggered if primary has hit
  if (!cu.getBase().hasHit(cu.getICTeam())) { return 0; }

  const move* cMove = &mNV.getBase();

  if (cMove != struggle_t) { return 0; }

  // subtract hitpoints:
  cTMV.cModPercentHP(cPKNV, -0.25);

  return cPKV.isAlive()?1:2;
};

int engine_move_struggle_alwaysHits
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& probabilityToHit)
{
  const move* cMove = &mNV.getBase();
  if (cMove != struggle_t) { return 0; }

  probabilityToHit = 1.0;

  // do not allow anything to affect hit chance other than this if no guard occurs;
  return 2;
}

int engine_modifyAttackPower_burn
  (pkCU& cu,
  const move_nonvolatile& mNV,
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV,
  fpType& modifier)
{
  modifier *= ((cPKV.getStatusAilment() == AIL_NV_BURN) && (mNV.getBase().getDamageType() == ATK_PHYSICAL))?0.5:1.0;

  return 1;
};

int engine_onModifySpeed_paralyze
  (pkCU&,
  const pokemon_nonvolatile& cPKNV,
  const team_volatile& cTMV,
  const pokemon_volatile& cPKV,
  uint32_t& speed)
{
  // divide by 4 if pokemon is paralyzed
  speed /= (cPKV.getStatusAilment()==AIL_NV_PARALYSIS)?4:1;

  return 1;
};

int engine_endRoundDamageEffect(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  // nonvolatile:
  uint32_t condition = cPKV.getStatusAilment();
  if (condition == AIL_NV_POISON || condition == AIL_NV_BURN)
  {
    // reduce HP of pokemon by (1/8) or .125
    cTMV.cModPercentHP(cPKNV, -0.125);
  }
  else if (condition == AIL_NV_POISON_TOXIC)
  {
    uint32_t toxicTier = cTMV.getVolatile().toxicPoison_tier;
      
    // increment toxic tier, more added damage per round
    if (toxicTier < 15)
    {
      cTMV.getVolatile().toxicPoison_tier++;
    }
      
    cTMV.cModPercentHP(cPKNV, -0.0625 * (fpType)(toxicTier + 1));
  }

  // volatile: 
  // flinch only lasts for the current round. Only a pokemon moving first can flinch the other pokemon
  cTMV.getVolatile().flinch = 0;

  return (cPKV.isAlive()?1:2);
};

int engine_beginTurnNonvolatileEffect(
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  // Does this pokemon have a non-volatile condition?
  uint32_t cStatus = cPKV.getStatusAilment();
  switch(cStatus)
  {
    case AIL_NV_FREEZE:
    {
      // generate a new environment on the result array:
      boost::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.8);

      // 80% chance for frozen status effect to prevent user from moving:
      {
        // modify the status environment:
        environment_possible& statEnv = cu.getStack()[iREnv[1]];
        statEnv.setBlocked(cu.getICTeam());
      }
      // 20% chance for pokemon to not be completely frozen:
      {
        if (&cPKNV.getMove_base(cu.getICAction()).getType() == fire_t)
        {
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
      static const boost::array<fpType, 4> sleepStatusProb = {{ 0.5, 1.0/3.0, 0.25, 0.0 }};
      // decrement sleep counter (no effect until next turn)
      cPKV.setStatusAilment(cPKV.getStatusAilment() - 1);

      uint32_t iSleepProb = std::min((uint32_t)4 , (uint32_t)(cStatus - AIL_NV_SLEEP_0T)) - 1;
      boost::array<size_t, 2> iREnv = { cu.getIBase(), SIZE_MAX };
      if (iSleepProb != 3)
      {
        // generate a new environment on the result array:
        cu.duplicateState(iREnv, sleepStatusProb[iSleepProb]);

        // variable % chance for the pokemon to move this turn:
        cu.getPKV(iREnv[1]).clearStatusAilment();
      }
      // pokemon has a chance to move this turn:
      cu.getStack()[iREnv[0]].setBlocked(cu.getICTeam());
      break;
    }
    case AIL_NV_PARALYSIS:
    {
      // generate a new environment on the result array:
      boost::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.25);
      // 25% chance to be paralyzed and not move
      cu.getStack()[iREnv[1]].setBlocked(cu.getICTeam());
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
  pkCU& cu,
  const pokemon_nonvolatile& cPKNV,
  team_volatile& cTMV,
  pokemon_volatile& cPKV)
{
  // Does this pokemon have a volatile condition?
  if (cTMV.getVolatile().flinch > 0)
  {
    // set user blocked 100% of the time
    cu.getBase().setBlocked(cu.getICTeam());
  }
  if (cTMV.getVolatile().confused > 0)
  {
    uint32_t iConfused = cTMV.getVolatile().confused;
    if (iConfused != AIL_V_CONFUSED_0T)
    {
      // 50% chance to move:
      boost::array<size_t, 2> iREnv;
      cu.duplicateState(iREnv, 0.5);

      // 50% chance to not move:
      {
        cu.getStack()[iREnv[1]].setBlocked(cu.getICTeam());
        cu.getTMV(iREnv[1]).getVolatile().confused--;
        // TODO: actual damage calculation
        cu.getTMV(iREnv[1]).cModHP(cPKNV, -40);
      }
      // if pokemon did not kill its self with hurt confusion:
      if (cu.getPKV(iREnv[1]).isAlive())
      {
        uint32_t numTotalEnv = std::min((unsigned)4 , iConfused - AIL_V_CONFUSED_0T);
        uint32_t numTerminalEnv = ((iConfused - AIL_V_CONFUSED_0T)>=5)?0:1;
        fpType terminalProbability = ((fpType) numTerminalEnv) / ((fpType)numTotalEnv);

        boost::array<size_t, 2> iTEnv;
      
        if ((numTerminalEnv > 0) && (numTotalEnv > numTerminalEnv))
        {
          cu.duplicateState(iTEnv, terminalProbability, iREnv[1]);

          // variable % chance for this env to be the last environment confused:
          cu.getTMV(iREnv[1]).getVolatile().confused = 0;
        }
      }
    }
    else /* equals AIL_V_CONFUSED_0T */
    {
      // pokemon breaks out of confusion this round
      cTMV.getVolatile().confused = 0;
    }
  } // end of confused

  return 1;
} // endOf begin turn volatile effect

int engine_secondaryBoostEffect(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  const move& cMove = cPKNV.getMove_base(cu.getICAction());

  // apply buffs to the current pokemon, and debuffs to the other pokemon:
  for (size_t iBuff = 0; iBuff != 9; ++iBuff)
  {
    if (cMove.getSelfBuff(iBuff) != 0) 
    {
      cTMV.cModBoost(iBuff, cMove.getSelfBuff(iBuff));
    }
  }

  // all other effects modify the target pokemon, and we don't want to modify a dead one (this wills top all other plugins from running as well)
  if (!tPKV.isAlive()) { return 2; }

  for (size_t iBuff = 0; iBuff != 9; ++iBuff)
  {
    if (cMove.getTargetDebuff(iBuff) != 0) 
    {
      tTMV.cModBoost(iBuff, -1 * cMove.getTargetDebuff(iBuff));
    }
  }

  return 1;
} //endOf apply buffs / debuffs

int engine_secondaryNonvolatileEffect(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  const move& cMove = cPKNV.getMove_base(cu.getICAction());
  
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
      tTMV.getVolatile().toxicPoison_tier = 0;
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
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV,
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  const move& cMove = cPKNV.getMove_base(cu.getICAction());

  // apply volatile status conditions to the other pokemon:
  switch(cMove.getTargetVolatileAilment())
  {
    case AIL_V_CONFUSED:
      // confused for (at most) 5 turns, and (at least) 2 turns:
      tTMV.getVolatile().confused = AIL_V_CONFUSED_5T;
      // implicitly push back bEnv
      break;
    case AIL_V_FLINCH:
      tTMV.getVolatile().flinch = 1;
    case AIL_V_INFATUATED:
    default:
    case AIL_V_NONE:
      break; // do not apply a status condition, or push anything back
  } //endOf targetVolatileAilment switch
  return 1;
} //endOf apply buffs / debuffs

int engine_decrementPP(
  pkCU& cu,
  const move_nonvolatile& mNV, 
  const pokemon_nonvolatile& cPKNV, 
  const pokemon_nonvolatile& tPKNV,
  team_volatile& cTMV,
  team_volatile& tTMV,
  pokemon_volatile& cPKV,
  pokemon_volatile& tPKV)
{
  if (!cu.getBase().hasHit(cu.getICTeam()) || (&mNV.getBase() == struggle_t)) { return 0; }

  cPKV.getMV(cu.getICAction()).modPP(mNV.getBase(), -1);

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

  boost::array<const type*, 2> cTypes = 
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





bool registerExtensions(const pokedex& pkAI, std::vector<plugin>& extensions)
{
#ifndef GEN4_SCRIPTS_STATIC
  // register needed types:
  pkdex = &pkAI;
#endif
  //moves:
  const std::vector<move>& moves = pkdex->getMoves();
  absorb_t = orphan::orphanCheck_ptr(moves, NULL, "absorb");
  aerialAce_t = orphan::orphanCheck_ptr(moves, NULL, "aerial ace");
  airCutter_t = orphan::orphanCheck_ptr(moves, NULL, "air cutter");
  aromatherapy_t = orphan::orphanCheck_ptr(moves, NULL, "aromatherapy");
  attackOrder_t = orphan::orphanCheck_ptr(moves, NULL, "attack order");
  auraSphere_t = orphan::orphanCheck_ptr(moves, NULL, "aura sphere");
  blazeKick_t = orphan::orphanCheck_ptr(moves, NULL, "blaze kick");
  braveBird_t = orphan::orphanCheck_ptr(moves, NULL, "brave bird");
  crabHammer_t = orphan::orphanCheck_ptr(moves, NULL, "crabhammer");
  crossChop_t = orphan::orphanCheck_ptr(moves, NULL, "cross chop");
  crossPoison_t = orphan::orphanCheck_ptr(moves, NULL, "cross poison");
  doubleEdge_t = orphan::orphanCheck_ptr(moves, NULL, "double-edge");
  drainPunch_t = orphan::orphanCheck_ptr(moves, NULL, "drain punch");
  explosion_t = orphan::orphanCheck_ptr(moves, NULL, "explosion");
  faintAttack_t = orphan::orphanCheck_ptr(moves, NULL, "faint attack");
  flareBlitz_t = orphan::orphanCheck_ptr(moves, NULL, "flare blitz");
  gigaDrain_t = orphan::orphanCheck_ptr(moves, NULL, "giga drain");
  healBell_t = orphan::orphanCheck_ptr(moves, NULL, "heal bell");
  healOrder_t = orphan::orphanCheck_ptr(moves, NULL, "heal order");
  hiddenPower_t = orphan::orphanCheck_ptr(moves, NULL, "hidden power");
  leafBlade_t = orphan::orphanCheck_ptr(moves, NULL, "leaf blade");
  leechLife_t = orphan::orphanCheck_ptr(moves, NULL, "leech life");
  magicalLeaf_t = orphan::orphanCheck_ptr(moves, NULL, "magical leaf");
  magnetBomb_t = orphan::orphanCheck_ptr(moves, NULL, "magnet bomb");
  megaDrain_t = orphan::orphanCheck_ptr(moves, NULL, "mega drain");
  memento_t = orphan::orphanCheck_ptr(moves, NULL, "memento");
  milkDrink_t = orphan::orphanCheck_ptr(moves, NULL, "milk drink");
  nightShade_t = orphan::orphanCheck_ptr(moves, NULL, "night shade");
  nightSlash_t = orphan::orphanCheck_ptr(moves, NULL, "night slash");
  painSplit_t = orphan::orphanCheck_ptr(moves, NULL, "pain split");
  psychoCut_t = orphan::orphanCheck_ptr(moves, NULL, "psycho cut");
  rapidSpin_t = orphan::orphanCheck_ptr(moves, NULL, "rapid spin");
  razorLeaf_t = orphan::orphanCheck_ptr(moves, NULL, "razor leaf");
  roost_t = orphan::orphanCheck_ptr(moves, NULL, "roost");
  seismicToss_t = orphan::orphanCheck_ptr(moves, NULL, "seismic toss");
  selfDestruct_t = orphan::orphanCheck_ptr(moves, NULL, "selfdestruct");
  shadowClaw_t = orphan::orphanCheck_ptr(moves, NULL, "shadow claw");
  shadowPunch_t = orphan::orphanCheck_ptr(moves, NULL, "shadow punch");
  shockWave_t = orphan::orphanCheck_ptr(moves, NULL, "shock wave");
  slackOff_t = orphan::orphanCheck_ptr(moves, NULL, "slack off");
  slash_t = orphan::orphanCheck_ptr(moves, NULL, "slash");
  spikes_t = orphan::orphanCheck_ptr(moves, NULL, "spikes");
  softBoiled_t = orphan::orphanCheck_ptr(moves, NULL, "softboiled");
  stealthRock_t = orphan::orphanCheck_ptr(moves, NULL, "stealth rock");
  stoneEdge_t = orphan::orphanCheck_ptr(moves, NULL, "stone edge");
  struggle_t = orphan::orphanCheck_ptr(moves, NULL, "struggle");
  swift_t = orphan::orphanCheck_ptr(moves, NULL, "swift");
  toxicSpikes_t = orphan::orphanCheck_ptr(moves, NULL, "toxic spikes");
  voltTackle_t = orphan::orphanCheck_ptr(moves, NULL, "volt tackle");
  woodHammer_t = orphan::orphanCheck_ptr(moves, NULL, "wood hammer");
  //items:
  const std::vector<item>& items = pkdex->getItems();
  leftovers_t = orphan::orphanCheck_ptr(items, NULL, "leftovers");
  lifeOrb_t = orphan::orphanCheck_ptr(items, NULL, "life orb");
  lumBerry_t = orphan::orphanCheck_ptr(items, NULL, "lum berry");
  //abilities:
  const std::vector<ability>& abilities = pkdex->getAbilities();
  levitate_t = orphan::orphanCheck_ptr(abilities, NULL, "levitate");
  naturalCure_t = orphan::orphanCheck_ptr(abilities, NULL, "natural cure");
  noGuard_t = orphan::orphanCheck_ptr(abilities, NULL, "no guard");
  technician_t = orphan::orphanCheck_ptr(abilities, NULL, "technician");
  sereneGrace_t = orphan::orphanCheck_ptr(abilities, NULL, "serene grace");
  //types:
  const std::vector<type>& types = pkdex->getTypes();
  normal_t = orphan::orphanCheck_ptr(types, NULL, "normal");
  fighting_t = orphan::orphanCheck_ptr(types, NULL, "fighting");
  flying_t = orphan::orphanCheck_ptr(types, NULL, "flying");
  poison_t = orphan::orphanCheck_ptr(types, NULL, "poison");
  ground_t = orphan::orphanCheck_ptr(types, NULL, "ground");
  rock_t = orphan::orphanCheck_ptr(types, NULL, "rock");
  bug_t = orphan::orphanCheck_ptr(types, NULL, "bug");
  ghost_t = orphan::orphanCheck_ptr(types, NULL, "ghost");
  steel_t = orphan::orphanCheck_ptr(types, NULL, "steel");
  fire_t = orphan::orphanCheck_ptr(types, NULL, "fire");
  water_t = orphan::orphanCheck_ptr(types, NULL, "water");
  grass_t = orphan::orphanCheck_ptr(types, NULL, "grass");
  electric_t = orphan::orphanCheck_ptr(types, NULL, "electric");
  psychic_t = orphan::orphanCheck_ptr(types, NULL, "psychic");
  ice_t = orphan::orphanCheck_ptr(types, NULL, "ice");
  dragon_t = orphan::orphanCheck_ptr(types, NULL, "dragon");
  dark_t = orphan::orphanCheck_ptr(types, NULL, "dark");

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
  extensions.push_back(plugin(ENGINE_PLUGIN, "struggle always hits effect", PLUGIN_ON_MODIFYHITPROBABILITY, engine_move_struggle_alwaysHits, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile speed change", PLUGIN_ON_MODIFYSPEED, engine_onModifySpeed_paralyze, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile beginning-of-round damage", PLUGIN_ON_BEGINNINGOFTURN, engine_beginTurnNonvolatileEffect, -2, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "volatile beginning-of-round damage", PLUGIN_ON_BEGINNINGOFTURN, engine_beginTurnVolatileEffect, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect boosts", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryBoostEffect, -3, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect nonvolatile", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryNonvolatileEffect, -2, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "secondary effect volatile", PLUGIN_ON_SECONDARYEFFECT, engine_secondaryVolatileEffect, -1, 2));
  extensions.push_back(plugin(ENGINE_PLUGIN, "nonvolatile end-of-round damage", PLUGIN_ON_ENDOFROUND, engine_endRoundDamageEffect, -1, 2));

  return true;
}
