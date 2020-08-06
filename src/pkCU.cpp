
//#define PKAI_EXPORT
#include "../inc/pkCU.h"
//#undef PKAI_EXPORT

#include <iostream>
#include <limits>
#include <algorithm>

#include <stdint.h>
#include <math.h>

#include "../inc/fp_compare.h"
#include "../src/fixedpoint/fixed_func.h"

//#define PKAI_STATIC
#include "../inc/plugin.h"
#include "../inc/engine.h"
//#undef PKAI_STATIC

typedef std::vector<plugin_t>::const_iterator pluginIt;

#define CALLPLUGIN(retValue, pluginType, pluginFunction, ...) \
{\
  std::vector<plugin_t>& cPlugins = getCPluginSet()[(size_t)pluginType];\
  for (pluginIt iPlugin = cPlugins.begin(), iPSize = cPlugins.end(); iPlugin != iPSize; ++iPlugin)\
  {\
    pluginFunction cPlugin = (pluginFunction)iPlugin->pFunction; \
    retValue = retValue | cPlugin( __VA_ARGS__ ); \
    if (retValue > 1) { break; } \
  }\
}

PkCU::PkCU(const EnvironmentNonvolatile& _nv, size_t engineAccuracy)
  : nv(&_nv),
  pluginSets(),
  cPluginSet(NULL),
  _stack(NULL),
  stackStage(),
  PKVOffsets(),
  damageComponents(),
  iTeams(),
  iActions(),
  iBase(SIZE_MAX),
  prevStackCapacity(0),
  numRandomEnvironments((engineAccuracy>16)?1:engineAccuracy)
{
  iTeams.fill(SIZE_MAX);
  iActions.fill(SIZE_MAX);
  if (!initialize())
  {
    assert(false && "PKCU could not generate valid script database!");
  }
};

PkCU::PkCU(const PkCU& other)
  : nv(other.nv),
  pluginSets(other.pluginSets),
  cPluginSet(NULL),
  _stack(NULL),
  stackStage(),
  PKVOffsets(),
  damageComponents(),
  iTeams(),
  iActions(),
  iBase(SIZE_MAX),
  prevStackCapacity(0),
  numRandomEnvironments(other.numRandomEnvironments)
{
  iTeams.fill(SIZE_MAX);
  iActions.fill(SIZE_MAX);
};

void PkCU::setEnvironment(const EnvironmentNonvolatile& _envNV)
{
  nv = &_envNV;
  if (!initialize())
  {
    assert(false && "PKCU could not generate valid script database!");
  }
};

void PkCU::setAccuracy(size_t accuracy)
{
  numRandomEnvironments = std::max((size_t)1, std::min((size_t)16, accuracy));
};





PkCU::~PkCU()
{
};





bool PkCU::initialize()
{
  size_t numPlugins = 0;
  // clear plugin arrays:
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets.size(); ++iNCTeammate)
  {
    for (size_t iOTeammate = 0; iOTeammate != pluginSets[iNCTeammate].size(); ++iOTeammate)
    {
      std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& _cPluginSet = pluginSets[iNCTeammate][iOTeammate];
      for (size_t iPlugin = 0; iPlugin != _cPluginSet.size(); ++iPlugin)
      {
        _cPluginSet[iPlugin].clear();
      }
    }
  }

  // add individual plugins: (for each possible current pokemon, 12 total)
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets.size(); ++iNCTeammate)
  {
    size_t iCTeam = iNCTeammate >= 6;
    size_t iCTeammate = iNCTeammate - 6*iCTeam;
    // don't add to a teammate that doesn't exist:
    if ((iCTeammate) >= getNV().getTeam(iCTeam).getNumTeammates()) { continue; }
    // current nonvolatile teammate:
    const PokemonNonVolatile& cPKNV = getNV().getTeam(iCTeam).teammate(iCTeammate);

    // plugins for moves:
    for (size_t iCMove = 0; iCMove != cPKNV.getNumMoves(); ++iCMove)
    {
      const Pluggable& cPluggable = cPKNV.getMove_base(iCMove);
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin)
      {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }

    // plugins for abilities:
    if (cPKNV.abilityExists()) 
    { 
      const Pluggable& cPluggable = cPKNV.getAbility();
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin)
      {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }

    // plugins for items: TODO: what if the items switch teammates?
    if (cPKNV.hasInitialItem())
    { 
      const Pluggable& cPluggable = cPKNV.getInitialItem();
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin)
      {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }
  } // endOf cTeammate

  // TODO: add plugins that affect the other pokemon:

  // add global (and engine) plugins second:
  for (size_t iPluginType = 0; iPluginType != PLUGIN_MAXSIZE; ++iPluginType)
  {
    for (size_t iPlugin = 0; iPlugin != pkdex->getExtensions().getNumPlugins(iPluginType); ++iPlugin)
    {
      plugin_t cPlugin = pkdex->getExtensions().getPlugin(iPluginType, iPlugin);
      numPlugins++;

      insertPluginHandler(cPlugin, iPluginType);
    }
  }

  // SORT plugins by priority:
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets.size(); ++iNCTeammate)
  {
    for (size_t iOTeammate = 0; iOTeammate != pluginSets[iNCTeammate].size(); ++iOTeammate)
    {
      std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& _cPluginSet = pluginSets[iNCTeammate][iOTeammate];
      for (size_t iPlugin = 0; iPlugin != _cPluginSet.size(); ++iPlugin)
      {
        std::sort(_cPluginSet[iPlugin].begin(), _cPluginSet[iPlugin].end());
      }
    }
  }

  assert(numPlugins <= MAXPLUGINS);
  return true;
}





size_t PkCU::insertPluginHandler(plugin_t& cPlugin, size_t pluginType, size_t iNTeammate)
{
  size_t numAdded = 0;

  uint32_t target = cPlugin.target;
  size_t iNTeam = (iNTeammate>=6)?1:0;
  size_t iTeammate = iNTeammate - 6*iNTeam;

  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets.size(); ++iNCTeammate)
  {
    size_t iCTeam = iNCTeammate >= 6;
    size_t iCTeammate = iNCTeammate - 6*iCTeam;
    // don't add to a teammate that doesn't exist:
    if ((iCTeammate) >= getNV().getTeam(iCTeam).getNumTeammates()) { continue; }

    if ((target==TEAM_A) && (iCTeam != iNTeam)) { continue; }
    else if ((target==TEAM_B) && (iCTeam == iNTeam)) { continue; }

    //if on team of adding team, only match to this teammate. Otherwise match to all teammates
    if ((iNTeammate != SIZE_MAX) && (iNTeam == iCTeam) && (iTeammate != iCTeammate)) { continue; }

    for (size_t iOTeammate = 0; iOTeammate != getNV().getOtherTeam(iCTeam).getNumTeammates(); ++iOTeammate)
    {
      // if on opposite team of adding team, and not add
      if ((iNTeammate != SIZE_MAX) && (iCTeam != iNTeam) && (iTeammate != iOTeammate)) { continue; }

      std::vector<plugin_t>& _cPluginSet = pluginSets[iNCTeammate][iOTeammate][pluginType];
      // check for duplicate:
      if (std::find(_cPluginSet.begin(), _cPluginSet.end(), cPlugin) != _cPluginSet.end()) { continue; }
      // add:
      _cPluginSet.push_back(cPlugin);
      numAdded++;
    }
  }

  return numAdded;
}





uint32_t PkCU::movePriority_Speed()
{
  const TeamVolatile& cTMV = getTMV();
  const PokemonVolatile& cPKV = getPKV();
  const PokemonNonVolatile& cPKNV = getPKNV();

  // grab FV_boosted speed
  uint32_t cSpeed = cTMV.cGetFV_boosted(cPKNV, FV_SPEED);

  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_MODIFYSPEED, onModifySpeed_rawType, 
    *this, cPKNV, cTMV, cPKV, cSpeed);

  return cSpeed;
}





int32_t PkCU::movePriority_Bracket()
{
  // SOURCE: http://www.smogon.com/dp/articles/move_priority
  
   /* action:
   * AT_MOVE_0-3: pokemon's move
   * AT_MOVE_STRUGGLE  : struggle
   * AT_MOVE_NOTHING  : do nothing
   * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
   * AT_ITEM_USE: pokemon uses an item (not implemented)
   */
  
  // team:
  // 0 - a
  // 1 - b

  int32_t actionResult = 0;
  const EnvironmentVolatile& base = getBase().getEnv();
  size_t iCAction = getICAction();
  size_t iCTeam = getICTeam();
  
  // if the pokemon is switching out, its move priority is +6
  switch(iActions[0])
  {
    case AT_SWITCH_0:
    case AT_SWITCH_1:
    case AT_SWITCH_2:
    case AT_SWITCH_3:
    case AT_SWITCH_4:
    case AT_SWITCH_5:
      actionResult = 6;
      break;
    case AT_MOVE_0:
    case AT_MOVE_1:
    case AT_MOVE_2:
    case AT_MOVE_3:
    {
      // if the pokemon is performing a move, find the move's priority
      actionResult = (int32_t) getNV().getTeam(iCTeam).getPKNV(base.getTeam(iCTeam)).getMove_base(iCAction).getSpeedPriority();
      
      //script - modify movePriority - action
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_SETSPEEDBRACKET, onModifyBracket_rawType, 
        *this, getPKNV().getMove_base(iCAction), getPKNV(), getTMV(), getPKV(), actionResult);
      break;
    }
    case AT_MOVE_NOTHING:
      actionResult = -7;
      break;
    case AT_MOVE_STRUGGLE:
    case AT_ITEM_USE:
    default:
      actionResult = 0;
      break;
  }
  
  return actionResult;
}





uint32_t PkCU::movePriority()
{
  std::array<_moveBracket, 2> moveBracket;

  size_t iCTeam = getICTeam();
  size_t iOTeam = getIOTeam();
  
  // determine speed brackets of the move
  for (size_t iTeam = 0; iTeam != 2; ++iTeam)
  {
    moveBracket[iTeam].actionBracket = movePriority_Bracket();

    swapTeamIndexes();
  }
  
  // exceptions to actionBracket? (pursuit if switch is used)
  
  
  // are the priority brackets equal? If so, use speed as determining factor
  if (moveBracket[iCTeam].actionBracket > moveBracket[iOTeam].actionBracket)
  {
    return iCTeam;
  }
  else if (moveBracket[iCTeam].actionBracket < moveBracket[iOTeam].actionBracket)
  {
    return iOTeam;
  }
  else // speed bracket tie, determine speeds
  {

    // determine speeds of pokemon
    for (size_t iTeam = 0; iTeam != 2; ++iTeam)
    {
      moveBracket[iTeam].speed = movePriority_Speed();

      swapTeamIndexes();
    }
    
    if (moveBracket[iCTeam].speed > moveBracket[iOTeam].speed) { return iCTeam; }
    else if (moveBracket[iCTeam].speed < moveBracket[iOTeam].speed) { return iOTeam; }
    else { return 2; } // speed bracket and speed tie
  }
}





void PkCU::evaluateRound_end()
{
  iBase = 0;

  for (size_t iSize = getStack().size(); iBase != iSize; ++iBase)
  {
    // do not allow for partially completed stages in this step of computation:
    assert(getStackStage() == STAGE_POSTROUND);
    advanceStackStage();

    // ignore end of move environments if we're calculating a dummy move, such as a switch in upon death
    if (getBase().hasFreeMove(TEAM_A) || getBase().hasFreeMove(TEAM_B)) { continue; }
  
    for (size_t iTeam = 0; iTeam != 2; ++iTeam)
    {
      // do not call plugin if current pokemon is dead
      if (!getPKV().isAlive()) { continue; }

      // parse end of round plugins:
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_ENDOFROUND, onEndOfRound_rawType, 
        *this, getPKNV(), getTMV(), getPKV());

      swapTeamIndexes();
    }// endOf per team
  } // endOf per base
} // endOf evaluateRound_end





void PkCU::evaluateMove()
{
  // NOTE: ONLY ONE stage is set to preturn at a time
  assert(getStackStage() == STAGE_PRETURN);
  size_t iCAction = getICAction();
  size_t iCTeam = getICTeam();
  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size(), iNBase;

  // TODO: does this model the actual game?
  // if either pokemon is dead at this point, the only valid moves are switching and waiting
  if ( (!getPKV().isAlive() || !getTPKV().isAlive()) && isMoveAction(iCAction) )
  {
    iCAction = AT_MOVE_NOTHING;
  }


  // does this move require a switch-out?
  if (isSwitchAction(iCAction))
  {
    stackStage[iBase] = STAGE_PRESWITCH;
    evaluateMove_switch();
  } // end of is Switch type action

  // is this pokemon doing nothing?
  else if (iCAction == AT_MOVE_NOTHING)
  {
    stackStage[iBase] = STAGE_POSTSECONDARY;

    // set that the current team did nothing this turn:
    getBase().setWaited(iCTeam);
    // pokemon performs no action, no update to the state is needed
  } // end of is Wait type action

  // is the pokemon moving normally?
  else if (isMoveAction(iCAction))
  {
    assert(getPKV().isAlive() && getTPKV().isAlive());

    // this is the first function which may generate more than one state of STAGE_STATUS type
    {
      stackStage[iBase] = STAGE_STATUS;
      evaluateMove_preMove();
    }

    // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

    const Move& cMove = getPKNV().getMove_base(iCAction);
    void (PkCU::*evaluate_t)();
    if ( cMove.damageType_ == ATK_PHYSICAL || cMove.damageType_ == ATK_SPECIAL)
    { evaluate_t = &PkCU::evaluateMove_damage;}
    else
    { evaluate_t = &PkCU::evaluateMove_script;}

    // evaluate either move or plugin move: (increment with iNBase, as evaluateMove_damage will manipulate stack)
    for (iNBase = baseFloor, iBase = iNBase, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase = iNBase)
    {
      if (getStackStage() != STAGE_STATUS) { continue; }
      advanceStackStage();

      // was this move blocked by a status?
      // did this pokemon die from the last pokemon's action?
      if (getBase().wasBlocked(iCTeam) || !getPKV().isAlive()) { stackStage[iBase] = STAGE_POSTTURN; continue; }

      // evaluate either evaluateMove_damage from stackstage movebase, or evaluateMove_script from stackstage postmove
      (this->*evaluate_t)();
    }

    // for all worlds: (increment with iNBase, as evaluateMove_postMove will manipulate stack)
    for (iNBase = baseFloor, iBase = iNBase, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase = iNBase)
    {	
      if (getStackStage() != STAGE_POSTDAMAGE) { continue; }
      advanceStackStage();

      evaluateMove_postMove();
    } // endOf forEach environment
  } // endOf isMove

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  // end of turn occurences:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_POSTSECONDARY) { continue; }
    stackStage[iBase] = STAGE_POSTTURN;

    evaluateMove_postTurn();
  }
  
  return;
} // end of evaluateMove





void PkCU::evaluateMove_switch()
{
  assert(getStackStage() == STAGE_PRESWITCH);

  size_t iCTeam = getICTeam();
  size_t iCAction = getICAction();
  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size();

  //for (/*iBase = baseFloor, baseCeil = getStack().size()*/; iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_PRESWITCH) { continue; }
    advanceStackStage();

    // set that we are switching this environment:
    getBase().setSwitched(iCTeam);

    // is the switching out pokemon dead? If so, this is a free move
    if (!getPKV().isAlive()) { getBase().setFreeMove(iCTeam); }
    else
    {
      const PokemonNonVolatile* cPKNV = &getPKNV();

      // pre-move switch scripts:
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_SWITCHOUT, onSwitch_rawType, 
        *this, *cPKNV, getTMV(), getPKV());
    }
  } // endOf switchout script

  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_POSTSWITCH) { continue; }
    stackStage[iBase] = STAGE_POSTSECONDARY;

    // switch out
    getTMV().swapPokemon(iCAction);

    // set the current array of plugins:
    setCPluginSet();
    // reset memoized plugin:
    setPKV();

    const PokemonNonVolatile* cPKNV = &getPKNV();
    
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SWITCHIN, onSwitch_rawType, 
      *this, *cPKNV, getTMV(), getPKV());
    
  } // end of switchin script
} // endOf evaluateMove_switch





void PkCU::evaluateMove_preMove()
{
  assert(getStackStage() == STAGE_STATUS);

  // parse beginning of turn plugins:
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_BEGINNINGOFTURN, onBeginningOfTurn_rawType, 
    *this, getPKNV(), getTMV(), getPKV());
}




void PkCU::evaluateMove_postMove()
{
  assert(getStackStage() == STAGE_POSTMOVE);

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size();

  // the current environment we are evaluating upon. Will be copied, modified and pushed back eventually
  const PokemonNonVolatile& cPKNV = getPKNV();
  const PokemonNonVolatile& tPKNV = getTPKNV();
  const MoveNonVolatile& cMNV = cPKNV.getMove(getICAction());
  const Move& cMove = cMNV.getBase();
  size_t iCTeam = getICTeam();

  // effects which occur regardless of a secondary effect occuring, but only if the move hit:
  //for (/*iBase = baseFloor, baseCeil = getStack().size()*/; iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_POSTMOVE) { continue; }
    advanceStackStage();

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_ENDOFMOVE, onEvaluateMove_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV());
  }

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  // calculate probability to perform secondary:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_PRESECONDARY) { continue; }
    advanceStackStage();

    // special behavior if the teammate is dead: (cannot compute secondary effects)
    if (!getPKV().isAlive()) { stackStage[iBase] = STAGE_POSTTURN; continue; }

    // does this move even have a secondary effect? (this check is in-loop because multiple environments could arise from previous call)
    if (!mostlyGT(cMove.getSecondaryAccuracy(), 0.0)) { stackStage[iBase] = STAGE_POSTSECONDARY; continue; }

    fpType& secondaryHitProbability = getDamageComponent().tProbability;

    /* probability to inflict secondary condition*/
    secondaryHitProbability = cMove.getSecondaryAccuracy(); // lowest is 10%

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYSECONDARYPROBABILITY, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), secondaryHitProbability);
  } // endOf calculate secondary probability

  // split environments based on their secondary chance:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYSECONDARYHITCHANCE) { continue; }
    advanceStackStage();

    std::array<size_t, 2> iREnv = {{ getIBase() , SIZE_MAX }};

    fpType& secondaryHitProbability = getDamageComponent().tProbability;

    secondaryHitProbability = std::max(std::min(secondaryHitProbability, (fpType)1.0), (fpType)0.0);

    // did the ability hit its target? Is it possible for the secondary ability to miss?
    if (getBase().hasHit(iCTeam) && (mostlyGT(secondaryHitProbability, 0.0)))
    {
      // if there's a chance the secondary effect will not occur:
      if (mostlyLT(secondaryHitProbability, 1.0))
      {
        // duplicate the environment (duplicated environment is the secondary effect missed):
        duplicateState(iREnv, (1.0 - secondaryHitProbability));
      }

      // modify bitmask as secondary effect occuring:
      getStack()[iREnv[0]].setSecondary(iCTeam);

    } // end of primary attack hits, and secondary attack is not assured
    else
    {
      // pass-through: no chance to secondary
      stackStage[iBase] = STAGE_POSTSECONDARY;
      continue;
    }
  }

  // calculate effect of secondary, if it occured:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_SECONDARY) { continue; }
    advanceStackStage();

    // add extra effects to the move, such as secondaries and trigger effects
    if (!getBase().hasSecondary(iCTeam)) { continue; }

    // parse secondary effect plugins:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SECONDARYEFFECT, onEvaluateMove_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV());
  } // endOf if primary and secondary attacks hit
} // endOf evaluateMove_postMove





void PkCU::evaluateMove_postTurn()
{
  // parse end of turn plugins:
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_ENDOFTURN, onEndOfTurn_rawType, 
    *this, getPKNV(), getTMV(), getPKV());
};





void PkCU::evaluateMove_damage()
{
  assert(getStackStage() == STAGE_MOVEBASE);
  assert(getTPKV().isAlive() && getPKV().isAlive());

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size();

  const PokemonNonVolatile& cPKNV = getPKNV();
  const PokemonNonVolatile& tPKNV = getTPKNV();
  const MoveNonVolatile& cMNV = cPKNV.getMove(getICAction());
  const PokemonBase& cPKB = cPKNV.getBase();
  const PokemonBase& tPKB = tPKNV.getBase();
  const Move& cMove = cMNV.getBase();
  
  //Source: http://www.smogon.com/dp/articles/damage_formula

  /*BasePower = HH × BP × IT × CHG × (MS × WS) × UA × FA*/
  // set basePower:
  //for (/*iBase = baseFloor, baseCeil = getStack().size()*/; iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_MOVEBASE) { continue; }
    advanceStackStage();

    uint32_t& basePower = getDamageComponent().damage;
    basePower = cMove.getPower();

    int result = (basePower != UINT8_MAX)?1:0;
    CALLPLUGIN(result, PLUGIN_ON_SETBASEPOWER, onSetPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), basePower);

    assert(result > 0 && basePower > 0);
  }

  // calculate this move's type:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_SETBASEPOWER) { continue; }
    advanceStackStage();

    const Type*& cType = getDamageComponent().mType;
    cType = &cMove.getType();

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SETMOVETYPE, onModifyMoveType_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), cType);
  }

  // modify basePower:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_SETMOVETYPE) { continue; }
    advanceStackStage();

    uint32_t& basePower = getDamageComponent().damage;
    fpType baseModifier = 1.0;

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYBASEPOWER, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), baseModifier);

    basePower = (uint32_t)(basePower * baseModifier);
  }

  // calculate attack and damage modifiers:
  uint32_t levelModifier = ((cPKNV.getLevel() * 2) / 5) + 2;
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYBASEPOWER) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    size_t attackType;
    size_t defenseType;
    if (cMove.getDamageType() == ATK_PHYSICAL)
    { attackType = FV_ATTACK; defenseType = FV_DEFENSE; }
    else
    { attackType = FV_SPATTACK; defenseType = FV_SPDEFENSE; }

    uint32_t attackPower = getTMV().cGetFV_boosted(cPKNV, attackType);
    uint32_t attackPowerCrit = std::max(cPKNV.getFV_base(attackType), attackPower);

    uint32_t defensePower = getTTMV().cGetFV_boosted(tPKNV, defenseType);
    uint32_t defensePowerCrit = std::min(tPKNV.getFV_base(defenseType), defensePower);

    // calculate crit first:
    cDamage.damageCrit = ((levelModifier * cDamage.damage * attackPowerCrit) / 50) / defensePowerCrit;
    // and regular damage:
    cDamage.damage = ((levelModifier * cDamage.damage * attackPower) / 50) / defensePower;


    /* Mod1 = BRN × RL × TVT × SR × FF */
    // modifier1:
    fpType attackPowerModifier = 1.0;

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYATTACKPOWER, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), attackPowerModifier);

    // incorporate attack power modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * attackPowerModifier) + 2;
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * attackPowerModifier) + 2;
  }

  // calculate critical hit modifiers:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYATTACKPOWER) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    /* CH - Critical Hit modifier
      3 if has sniper ability AND critical hit (mult 1.5)
      2 if critical hit (mult 1.0)
      1 else
     */
    fpType criticalHitModifier = 2.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYCRITICALPOWER, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), criticalHitModifier);

    // incorporate critical power modifier:
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * criticalHitModifier);
  }

  // calculate raw damage modifiers:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYCRITICALPOWER) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    /* Mod2 = Other modifier
      1.3 if item = life orb
      1+.1*n if item = metronome and used the same move n previous times, to a max of n=10
      1.5 if attacking with Me First and attacks first (NOTE: SPECIAL BEHAVIOR with life orb / metronome!)
      1 else
     */
    fpType rawDamageMultiplier = 1.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYRAWDAMAGE, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), rawDamageMultiplier);

    // incorporate critical power modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * rawDamageMultiplier);
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * rawDamageMultiplier);
  }

  // calculate this move's STAB:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYRAWDAMAGE) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    bool hasStab = ((&cPKB.getType(0) == cDamage.mType) || (&cPKB.getType(1) == cDamage.mType));
    fpType STABMultiplier = hasStab?1.5:1.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYSTAB, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), STABMultiplier);

    // incorporate STAB modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * STABMultiplier);
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * STABMultiplier);
  }

  // calculate the enemy pokemon's type resistance:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYSTAB) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    fpType typeModifier = 1.0;
    {
      // type1:
      typeModifier *= cDamage.mType->getModifier(tPKB.getType(0));
      // type 2:
      typeModifier *= cDamage.mType->getModifier(tPKB.getType(1));
    }
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SETDEFENSETYPE, onModifyTypePower_rawType, 
      *this, cMNV, cPKNV, tPKNV, *cDamage.mType, getTMV(), getTTMV(), getPKV(), getTPKV(), typeModifier);

    // incorporate type modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * typeModifier);
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * typeModifier);
  }

  // calculate item resistance modifiers:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYTYPERESISTANCE) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    /* Mod3 = SRF × EB × TL × TRB */
    fpType itemModifier = 1.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYITEMPOWER, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), itemModifier);

    // incorporate type modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * itemModifier);
    cDamage.damageCrit = (uint32_t)(cDamage.damageCrit * itemModifier);
  }

  /* Damage Formula = (((((((Level × 2 ÷ 5) + 2) × BasePower × [Sp]Atk ÷ 50) ÷ [Sp]Def) × Mod1) + 2) × CH × Mod2 × R ÷ 100) × STAB × Type1 × Type2 × Mod3 */

  // calculate probability to hit, miss:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYITEMPOWER) { continue; }
    advanceStackStage();

    fpType& probabilityToHit = getDamageComponent().tProbability;

    /* probability to hit enemy pokemon */
    probabilityToHit = 
        getTMV().cGetAccuracy_boosted(FV_ACCURACY) // lowest is 33% or 1/3
        * getTTMV().cGetAccuracy_boosted(FV_EVASION) // highest is 300% or 3
        * cMove.getPrimaryAccuracy(); // lowest is 30%

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYHITPROBABILITY, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), probabilityToHit);

  }

  // evaluate miss(1), hit(0):
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYHITCHANCE) { continue; }
    advanceStackStage();

    fpType& probabilityToHit = getDamageComponent().tProbability;

    // bound at MIN 0.033~ ... MAX 1.0
    probabilityToHit = std::max(std::min(probabilityToHit, (fpType)1.0), (fpType)0.0);

    std::array<size_t, 2> iHEnv = {{ getIBase(), SIZE_MAX }};
    // did the move hit its target? Is it possible for the move to miss?
    if (mostlyGT(probabilityToHit, 0.0))
    {
      // if there's a chance the primary effect will not occur:
      if (mostlyLT(probabilityToHit, 1.0))
      {
        // duplicate the environment (duplicated environment is the miss environment):
        duplicateState(iHEnv, (1.0 - probabilityToHit));
      }

      // modify bitmask as the hit effect occuring:
      getStack()[iHEnv[0]].setHit(getICTeam());

    } // end of primary attack hits, and secondary attack is not assured
    else
    {
      // pass-through: no chance to hit or crit
      stackStage[iBase] = STAGE_POSTDAMAGE;
    }
  }

  // calculate probability to crit:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_EVALUATEHITCHANCE) { continue; }
    advanceStackStage();

    // don't continue to evaluate a stage that has not hit the enemy team:
    if (!getBase().hasHit(getICTeam())) { stackStage[iBase] = STAGE_POSTDAMAGE; continue; }

    fpType& probabilityToCrit = getDamageComponent().tProbability;

    /* Probability to critical hit, if the move has already hit */
    probabilityToCrit = 
      getTMV().cGetAccuracy_boosted(FV_CRITICALHIT);

    // to-crit modifying values:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYCRITPROBABILITY, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), probabilityToCrit);
  }

  // evaluate crit(2):
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYCRITCHANCE) { continue; }
    advanceStackStage();

    fpType& probabilityToCrit = getDamageComponent().tProbability;

    // determine the possibility that the move crit:
    std::array<size_t, 2> iCEnv = {{ SIZE_MAX, getIBase() }};
    
    if (mostlyGT(probabilityToCrit, 0.0) )
    {
      if (mostlyLT(probabilityToCrit, 1.0))
      {
        // duplicate the environment (duplicated environment is the crit environment):
        duplicateState(iCEnv, probabilityToCrit);
      }

      // modify bitmask as the crit effect occuring:
      getStack()[iCEnv[1]].setCrit(getICTeam());
    }
    // even with no chance to crit there's still the possibility of damage
  }

  // perform actual damage calculation
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_PREDAMAGE) { continue; }
    advanceStackStage();

    if (!getBase().hasHit(getICTeam())) { continue; }

    calculateDamage(getBase().hasCrit(getICTeam()));
  }
} // end of evaluateMove_damage





void PkCU::evaluateMove_script()
{
  assert(getStackStage() == STAGE_MOVEBASE);
  assert(getTPKV().isAlive() && getPKV().isAlive());

  const PokemonNonVolatile& cPKNV = getPKNV();
  const PokemonNonVolatile& tPKNV = getTPKNV();
  const MoveNonVolatile& cMNV = cPKNV.getMove(getICAction());
  const Move& cMove = cMNV.getBase();

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size();

  // calculate probability to hit, miss:
  //for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_MOVEBASE) { continue; }
    stackStage[iBase] = STAGE_MODIFYHITCHANCE;

    fpType& probabilityToHit = getDamageComponent().tProbability;

    // TODO: take target into account when calculating probability to hit!
    /* probability to hit enemy pokemon */
    if (cMove.targetsEnemy())
    {
      probabilityToHit = 
          getTMV().cGetAccuracy_boosted(FV_ACCURACY) // lowest is 33% or 3333 / 10000
          * getTTMV().cGetAccuracy_boosted(FV_EVASION) // highest is 300% or 3
          * cMove.getPrimaryAccuracy(); // lowest is 30% or 30 / 100
    }
    else
    {
      probabilityToHit = cMove.getPrimaryAccuracy();
    }

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYHITPROBABILITY, onModifyPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), probabilityToHit);
  }

  // evaluate miss(1), hit(0),
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_MODIFYHITCHANCE) { continue; }
    stackStage[iBase] = STAGE_PREDAMAGE;

    fpType& probabilityToHit = getDamageComponent().tProbability;

    // bound at MIN 0.033~ ... MAX 1.0
    probabilityToHit = std::max(std::min(probabilityToHit, (fpType)1.0), (fpType)0.0);

    std::array<size_t, 2> iHEnv = {{ getIBase(), SIZE_MAX }};
    // did the move hit its target? Is it possible for the move to miss?
    if (mostlyGT(probabilityToHit, 0.0))
    {
      // if there's a chance the primary effect will not occur:
      if (mostlyLT(probabilityToHit, 1.0))
      {
        // duplicate the environment (duplicated environment is the miss environment):
        duplicateState(iHEnv, (1.0 - probabilityToHit));
      }

      // modify bitmask as the hit effect occuring:
      getStack()[iHEnv[0]].setHit(getICTeam());

    } // end of primary attack hits, and secondary attack is not assured
    else
    {
      // pass-through: no chance to hit
      stackStage[iBase] = STAGE_POSTDAMAGE;
    }
  }

  // perform script:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_PREDAMAGE) { continue; }
    advanceStackStage();

    if (!getBase().hasHit(getICTeam())) { continue; }

    // parse alternative move plugins:
    int result = getPKNV().getMove_base(getICAction()).isImplemented()?1:0; // TODO: this check isn't working!
    CALLPLUGIN(result, PLUGIN_ON_EVALUATEMOVE, onEvaluateMove_rawType, 
      *this, getPKNV().getMove(getICAction()), getPKNV(), getTPKNV(), getTMV(), getTTMV(), getPKV(), getTPKV());
  }

  return;
}





void PkCU::calculateDamage(bool hasCrit)
{
  const PokemonNonVolatile& cPKNV = getPKNV();
  const PokemonNonVolatile& tPKNV = getTPKNV();
  const MoveNonVolatile& cMNV = cPKNV.getMove(getICAction());

  fpType partitionEnvironmentProbability = (1.0 / (fpType) numRandomEnvironments);
  DamageComponents_t& cDMG = getDamageComponent();

  uint32_t power = (hasCrit)?cDMG.damageCrit:cDMG.damage;

  std::array<size_t, 2> iREnv = {{ SIZE_MAX, getIBase() }};
  for (size_t iEnv = 0; iEnv != numRandomEnvironments; ++iEnv )
  {
    if (numRandomEnvironments > 1) 
    {
      if ((iEnv + 1) == numRandomEnvironments) { iREnv[1] = getIBase(); }
      else { duplicateState(iREnv, partitionEnvironmentProbability); }
    };

    // find the mean random value for this partition of the random environment
    fpType randomValue = (iEnv / (fpType)numRandomEnvironments) + ( 1.0 / (fpType)numRandomEnvironments / 2.0);

    // scale our random value modifier to 0.85..1.0
    randomValue = deScale(randomValue, 1.0, 0.85);

    uint32_t& actualDamage = getDamageComponent(iREnv[1]).damage;
    actualDamage = (uint32_t)((fpType)power * randomValue);

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_CALCULATEDAMAGE, onSetPower_rawType, 
      *this, cMNV, cPKNV, tPKNV, getTMV(), getTTMV(), getPKV(), getTPKV(), actualDamage);

    // inflict damage caused upon the targetPokemon:
    getTTMV(iREnv[1]).cModHP(getTPKNV(), -1 * actualDamage);
  }
}





size_t PkCU::combineSimilarEnvironments()
{
  PossibleEnvironments& stack = getStack();

#ifndef NDEBUG
#ifndef _DISABLEPROBABILITYCHECK
  fpType probabilityAccumulator = 0.0;
#endif
#endif

  // hash environments (and summate probabilities for check):
  for (iBase = 0; iBase != stack.size(); ++iBase)
  {
    EnvironmentPossible& cEnvironment = getBase();

    // assert that each of these environments is getting hashed:
    assert(getStackStage() == STAGE_FINAL);
    advanceStackStage();

    // WARNING: EXPENSIVE!
    cEnvironment.generateHash();
  }

  // compare environment hashes:
  for (size_t iOEnv = 0, iSize = stack.size(); iOEnv != iSize; iOEnv++)
  {
    EnvironmentPossible& oEnv = stack[iOEnv];
    fpType& oProbability = damageComponents[iOEnv].cProbability;

    // don't attempt to merge pruned environments
    if (oEnv.isPruned()) { continue; }

    for (size_t iIEnv = iOEnv + 1; iIEnv != iSize; iIEnv++)
    {
      EnvironmentPossible& iEnv = stack[iIEnv];
      fpType& iProbability = damageComponents[iIEnv].cProbability;

      // don't re-prune already pruned environments
      if (iEnv.isPruned()) { continue; }
      
      // don't merge non-identical environments
#ifdef _PKCUCHECKSIGNATURE
      assert((oEnv.hash == iEnv.hash) == (oEnv.env == iEnv.env));
#endif
      if (oEnv.getHash() != iEnv.getHash()) { continue; }

      // combine the two environments by adding their probabilities and deleting the second
      oProbability += iProbability;
      
      // this is probably not representative of the current environment now
      oEnv.getBitmask() &= iEnv.getBitmask();
      
      // flag this environment as merged with another environment
      oEnv.setMerged();

      // flag the inner environment as pruned by another environment
      iEnv.setPruned();

      // decrement number of unique values in vector
      stack.decrementUnique();

    } //endOf iInner

    // assign the collected probability to envP's smaller fixed point variable
    oEnv.getProbability() = fixedpoint::create<30>(oProbability);

#ifndef NDEBUG
#ifndef _DISABLEPROBABILITYCHECK
    probabilityAccumulator += oProbability;
#endif
#endif
    assert(mostlyGT(oProbability, 0.0) && mostlyLTE(oProbability, 1.0));
  } // endOf iOuter

#ifndef _DISABLEPROBABILITYCHECK
  assert(mostlyEQ(probabilityAccumulator, 1.0));
#endif

  return stack.getNumUnique();
} //endOf combineSimilarEnvironments





void PkCU::updateState_move()
{
  assert(getStackStage() == STAGE_SEEDED);
  advanceStackStage();

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase, baseCeil = getStack().size(), iNBase;

  // evaluate first pokemon's moves, and their probabilities (iBase provided by updateState)
  evaluateMove();

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  iBase = baseFloor;
  swapTeamIndexes();

  // TODO: STAGE_POSTROUND should automatically be set after the second go-around as stage keeps incrementing
  // increment with iNBase, as evaluateMove will manipulate stack
  for (iNBase = baseFloor, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase = iNBase)
  {
    // do not evaluate a move that has not been evaluated by first pokemon yet:
    if (getStackStage() != STAGE_POSTTURN) { continue; }

    // if the pokemon died, no reason for it to take its turn
    //if (!getPKV().isAlive()) { continue; }

    // evaluate second pokemon's moves and their probabilities
    stackStage[iBase] = STAGE_PRETURN;
    evaluateMove();
  }

  // assert ALL moves have completed both turns:
  for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    if (getStackStage() != STAGE_POSTTURN) { continue; }
    advanceStackStage();
  }
}





PossibleEnvironments PkCU::updateState(
    const EnvironmentVolatile& currentEnvironment, size_t actionA, size_t actionB) {
  PossibleEnvironments result;
  updateState(currentEnvironment, result, actionA, actionB);
  
  return result;
}


size_t PkCU::updateState(
    const EnvironmentVolatile& cEnv, 
    PossibleEnvironments& rEnv,
    size_t actionA, 
    size_t actionB) {
  // set stack callback value:
  _stack = &rEnv;

  // seed teams and actions:
  iTeams[TEAM_A] = TEAM_A;
  iTeams[TEAM_B] = TEAM_B;
  iActions[TEAM_A] = actionA;
  iActions[TEAM_B] = actionB;
  // seed an initial value onto the stack:
  seedCurrentState(cEnv);
  // set the current set of plugins:
  setCPluginSet();

  // determine who moves first
  uint32_t priority = movePriority();

  switch(priority)
  {
  case TEAM_B:
    // swap the indexes, as TEAM_B is moving first:
    swapTeamIndexes();
  default:
  case TEAM_A:
    updateState_move();
    break;
  case 2:
    {
      // both teams are moving:
      std::array<size_t, 2> iStages;
      duplicateState(iStages, 0.5);

      // first team moves first:
      iBase = iStages[0];
      updateState_move();

      // swap indexes:
      iBase = iStages[1];
      //swapTeamIndexes(); (updateState_move swaps team indexes but does not swap them back)

      // second team moves first:
      updateState_move();
      break;
    }
  }
  
  // compute end of round component:
  evaluateRound_end();
  
  // combine environments that equal eachother:
  size_t numUnique = combineSimilarEnvironments();
  
  return numUnique;
}; // end of updateState





EnvironmentVolatile PkCU::initialState() const {
  return EnvironmentVolatile::create(*nv);
}





MatchState PkCU::isGameOver(const EnvironmentVolatile& envV)
{
  bool teamAisDead = envV.getTeam(TEAM_A).numTeammatesAlive() == 0;
  bool teamBisDead = envV.getTeam(TEAM_B).numTeammatesAlive() == 0;
  int status = 
    (teamAisDead==true?1:0) * 1 
    + 
    (teamBisDead==true?1:0) * 2;

  switch(status)
  {
  case 0: // game isn't over, neither team dead
    assert(!(teamAisDead) || !(teamBisDead));
    return MATCH_MIDGAME;
  case 1: // game is over, team A is dead
    return MATCH_TEAM_B_WINS;
  case 2: // game is over, team B is dead
    return MATCH_TEAM_A_WINS;
  default:
    assert(false && "isGameOver returned an unacceptable terminal game value!");
  case 3: // game is over, tie
    return MATCH_TIE;
  };
}





bool PkCU::isValidAction(const EnvironmentVolatile& envV, size_t action, size_t iTeam)
{
  const TeamVolatile& cTV = envV.getTeam(iTeam);
  const TeamVolatile& oTV = envV.getOtherTeam(iTeam);
  const TeamNonVolatile& cTNV = getNV().getTeam(iTeam);
  const PokemonNonVolatile& cPokemon = cTNV.getPKNV(cTV);
  
  switch(action)
  {
    case AT_MOVE_0:
    case AT_MOVE_1:
    case AT_MOVE_2:
    case AT_MOVE_3:
    {
      /* is this a valid move? */
      if ((action - AT_MOVE_0) >= cPokemon.getNumMoves()) { return false; }

      // is the other pokemon alive?
      if (!(oTV.getPKV().isAlive())) { return false; }

      // is the pokemon we're currently using alive?
      if (!cTV.getPKV().isAlive()) { return false; }

      // does the move we're using have any PP left?
      if ( cTV.getPKV().getMV(action - AT_MOVE_0).hasPP() != true ) { return false; }
    
      //TODO: are we locked out of the current move?

      return true; // by default, allow moves
    }
    case AT_SWITCH_0:
    case AT_SWITCH_1:
    case AT_SWITCH_2:
    case AT_SWITCH_3:
    case AT_SWITCH_4:
    case AT_SWITCH_5:
    {
      // is the pokemon we're switching to a valid teammate?
      if ( ((action - AT_SWITCH_0) < cTNV.getNumTeammates()) != true) { return false; }
    
      // are we trying to switch to ourself?
      if ((action - AT_SWITCH_0) == cTV.getICPKV()) { return false; }

      // is the pokemon we're switching to even alive?
      if ( cTV.teammate(action - AT_SWITCH_0).isAlive() != true) { return false; }

      // are we trying to move during the other team's free move?
      if (
        !(oTV.getPKV().isAlive())
        && cTV.getPKV().isAlive()
        ) { return false; }
    
      // TODO: are we locked out of switching?

      return true; // by default, allow switches
    }
    case AT_MOVE_NOTHING:
      // are we waiting for the other team to take its free move?
      if (
        !(oTV.getPKV().isAlive())
        && cTV.getPKV().isAlive()
        ) { return true; }

      // in most cases, do not allow not moving
      return false;
    case AT_MOVE_STRUGGLE:
      // is the other pokemon alive?
      if (!(oTV.getPKV().isAlive())) { return false; }

      // is the pokemon we're currently using alive?
      if (!cTV.getPKV().isAlive()) { return false; }

      // can the pokemon struggle?
      if (!cTV.cHasPP()) { return true; }

      // cannot struggle by default
      return false;
    // disabled action types:
    case AT_ITEM_USE:
    case AT_MOVE_CONFUSED:
      return false;
    default: // what the hell happened here? No, this isn't a valid move. Shut up.
      return false;
  }
} // endOf is valid action





void PkCU::seedCurrentState(const EnvironmentVolatile& cEnv)
{
  PossibleEnvironments& stack = getStack();

  size_t numRandomSquared = numRandomEnvironments * numRandomEnvironments;

  // clear the stack, just in case
  _stack->clear();
  _stack->reserve(MINSTACKSIZE * numRandomSquared); // moves can crit, hit, and miss. (max I've seen is branch factor 72) The vector will grow if this is too few
  PKVOffsets.clear();
  PKVOffsets.reserve(MINSTACKSIZE * numRandomSquared);
  stackStage.clear();
  stackStage.reserve(MINSTACKSIZE * numRandomSquared);
  damageComponents.clear();
  damageComponents.reserve(MINSTACKSIZE * numRandomSquared);

  // set counter vars:
  prevStackCapacity = _stack->capacity();
  iBase = stack.size();

  // actually push back stack var:
  stack.push_back(EnvironmentPossible::create(cEnv));

  // push back memoization vars:
  stackStage.push_back(STAGE_SEEDED);
  
  damageComponents.push_back(DamageComponents_t());
  damageComponents.back().cProbability = 1.0;

  pushPKV();
}





bool saneStackProbability(std::vector<DamageComponents_t>& dComponents)
{
  fpType sum = 0.0;
  for (std::vector<DamageComponents_t>::const_iterator begin = dComponents.begin(), end = dComponents.end(); begin != end; ++begin)
  {
    sum += begin->cProbability;
    if (!mostlyGT(begin->cProbability, 0.0) || !mostlyLTE(begin->cProbability, 1.0)) { return false; }
  }

  return mostlyEQ(sum, 1.0);
}

void PkCU::duplicateState(std::array<size_t, 2>& result, fpType _probability, size_t iState)
{
  assert(mostlyLTE(_probability, 1.0));

  // duplicate state 2 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  fpType totalProbability = damageComponents[result[0]].cProbability;
  damageComponents[result[1]].cProbability *= _probability;
  damageComponents[result[0]].cProbability = totalProbability - damageComponents[result[1]].cProbability;

  assert(saneStackProbability(damageComponents));
}





void PkCU::triplicateState(std::array<size_t, 3>& result, fpType _probability, fpType _oProbability, size_t iState)
{
  assert(mostlyLTE(_probability + _oProbability, 1.0));

  // duplicate state 3 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  fpType totalProbability = damageComponents[result[0]].cProbability;
  damageComponents[result[1]].cProbability *= _probability;
  damageComponents[result[2]].cProbability *= _oProbability;
  damageComponents[result[0]].cProbability = totalProbability - (damageComponents[result[1]].cProbability + damageComponents[result[2]].cProbability);

  assert(saneStackProbability(damageComponents));
}





const PokemonNonVolatile& PkCU::getPKNV()
{
  return getNV().getTeam(getICTeam()).getPKNV(getBase().getEnv().getTeam(getICTeam()));
}




void PkCU::setPKV(size_t iState)
{
  if (iState == SIZE_MAX) { iState = iBase; }
  PKVOffsets[iState] = &getStack()[iState].getEnv().getTeam(getICTeam()).getPKV();
}

void PkCU::setPKV()
{
  PKVOffsets[iBase] = &getStack()[iBase].getEnv().getTeam(getICTeam()).getPKV();
}





void PkCU::pushPKV()
{
  PKVOffsets.push_back(NULL);
}





PokemonVolatile& PkCU::getPKV(size_t iState)
{
  if (PKVOffsets[iState] == NULL) { setPKV(iState); }
  return *PKVOffsets[iState]; // memoized method
}

PokemonVolatile& PkCU::getPKV()
{
  if (PKVOffsets[iBase] == NULL) { setPKV(); }
  return *PKVOffsets[iBase]; // memoized method
}





void PkCU::resetPKVArray()
{
  memset(PKVOffsets.data(), 0, sizeof(PokemonVolatile*)*PKVOffsets.size());
  /*for (std::vector<pokemon_volatile*>::iterator begin = PKVOffsets.begin(), end = PKVOffsets.end(); begin != end; ++begin)
  {
    *begin = NULL;
  }*/
}





const PokemonNonVolatile& PkCU::getTPKNV()
{
  return getNV().getTeam(getIOTeam()).getPKNV(getBase().getEnv().getTeam(getIOTeam()));
}





PokemonVolatile& PkCU::getTPKV(size_t iState)
{
  return getStack()[iState].getEnv().getTeam(getIOTeam()).getPKV();
}

PokemonVolatile& PkCU::getTPKV()
{
  return getStack()[iBase].getEnv().getTeam(getIOTeam()).getPKV();
}




TeamVolatile& PkCU::getTMV()
{
  return getStack()[iBase].getEnv().getTeam(getICTeam());
}

TeamVolatile& PkCU::getTTMV()
{
  return getStack()[iBase].getEnv().getTeam(getIOTeam());
}

TeamVolatile& PkCU::getTMV(size_t iState)
{
  return getStack()[iState].getEnv().getTeam(getICTeam());
}

TeamVolatile& PkCU::getTTMV(size_t iState)
{
  return getStack()[iState].getEnv().getTeam(getIOTeam());
}





std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& PkCU::getCPluginSet()
{
  return *cPluginSet;
};





void PkCU::setCPluginSet()
{
  size_t iCPokemon = getBase().getEnv().getTeam(getICTeam()).getICPKV();
  size_t iOPokemon = getBase().getEnv().getTeam(getIOTeam()).getICPKV();
  cPluginSet = &pluginSets[getICTeam() * 6 + iCPokemon][iOPokemon];
}
