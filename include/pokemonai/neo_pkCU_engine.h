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
    uint32_t speed;         // The speed of the Pokemon, used as a tie-breaker.
  };

  struct StackFrame {
    size_t iStack;     // Index into stack_
    StageType stage;   // Next stage to execute
    size_t iActor;     // Index of the teammate currently executing

    std::unordered_map<Actor, DamageComponents_t> damageComponents;
  };

  NeoPkCUEngine(
      const NeoPkCU& cu,
      const EnvironmentVolatileData& initial,
      const ActionMap& actions);

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
   * @name Compatibility Methods
   * @brief Methods implemented for compatibility with LegacyPkCUEngine.
   * @{
   */
  void updateState_move();
  int32_t movePriority_Bracket();
  uint32_t movePriority_Speed();
  void evaluateMove_switch();
  void evaluateMove_preMove();
  void evaluateMove_postMove();
  void evaluateMove_damage();
  void evaluateMove_script();
  void evaluateMove_postTurn();
  void evaluateRound_end();
  size_t combineSimilarEnvironments();
  uint32_t movePriority();
  void evaluateMove();
  void calculateDamage();
  FixType getProbabilityToHit();

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

  void duplicateState(
      std::array<size_t, 2>& result,
      fpType probability,
      size_t iState = SIZE_MAX);

  uint32_t getStackStage() const;
  void advanceStackStage();
  void swapTeamIndexes();
  const PluginSet& getCPluginSet();
  void setCPluginSet();

  TeamVolatile getTV();
  TeamVolatile getTTV();
  TeamVolatile getTV(size_t iState);
  TeamVolatile getTTV(size_t iState);
  PokemonVolatile getPKV();
  PokemonVolatile getTPKV();
  PokemonVolatile getPKV(size_t iState);
  PokemonVolatile getTPKV(size_t iState);
  MoveVolatile getMV();
  MoveVolatile getTMV();
  MoveVolatile getMV(size_t iState);
  MoveVolatile getTMV(size_t iState);

  DamageComponents_t& getDamageComponent(size_t iStack);
  DamageComponents_t& getDamageComponent(size_t iStack, const Actor& actor);
  DamageComponents_t& getDamageComponent(size_t iStack, size_t iTeam);
  DamageComponents_t& getDamageComponent();
  const DamageComponents_t& getDamageComponent() const;

  size_t getICTeam() const;
  size_t getIOTeam() const;
  const Action& getCAction() const;
  const Action& getOAction() const;
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
   * @brief The order in which actors will move.
   */
  std::vector<Actor> actors_;

  /**
   * @brief The stack of possible environments being generated.
   */
  PossibleEnvironments stack_;

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
   * @brief The actor that is currently executing.
   */
  Actor* currentActor_;

  const PluginSet& getCPluginSet() const { return *cPlugins_; }

  /**
   * @brief Asserts that the sum of the probabilities of all environments on
   * the stack is equal to 1.
   */
  bool saneStackProbability() const;
};

#endif /* NEO_PKCU_ENGINE_H */