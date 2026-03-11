#include "pokemonai/neo_pkCU_engine.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <map>
#include <stdexcept>

#include "pokemonai/fp_compare.h"
#include "pokemonai/neo_pkCU.h"
#include "pokemonai/pkCU_types.h"
#include "pokemonai/pluggable_types.h"


NeoPkCUEngine::NeoPkCUEngine(
    const NeoPkCU& cu,
    const EnvironmentVolatileData& initial,
    const ActionMap& actions)
    : cu_(cu),
      cPlugins_(&cu.pluginSet_),
      actions_(actions),
      iBase_(0),
      lastStackSize_(1) {
  stack_.setNonvolatileEnvironment(cu.nv_);
  stack_.push_back(EnvironmentPossibleData::create(initial, false));

  StackFrame firstFrame;
  firstFrame.iStack = 0;
  firstFrame.stage = StageType::SEEDED;
  firstFrame.iActor = 0;
  firstFrame.iTarget = 0;
  firstFrame.targets = computeMoveTargets(stack_.at(0), actions_);
  stackFrame_.push_back(firstFrame);
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
PossibleEnvironments NeoPkCUEngine::updateState() {
  evaluateMove();

  // combine environments that equal eachother:
  combineSimilarEnvironments();

  return std::move(stack_);
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


std::unordered_map<Actor, std::vector<Actor>> NeoPkCUEngine::computeMoveTargets(
    const ConstEnvironmentPossible& envP, const ActionMap& actions) const {
  std::unordered_map<Actor, std::vector<Actor>> targets;

  // Initialize targets for each actor
  for (const auto& pair : actions) {
    const Actor& actor = pair.first;
    const Action& action = pair.second;
    if (action.isMove()) {
      ConstMoveVolatile mv = envP.teammate(actor).getMV(action);

      // single target friendly:
      if (action.friendlyTarget() != Action::FRIENDLY_DEFAULT) {
        targets[actor] = {Actor(actor.iTeam(), action.iFriendly())};
      } else {  // single-target enemy:
        targets[actor] = {Actor(actor.iTeam() == TEAM_A ? TEAM_B : TEAM_A, 0)};
      }
      // TODO multi-target moves:
    } else if (action.isSwitch()) {
      targets[actor] = {Actor(actor.iTeam(), action.iFriendly())};
    } else {  // self target:
      targets[actor] = {actor};
    }
  }

  return std::move(targets);
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
    } else if (bracketA.speed != bracketB.speed) {
      return bracketA.speed > bracketB.speed;
    } else {
      // TODO - implement tiebreaking
      return bracketA.tiebreaker > bracketB.tiebreaker;
    }
    
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
    std::array<size_t, 2>& result, FixType branchProbability, size_t iState) {
  if (iState == SIZE_MAX) { iState = iBase_; }
  if (!mostlyGT(branchProbability, FixType(0))) {
    result[0] = iState;
    result[1] = iState; // Or some other indication of no split
    return;
  }
  assert(mostlyLT(branchProbability, FixType(1)));

  // duplicate state 2 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType absoluteBranchProb = totalProbability * branchProbability;

  assert(mostlyGT(absoluteBranchProb, FixType(0)));
  assert(mostlyLT(absoluteBranchProb, totalProbability));

  getBase(result[1]).getProbability() = absoluteBranchProb;
  getBase(result[0]).getProbability() = totalProbability - absoluteBranchProb;

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


void NeoPkCUEngine::reportStackSizeChange() const {
  if (getStack().size() == lastStackSize_) { return; }

  SPDLOG_TRACE(
      "STACKSIZE CHANGED FROM {} TO {}", lastStackSize_, getStack().size());
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
size_t NeoPkCUEngine::combineSimilarEnvironments() {
  PossibleEnvironments& stack = getStack();

  // hash environments (and summate probabilities for check):
  for (iBase_ = 0; iBase_ != stack.size(); ++iBase_) {
    EnvironmentPossible cEnvironment = getBase();

    // assert that each of these environments is getting hashed:
    assert(getStackStage() == StageType::FINAL);

    cEnvironment.data().generateHash();
  }

  size_t iSize = stack.size();
  std::unordered_map<uint64_t, size_t> envMap;
  envMap.reserve(iSize);

  // compare environment hashes:
  for (size_t iEnv = 0; iEnv != iSize; iEnv++) {
    EnvironmentPossible cEnv = stack.at(iEnv);

    // don't attempt to merge pruned environments
    if (cEnv.isPruned()) { continue; }

    uint64_t hash = cEnv.getHash();
    auto it = envMap.find(hash);

    if (it != envMap.end()) {
      // Found a duplicate! Merge into the existing environment
      size_t existIndex = it->second;
      EnvironmentPossible existEnv = stack.at(existIndex);

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
  assert(saneStackProbability());
  return stack.getNumUnique();
}  // endOf combineSimilarEnvironments


uint32_t NeoPkCUEngine::movePriority() {
  throw std::runtime_error("NeoPkCUEngine::movePriority not implemented");
}


void NeoPkCUEngine::calculateDamage() {
  SPDLOG_ERROR("NeoPkCUEngine::calculateDamage not implemented");
  getDamageComponent().damage = 10;
}


FixType NeoPkCUEngine::getProbabilityToHit() {
  SPDLOG_ERROR("NeoPkCUEngine::getProbabilityToHit not implemented");
  return getMV().getBase().getPrimaryAccuracy();
}


void NeoPkCUEngine::duplicateState(
    std::array<size_t, 2>& result, fpType probability, size_t iState) {
  duplicateState(result, FixType(probability), iState);
}



void NeoPkCUEngine::nPlicateStateDynamic(
    std::vector<size_t>& result, size_t numEnvironments, size_t iState) {
  if (iState == SIZE_MAX) { iState = iBase_; }
  PossibleEnvironments& stack = getStack();

  result.resize(numEnvironments);
  result[0] = iState;
  const auto& baseFrame = stackFrame_[iState];
  for (size_t iEnvironment = 1; iEnvironment < numEnvironments;
       ++iEnvironment) {
    size_t cSize = stack.size();

    result[iEnvironment] = cSize;
    stackFrame_.push_back(baseFrame);
    stackFrame_.back().iStack = cSize;

    stack.push_back(stack[iState]);
  }
}


const PluginSet& NeoPkCUEngine::getCPluginSet() {
  if (cPlugins_) { return *cPlugins_; }
  throw std::runtime_error("NeoPkCUEngine::getCPluginSet: cPlugins_ is null");
}


void NeoPkCUEngine::setCPluginSet() {
  throw std::runtime_error("NeoPkCUEngine::setCPluginSet not implemented");
}


void NeoPkCUEngine::gotoStackStage(StageType stage) {
  gotoStackStage(getStackFrame().iStack, stage);
}


void NeoPkCUEngine::gotoStackStage(size_t iStage, StageType stage) {
  auto& frame = getStackFrame(iStage);
  // stage will auto-increment at the end of the loop
  frame.stage = static_cast<StageType>(stage - 1);
}


size_t NeoPkCUEngine::advanceStackStage(size_t iStack) {
  auto& frame = getStackFrame(iStack);
  if (frame.stage < StageType::FINAL) {
    frame.stage = static_cast<StageType>(frame.stage + 1);
    return (frame.stage == StageType::FINAL) ? 1 : 0;
  }
  return 0;
}


size_t NeoPkCUEngine::advanceAllStages() {
  size_t numCompletedFrames = 0;

  // advance the current stage:
  numCompletedFrames += advanceStackStage(iBase_);

  // advance all new stages:
  for (size_t iStack = lastStackSize_; iStack < stackFrame_.size(); ++iStack) {
    numCompletedFrames += advanceStackStage(iStack);
  }

  reportStackSizeChange();
  lastStackSize_ = stackFrame_.size();
  return numCompletedFrames;
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
  const StackFrame& frame = stackFrame_[iState];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return getBase(iState).teammate(actor);
  }
  throw std::runtime_error("NeoPkCUEngine::getPKV: no active actor");
}


PokemonVolatile NeoPkCUEngine::getTPKV(size_t iState) {
  return getBase(iState).getTeam(getIOTeam()).getPKV();
}


MoveVolatile NeoPkCUEngine::getMV() { return getMV(iBase_); }


MoveVolatile NeoPkCUEngine::getTMV() { return getTMV(iBase_); }


MoveVolatile NeoPkCUEngine::getMV(size_t iState) {
  const StackFrame& frame = stackFrame_[iState];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return getBase(iState).teammate(actor).getMV(actions_.at(actor));
  }
  throw std::runtime_error("NeoPkCUEngine::getMV: no active actor");
}


MoveVolatile NeoPkCUEngine::getTMV(size_t iState) {
  // Target move is not usually needed in damage calc unless for specific plugins
  // For now, return first move of target
  return getBase(iState).getTeam(getIOTeam()).getPKV().getMV(0);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(size_t iStack) {
  const StackFrame& frame = stackFrame_[iStack];
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& actor = frame.moveOrder[frame.iActor];
    return stackFrame_[iStack].damageComponents[actor];
  }
  throw std::runtime_error("NeoPkCUEngine::getDamageComponent: no active actor");
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
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return frame.damageComponents.at(frame.moveOrder[frame.iActor]);
  }
  throw std::runtime_error("NeoPkCUEngine::getDamageComponent: no active actor");
}


size_t NeoPkCUEngine::getICTeam() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return frame.moveOrder[frame.iActor].iTeam();
  }
  throw std::runtime_error("NeoPkCUEngine::getICTeam: no active actor");
}


size_t NeoPkCUEngine::getIOTeam() const {
  return getICTeam() == TEAM_A ? TEAM_B : TEAM_A;
}


const Action& NeoPkCUEngine::getCAction() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    return actions_.at(frame.moveOrder[frame.iActor]);
  }
  throw std::runtime_error("NeoPkCUEngine::getCAction: no active actor");
}


const Action& NeoPkCUEngine::getOAction() const {
  const StackFrame& frame = getStackFrame();
  if (frame.iActor < frame.moveOrder.size()) {
    const Actor& currentActor = frame.moveOrder[frame.iActor];
    for (const auto& actor : frame.moveOrder) {
      if (actor.iTeam() != currentActor.iTeam()) {
        return actions_.at(actor);
      }
    }
  }
  throw std::runtime_error("NeoPkCUEngine::getOAction not implemented");
}
