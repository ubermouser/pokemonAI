#include "pokemonai/neo_pkCU_engine.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <stdexcept>
#include <vector>

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
      lastStackSize_(1),
      minStateProb_(0),
      collapseHighWaterMark_(SIZE_MAX) {
  if (cu_.cfg_.stateSelectMethod != NeoPkCU::StateSelectMethod::ALL) {
    if (cu_.cfg_.maxNumStates == 0) {
      throw std::invalid_argument(
          "NeoPkCUEngine: maxNumStates must be >= 1");
    }
    if (std::isnan(cu_.cfg_.maxStatesMultiplier) ||
        cu_.cfg_.maxStatesMultiplier < 1.0) {
      throw std::invalid_argument(
          "NeoPkCUEngine: maxStatesMultiplier must be >= 1.0");
    }
    if (std::isnan(cu_.cfg_.minStateProbability) ||
        cu_.cfg_.minStateProbability < 0.0) {
      throw std::invalid_argument(
          "NeoPkCUEngine: minStateProbability must be >= 0.0");
    }
    minStateProb_ = FixType(cu_.cfg_.minStateProbability);
    collapseHighWaterMark_ = static_cast<size_t>(std::ceil(
        static_cast<double>(cu_.cfg_.maxNumStates) *
        cu_.cfg_.maxStatesMultiplier));
  }

  stack_.setNonvolatileEnvironment(cu.nv_);
  stack_.push_back(EnvironmentPossibleData::create(initial, false));

  seedStack();
}


void NeoPkCUEngine::seedStack() {
  stackFrame_.push_back(StackFrame{});
  StackFrame& firstFrame = stackFrame_.back();
  firstFrame.iStack = 0;
  firstFrame.stage = StageType::SEEDED;
  firstFrame.iActor = 0;
  firstFrame.iTarget = 0;
  firstFrame.actions = actions_;

  // First pass: populate basic frame data
  for (const auto& [actor, action] : actions_) {
    // default ordering is arbitrary until we've calculated speed bracket
    firstFrame.moveOrder.push_back(actor);
    firstFrame.targets[actor] =
        computeMoveTarget(stack_.at(0).getEnv(), actor, action);
    firstFrame.damageComponents[actor] = {};
  }
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

  return std::move(stack_);
}


uint32_t NeoPkCUEngine::computeSpeed(const Actor& actor) {
  PokemonVolatile pkv = getBase().getEnv().teammate(actor);
  uint32_t speed = pkv.getFV_boosted(FV_SPEED);

  int result = 0;
  result = callPlugins<onModifySpeed_rawType>(
      PLUGIN_ON_MODIFYSPEED, *this, actor, speed);

  return speed;
}


NeoPkCUEngine::MoveBracket NeoPkCUEngine::computeMoveBracket(
    const Actor& actor) {
  const Action& action = actions_.at(actor);
  int32_t actionBracket = 0;
  uint32_t speed = 0;
  uint32_t tiebreaker = 0;


  if (action.isSwitch() || action.isActivate()) {
    actionBracket = 6;
  } else if (action.isMove()) {
    PokemonVolatile pkv = getBase().teammate(actor);
    if (action.isStruggle()) {
      actionBracket = 0;
    } else {
      MoveVolatile mv = pkv.getMV(action);
      actionBracket = mv.getBase().getSpeedPriority();

      int result = 0;
      result = callPlugins<onModifyBracket_rawType>(
          PLUGIN_ON_SETSPEEDBRACKET,
          *this,
          actor,
          action,
          actionBracket);
    }

    speed = computeSpeed(actor);
  } else if (action.isWait()) {
    actionBracket = -7;
    // no need to disambiguate wait actions
    tiebreaker = actor.index();
  }


  return {actionBracket, speed, tiebreaker};
}


std::vector<Actor> NeoPkCUEngine::computeMoveTarget(
    const ConstEnvironmentVolatile& env,
    const Actor& actor,
    const Action& action) const {
  auto t = env.getTargets(actor, action);
  if (t.empty()) { t.push_back(actor); }
  return std::move(t);
}


std::unordered_map<Actor, NeoPkCUEngine::MoveBracket>
NeoPkCUEngine::computeMoveBrackets() {
  std::unordered_map<Actor, MoveBracket> results;
  for (const auto& actor : getBase().getEnv().yieldActiveActors()) {
    results[actor] = computeMoveBracket(actor);
  }
  return std::move(results);
}


std::vector<Actor> NeoPkCUEngine::computeActorOrder() {
  auto brackets = computeMoveBrackets();
  std::vector<Actor> actors = getBase().getEnv().getActiveActors();

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

size_t NeoPkCUEngine::maybeCollapseStages(size_t prevStackSize) {
  if (cu_.cfg_.stateSelectMethod == NeoPkCU::StateSelectMethod::ALL ||
      stack_.size() == prevStackSize) {
    return 0;
  }

  bool needsCollapse = stack_.size() > collapseHighWaterMark_;
  if (!needsCollapse && minStateProb_ > FixType(0)) {
    auto isUnderflowRisk = [&](size_t idx) {
      return stackFrame_[idx].stage != StageType::FINAL &&
             stack_[idx].getProbability() < minStateProb_;
    };
    needsCollapse = isUnderflowRisk(iBase_);
    for (size_t i = prevStackSize; !needsCollapse && i < stack_.size(); ++i) {
      needsCollapse = isUnderflowRisk(i);
    }
  }

  return needsCollapse ? collapseStages() : 0;
}


size_t NeoPkCUEngine::collapseStages() {
  if (cu_.cfg_.stateSelectMethod == NeoPkCU::StateSelectMethod::ALL ||
      stack_.size() <= 1) {
    return 0;
  }

  const size_t maxStates = cu_.cfg_.maxNumStates;
  auto isUnderflowRisk = [&](size_t idx) {
    return minStateProb_ > FixType(0) &&
           stackFrame_[idx].stage != StageType::FINAL &&
           stack_[idx].getProbability() < minStateProb_;
  };

  if (stack_.size() <= maxStates) {
    bool hasUnderflowState = false;
    for (size_t i = 0; i < stack_.size(); ++i) {
      if (isUnderflowRisk(i)) {
        hasUnderflowState = true;
        break;
      }
    }
    if (!hasUnderflowState) { return 0; }
  }

  std::vector<size_t> keptIndices;
  switch (cu_.cfg_.stateSelectMethod) {
  case NeoPkCU::StateSelectMethod::RANDOM:
    keptIndices = stack_.stateSelect_rouletteN(maxStates);
    break;
  case NeoPkCU::StateSelectMethod::MOST_LIKELY:
    keptIndices = stack_.stateSelect_mostLikelyN(maxStates);
    break;
  default:
    throw std::runtime_error(
        "NeoPkCUEngine::collapseStages: invalid stateSelectMethod");
  }

  if (minStateProb_ > FixType(0)) {
    std::erase_if(keptIndices, isUnderflowRisk);
  }
  if (keptIndices.empty()) {
    keptIndices.push_back(stack_.mostProbableIndex());
  }

  std::sort(keptIndices.begin(), keptIndices.end());

  const size_t oldSize = stack_.size();
  size_t writeIdx = 0;
  size_t countKeptUpToIBase = 0;
  size_t removedCompleted = 0;
  stackHashToIdx_.clear();

  for (size_t readIdx = 0; readIdx < oldSize; ++readIdx) {
    if (writeIdx < keptIndices.size() && readIdx == keptIndices[writeIdx]) {
      if (readIdx <= iBase_) { ++countKeptUpToIBase; }
      if (writeIdx != readIdx) {
        stack_[writeIdx] = std::move(stack_[readIdx]);
        stackFrame_[writeIdx] = std::move(stackFrame_[readIdx]);
      }
      stackFrame_[writeIdx].iStack = writeIdx;
      if (stackFrame_[writeIdx].stage == StageType::FINAL) {
        stackHashToIdx_[stack_[writeIdx].getHash()] = writeIdx;
      }
      ++writeIdx;
    } else if (stackFrame_[readIdx].stage == StageType::FINAL) {
      ++removedCompleted;
    }
  }

  stack_.resize(writeIdx);
  stack_.resetMerged();
  stackFrame_.resize(writeIdx);

  SPDLOG_TRACE(
      "Collapsed ({}) stack from {:4d} to {:4d} states",
      (cu_.cfg_.stateSelectMethod == NeoPkCU::StateSelectMethod::RANDOM
           ? "RANDOM"
           : "MOST_LIKELY"),
      oldSize,
      writeIdx);

  lastStackSize_ = writeIdx;
  iBase_ = (countKeptUpToIBase + writeIdx - 1) % writeIdx;
  renormalizeStackProbabilities();
  return removedCompleted;
}


void NeoPkCUEngine::renormalizeStackProbabilities() {
  if (stack_.empty()) { return; }

  FixType probabilitySum = FixType(0);
  size_t mostProbableIdx = 0;
  for (size_t i = 0; i < stack_.size(); ++i) {
    assert(!stack_[i].isPruned());
    probabilitySum += stack_[i].getProbability();
    if (stack_[i].getProbability() > stack_[mostProbableIdx].getProbability()) {
      mostProbableIdx = i;
    }
  }

  FixType accumulated = FixType(0);
  for (size_t i = 0; i < stack_.size(); ++i) {
    stackFrame_[i].iStack = i;
    if (i == mostProbableIdx) { continue; }
    stack_[i].getProbability() = stack_[i].getProbability() / probabilitySum;
    accumulated += stack_[i].getProbability();
  }
  stack_[mostProbableIdx].getProbability() = FixType(1) - accumulated;
  assert(saneStackProbability());
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
  result = duplicateState(branchProbability, iState);
}


std::array<size_t, 2> NeoPkCUEngine::duplicateState(
    FixType branchProbability, size_t iState) {
  assert(branchProbability > FixType(0) && branchProbability < FixType(1));
  std::array<size_t, 2> result;

  // duplicate state 2 times
  nPlicateState(result, iState);

  // modify probabilities of resulting states:
  FixType totalProbability = getBase(result[0]).getProbability();
  FixType absoluteBranchProb = totalProbability * branchProbability;

  assert(absoluteBranchProb > FixType(0));
  assert(absoluteBranchProb < totalProbability);

  getBase(result[1]).getProbability() = absoluteBranchProb;
  getBase(result[0]).getProbability() = totalProbability - absoluteBranchProb;

  assert(saneStackProbability());
  return std::move(result);
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
  return stack_.saneProbabilities();
}


void NeoPkCUEngine::calculateDamage() {
  FixType partitionEnvironmentProbability =
      (FixType(1) / (int32_t)cu_.cfg_.numRandomEnvironments);
  DamageComponents_t& cDMG = getDamageComponent();

  uint32_t power = cDMG.damage;

  std::array<size_t, 2> iREnv = {{SIZE_MAX, getIBase()}};
  for (size_t iEnv = 0; iEnv != cu_.cfg_.numRandomEnvironments; ++iEnv) {
    if (cu_.cfg_.numRandomEnvironments > 1) {
      if ((iEnv + 1) == cu_.cfg_.numRandomEnvironments) {
        iREnv[1] = getIBase();
      } else {
        duplicateState(iREnv, partitionEnvironmentProbability);
      }
    };

    // find the mean random value for this partition of the random environment
    fpType randomValue =
        (iEnv / (fpType)cu_.cfg_.numRandomEnvironments) +
        ((fpType)1.0 / (fpType)cu_.cfg_.numRandomEnvironments / (fpType)2.0);

    // scale our random value modifier to 0.85..1.0
    randomValue = deScale(randomValue, (fpType)1.0, (fpType)0.85);

    uint32_t& actualDamage = getDamageComponent(iREnv[1]).damage;
    actualDamage = (uint32_t)((fpType)power * randomValue);

    int result = 0;
    result = callPlugins<onSetPower_rawType>(
        PLUGIN_ON_CALCULATEDAMAGE,
        *this,
        getCActor(),
        getCAction(),
        getTarget(),
        actualDamage);

    // inflict damage caused upon the targetPokemon:
    getTPKV(iREnv[1]).modHP(-1 * actualDamage);
  }
}


FixType NeoPkCUEngine::getProbabilityToHit() {
  PokemonVolatile cPKV = getPKV();
  PokemonVolatile tPKV = getTPKV();
  const Move& cMove = getMV().getBase();

  /* Moves with no accuracy component are guaranteed to hit */
  if (cMove.primaryAccuracy_ <= 0) { return FixType(1); }

  // combine accuracy/evasion stages before look-up to ensure precision
  int32_t netBoost = cPKV.getBoost(FV_ACCURACY) - tPKV.getBoost(FV_EVASION);
  netBoost = std::min(std::max(netBoost, -6), 6);

  FixType probabilityToHit =
      // map net boost stage to precision look-up table
      PokemonNonVolatile::aFV_base[FV_ACCURACY - 6][netBoost + 6] *
      // lowest is 30% or 30 / 100
      cMove.getPrimaryAccuracy();

  return probabilityToHit;
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


void NeoPkCUEngine::reportStackSizeChange() const {
  if (getStack().size() == lastStackSize_) { return; }

  SPDLOG_TRACE(
      "iSTACK={:4d} STACK GROWS +{} TO {}",
      iBase_,
      getStack().size() - lastStackSize_,
      getStack().size());
}


size_t NeoPkCUEngine::advanceAllStages() {
  size_t numCompletedFrames = 0;
  reportStackSizeChange();

  // advance the current stage:
  numCompletedFrames += advanceStackStage(iBase_);

  // advance all new stages:
  for (size_t iStack = lastStackSize_; iStack < stackFrame_.size(); ++iStack) {
    numCompletedFrames += advanceStackStage(iStack);
  }

  lastStackSize_ = stackFrame_.size();
  return numCompletedFrames;
}


EnvironmentVolatile NeoPkCUEngine::getEnv() const {
  return const_cast<NeoPkCUEngine*>(this)->getBase().getEnv();
}

const Move& NeoPkCUEngine::getMV() const {
  return const_cast<NeoPkCUEngine*>(this)->getMV().getBase();
}

bool NeoPkCUEngine::isPluginSourceActive(const plugin& p) {
  return p.isActive(*this);
}


void NeoPkCUEngine::handleActorSwitch(Actor switchingActor, Actor swapTarget) {
  StackFrame& frame = getStackFrame();

  // Update target lists for all other actors: if they were targeting the
  // pokemon that just switched out, redirect to the pokemon that switched in.
  for (auto& [actor, targetList] : frame.targets) {
    if (actor == switchingActor) { continue; }
    for (auto& target : targetList) {
      if (target == switchingActor) { target = swapTarget; }
    }
  }

  // Update the current actor:
  frame.moveOrder[frame.iActor] = swapTarget;
  frame.targets[swapTarget] = frame.targets.at(switchingActor);
  frame.actions[swapTarget] = Action::wait();
  frame.moveBrackets[swapTarget] = frame.moveBrackets.at(switchingActor);
  frame.damageComponents[swapTarget] =
      frame.damageComponents.at(switchingActor);
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
  return getBase(iState).teammate(getCActor(iState));
}


PokemonVolatile NeoPkCUEngine::getPKV(const Actor& actor) { return getPKV(actor, iBase_); }


PokemonVolatile NeoPkCUEngine::getPKV(const Actor& actor, size_t iState) {
  return getBase(iState).teammate(actor);
}


PokemonVolatile NeoPkCUEngine::getTPKV(size_t iState) {
  return getBase(iState).teammate(getTarget(iState));
}


MoveVolatile NeoPkCUEngine::getMV() { return getMV(iBase_); }


MoveVolatile NeoPkCUEngine::getMV(size_t iState) {
  auto& actor = getCActor(iState);
  auto& action = getStackFrame(iState).actions.at(actor);
  return getBase(iState).teammate(actor).getMV(action);
}


MoveVolatile NeoPkCUEngine::getMV(const Actor& actor) { return getMV(actor, iBase_); }


MoveVolatile NeoPkCUEngine::getMV(const Actor& actor, size_t iState) {
  auto& action = getStackFrame(iState).actions.at(actor);
  return getBase(iState).teammate(actor).getMV(action);
}


MoveVolatile NeoPkCUEngine::getTMV() { return getTMV(iBase_); }


MoveVolatile NeoPkCUEngine::getTMV(size_t iState) {
  auto& target = getTarget(iState);
  auto& action = getStackFrame(iState).actions.at(target);
  return getBase(iState).teammate(target).getMV(action);
}


DamageComponents_t& NeoPkCUEngine::getDamageComponent(size_t iStack) {
  auto& actor = getCActor(iStack);
  return stackFrame_[iStack].damageComponents.at(actor);
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


size_t NeoPkCUEngine::getICTeam() const { return getCActor().iTeam(); }


size_t NeoPkCUEngine::getIOTeam() const {
  return getICTeam() == TEAM_A ? TEAM_B : TEAM_A;
}


const Actor& NeoPkCUEngine::getCActor() const {
  const StackFrame& frame = getStackFrame();
  assert(frame.iActor < frame.moveOrder.size());

  return frame.moveOrder[frame.iActor];
}


const Actor& NeoPkCUEngine::getCActor(size_t iStack) const {
  const StackFrame& frame = getStackFrame(iStack);
  assert(frame.iActor < frame.moveOrder.size());

  return frame.moveOrder[frame.iActor];
}


const Actor& NeoPkCUEngine::getTarget() const { return getTarget(getIBase()); }


const Actor& NeoPkCUEngine::getTarget(size_t iStack) const {
  const StackFrame& frame = getStackFrame(iStack);
  const Actor& actor = getCActor(iStack);

  return frame.targets.at(actor).at(frame.iTarget);
}


const Action& NeoPkCUEngine::getCAction() const {
  return getStackFrame().actions.at(getCActor());
}


const Action& NeoPkCUEngine::getOAction() const {
  return getStackFrame().actions.at(getTarget());
}


Actor NeoPkCUEngine::getOActor() const { return getOActor(getIBase()); }


Actor NeoPkCUEngine::getOActor(size_t iStack) const {
  size_t iOTeam = getCActor(iStack).iTeam() == TEAM_A ? TEAM_B : TEAM_A;
  return Actor(
      iOTeam, getBase(iStack).getTeam(iOTeam).getICPKV());
}


size_t NeoPkCUEngine::getActorIndex(const Actor& actor, size_t iStack) const {
  if (iStack == SIZE_MAX) { iStack = iBase_; }
  const auto& moveOrder = getStackFrame(iStack).moveOrder;

  for (size_t i = 0; i < moveOrder.size(); ++i) {
    if (moveOrder[i] == actor) { return i; }
  }

  throw std::runtime_error(fmt::format("Actor {} not found in move ordering", fmt::streamed(actor)));
}
