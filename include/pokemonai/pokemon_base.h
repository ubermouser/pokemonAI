/* 
 * File:   pokemon_base.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:27 PM
 */

#ifndef POKEMON_BASE_H
#define	POKEMON_BASE_H

#include "pokemonai/pkai.h"

#include <stdint.h>
#include <unordered_set>
#include <vector>
#include <array>

#include "pokemonai/name.h"
#include "pokemonai/collection.h"

class Type;
class Types;
class Ability;
class Abilities;
class Move;
class Moves;
class Pokedex;

/*
 * contains metrics of a given pokemon which do not change in combat, and
 * values that the user cannot modify
 */
class PKAISHARED PokemonBase: public Name
{
public:
  using TypeArray = std::array<const Type*, 2>;
  using AbilitySet = std::unordered_set<const Ability*>;
  using MoveSet = std::unordered_set<const Move*>;
  using StatsArray = std::array<uint8_t, 6>;

  static const PokemonBase* no_base;
  /*
   * pokemon's typeage, lostChild is true if either of these types don't exist
   * type (HG/SS): (NOTE - ORDER AND TYPES DEFINED BY INPUT FILE!)
   *  0: normal
   *  1: fire
   *  2: water
   *  3: electric
   *  4: grass
   *  5: ice
   *  6: fighting
   *  7: poison
   *  8: ground
   *  9: flying
   * 10: psychic
   * 11: bug
   * 12: rock
   * 13: ghost
   * 14: dragon
   * 15: dark
   * 16: steel
   */
  TypeArray types_;

  /*
   * the weight / heftiness of the pokemon. Used for some damage calculations
   */
  uint16_t weight_;
  
  /*
   * baseStats:
   * 0: Attack
   * 1: Special Attack
   * 2: Defense
   * 3: Special Defense
   * 4: Speed
   * 5: Hit-Points
   */
  StatsArray baseStats_; // pokemon's basic stats

  /* 
   * primary and secondary ability
   * NULL if not a choice
   */
  AbilitySet abilities_;

  /* pointers to actions the pokemon may perform in combat */
  MoveSet moves_;

  bool lostChild_;


  const Type& getType(size_t iType) const { return *types_[iType]; };

  bool hasType(const Type* oType) const { return types_[0] == oType || types_[1] == oType; }

  const AbilitySet& getAbilities() const { return abilities_; }

  PokemonBase() = default;
  PokemonBase(
    const std::string& name,
    const TypeArray& types,
    uint16_t weight,
    const StatsArray& stats,
    const AbilitySet& abilities,
    const MoveSet& moves);
};


class PKAISHARED Pokemons: public Collection<PokemonBase>
{
public:
  bool initialize(
      const std::string& pokemonPath,
      const std::string& movelistPath,
      const Types& types,
      const Abilities& abilities,
      const Moves& moves);

protected:
  bool loadFromFile(
      const std::string& path,
      const Types& types,
      const Abilities& abilities);
  bool loadFromFile_lines(
      const Types& types,
      const Abilities& abilities,
      const std::vector<std::string>& lines,
      size_t& iLine);

  bool loadMovelistFromFile(const std::string& path, const Moves& moves);
  bool loadMovelistFromFile_lines(
      const Moves& moves, const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* POKEMON_BASE_H */

