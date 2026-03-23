/**
 * @file legacy_pkCU.cpp
 * @brief Implements the core battle engine (LegacyPkCU) and its internal state
 * machine (LegacyPkCUEngine).
 *
 * This file contains the implementation of the LegacyPkCU and LegacyPkCUEngine classes,
 * which are responsible for simulating Pokemon battles. The logic follows the
 * standard Pokemon battle mechanics, including move priority, damage
 * calculation, status effects, and plugin handling.
 */
#include "pokemonai/legacy_pkCU.h"

#include <algorithm>
#include <iostream>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <boost/program_options.hpp>

#include "pokemonai/fp_compare.h"
#include "pokemonai/fixedpoint/fixed_func.h"

#include "pokemonai/pluggable_types.h"
#include "pokemonai/plugin.h"
#include "pokemonai/engine.h"

namespace po = boost::program_options;
typedef std::vector<plugin_t>::const_iterator pluginIt;

/**
 * @def CALLPLUGIN
 * @brief A macro for invoking plugins of a specific type.
 *
 * This macro iterates through all registered plugins of a given `pluginType`
 * for the current matchup and calls their `pluginFunction`. The return value
 * of each plugin is OR'd with `retValue`. The loop breaks if `retValue`
 * becomes greater than 1, which is a convention to indicate that a plugin has
 * handled the event and no further plugins should be called.
 *
 * @param retValue The variable to store the combined return values of the plugins.
 * @param pluginType The type of plugin to call (e.g., `PLUGIN_ON_MODIFYSPEED`).
 * @param pluginFunction The function signature of the plugin to be called.
 * @param ... The arguments to pass to the plugin function.
 */
#define CALLPLUGIN(retValue, pluginType, pluginFunction, ...) \
{\
  const std::vector<plugin_t>& cPlugins = getCPluginSet()[(size_t)pluginType];\
  for (pluginIt iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); iPlugin != iPSize; ++iPlugin)\
  {\
    pluginFunction cPlugin = (pluginFunction)iPlugin->pFunction; \
    retValue = retValue | cPlugin( __VA_ARGS__ ); \
    if (retValue > 1) { break; } \
  }\
}


LegacyPkCU* LegacyPkCU::clone() const {
  return new LegacyPkCU(*this);
}


po::options_description LegacyPkCU::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "engine-accuracy").c_str(),
      po::value<size_t>(&numRandomEnvironments)->default_value(defaults.numRandomEnvironments),
      "number of random environments to create per hit/crit 1-16.");

  return desc;

}


LegacyPkCU& LegacyPkCU::setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
  if (nv_ != nv) {
    nv_ = nv;
    initialState_ = EnvironmentVolatileData::create(*nv_);
    if (!initialize()) {
      throw std::runtime_error("Could not generate a script database!");
    }
  }

  return *this;
};

LegacyPkCU& LegacyPkCU::setAccuracy(size_t accuracy)
{
  cfg_.numRandomEnvironments = std::max((size_t)1, std::min((size_t)16, accuracy));
  return *this;
};

/**
 * @brief Initializes the plugin sets for the configured non-volatile environment.
 *
 * This method clears any existing plugin data and then populates the
 * `pluginSets_` data structure with all the relevant plugins for the upcoming
 * battle. It iterates through all Pokemon on both teams, adding plugins from
 * their moves, abilities, and items. It also adds global plugins from the
 * Pokedex. Finally, it sorts the plugins by priority.
 *
 * @return `true` if initialization was successful, `false` otherwise.
 */
bool LegacyPkCU::initialize() {
  size_t numPlugins = 0;
  // clear plugin arrays:
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets_.size(); ++iNCTeammate)
  {
    for (size_t iOTeammate = 0; iOTeammate != pluginSets_[iNCTeammate].size(); ++iOTeammate)
    {
      std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& _cPluginSet = pluginSets_[iNCTeammate][iOTeammate];
      for (size_t iPlugin = 0; iPlugin != _cPluginSet.size(); ++iPlugin)
      {
        _cPluginSet[iPlugin].clear();
      }
    }
  }

  // add individual plugins: (for each possible current pokemon, 12 total)
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets_.size(); ++iNCTeammate) {
    size_t iCTeam = iNCTeammate >= 6;
    size_t iCTeammate = iNCTeammate - 6*iCTeam;
    // don't add to a teammate that doesn't exist:
    if ((iCTeammate) >= nv_->getTeam(iCTeam).getNumTeammates()) { continue; }
    // current nonvolatile teammate:
    const PokemonNonVolatile& cPKNV = nv_->getTeam(iCTeam).teammate(iCTeammate);

    // plugins for moves:
    for (size_t iCMove = 0; iCMove != cPKNV.getNumMoves(); ++iCMove) {
      const Pluggable& cPluggable = cPKNV.getMove_base(iCMove);
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin) {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }

    // plugins for abilities:
    if (cPKNV.abilityExists())  {
      const Pluggable& cPluggable = cPKNV.getAbility();
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin) {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }

    // plugins for items: TODO: what if the items switch teammates?
    if (cPKNV.hasInitialItem()) {
      const Pluggable& cPluggable = cPKNV.getInitialItem();
      for (size_t iPlugin = 0; iPlugin != PLUGIN_MAXSIZE; ++iPlugin) {
        plugin_t cPlugin = cPluggable.getPlugin(iPlugin);
        if (cPlugin.pFunction == NULL) { continue; }
        insertPluginHandler(cPlugin, iPlugin, iNCTeammate);
        numPlugins++;
      }
    }
  } // endOf cTeammate

  // TODO: add plugins that affect the other pokemon:

  // add global (and engine) plugins second:
  for (size_t iPluginType = 0; iPluginType != PLUGIN_MAXSIZE; ++iPluginType) {
    for (size_t iPlugin = 0; iPlugin != pkdex->getExtensions().getNumPlugins(iPluginType); ++iPlugin) {
      plugin_t cPlugin = pkdex->getExtensions().getPlugin(iPluginType, iPlugin);
      numPlugins++;

      insertPluginHandler(cPlugin, iPluginType);
    }
  }

  // SORT plugins by priority:
  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets_.size(); ++iNCTeammate) {
    for (size_t iOTeammate = 0; iOTeammate != pluginSets_[iNCTeammate].size(); ++iOTeammate) {
      std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& _cPluginSet = pluginSets_[iNCTeammate][iOTeammate];
      for (size_t iPlugin = 0; iPlugin != _cPluginSet.size(); ++iPlugin) {
        std::sort(_cPluginSet[iPlugin].begin(), _cPluginSet[iPlugin].end());
      }
    }
  }

  return true;
}

/**
 * @brief Inserts a plugin into the appropriate plugin sets based on its target.
 *
 * This method adds a given plugin to the `pluginSets_` data structure. The
 * plugin is added to the plugin sets for all matchups where it is relevant,
 * based on the plugin's `target` property.
 *
 * @param cPlugin The plugin to insert.
 * @param pluginType The type of the plugin.
 * @param iNTeammate The index of the teammate the plugin is associated with. If
 *        `SIZE_MAX`, the plugin is considered global.
 * @return The number of plugin sets the plugin was added to.
 */
size_t LegacyPkCU::insertPluginHandler(plugin_t& cPlugin, size_t pluginType, size_t iNTeammate) {
  size_t numAdded = 0;

  pluginTarget target = cPlugin.target;
  size_t iNTeam = (iNTeammate>=6)?1:0;
  size_t iTeammate = iNTeammate - 6*iNTeam;

  for (size_t iNCTeammate = 0; iNCTeammate != pluginSets_.size(); ++iNCTeammate) {
    size_t iCTeam = iNCTeammate >= 6;
    size_t iCTeammate = iNCTeammate - 6*iCTeam;
    // don't add to a teammate that doesn't exist:
    if ((iCTeammate) >= nv_->getTeam(iCTeam).getNumTeammates()) { continue; }

    if ((target==current_team) && (iCTeam != iNTeam)) { continue; }
    else if ((target==other_team) && (iCTeam == iNTeam)) { continue; }

    //if on team of adding team, only match to this teammate. Otherwise match to all teammates
    if ((iNTeammate != SIZE_MAX) && (iNTeam == iCTeam) && (iTeammate != iCTeammate)) { continue; }

    for (size_t iOTeammate = 0; iOTeammate != nv_->getOtherTeam(iCTeam).getNumTeammates(); ++iOTeammate) {
      // if on opposite team of adding team, and not add
      if ((iNTeammate != SIZE_MAX) && (iCTeam != iNTeam) && (iTeammate != iOTeammate)) { continue; }

      std::vector<plugin_t>& _cPluginSet = pluginSets_[iNCTeammate][iOTeammate][pluginType];
      // check for duplicate:
      if (std::find(_cPluginSet.begin(), _cPluginSet.end(), cPlugin) != _cPluginSet.end()) { continue; }
      // add:
      _cPluginSet.push_back(cPlugin);
      numAdded++;
    }
  }

  return numAdded;
}

LegacyPkCUEngine::LegacyPkCUEngine(
    const LegacyPkCU& cu,
    PossibleEnvironments& stack,
    const EnvironmentVolatileData& initial,
    const Action& actionA,
    const Action& actionB)
    : cu_(cu),
      cfg_(cu.cfg_),
      stack_(stack),
      pluginSets_(cu.pluginSets_),
      iTeams_({TEAM_A, TEAM_B}),
      actions_({actionA, actionB}),
      iBase_(0) {
  stack_.clear();
  stack_.setNonvolatileEnvironment(cu.nv_);

  // push-back first stack stage:
  stack_.push_back(EnvironmentPossibleData::create(initial, false));
  stackStage_.push_back(STAGE_SEEDED);
  damageComponents_.push_back({DamageComponents_t{}, DamageComponents_t{}});
  damageComponents_.back()[0].cProbability = FixType(1.0f);
  damageComponents_.back()[1].cProbability = FixType(1.0f);

  setCPluginSet();
}


void LegacyPkCUEngine::swapTeamIndexes() {
  std::swap(iTeams_[0], iTeams_[1]);
  std::swap(actions_[0], actions_[1]);

  setCPluginSet();
}

/**
 * @brief The main entry point for the engine, simulating a single turn.
 *
 * This method orchestrates the entire process of a battle turn. It first
 * determines the move priority to decide which Pokemon acts first. It then
 * calls `updateState_move` to process both Pokemon's moves. If there's a
 * speed tie, it creates two separate scenarios, one for each Pokemon moving
 * first. Finally, it evaluates end-of-round effects and combines similar
 * resulting environments.
 */
void LegacyPkCUEngine::updateState() {
  // determine who moves first
  uint32_t priority = movePriority();

  switch(priority) {
  case TEAM_B:
    // swap the indexes, as TEAM_B is moving first:
    swapTeamIndexes();
  default:
  case TEAM_A:
    getBase().setMovedFirst(priority);
    updateState_move();
    break;
  case 2: {
      // both teams are moving:
      std::array<size_t, 2> iStages;
      duplicateState(iStages, FixType(0.5));

      // first team moves first:
      iBase_ = iStages[0];
      getBase().setMovedFirst(TEAM_A);
      updateState_move();

      // swap indexes:
      iBase_ = iStages[1];
      //swapTeamIndexes(); (updateState_move swaps team indexes but does not swap them back)
      getBase().setMovedFirst(TEAM_B);

      // second team moves first:
      updateState_move();
      break;
    }
  }

  // compute end of round component:
  evaluateRound_end();

  // combine environments that equal eachother:
  combineSimilarEnvironments();
}


uint32_t LegacyPkCUEngine::movePriority_Speed() {
  PokemonVolatile cPKV = getPKV();

  // grab FV_boosted speed
  uint32_t cSpeed = cPKV.getFV_boosted(FV_SPEED);

  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_MODIFYSPEED, onModifySpeed_rawType,
    *this, cPKV, cSpeed);

  return cSpeed;
}


int32_t LegacyPkCUEngine::movePriority_Bracket() {
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

  // if the pokemon is switching out, its move priority is +6
  switch(actions_[0].type()) {
    case Action::MOVE_SWITCH:
      actionResult = 6;
      break;
    case Action::MOVE_0:
    case Action::MOVE_1:
    case Action::MOVE_2:
    case Action::MOVE_3:
    {
      // if the pokemon is performing a move, find the move's priority
      MoveVolatile mv = getMV();
      actionResult = mv.getBase().getSpeedPriority();

      //script - modify movePriority - action
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_SETSPEEDBRACKET, onModifyBracket_rawType,
          *this, mv, getPKV(), actionResult);
      break;
    }
    case Action::MOVE_WAIT:
      actionResult = -7;
      break;
    case Action::MOVE_STRUGGLE:
    default:
      actionResult = 0;
      break;
  }

  return actionResult;
}


uint32_t LegacyPkCUEngine::movePriority() {
  std::array<MoveBracket, 2> moveBracket;

  size_t iCTeam = getICTeam();
  size_t iOTeam = getIOTeam();

  // determine speed brackets of the move
  for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
    moveBracket[iTeam].actionBracket = movePriority_Bracket();

    swapTeamIndexes();
  }

  // exceptions to actionBracket? (pursuit if switch is used)


  // are the priority brackets equal? If so, use speed as determining factor
  if (moveBracket[iCTeam].actionBracket > moveBracket[iOTeam].actionBracket) {
    return iCTeam;
  } else if (moveBracket[iCTeam].actionBracket < moveBracket[iOTeam].actionBracket) {
    return iOTeam;
  } else { // speed bracket tie, determine speeds

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


void LegacyPkCUEngine::evaluateRound_end() {
  iBase_ = 0;

  for (size_t iSize = getStack().size(); iBase_ != iSize; ++iBase_)
  {
    // do not allow for partially completed stages in this step of computation:
    assert(getStackStage() == STAGE_POSTROUND);
    advanceStackStage();

    // ignore end of move environments if we're calculating a dummy move, such as a switch in upon death
    if (getBase().hasFreeMove(TEAM_A) || getBase().hasFreeMove(TEAM_B)) { continue; }

    for (size_t iTeam = 0; iTeam != 2; ++iTeam)
    {
      PokemonVolatile pkv = getPKV();
      // do not call plugin if current pokemon is dead
      if (!pkv.isAlive()) { continue; }

      // parse end of round plugins:
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_ENDOFROUND, onEndOfRound_rawType,
          *this, pkv);

      swapTeamIndexes();
    }// endOf per team
  } // endOf per base
} // endOf evaluateRound_end

/**
 * @brief Evaluates a single Pokemon's move for the current turn.
 *
 * This is the central function for processing a single action. It handles
 * different action types (switch, wait, or move) and progresses the state
 * machine through the appropriate stages. For a standard move, it will call
 * pre-move status checks, then dispatch to either `evaluateMove_damage` for
 * damaging moves or `evaluateMove_script` for other moves, and finally handle
 * post-move effects.
 */
void LegacyPkCUEngine::evaluateMove() {
  // NOTE: ONLY ONE stage is set to preturn at a time
  assert(getStackStage() == STAGE_PRETURN);
  Action cAction = getCAction();
  size_t iCTeam = getICTeam();
  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size(), iNBase;

  // TODO: does this model the actual game?
  // if either pokemon is dead at this point, the only valid moves are switching and waiting
  if ( (!getPKV().isAlive() || !getTPKV().isAlive()) && cAction.isMove() ) {
    cAction = Action::wait();
  }

  // if the current pokemon has been switched out, its move should be canceled
  // We skip evaluation entirely to avoid illegal move access and incorrect wait
  // flags.
  if (getBase().hasSwitched(iCTeam) && cAction.isMove()) {
    stackStage_[iBase_] = STAGE_POSTTURN;
    evaluateMove_postTurn();
    return;
  }

  // Pre-move script: modify action?
  if (cAction.isMove()) {
    int result = 0;
    CALLPLUGIN(
        result, PLUGIN_ON_MODIFYACTION, onModifyAction_rawType, *this, cAction);

    // If the plugin changed the action, we must validate it.
    if (cAction != getCAction()) {
      auto validation = cu_.isValidAction(getBase().getEnv(), cAction, iCTeam);
      if (!validation) { cAction = Action::struggle(); }
      // Update the action in the engine's internal state
      actions_[0] = cAction;

      // Set blocked if we want the environment to reflect that the original
      // choice was preempted
      getBase().setBlocked(iCTeam);
    }
  }


  // does this move require a switch-out?
  if (cAction.isSwitch()) {
    stackStage_[iBase_] = STAGE_PRESWITCH;
    evaluateMove_switch();
    // end of is Switch type action
  } else if (cAction.isWait()) { // is this pokemon doing nothing?
    stackStage_[iBase_] = STAGE_POSTSECONDARY;

    // set that the current team did nothing this turn:
    getBase().setWaited(iCTeam);
    // pokemon performs no action, no update to the state is needed
    // end of is Wait type action
  } else if (cAction.isMove()) { // is the pokemon moving normally?
    assert(getPKV().isAlive() && getTPKV().isAlive());

    // this is the first function which may generate more than one state of STAGE_STATUS type
    {
      stackStage_[iBase_] = STAGE_STATUS;
      evaluateMove_preMove();
    }

    // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

    const Move& cMove = getMV().getBase();
    void (LegacyPkCUEngine::*evaluate_t)();
    if ( cMove.damageType_ == ATK_PHYSICAL || cMove.damageType_ == ATK_SPECIAL)
    { evaluate_t = &LegacyPkCUEngine::evaluateMove_damage;}
    else
    { evaluate_t = &LegacyPkCUEngine::evaluateMove_script;}

    // evaluate either move or plugin move: (increment with iNBase, as evaluateMove_damage will manipulate stack)
    for (iNBase = baseFloor, iBase_ = iNBase, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase_ = iNBase) {
      if (getStackStage() != STAGE_STATUS) { continue; }
      advanceStackStage();

      // was this move blocked by a status?
      // did this pokemon die from the last pokemon's action?
      if (getBase().wasBlocked(iCTeam) || !getPKV().isAlive()) { stackStage_[iBase_] = STAGE_POSTTURN; continue; }

      // evaluate either evaluateMove_damage from stackstage movebase, or evaluateMove_script from stackstage postmove
      (this->*evaluate_t)();
    }

    // for all worlds: (increment with iNBase, as evaluateMove_postMove will manipulate stack)
    for (iNBase = baseFloor, iBase_ = iNBase, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase_ = iNBase) {
      if (getStackStage() != STAGE_POSTDAMAGE) { continue; }
      advanceStackStage();

      evaluateMove_postMove();
    } // endOf forEach environment
  } // endOf isMove

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  // end of turn occurences:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_POSTSECONDARY) { continue; }
    stackStage_[iBase_] = STAGE_POSTTURN;

    evaluateMove_postTurn();
  }

  return;
} // end of evaluateMove


void LegacyPkCUEngine::evaluateMove_switch()
{
  assert(getStackStage() == STAGE_PRESWITCH);

  size_t iCTeam = getICTeam();
  const Action& cAction = getCAction();
  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size();

  //for (/*iBase = baseFloor, baseCeil = getStack().size()*/; iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_PRESWITCH) { continue; }
    advanceStackStage();

    // set that we are switching this environment:
    getBase().setSwitched(iCTeam);

    // is the switching out pokemon dead? If so, this is a free move
    if (!getPKV().isAlive()) {
      getBase().setFreeMove(iCTeam);
    } else {
      // pre-move switch scripts:
      int result = 0;
      CALLPLUGIN(result, PLUGIN_ON_SWITCHOUT, onSwitch_rawType,
          *this, getPKV());
    }
  } // endOf switchout script

  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_POSTSWITCH) { continue; }
    stackStage_[iBase_] = STAGE_POSTSECONDARY;

    // switch out
    getTV().swapPokemon(cAction.iFriendly());

    // set the current array of plugins:
    setCPluginSet();

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SWITCHIN, onSwitch_rawType,
        *this, getPKV());

  } // end of switchin script
} // endOf evaluateMove_switch


void LegacyPkCUEngine::evaluateMove_preMove() {
  assert(getStackStage() == STAGE_STATUS);

  // parse beginning of turn plugins:
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_BEGINNINGOFTURN, onBeginningOfTurn_rawType,
      *this, getPKV());
}


void LegacyPkCUEngine::evaluateMove_postMove() {
  assert(getStackStage() == STAGE_POSTMOVE);

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size();

  // the current environment we are evaluating upon. Will be copied, modified and pushed back eventually
  const Move& cMove = getMV().getBase();
  size_t iCTeam = getICTeam();

  // effects which occur regardless of a secondary effect occuring, but only if the move hit:
  //for (/*iBase = baseFloor, baseCeil = getStack().size()*/; iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_POSTMOVE) { continue; }
    advanceStackStage();

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_ENDOFMOVE, onEvaluateMove_rawType,
        *this, getMV(), getPKV(), getTPKV());
  }

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  // calculate probability to perform secondary:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_PRESECONDARY) { continue; }
    advanceStackStage();

    // special behavior if the teammate is dead: (cannot compute secondary effects)
    if (!getPKV().isAlive()) { stackStage_[iBase_] = STAGE_POSTTURN; continue; }

    // does this move even have a secondary effect? (this check is in-loop because multiple environments could arise from previous call)
    if (!(cMove.getSecondaryAccuracy() > FixType(0))) {
      stackStage_[iBase_] = STAGE_POSTSECONDARY;
      continue;
    }

    FixType& secondaryHitProbability = getDamageComponent().tProbability;

    /* probability to inflict secondary condition*/
    secondaryHitProbability = cMove.getSecondaryAccuracy(); // lowest is 10%

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYSECONDARYPROBABILITY,
        onModifyProbability_rawType,
        *this,
        getMV(),
        getPKV(),
        getTPKV(),
        secondaryHitProbability);
  } // endOf calculate secondary probability

  // split environments based on their secondary chance:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYSECONDARYHITCHANCE) { continue; }
    advanceStackStage();

    std::array<size_t, 2> iREnv = {{ getIBase() , SIZE_MAX }};

    FixType& secondaryHitProbability = getDamageComponent().tProbability;

    secondaryHitProbability =
        std::max(std::min(secondaryHitProbability, FixType(1)), FixType(0));

    // did the ability hit its target? Is it possible for the secondary ability to miss?
    if (getBase().hasHit(iCTeam) &&
        mostlyGT(secondaryHitProbability, FixType(0))) {
      // if there's a chance the secondary effect will not occur:
      if (mostlyLT(secondaryHitProbability, FixType(1))) {
        // duplicate the environment (duplicated environment is the secondary effect missed):
        duplicateState(iREnv, (FixType(1) - secondaryHitProbability));
      }

      // modify bitmask as secondary effect occuring:
      getStack().at(iREnv[0]).setSecondary(iCTeam);

    } else {  // end of primary attack hits, and secondary attack is not assured
      // pass-through: no chance to secondary
      stackStage_[iBase_] = STAGE_POSTSECONDARY;
      continue;
    }
  }

  // calculate effect of secondary, if it occured:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_)
  {
    if (getStackStage() != STAGE_SECONDARY) { continue; }
    advanceStackStage();

    // add extra effects to the move, such as secondaries and trigger effects
    if (!getBase().hasSecondary(iCTeam)) { continue; }

    // parse secondary effect plugins:
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SECONDARYEFFECT, onEvaluateMove_rawType,
        *this, getMV(), getPKV(), getTPKV());
  } // endOf if primary and secondary attacks hit
} // endOf evaluateMove_postMove


void LegacyPkCUEngine::evaluateMove_postTurn() {
  // parse end of turn plugins:
  int result = 0;
  CALLPLUGIN(result, PLUGIN_ON_ENDOFTURN, onEndOfTurn_rawType,
      *this, getPKV());
};


/**
 * @brief Evaluates the damage of a move according to the standard formula.
 *
 * This function implements the damage calculation process as a multi-stage
 * state machine. It follows the formula used in the Pokemon games, which is
 * detailed on sites like Smogon. The calculation is broken down into the
 * following steps:
 * 1.  Set the base power of the move.
 * 2.  Determine the move's type.
 * 3.  Modify the base power with plugins (e.g., for abilities like Technician).
 * 4.  Calculate the base damage using the attacker's Attack/Special Attack and
 *     the defender's Defense/Special Defense.
 * 5.  Apply modifiers for critical hits, random damage rolls, STAB, type
 *     effectiveness, and items.
 * 6.  Calculate the probability of the move hitting and create separate
 *     environments for hit and miss scenarios.
 * 7.  Calculate the probability of a critical hit and create separate
 *     environments for crit and non-crit scenarios.
 * 8.  Apply the final calculated damage to the target Pokemon.
 */
void LegacyPkCUEngine::evaluateMove_damage() {
  assert(getStackStage() == STAGE_MOVEBASE);
  assert(getTPKV().isAlive() && getPKV().isAlive());

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size();

  const Move& cMove = getMV().getBase();
  getDamageComponent().category = cMove.getDamageType();

  stackStage_[iBase_] = STAGE_MODIFYHITCHANCE;

  //Source: http://www.smogon.com/dp/articles/damage_formula

  // calculate probability to hit, miss:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil;
       ++iBase_) {
    if (getStackStage() != STAGE_MODIFYHITCHANCE) { continue; }
    advanceStackStage();

    PokemonVolatile cPKV = getPKV();
    PokemonVolatile tPKV = getTPKV();
    MoveVolatile mV = getMV();
    FixType& probabilityToHit = getDamageComponent().tProbability;

    /* probability to hit enemy pokemon */
    probabilityToHit = getProbabilityToHit();

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYHITPROBABILITY,
        onModifyProbability_rawType,
        *this,
        mV,
        cPKV,
        tPKV,
        probabilityToHit);
  }

  // evaluate miss(1), hit(0):
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil;
       ++iBase_) {
    if (getStackStage() != STAGE_EVALUATEHITCHANCE) { continue; }
    advanceStackStage();

    FixType& probabilityToHit = getDamageComponent().tProbability;

    // bound at MIN 0.033~ ... MAX 1.0
    probabilityToHit =
        std::max(std::min(probabilityToHit, FixType(1)), FixType(0));

    std::array<size_t, 2> iHEnv = {{getIBase(), SIZE_MAX}};
    // did the move hit its target? Is it possible for the move to miss?
    if (mostlyGT(probabilityToHit, FixType(0))) {
      // if there's a chance the primary effect will not occur:
      if (mostlyLT(probabilityToHit, FixType(1))) {
        // duplicate the environment (duplicated environment is the miss
        // environment):
        duplicateState(iHEnv, (FixType(1) - probabilityToHit));
      }

      // modify bitmask as the hit effect occuring:
      getStack().at(iHEnv[0]).setHit(getICTeam());

    } else {  // end of primary attack hits, and secondary attack is not assured
      // pass-through: no chance to hit or crit
      stackStage_[iBase_] = STAGE_POSTDAMAGE;
    }
  }

  // calculate probability to crit:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil;
       ++iBase_) {
    if (getStackStage() != STAGE_MODIFYCRITCHANCE) { continue; }
    advanceStackStage();

    // don't continue to evaluate a stage that will not hit the enemy team:
    if (!getBase().hasHit(getICTeam())) {
      stackStage_[iBase_] = STAGE_POSTDAMAGE;
      continue;
    }

    PokemonVolatile cPKV = getPKV();
    FixType& probabilityToCrit = getDamageComponent().tProbability;

    /* Probability to critical hit, if the move has already hit */
    probabilityToCrit = cPKV.getAccuracy_boosted(FV_CRITICALHIT);

    // to-crit modifying values:
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYCRITPROBABILITY,
        onModifyProbability_rawType,
        *this,
        getMV(),
        cPKV,
        getTPKV(),
        probabilityToCrit);
  }

  // evaluate crit(2):
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil;
       ++iBase_) {
    if (getStackStage() != STAGE_EVALUATECRITCHANCE) { continue; }
    advanceStackStage();

    FixType& probabilityToCrit = getDamageComponent().tProbability;

    // determine the possibility that the move crit:
    std::array<size_t, 2> iCEnv = {{SIZE_MAX, getIBase()}};

    if (mostlyGT(probabilityToCrit, FixType(0))) {
      if (mostlyLT(probabilityToCrit, FixType(1))) {
        // duplicate the environment (duplicated environment is the crit
        // environment):
        duplicateState(iCEnv, probabilityToCrit);
      }

      // modify bitmask as the crit effect occuring:
      getStack().at(iCEnv[1]).setCrit(getICTeam());
    }
    // even with no chance to crit there's still the possibility of damage
  }

  /*BasePower = HH × BP × IT × CHG × (MS × WS) × UA × FA*/
  // set basePower:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil;
       ++iBase_) {
    if (getStackStage() != STAGE_SETBASEPOWER) { continue; }
    advanceStackStage();

    uint32_t& basePower = getDamageComponent().damage;
    basePower = cMove.getPower();

    int result = (basePower != UINT8_MAX)?1:0;
    CALLPLUGIN(result, PLUGIN_ON_SETBASEPOWER, onSetPower_rawType,
        *this, getMV(), getPKV(), getTPKV(), basePower);

    assert(result > 0 && basePower > 0);
  }

  // calculate this move's type:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_SETMOVETYPE) { continue; }
    advanceStackStage();

    const Type*& cType = getDamageComponent().mType;
    cType = &cMove.getType();

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SETMOVETYPE, onModifyMoveType_rawType,
        *this, getMV(), getPKV(), getTPKV(), cType);
  }

  // modify basePower:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYBASEPOWER) { continue; }
    advanceStackStage();

    uint32_t& basePower = getDamageComponent().damage;
    fpType baseModifier = 1.0;

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYBASEPOWER, onModifyPower_rawType,
        *this, getMV(), getPKV(), getTPKV(), baseModifier);

    basePower = (uint32_t)(basePower * baseModifier);
  }

  // calculate attack and damage modifiers:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYATTACKPOWER) { continue; }
    advanceStackStage();

    PokemonVolatile cPKV = getPKV();
    PokemonVolatile tPKV = getTPKV();
    DamageComponents_t& cDamage = getDamageComponent();

    size_t attackType;
    size_t defenseType;
    if (cMove.getDamageType() == ATK_PHYSICAL) {
      attackType = FV_ATTACK; defenseType = FV_DEFENSE;
    } else {
      attackType = FV_SPATTACK; defenseType = FV_SPDEFENSE;
    }

    /* Mod1 = BRN × RL × TVT × SR × FF */
    // modifier1:
    fpType attackPowerModifier = 1.0;

    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYATTACKPOWER,
        onModifyPower_rawType,
        *this,
        getMV(),
        cPKV,
        tPKV,
        attackPowerModifier);

    // regular damage:
    uint32_t attackPower = cPKV.getFV_boosted(attackType);
    uint32_t defensePower = tPKV.getFV_boosted(defenseType);
    uint32_t levelModifier = ((cPKV.nv().getLevel() * 2) / 5) + 2;

    // has the pokemon crit?
    if (getBase().hasCrit(getICTeam())) {
      attackPower = std::max(cPKV.nv().getFV_base(attackType), attackPower);
      defensePower = std::min(tPKV.nv().getFV_base(defenseType), defensePower);
    }

    // incorporate attack power modifier:
    cDamage.damage =
        ((levelModifier * cDamage.damage * attackPower) / 50) / defensePower;
    cDamage.damage = (uint32_t)(cDamage.damage * attackPowerModifier) + 2;
  }

  // calculate critical hit modifiers:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYCRITICALPOWER) { continue; }
    advanceStackStage();

    // do nothing if the move didn't crit:
    if (!getBase().hasCrit(getICTeam())) { continue; }

    DamageComponents_t& cDamage = getDamageComponent();

    /* CH - Critical Hit modifier
      3 if has sniper ability AND critical hit (mult 1.5)
      2 if critical hit (mult 1.0)
      1 else
     */
    fpType criticalHitModifier = 2.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYCRITICALPOWER, onModifyPower_rawType,
        *this, getMV(), getPKV(), getTPKV(), criticalHitModifier);

    // incorporate critical power modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * criticalHitModifier);
  }

  // calculate raw damage modifiers:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYRAWDAMAGE) { continue; }
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
      *this, getMV(), getPKV(), getTPKV(), rawDamageMultiplier);

    // incorporate raw damage modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * rawDamageMultiplier);
  }

  // calculate this move's STAB:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYSTAB) { continue; }
    advanceStackStage();

    PokemonVolatile cPKV = getPKV();
    DamageComponents_t& cDamage = getDamageComponent();

    bool hasStab = (
        (&cPKV.getBase().getType(0) == cDamage.mType) ||
        (&cPKV.getBase().getType(1) == cDamage.mType));
    fpType STABMultiplier = hasStab?1.5:1.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYSTAB, onModifyPower_rawType,
        *this, getMV(), cPKV, getTPKV(), STABMultiplier);

    // incorporate STAB modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * STABMultiplier);
  }

  // calculate the enemy pokemon's type resistance:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYTYPERESISTANCE) { continue; }
    advanceStackStage();

    PokemonVolatile tPKV = getTPKV();
    DamageComponents_t& cDamage = getDamageComponent();

    fpType typeModifier = 1.0;
    {
      // type1:
      typeModifier *= cDamage.mType->getModifier(tPKV.getBase().getType(0));
      // type 2:
      typeModifier *= cDamage.mType->getModifier(tPKV.getBase().getType(1));
    }
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_SETDEFENSETYPE, onModifyTypePower_rawType,
        *this, *cDamage.mType, getMV(), getPKV(), getTPKV(), typeModifier);

    // incorporate type modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * typeModifier);
  }

  // calculate item resistance modifiers:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYITEMPOWER) { continue; }
    advanceStackStage();

    DamageComponents_t& cDamage = getDamageComponent();

    /* Mod3 = SRF × EB × TL × TRB */
    fpType itemModifier = 1.0;
    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_MODIFYITEMPOWER, onModifyPower_rawType,
      *this, getMV(), getPKV(), getTPKV(), itemModifier);

    // incorporate item modifier:
    cDamage.damage = (uint32_t)(cDamage.damage * itemModifier);
  }

  /* Damage Formula = (((((((Level × 2 ÷ 5) + 2) × BasePower × [Sp]Atk ÷ 50) ÷
   * [Sp]Def) × Mod1) + 2) × CH × Mod2 × R ÷ 100) × STAB × Type1 × Type2 × Mod3
   */
  // perform actual damage calculation
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_PREDAMAGE) { continue; }
    advanceStackStage();

    if (!getBase().hasHit(getICTeam())) { continue; }

    calculateDamage();
  }
} // end of evaluateMove_damage


void LegacyPkCUEngine::evaluateMove_script() {
  assert(getStackStage() == STAGE_MOVEBASE);
  assert(getTPKV().isAlive() && getPKV().isAlive());

  const Move& cMove = getMV().getBase();

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size();

  // calculate probability to hit, miss:
  //for (iBase = baseFloor, baseCeil = getStack().size(); iBase != baseCeil; ++iBase)
  {
    //if (getStackStage() != STAGE_MOVEBASE) { continue; }
    stackStage_[iBase_] = STAGE_MODIFYHITCHANCE;

    FixType& probabilityToHit = getDamageComponent().tProbability;

    /* probability to hit enemy pokemon */
    if (cMove.targetsEnemy()) {
      probabilityToHit = getProbabilityToHit();
    }
    else
    {
      // TODO(@drendleman) - the probabilityToHit of a move with no accuracy is always 100%
      //probabilityToHit = cMove.getPrimaryAccuracy(); // REMOVED DUE TO ABOVE TODO
      probabilityToHit = FixType(1);
    }

    // to-hit modifying values:
    int result = 0;
    CALLPLUGIN(
        result,
        PLUGIN_ON_MODIFYHITPROBABILITY,
        onModifyProbability_rawType,
        *this,
        getMV(),
        getPKV(),
        getTPKV(),
        probabilityToHit);
  }

  // evaluate miss(1), hit(0),
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_MODIFYHITCHANCE) { continue; }
    stackStage_[iBase_] = STAGE_PREDAMAGE;

    FixType& probabilityToHit = getDamageComponent().tProbability;

    // bound at MIN 0.033~ ... MAX 1.0
    probabilityToHit =
        std::max(std::min(probabilityToHit, FixType(1)), FixType(0));

    std::array<size_t, 2> iHEnv = {{ getIBase(), SIZE_MAX }};
    // did the move hit its target? Is it possible for the move to miss?
    if (mostlyGT(probabilityToHit, FixType(0))) {
      // if there's a chance the primary effect will not occur:
      if (mostlyLT(probabilityToHit, FixType(1))) {
        // duplicate the environment (duplicated environment is the miss environment):
        duplicateState(iHEnv, (FixType(1) - probabilityToHit));
      }

      // modify bitmask as the hit effect occuring:
      getStack().at(iHEnv[0]).setHit(getICTeam());

    } else {  // end of primary attack hits, and secondary attack is not assured
      // pass-through: no chance to hit
      stackStage_[iBase_] = STAGE_POSTDAMAGE;
    }
  }

  // perform script:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_) {
    if (getStackStage() != STAGE_PREDAMAGE) { continue; }
    advanceStackStage();

    if (!getBase().hasHit(getICTeam())) { continue; }

    MoveVolatile mV = getMV();

    // parse alternative move plugins:
    int result = mV.getBase().isImplemented()?1:0; // TODO: this check isn't working!
    CALLPLUGIN(result, PLUGIN_ON_EVALUATEMOVE, onEvaluateMove_rawType,
        *this, mV, getPKV(), getTPKV());
  }

  return;
}


void LegacyPkCUEngine::calculateDamage() {
  FixType partitionEnvironmentProbability =
      (FixType(1) / (int32_t)cfg_.numRandomEnvironments);
  DamageComponents_t& cDMG = getDamageComponent();

  uint32_t power = cDMG.damage;

  std::array<size_t, 2> iREnv = {{ SIZE_MAX, getIBase() }};
  for (size_t iEnv = 0; iEnv != cfg_.numRandomEnvironments; ++iEnv ) {
    if (cfg_.numRandomEnvironments > 1) {
      if ((iEnv + 1) == cfg_.numRandomEnvironments) { iREnv[1] = getIBase(); }
      else { duplicateState(iREnv, partitionEnvironmentProbability); }
    };

    // find the mean random value for this partition of the random environment
    fpType randomValue =
        (iEnv / (fpType)cfg_.numRandomEnvironments) + ( (fpType)1.0 / (fpType)cfg_.numRandomEnvironments / (fpType)2.0);

    // scale our random value modifier to 0.85..1.0
    randomValue = deScale(randomValue, (fpType)1.0, (fpType)0.85);

    uint32_t& actualDamage = getDamageComponent(iREnv[1], getICTeam()).damage;
    actualDamage = (uint32_t)((fpType)power * randomValue);

    int result = 0;
    CALLPLUGIN(result, PLUGIN_ON_CALCULATEDAMAGE, onSetPower_rawType,
        *this, getMV(), getPKV(), getTPKV(), actualDamage);

    // inflict damage caused upon the targetPokemon:
    getTPKV(iREnv[1]).modHP(-1 * actualDamage);
  }
}  // end of evaluateMove_damage


FixType LegacyPkCUEngine::getProbabilityToHit() {
  PokemonVolatile cPKV = getPKV();
  PokemonVolatile tPKV = getTPKV();
  MoveVolatile mV = getMV();

  /* probability to hit enemy pokemon */
  // combine accuracy/evasion stages before look-up to ensure precision
  int32_t netBoost = cPKV.getBoost(FV_ACCURACY) - tPKV.getBoost(FV_EVASION);
  netBoost = std::min(std::max(netBoost, -6), 6);

  FixType probabilityToHit =
      // map net boost stage to precision look-up table
      PokemonNonVolatile::aFV_base[FV_ACCURACY - 6][netBoost + 6] *
      // lowest is 30% or 30 / 100
      mV.getBase().getPrimaryAccuracy();

  return probabilityToHit;
}


/**
 * @brief Combines environments on the stack that are identical.
 *
 * After all the branching from stochastic events, the stack may contain
 * multiple environments that are in the same state. This function identifies
 * these duplicates by hashing each environment and then merging the ones with
 * the same hash. The probabilities of the merged environments are summed up.
 * This is a crucial optimization to keep the number of possible environments
 * manageable.
 *
 * @return The number of unique environments remaining on the stack.
 */
size_t LegacyPkCUEngine::combineSimilarEnvironments() {
  PossibleEnvironments& stack = getStack();

  // hash environments (and summate probabilities for check):
  for (iBase_ = 0; iBase_ != stack.size(); ++iBase_) {
    EnvironmentPossible cEnvironment = getBase();

    // assert that each of these environments is getting hashed:
    assert(getStackStage() == STAGE_FINAL);
    advanceStackStage();

    // WARNING: EXPENSIVE!
    cEnvironment.data().generateHash();
  }

  size_t iSize = stack.size();
  std::unordered_map<uint64_t, size_t> envMap;
  envMap.reserve(iSize);

  // compare environment hashes:
  for (size_t iEnv = 0; iEnv != iSize; iEnv++)
  {
    EnvironmentPossible cEnv = stack.at(iEnv);

    // don't attempt to merge pruned environments
    if (cEnv.isPruned()) { continue; }

    uint64_t hash = cEnv.getHash();
    auto it = envMap.find(hash);

    if (it != envMap.end()) {
      // Found a duplicate! Merge into the existing environment
      size_t existIndex = it->second;
      EnvironmentPossible existEnv = stack.at(existIndex);

#ifdef _PKCUCHECKSIGNATURE
      // Assert that same hash implies same environment data
      assert((oEnv.hash == iEnv.hash) == (oEnv.env == iEnv.env));
#endif

      // combine the two environments by adding their probabilities
      existEnv.getProbability() += cEnv.getProbability();

      // this is probably not representative of the current environment now
      existEnv.getBitmask() &= cEnv.getBitmask();

      // flag the destination environment as merged
      existEnv.setMerged();

      // flag the current environment as pruned
      cEnv.setPruned();

      // decrement number of unique values in vector
      stack.decrementUnique();
    } else {
      // First time seeing this hash, add to map
      envMap[hash] = iEnv;
    }
  }

  // Calculate accumulated probability for verification
  assert(LegacyPkCU::saneStackProbability(stack));
  return stack.getNumUnique();
} //endOf combineSimilarEnvironments

/**
 * @brief Processes the moves of both Pokemon in the correct order.
 *
 * This method is responsible for executing the actions of both Pokemon for a
 * single turn. It first calls `evaluateMove` for the Pokemon that has priority,
 * then swaps the teams and iterates through all the resulting environments to
 * call `evaluateMove` for the second Pokemon. This ensures that the second
 * Pokemon's action is evaluated in the context of every possible outcome of the
 * first Pokemon's action.
 */
void LegacyPkCUEngine::updateState_move() {
  assert(getStackStage() == STAGE_SEEDED);
  advanceStackStage();

  // the floor of the stack: everything below this stack value has been evaluated
  size_t baseFloor = iBase_, baseCeil = getStack().size(), iNBase;

  // evaluate first pokemon's moves, and their probabilities (iBase provided by updateState)
  evaluateMove();

  // POSSIBLE THAT POKEMON MIGHT HAVE DIED IN PREVIOUS STEP

  iBase_ = baseFloor;
  swapTeamIndexes();

  // TODO: STAGE_POSTROUND should automatically be set after the second go-around as stage keeps incrementing
  // increment with iNBase, as evaluateMove will manipulate stack
  for (iNBase = baseFloor, baseCeil = getStack().size(); iNBase != baseCeil; ++iNBase, iBase_ = iNBase)
  {
    // do not evaluate a move that has not been evaluated by first pokemon yet:
    if (getStackStage() != STAGE_POSTTURN) { continue; }

    // if the pokemon died, no reason for it to take its turn
    //if (!getPKV().isAlive()) { continue; }

    // evaluate second pokemon's moves and their probabilities
    stackStage_[iBase_] = STAGE_PRETURN;
    evaluateMove();
  }

  // assert ALL moves have completed both turns:
  for (iBase_ = baseFloor, baseCeil = getStack().size(); iBase_ != baseCeil; ++iBase_)
  {
    if (getStackStage() != STAGE_POSTTURN) { continue; }
    advanceStackStage();
  }
}


PossibleEnvironments LegacyPkCU::updateState(
    const ConstEnvironmentVolatile& cEnv,
    const Action& actionA,
    const Action& actionB) const {
  PossibleEnvironments result;

  guardNonvolatileState(cEnv);
  guardOnly1v1Supported(cEnv);
  if (!cfg_.allowInvalidMoves) {
    auto reasonA = isValidAction(cEnv, actionA, TEAM_A);
    if (!reasonA) {
      throw std::runtime_error("Invalid Action for Team A: " + std::string(invalidActionReasonToString(reasonA)));
    }
    auto reasonB = isValidAction(cEnv, actionB, TEAM_B);
    if (!reasonB) {
      throw std::runtime_error("Invalid Action for Team B: " + std::string(invalidActionReasonToString(reasonB)));
    }
  }

  // construct an engine from the action + state combination:
  LegacyPkCUEngine engine{*this, result, cEnv.data(), actionA, actionB};
  // and evaluate the state
  engine.updateState();

  return result;
}; // end of updateState


MatchState LegacyPkCU::getGameState(const ConstEnvironmentVolatile& envV) const {
  guardNonvolatileState(envV);
  bool teamAisDead = !envV.getTeam(TEAM_A).isAlive();
  bool teamBisDead = !envV.getTeam(TEAM_B).isAlive();
  int status = (teamAisDead * 1) + (teamBisDead * 2);

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


ActionPairVector LegacyPkCU::getAllValidActions(
    const ConstEnvironmentVolatile& envV, size_t agentTeam) const {
  auto agentActions = getValidActions(envV, agentTeam);
  auto otherActions = getValidActions(envV, (agentTeam+1) % 2);
  ActionPairVector result; result.reserve(agentActions.size() * otherActions.size());
  for (auto agentMove: agentActions) {
    for (auto otherMove: otherActions) {
      result.push_back({agentMove, otherMove});
    }
  }
  // the list of valid actions should never be empty if the game is not over
  assert(!result.empty() || isGameOver(envV));
  return result;
}


ActionVector LegacyPkCU::getValidActionsInRange(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    size_t iFirst,
    size_t iLast) const {
  ActionVector result;
  result.reserve(
      (iLast - iFirst) + envV.getTeam(actor.iTeam()).nv().getNumTeammates());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = envV.teammate(actor);
  // foreach move type:
  for (size_t iType = iFirst; iType < iLast; ++iType) {
    Action action{iType};
    bool isMoveAction = action.isMove() && (action.iMove() < cPKV.nv().getNumMoves());
    bool isSwitchAction = action.isSwitch();
    bool targetsFriendly =
        (isMoveAction) ? cPKV.getMV(action).getBase().targetsAlly() : false;
    targetsFriendly |= isSwitchAction;
    size_t iFriendlyMin = targetsFriendly?(Action::FRIENDLY_0):(Action::FRIENDLY_DEFAULT);
    size_t iFriendlyMax = targetsFriendly?(Action::FRIENDLY_0 + cTV.nv().getNumTeammates()):(Action::FRIENDLY_DEFAULT);
    // TODO(@drendleman) - moves that target adjacent pokemon
    size_t iHostileMin = Action::HOSTILE_DEFAULT;
    size_t iHostileMax = Action::HOSTILE_DEFAULT;

    // foreach friendly:
    for (size_t iFriendly = iFriendlyMin; iFriendly <= iFriendlyMax; ++iFriendly) {
      // foreach hostile:
      for (size_t iHostile = iHostileMin; iHostile <= iHostileMax; ++iHostile) {
        // test if move is valid:
        action = Action{iType, iFriendly, iHostile};
        if (isValidAction(envV, actor, action)) {
          // and if so, add it to result vector
          result.push_back(action);
        }
      }
    }
  }

  return result;
}

/**
 * @brief Checks if a given action is valid for a team in the current state.
 *
 * This function determines if an action is legal according to the rules of
 * Pokemon. It checks various conditions based on the action type (move,
 * switch, etc.). For moves, it verifies that the Pokemon is alive, has PP for
 * the move, and that the target is valid. For switches, it ensures the targeted
 * Pokemon is a valid, living teammate. The function also allows plugins to
 * override the default behavior, enabling the implementation of complex
 * mechanics like Truant or Choice items.
 * @param envV The current volatile environment.
 * @param action The action to check.
 * @param iTeam The index of the team performing the action.
 * @return An `IsValidResult` object indicating if the action is valid and, if
 *         not, the reason why.
 */
IsValidResult LegacyPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV,
    const Action& action,
    size_t iTeam) const {
  return isValidAction(envV, Actor{iTeam, envV.getTeam(iTeam).getICPKV()}, action);
}


IsValidResult LegacyPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  guardNonvolatileState(envV);
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstTeamVolatile oTV = envV.getOtherTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());
  ConstPokemonVolatile tPKV = oTV.getPKV();

  switch (action.type()) {
    case Action::MOVE_0:
    case Action::MOVE_1:
    case Action::MOVE_2:
    case Action::MOVE_3: {
      // is this a valid move?
      if (action.iMove() >= cPKV.nv().getNumMoves()) {
        return IsValidResult::MOVE_INVALID;
      }

      // By default, allow moves
      ValidMoveSet doAllowMove((1 << VALID_MOVE_SIZE) - 1);

      // is the other pokemon alive?
      doAllowMove[VALID_MOVE_TARGET_ALIVE] = tPKV.isAlive();

      // is the pokemon we're currently using alive?
      doAllowMove[VALID_MOVE_SELF_ALIVE] = cPKV.isAlive();

      // is the pokemon we're currently using on the field?
      doAllowMove[VALID_MOVE_ACTOR_ACTIVE] = cPKV.isActive();

      // does the move we're using have any PP left?
      ConstMoveVolatile cMV = cPKV.getMV(action);
      doAllowMove[VALID_MOVE_HAS_PP] = cMV.hasPP();

      // if the target of the move is friendly, is the friendly pokemon alive?
      if (cMV.getBase().targetsAlly()) {
        // is this a valid friendly-targeting move?
        if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
          return IsValidResult::INVALID_FRIENDLY_TARGET;
        }
        ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());

        // is the friendly target alive?
        doAllowMove[VALID_MOVE_FRIENDLY_ALIVE] = fPKV.isAlive();

        // is the friendly target us?
        doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER] =
            action.iFriendly() != actor.iTeammate();
      } else if (action.friendlyTarget() != Action::FRIENDLY_DEFAULT) {
        return IsValidResult::INVALID_FRIENDLY_TARGET;
      }

      // Are we locked out of the current move?
      for (const auto& cPlugin :
           getCPluginSet(envV, actor.iTeam())[PLUGIN_ON_TESTMOVE]) {
        onTestMove_rawType pFunction = (onTestMove_rawType)cPlugin.pFunction;
        if (pFunction(cTV, cPKV, cMV, action, doAllowMove) > 1) { break; }
      }

      if (!doAllowMove[VALID_MOVE_TARGET_ALIVE]) {
        return IsValidResult::MOVE_TARGET_DEAD;
      }
      if (!doAllowMove[VALID_MOVE_SELF_ALIVE]) {
        return IsValidResult::MOVE_SELF_DEAD;
      }
      if (!doAllowMove[VALID_MOVE_ACTOR_ACTIVE]) {
        return IsValidResult::MOVE_ACTOR_NOT_ACTIVE;
      }
      if (!doAllowMove[VALID_MOVE_HAS_PP]) { return IsValidResult::MOVE_NO_PP; }
      if (cMV.getBase().targetsAlly()) {
        if (!doAllowMove[VALID_MOVE_FRIENDLY_ALIVE]) {
          return IsValidResult::MOVE_FRIENDLY_TARGET_DEAD;
        }
        if (!doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER]) {
          return IsValidResult::MOVE_FRIENDLY_TARGET_SELF;
        }
      }
      if (!doAllowMove[VALID_MOVE_SCRIPT]) {
        return IsValidResult::MOVE_LOCKED_BY_SCRIPT;
      }

      return IsValidResult::VALID;
    }
    case Action::MOVE_SWITCH: {
      // is the pokemon we're switching to a valid teammate?
      if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
        return IsValidResult::SWITCH_INVALID_POKEMON;
      }

      // By default, allow switches
      ValidSwapSet doAllowSwitch((1 << VALID_SWAP_SIZE) - 1);

      // are we trying to switch to ourself?
      doAllowSwitch[VALID_SWAP_FRIENDLY_IS_OTHER] =
          action.iFriendly() != actor.iTeammate();

      // is the pokemon we're switching to even alive?
      ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());
      doAllowSwitch[VALID_SWAP_FRIENDLY_ALIVE] = fPKV.isAlive();

      // is the pokemon we're switching to already on the field?
      doAllowSwitch[VALID_SWAP_TARGET_INACTIVE] = !fPKV.isActive();

      // are we trying to move during the other team's free move?
      doAllowSwitch[VALID_SWAP_MUST_WAIT] = tPKV.isAlive() || !cPKV.isAlive();

      // Are we locked out of switching?
      for (const auto& cPlugin :
           getCPluginSet(envV, actor.iTeam())[PLUGIN_ON_TESTSWITCH]) {
        onTestSwitch_rawType pFunction = (onTestSwitch_rawType)cPlugin.pFunction;
        if (pFunction(cPKV, fPKV, action, doAllowSwitch) > 1) { break; }
      }

      if (!doAllowSwitch[VALID_SWAP_FRIENDLY_IS_OTHER]) {
        return IsValidResult::SWITCH_TO_SELF;
      }
      if (!doAllowSwitch[VALID_SWAP_TARGET_INACTIVE]) {
        return IsValidResult::SWITCH_ACTIVE_POKEMON;
      }
      if (!doAllowSwitch[VALID_SWAP_FRIENDLY_ALIVE]) {
        return IsValidResult::SWITCH_POKEMON_DEAD;
      }
      if (!doAllowSwitch[VALID_SWAP_MUST_WAIT]) {
        return IsValidResult::SWITCH_MUST_WAIT;
      }
      if (!doAllowSwitch[VALID_SWAP_SCRIPT]) {
        return IsValidResult::SWITCH_LOCKED_BY_SCRIPT;
      }

      return IsValidResult::VALID;
    }
    case Action::MOVE_WAIT:
      // are we waiting for the other team to take its free move?
      if (!(tPKV.isAlive()) && cPKV.isAlive()) { return IsValidResult::VALID; }

      // in most cases, do not allow not moving
      return IsValidResult::WAIT_NOT_ALLOWED;
    case Action::MOVE_STRUGGLE:
      // is the other pokemon alive?
      if (!(tPKV.isAlive())) { return IsValidResult::MOVE_TARGET_DEAD; }

      // is the pokemon we're currently using alive?
      if (!cPKV.isAlive()) { return IsValidResult::MOVE_SELF_DEAD; }

      // are all other moves unusable?
      for (size_t iMove = 0, iSize = cPKV.nv().getNumMoves(); iMove != iSize;
           ++iMove) {
        const Move& move = cPKV.nv().getMove_base(iMove);

        if (move.targetsAlly()) {
          for (size_t iFriendly = 0, numTeammates = cTV.nv().getNumTeammates();
               iFriendly != numTeammates;
               ++iFriendly) {
            if (isValidAction(envV, actor, Action::moveAlly(iMove, iFriendly))) {
              return IsValidResult::STRUGGLE_NOT_ALLOWED;
            }
          }
        } else {
          if (isValidAction(envV, actor, Action::move(iMove))) {
            return IsValidResult::STRUGGLE_NOT_ALLOWED;
          }
        }
      }

      // may struggle when all other moves are unusable:
      return IsValidResult::VALID;
    default:  // disabled action types (item use):
      return IsValidResult::ACTION_TYPE_DISABLED;
  }
} // endOf is valid action


void LegacyPkCU::guardNonvolatileState(const ConstEnvironmentVolatile& cEnv) const {
  if (cEnv.nv_ != nv_.get()) {
    throw std::runtime_error("mismatched nonvolatile state - call setEnvironment first");
  }
}


void LegacyPkCU::guardOnly1v1Supported(
    const ConstEnvironmentVolatile& cEnv) const {
  if (cEnv.getNumActivePokemon() != 2) {
    throw std::runtime_error("only 1v1 environments are supported!");
  }
}


bool LegacyPkCU::saneStackProbability(PossibleEnvironments& envs) {
  FixType sum = FixType(0);
  for (auto begin = envs.begin(), end = envs.end(); begin != end; ++begin) {
    auto probability = begin->getProbability();
    if (begin->isPruned()) { continue; }

    sum += probability;
    if (!(probability > FixType(0)) || !(probability <= FixType(1))) {
      return false;
    }
  }

  return sum == FixType(1);
}

/**
 * @brief Duplicates an environment on the stack to represent two possible outcomes.
 *
 * This function takes a single environment and splits it into two, each with a
 * different probability. This is used to model stochastic events, such as a
 * move hitting or missing.
 *
 * @param result An array to store the indices of the two resulting environments.
 * @param _probability The probability of the second outcome. The probability of
 *        the first outcome is calculated as `1.0 - _probability`.
 * @param iState The index of the environment to duplicate.
 */
void LegacyPkCUEngine::duplicateState(
    std::array<size_t, 2>& result, FixType _probability, size_t iState) {
  assert(_probability > FixType(0) && _probability < FixType(1));

  // duplicate state 2 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType branchProbability = totalProbability * _probability;

  assert(branchProbability > FixType(0));
  assert(branchProbability < totalProbability);

  getBase(result[1]).getProbability() = branchProbability;
  getBase(result[0]).getProbability() = totalProbability - branchProbability;

  assert(LegacyPkCU::saneStackProbability(getStack()));
}


/**
 * @brief Duplicates an environment on the stack to represent three possible outcomes.
 *
 * This function is similar to `duplicateState` but creates three environments
 * from a single one. This is used for events with three possible outcomes.
 *
 * @param result An array to store the indices of the three resulting environments.
 * @param _probability The probability of the second outcome.
 * @param _oProbability The probability of the third outcome.
 * @param iState The index of the environment to triplicate.
 */
void LegacyPkCUEngine::triplicateState(
    std::array<size_t, 3>& result,
    FixType _probability,
    FixType _oProbability,
    size_t iState) {
  assert(
      _probability > FixType(0) && _oProbability > FixType(0) &&
      (_probability + _oProbability) < FixType(1));

  // duplicate state 3 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType branchBProbability = totalProbability * _probability;
  FixType branchCProbability = totalProbability * _oProbability;
  FixType remainingProbability =
      totalProbability - branchBProbability - branchCProbability;

  // Branch B
  assert(branchBProbability > FixType(0));
  assert(branchBProbability < (totalProbability - branchCProbability));
  getBase(result[1]).getProbability() = branchBProbability;

  // Branch C
  assert(branchCProbability > FixType(0));
  assert(branchCProbability < (totalProbability - branchBProbability));
  getBase(result[2]).getProbability() = branchCProbability;

  // Branch A (Original)
  assert(remainingProbability > FixType(0));
  getBase(result[0]).getProbability() = remainingProbability;

  assert(LegacyPkCU::saneStackProbability(getStack()));
}


PokemonVolatile LegacyPkCUEngine::getPKV(size_t iState) {
  return getTV(iState).getPKV();
}


PokemonVolatile LegacyPkCUEngine::getTPKV(size_t iState) {
  return getTTV(iState).getPKV();
}


TeamVolatile LegacyPkCUEngine::getTV(size_t iState) {
  return getStack().at(iState).getTeam(getICTeam());
}


TeamVolatile LegacyPkCUEngine::getTTV(size_t iState) {
  return getStack().at(iState).getTeam(getIOTeam());
}


MoveVolatile LegacyPkCUEngine::getMV(size_t iState) {
  return getPKV(iState).getMV(getCAction());
}


MoveVolatile LegacyPkCUEngine::getTMV(size_t iState) {
  return getPKV(iState).getMV(getOAction());
}


const PluginSet& LegacyPkCUEngine::getCPluginSet() { return *cPluginSet_; }


const PluginSet& LegacyPkCU::getCPluginSet(
    const ConstEnvironmentVolatile& cEnv, size_t iTeam) const {
  size_t iCPokemon = cEnv.getTeam(iTeam).getICPKV();
  size_t iOPokemon = cEnv.getOtherTeam(iTeam).getICPKV();
  return pluginSets_[iTeam * 6 + iCPokemon][iOPokemon];
}


void LegacyPkCUEngine::setCPluginSet() {
  size_t iCPokemon = getBase().getTeam(getICTeam()).getICPKV();
  size_t iOPokemon = getBase().getTeam(getIOTeam()).getICPKV();
  cPluginSet_ = &pluginSets_[getICTeam() * 6 + iCPokemon][iOPokemon];
}
