#ifndef NEO_PKCU_ENGINE_H
#define NEO_PKCU_ENGINE_H

#include "engine.h"
#include "pkCU_types.h"

class NeoPkCU;


class NeoPkCUEngine {
 public:
  /**
  * @struct MoveBracket
  * @brief Holds the components used to determine move priority.
  */
  struct MoveBracket {
    int32_t actionBracket;  // The priority bracket of the action. Higher values
                            // go first.
    uint32_t speed;         // The speed of the Pokemon
    uint32_t tiebreaker;  // If actionBracket and speed are equal, tieBreaker is
                          // set to disambiguate ties.
  };

  struct StackFrame {
    size_t iStack;     // Index into stack_ (same as stackframe index)
    StageType stage;   // Next stage to execute
    size_t iActor;     // Index of the teammate currently executing
    size_t iTarget;    // Index of the target being executed upon

    std::vector<Actor> moveOrder;  // The order in which actors will move.
    std::unordered_map<Actor, MoveBracket> moveBrackets;
    std::unordered_map<Actor, std::vector<Actor>> targets;  // The targets of each actor's action.
    std::unordered_map<Actor, DamageComponents_t> damageComponents;
  };

  NeoPkCUEngine(
      const NeoPkCU& cu,
      const EnvironmentVolatileData& initial,
      const ActionMap& actions);

  void seedStack();
  PossibleEnvironments updateState();

  /**
   * @brief Duplicates the current state on the stack, creating two possible outcomes.
   * @param result An array to store the indices of the two new environments.
   * @param probability The probability of the second outcome.
   * @param iState The index of the state to duplicate.
   */
  void duplicateState(
      std::array<size_t, 2>& result,
      FixType probability,
      size_t iState = SIZE_MAX);

  /**
   * @brief Duplicates the current state on the stack, creating three possible outcomes.
   * @param result An array to store the indices of the three new environments.
   * @param probabilityA The probability of the second outcome.
   * @param probabilityB The probability of the third outcome.
   * @param iState The index of the state to duplicate.
   */
  void triplicateState(
      std::array<size_t, 3>& result,
      FixType probability,
      FixType oProbability,
      size_t iState = SIZE_MAX);


  /**
   * @brief Computes the move brackets for each actor.
   */
  std::unordered_map<Actor, MoveBracket> computeMoveBrackets();
  MoveBracket computeMoveBracket(const Actor& actor);

  /**
   * @brief Computes the move target for the current actor.
   */
  std::vector<Actor> computeMoveTarget(
      const ConstEnvironmentVolatile& env,
      const Actor& actor,
      const Action& action) const;

  /**
   * @brief Computes the speed of a Pokemon.
   */
  uint32_t computeSpeed(const Actor& actor);

  /**
   * @brief Computes the order in which actors will move.
   */
  std::vector<Actor> computeActorOrder();

  /**
   * Stack Accessors:
   */
  EnvironmentPossible getBase(size_t iStack) { return getStack().at(iStack); };
  ConstEnvironmentPossible getBase(size_t iStack) const {
    return getStack().at(iStack);
  };
  EnvironmentPossible getBase() { return getStack().at(iBase_); };
  ConstEnvironmentPossible getBase() const { return getStack().at(iBase_); };
  PossibleEnvironments& getStack() { return stack_; };
  const PossibleEnvironments& getStack() const { return stack_; };

  /**
   * Engine computation stages:
   */
  void evaluateMove_preturn();
  void evaluateMove_selectOrder();
  void evaluateMove_switch_onSwitchOut();
  void evaluateMove_switch_onSwitchIn();
  void evaluateMove_status();
  void evaluateMove_damage_moveBase();
  void evaluateMove_damage_modifyHitChance();
  void evaluateMove_damage_evaluateHitChance();
  void evaluateMove_damage_damagingMoveBase();
  void evaluateMove_damage_modifyCritChance();
  void evaluateMove_damage_evaluateCritChance();
  void evaluateMove_damage_setBasePower();
  void evaluateMove_damage_setMoveType();
  void evaluateMove_damage_modifyBasePower();
  void evaluateMove_damage_modifyAttackPower();
  void evaluateMove_damage_modifyCriticalPower();
  void evaluateMove_damage_modifyRawDamage();
  void evaluateMove_damage_modifySTAB();
  void evaluateMove_damage_modifyTypeResistance();
  void evaluateMove_damage_modifyItemPower();
  void evaluateMove_damage_preDamage();
  void evaluateMove_postMove();
  void evaluateMove_preSecondary();
  void evaluateMove_evaluateSecondaryHitChance();
  void evaluateMove_secondary();
  void evaluateMove_postTurn();
  void evaluateMove_postRound();
  void evaluateMove_round_hash();


  /**
   * @name Compatibility Methods
   * @brief Methods implemented for compatibility with LegacyPkCUEngine.
   * @{
   */
  void evaluateMove();
  void calculateDamage();
  FixType getProbabilityToHit();

  /**
   * If returnAllStates is false, choose a state at random to keep, discard the
   * rest
   */
  void maybeCollapseStages();
  void collapseStages();

  template <size_t numEnvironments>
  void nPlicateState(
      std::array<size_t, numEnvironments>& result, size_t iState = SIZE_MAX) {
    if (iState == SIZE_MAX) { iState = iBase_; }
    PossibleEnvironments& stack = getStack();

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
  };

  void nPlicateStateDynamic(
      std::vector<size_t>& result,
      size_t numEnvironments,
      size_t iState = SIZE_MAX);

  [[deprecated]] void duplicateState(
      std::array<size_t, 2>& result,
      fpType probability,
      size_t iState = SIZE_MAX);

  std::array<size_t, 2> duplicateState(
      FixType probability, size_t iState = SIZE_MAX);

  StackFrame& getStackFrame() { return stackFrame_[iBase_]; }
  const StackFrame& getStackFrame() const { return stackFrame_[iBase_]; }
  StackFrame& getStackFrame(size_t iFrame) { return stackFrame_.at(iFrame); }
  const StackFrame& getStackFrame(size_t iFrame) const {
    return stackFrame_.at(iFrame);
  }

  StageType getStackStage() const { return getStackFrame().stage; }
  const PluginSet& getCPluginSet();
  void setCPluginSet(){};

  bool isPluginSourceActive(const plugin& p);

  /**
   * @def callPlugins
   * @brief A method for invoking plugins of a specific type.
   *
   * This method iterates through all registered plugins of a given `pluginType`
   * for the current matchup and calls their `pluginFunction`. The return value
   * of each plugin is OR'd with `retValue`. The loop breaks if `retValue`
   * becomes greater than 1, which is a convention to indicate that a plugin has
   * handled the event and no further plugins should be called.
   *
   * @param retValue The variable to store the combined return values of the
   * plugins.
   * @param pType The type of plugin to call (e.g., `PLUGIN_ON_MODIFYSPEED`).
   * @param FuncType The function signature of the plugin to be called.
   * @param args The arguments to pass to the plugin function.
   */
  template <typename FuncType, typename... Args>
  int callPlugins(pluginType pType, Args&&... args) {
    int retValue = 0;
    const auto& cPlugins = getCPluginSet()[(size_t)pType];

    for (const auto& plugin : cPlugins) {
      if (plugin.getSource() && !isPluginSourceActive(plugin)) { continue; }

      SPDLOG_TRACE(
          "iSTACK={:4d} PLUGIN={}:{} PR={} TGT={} Name={}",
          iBase_,
          pluginCategoryToString(plugin.getCategory()),
          pluginTypeToString(pType),
          plugin.getPriority(),
          pluginTargetToString(plugin.getTarget()),
          plugin.getName());

      FuncType cPluginFunc = reinterpret_cast<FuncType>(plugin.getFunction());
      retValue |= cPluginFunc(std::forward<Args>(args)...);

      if (retValue > 1) {
        break;
      }
    }

    return retValue;
  }

  void reportStackSizeChange() const;
  size_t advanceStackStage(size_t iStack);
  size_t advanceAllStages();
  void gotoStackStage(size_t iStage, StageType stage);
  void gotoStackStage(StageType stage);

  TeamVolatile getTV();
  TeamVolatile getTTV();
  TeamVolatile getTV(size_t iState);
  TeamVolatile getTTV(size_t iState);
  PokemonVolatile getPKV();
  PokemonVolatile getTPKV();
  PokemonVolatile getPKV(size_t iState);
  PokemonVolatile getTPKV(size_t iState);
  MoveVolatile getTMV();
  MoveVolatile getTMV(size_t iState);
  MoveVolatile getMV();
  MoveVolatile getMV(size_t iState);

  DamageComponents_t& getDamageComponent(size_t iStack);
  DamageComponents_t& getDamageComponent(size_t iStack, const Actor& actor);
  [[deprecated]] DamageComponents_t& getDamageComponent(
      size_t iStack, size_t iTeam);
  DamageComponents_t& getDamageComponent();

  size_t getICTeam() const;
  size_t getIOTeam() const;

  const Actor& getCActor() const;
  const Actor& getCActor(size_t iStack) const;
  const Actor& getTarget() const;
  const Actor& getTarget(size_t iStack) const;
  Actor getOActor() const;
  Actor getOActor(size_t iStack) const;
  const Action& getCAction() const;
  [[deprecated]] const Action& getOAction() const;
  size_t getIBase() const { return iBase_; }
  /** @} */

 protected:
  const NeoPkCU& cu_;

  /**
   * @brief the set of all plugins in use for the given matchup.
   */
  const PluginSet* cPlugins_;

  /**
   * @brief An map that stores the user-selected actions of each actor.
   * Due to move pre-emption and lock-in, it is possible that the actual
   * executed move is different from this move.
   */
  ActionMap actions_;

  /**
   * @brief The stack of possible environments being generated.
   */
  PossibleEnvironments stack_;

  /**
   * @brief A map that stores the hash of each environment on the stack and the
   * index of the environment.
   */
  std::unordered_map<uint64_t, size_t> stackHashToIdx_;

  /**
   * @brief A deque that tracks the current stack frame for each environment on
   * the stack.
   */
  std::deque<StackFrame> stackFrame_;

  /**
   * @brief The index of the base environment on the stack that is
   * currently being processed.
   */
  size_t iBase_;


  /**
   * @brief The size of the stack just prior to the last stage being executed.
   */
  size_t lastStackSize_;

  const PluginSet& getCPluginSet() const { return *cPlugins_; }

  /**
   * @brief Asserts that the sum of the probabilities of all environments on
   * the stack is equal to 1.
   */
  bool saneStackProbability() const;
};

#endif /* NEO_PKCU_ENGINE_H */