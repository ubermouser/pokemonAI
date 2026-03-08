#include "pokemonai/neo_pkCU_engine.h"

#include <spdlog/spdlog.h>

#include <algorithm>

#include "pokemonai/neo_pkCU.h"
#include "pokemonai/pluggable_types.h"

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
 * @param retValue The variable to store the combined return values of the
 * plugins.
 * @param pluginType The type of plugin to call (e.g., `PLUGIN_ON_MODIFYSPEED`).
 * @param pluginFunction The function signature of the plugin to be called.
 * @param ... The arguments to pass to the plugin function.
 */
#define CALLPLUGIN(retValue, pluginType, pluginFunction, ...)        \
  {                                                                  \
    const std::vector<plugin_t>& cPlugins =                          \
        getCPluginSet()[(size_t)pluginType];                         \
    for (auto iPlugin = cPlugins.cbegin(), iPSize = cPlugins.cend(); \
         iPlugin != iPSize;                                          \
         ++iPlugin) {                                                \
      pluginFunction cPlugin = (pluginFunction)iPlugin->pFunction;   \
      retValue = retValue | cPlugin(__VA_ARGS__);                    \
      if (retValue > 1) { break; }                                   \
    }                                                                \
  }


NeoPkCUEngine::NeoPkCUEngine(
    const NeoPkCU& cu,
    const EnvironmentVolatileData& initial,
    const ActionMap& actions)
    : cu_(cu), cPlugins_(nullptr), actions_(actions), iBase_(0) {
  stack_.setNonvolatileEnvironment(cu.nv_);
  stack_.push_back(EnvironmentPossibleData::create(initial, false));
}


PossibleEnvironments NeoPkCUEngine::updateState() {
  throw std::runtime_error("NeoPkCUEngine::updateState not implemented");
}


uint32_t NeoPkCUEngine::computeSpeed(const Actor& actor) {
  PokemonVolatile pkv = getBase().getEnv().teammate(actor);
  uint32_t speed = pkv.getFV_boosted(FV_SPEED);

  int result = 0;
  CALLPLUGIN(
      result, PLUGIN_ON_MODIFYSPEED, onModifySpeed_rawType, *this, pkv, speed);

  return speed;
}


NeoPkCUEngine::MoveBracket NeoPkCUEngine::computeMoveBracket(
    const Actor& actor) {
  const Action& action = actions_.at(actor);
  int32_t actionBracket = 0;

  if (action.isSwitch()) {
    actionBracket = 6;
  } else if (action.isMove()) {
    PokemonVolatile pkv = getBase().teammate(actor);
    if (action.isStruggle()) {
      actionBracket = 0;
    } else {
      MoveVolatile mv = pkv.getMV(action);
      actionBracket = mv.getBase().getSpeedPriority();

      int result = 0;
      CALLPLUGIN(
          result,
          PLUGIN_ON_SETSPEEDBRACKET,
          onModifyBracket_rawType,
          *this,
          mv,
          pkv,
          actionBracket);
    }
  } else if (action.isWait()) {
    actionBracket = -7;
  }

  uint32_t speed = computeSpeed(actor);
  return {actionBracket, speed};
}


std::unordered_map<Actor, NeoPkCUEngine::MoveBracket>
NeoPkCUEngine::computeMoveBrackets() {
  std::unordered_map<Actor, MoveBracket> results;
  std::vector<Actor> active = getBase().getEnv().getActivePokemon();
  for (const auto& actor : active) {
    results[actor] = computeMoveBracket(actor);
  }
  return std::move(results);
}


std::vector<Actor> NeoPkCUEngine::computeActorOrder() {
  auto brackets = computeMoveBrackets();
  std::vector<Actor> actors = getBase().getEnv().getActivePokemon();

  std::sort(actors.begin(), actors.end(), [&](const Actor& a, const Actor& b) {
    const auto& bracketA = brackets.at(a);
    const auto& bracketB = brackets.at(b);
    if (bracketA.actionBracket != bracketB.actionBracket) {
      return bracketA.actionBracket > bracketB.actionBracket;
    }
    return bracketA.speed > bracketB.speed;
  });

  return std::move(actors);
}


/**
 * @brief Duplicates an environment on the stack to represent two possible
 * outcomes.
 *
 * This function takes a single environment and splits it into two, each with a
 * different probability. This is used to model stochastic events, such as a
 * move hitting or missing.
 *
 * @param result An array to store the indices of the two resulting
 * environments.
 * @param _probability The probability of the second outcome. The probability of
 *        the first outcome is calculated as `1.0 - _probability`.
 * @param iState The index of the environment to duplicate.
 */
void NeoPkCUEngine::duplicateState(
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

  assert(saneStackProbability());
}


/**
 * @brief Duplicates an environment on the stack to represent three possible
 * outcomes.
 *
 * This function is similar to `duplicateState` but creates three environments
 * from a single one. This is used for events with three possible outcomes.
 *
 * @param result An array to store the indices of the three resulting
 * environments.
 * @param _probability The probability of the second outcome.
 * @param _oProbability The probability of the third outcome.
 * @param iState The index of the environment to triplicate.
 */
void NeoPkCUEngine::triplicateState(
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

  assert(saneStackProbability());
}


bool NeoPkCUEngine::saneStackProbability() const {
  FixType sumOfStateProbabilities = FixType(0);
  for (auto begin = stack_.begin(), end = stack_.end(); begin != end; ++begin) {
    auto probability = begin->getProbability();
    if (begin->isPruned()) { continue; }

    sumOfStateProbabilities += probability;
    if (!(probability > FixType(0)) || !(probability <= FixType(1))) {
      return false;
    }
  }

  return sumOfStateProbabilities == FixType(1);
}


void NeoPkCUEngine::updateState_move() {
  throw std::runtime_error("NeoPkCUEngine::updateState_move not implemented");
}


int32_t NeoPkCUEngine::movePriority_Bracket() {
  throw std::runtime_error(
      "NeoPkCUEngine::movePriority_Bracket not implemented");
}


uint32_t NeoPkCUEngine::movePriority_Speed() {
  throw std::runtime_error("NeoPkCUEngine::movePriority_Speed not implemented");
}


void NeoPkCUEngine::evaluateMove_switch() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_switch not implemented");
}


void NeoPkCUEngine::evaluateMove_preMove() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_preMove not implemented");
}


void NeoPkCUEngine::evaluateMove_postMove() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_postMove not implemented");
}


void NeoPkCUEngine::evaluateMove_damage() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_damage not implemented");
}


void NeoPkCUEngine::evaluateMove_script() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_script not implemented");
}


void NeoPkCUEngine::evaluateMove_postTurn() {
  throw std::runtime_error(
      "NeoPkCUEngine::evaluateMove_postTurn not implemented");
}


void NeoPkCUEngine::evaluateRound_end() {
  throw std::runtime_error("NeoPkCUEngine::evaluateRound_end not implemented");
}


size_t NeoPkCUEngine::combineSimilarEnvironments() {
  throw std::runtime_error(
      "NeoPkCUEngine::combineSimilarEnvironments not implemented");
}


uint32_t NeoPkCUEngine::movePriority() {
  throw std::runtime_error("NeoPkCUEngine::movePriority not implemented");
}


void NeoPkCUEngine::evaluateMove() {
  throw std::runtime_error("NeoPkCUEngine::evaluateMove not implemented");
}


void NeoPkCUEngine::calculateDamage() {
  throw std::runtime_error("NeoPkCUEngine::calculateDamage not implemented");
}


FixType NeoPkCUEngine::getProbabilityToHit() {
  throw std::runtime_error(
      "NeoPkCUEngine::getProbabilityToHit not implemented");
}


void NeoPkCUEngine::duplicateState(
    std::array<size_t, 2>& result, fpType probability, size_t iState) {
  duplicateState(result, FixType(probability), iState);
}


uint32_t NeoPkCUEngine::getStackStage() const {
  return (uint32_t)stackFrame_[iBase_].stage;
}


void NeoPkCUEngine::advanceStackStage() {
  stackFrame_[iBase_].stage =
      (StageType)((uint32_t)stackFrame_[iBase_].stage + 1);
}


void NeoPkCUEngine::swapTeamIndexes() {
  throw std::runtime_error("NeoPkCUEngine::swapTeamIndexes not implemented");
}


const PluginSet& NeoPkCUEngine::getCPluginSet() {
  if (cPlugins_) { return *cPlugins_; }
  throw std::runtime_error("NeoPkCUEngine::getCPluginSet: cPlugins_ is null");
}


void NeoPkCUEngine::setCPluginSet() {
  throw std::runtime_error("NeoPkCUEngine::setCPluginSet not implemented");
}


TeamVolatile NeoPkCUEngine::getTV() { return getTV(iBase_); }


TeamVolatile NeoPkCUEngine::getTTV() { return getTTV(iBase_); }


TeamVolatile NeoPkCUEngine::getTV(size_t iState) {
  return getBase(iState).getTeam(getICTeam());
}


TeamVolatile NeoPkCUEngine::getTTV(size_t iState) {
  return getBase(iState).getTeam(getIOTeam());
}


PokemonVolatile NeoPkCUEngine::getPKV() { return getPKV(iBase_); }


PokemonVolatile NeoPkCUEngine::getTPKV() { return getTPKV(iBase_); }


PokemonVolatile NeoPkCUEngine::getPKV(size_t iState) {
  if (!currentActor_) {
    throw std::runtime_error("NeoPkCUEngine::getPKV: currentActor_ is null");
  }
  return getBase(iState).teammate(*currentActor_);
}


PokemonVolatile NeoPkCUEngine::getTPKV(size_t iState) {
  throw std::runtime_error("NeoPkCUEngine::getTPKV not implemented");
}


MoveVolatile NeoPkCUEngine::getMV() { return getMV(iBase_); }


MoveVolatile NeoPkCUEngine::getTMV() { return getTMV(iBase_); }


MoveVolatile NeoPkCUEngine::getMV(size_t iState) {
  if (!currentActor_) {
    throw std::runtime_error("NeoPkCUEngine::getMV: currentActor_ is null");
  }
  const Action& action = actions_.at(*currentActor_);
  return getPKV(iState).getMV(action);
}


MoveVolatile NeoPkCUEngine::getTMV(size_t iState) {
  throw std::runtime_error("NeoPkCUEngine::getTMV not implemented");
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(size_t iStack) {
  if (!currentActor_) {
    throw std::runtime_error(
        "NeoPkCUEngine::getDamageComponent: currentActor_ is null");
  }
  return stackFrame_[iStack].damageComponents.at(*currentActor_);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(
    size_t iStack, const Actor& actor) {
  return stackFrame_[iStack].damageComponents.at(actor);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(
    size_t iStack, size_t iTeam) {
  for (auto& pair : stackFrame_[iStack].damageComponents) {
    if (pair.first.iTeam() == iTeam) { return pair.second; }
  }
  throw std::runtime_error(
      "NeoPkCUEngine::getDamageComponent: no actor for team " +
      std::to_string(iTeam));
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent() {
  return getDamageComponent(iBase_);
}


const DamageComponents_t& NeoPkCUEngine::getDamageComponent() const {
  if (!currentActor_) {
    throw std::runtime_error(
        "NeoPkCUEngine::getDamageComponent: currentActor_ is null");
  }
  return stackFrame_[iBase_].damageComponents.at(*currentActor_);
}


size_t NeoPkCUEngine::getICTeam() const {
  if (!currentActor_) {
    throw std::runtime_error("NeoPkCUEngine::getICTeam: currentActor_ is null");
  }
  return currentActor_->iTeam();
}


size_t NeoPkCUEngine::getIOTeam() const {
  return getICTeam() == TEAM_A ? TEAM_B : TEAM_A;
}


const Action& NeoPkCUEngine::getCAction() const {
  if (!currentActor_) {
    throw std::runtime_error(
        "NeoPkCUEngine::getCAction: currentActor_ is null");
  }
  return actions_.at(*currentActor_);
}


const Action& NeoPkCUEngine::getOAction() const {
  throw std::runtime_error("NeoPkCUEngine::getOAction not implemented");
}
