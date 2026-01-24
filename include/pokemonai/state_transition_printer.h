#ifndef PKAI_STATE_TRANSITION_PRINTER_H
#define PKAI_STATE_TRANSITION_PRINTER_H

#include <array>
#include <iosfwd>

#include "pokemonai/action.h"
#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/environment_possible.h"
#include "pokemonai/environment_volatile.h"

class StateTransitionPrinter {
 public:
  /**
   * @brief Compares two game states and prints descriptive events to the
   * stream.
   *
   * Analyzes the transition between oldState and newState, reporting actions,
   * switching, damage, status effects, and stat changes in a Pokemon-style
   * format.
   *
   * @param os The output stream to print to.
   * @param oldState The state before the transition occurs.
   * @param newState The state after the transition occurs, containing turn
   * flags.
   * @param actions The actions performed by both teams during this turn.
   */
  static void print(
      std::ostream& os,
      const ConstEnvironmentVolatile& oldState,
      const ConstEnvironmentPossible& newState,
      const std::array<Action, 2>& actions);

 protected:
  static void reportSwitch(
      std::ostream& os, const ConstEnvironmentPossible& nsP, size_t iTeam);
  static void reportHitResult(
      std::ostream& os, const ConstEnvironmentPossible& nsP, size_t iTeam);
  static void reportFainting(
      std::ostream& os,
      const ConstEnvironmentVolatile& osP,
      const ConstEnvironmentPossible& nsP,
      size_t iTeam);
  static void reportDamage(
      std::ostream& os,
      const ConstTeamVolatile& teamOld,
      const ConstTeamVolatile& teamNew);
  static void reportStatusChange(
      std::ostream& os,
      const ConstPokemonVolatile& pkOld,
      const ConstPokemonVolatile& pkNew);
  static void reportVolatileStatusChange(
      std::ostream& os,
      const ConstTeamVolatile& teamOld,
      const ConstTeamVolatile& teamNew);
  static void reportTeamVolatileStatusChange(
      std::ostream& os,
      const ConstTeamVolatile& teamOld,
      const ConstTeamVolatile& teamNew);
  static void reportStatBoosts(
      std::ostream& os,
      const ConstTeamVolatile& teamOld,
      const ConstTeamVolatile& teamNew);
  static std::string pokemonName(const ConstPokemonVolatile& pk);
};

#endif // PKAI_STATE_TRANSITION_PRINTER_H
