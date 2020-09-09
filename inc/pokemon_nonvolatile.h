/* 
 * File:   pokemon_nonvolatile.h
 * Author: Ubermouser
 *
 * Created on June 30, 2011, 10:37 PM
 */

#ifndef POKEMON_NONVOLATILE_H
#define	POKEMON_NONVOLATILE_H

#include "pkai.h"

#include <stdint.h>
#include <ostream>
#include <array>
#include <vector>

#include "signature.h"

#include "action.h"
#include "orphan.h"
#include "name.h"
#include "move_nonvolatile.h"

#define STAGE0 6

class Pokedex;
class Item;
class Ability;
class Nature;
class PokemonBase;

class TeamVolatile;
class PokemonVolatile;

#define POKEMON_NONVOLATILE_DIGESTSIZE (MOVE_NONVOLATILE_DIGESTSIZE*4 + 94)

/* contains metrics that the user can modify, but are not modified in combat */
class PKAISHARED PokemonNonVolatile : public Name, public Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>
{
public:
  using OrphanSet = orphan::OrphanSet;

  /* The pokemon in which this volatile set is based off of*/
  const PokemonBase* base; 
  
  /* Pointer to the ability that the pokemon has currently */
  const Ability* chosenAbility; 
  
  /* Pointer to the nature that the pokemon has */
  const Nature* chosenNature;

  /* Pointer to the initial item (may change in combat) */
  const Item* initialItem;

  /* pointers to actions the pokemon may perform in combat */
  std::array<MoveNonVolatile, 4> actions;

  uint8_t numMoves;
  
  /* Pokemon's current level, from 1..100 */
  uint8_t level;
  
  /*
   * Pokemon's sex. Used for some damage calculations and moves
   * SEX_MALE: for male, 
   * SEX_FEMALE: for female, 
   * SEX_NEUTER: for other or unspecified
   */
  uint8_t sex;
  
  /* Pokemon's individual values (never referenced in combat) */
  std::array<uint8_t, 6> IV;
  
  /* Pokemon's effort values (never referenced in combat) */
  std::array<uint8_t, 6> EV;
  
  /*
   * pokemon's final values, or values used for computation.
   * 
   * includes atk, spa, def, spd, spe, hp, eva, acc, cht (using FV_ defines)
   * 
   * Stat = (((2 * BaseStat + IV + (EV / 4)) * Level / 100 + 5) * Nature)
   * HP = ((2 * BaseStat + IV + (EV / 4)) * Level / 100 + Level + 10)
   * ACC / EVA = 1
   * CRIT = .0625 * ACCURACY_EVASION_INTEGER
   * Formula: http://www.smogon.com/dp/articles/stats
   */
  std::array< std::array<uint16_t, 13>, 6> FV_base;

  static std::array< std::array<fpType, 13>, 3> aFV_base;

  /*
   * update the final value of a given pokemon_base, a value referenced in combat
   * but not modified. Modifying this value invalidates the pokemon_volatile
   * it represents.
   * 
   * targetFV:
   * FV_ATTACK 0
   * FV_SPATTACK 1
   * FV_DEFENSE 2
   * FV_SPDEFENSE 3
   * FV_SPEED 4
   * FV_HITPOINTS 5
   * FV_EVASION 6
   * FV_ACCURACY 7
   * FV_CRITICALHIT 8
   * 
   * Source: http://www.smogon.com/dp/articles/stats
   */
  void setFV(unsigned int targetFV);

  PokemonNonVolatile& operator=(const PokemonNonVolatile& other);

  PokemonNonVolatile();
  PokemonNonVolatile(const PokemonNonVolatile& orig);
  ~PokemonNonVolatile() {};

  bool pokemonExists() const;

  bool abilityExists() const;

  bool natureExists() const;
  
  const PokemonBase& getBase() const
  {
    assert(pokemonExists());
    return *base;
  };

  PokemonNonVolatile& setBase(const PokemonBase& _base)
  {
    base = &_base;
    return *this;
  };

  unsigned int getLevel() const
  {
    return level;
  };

  PokemonNonVolatile& setLevel(unsigned int _level)
  {
    level = _level;
    return *this;
  };

  unsigned int getSex() const
  {
    return sex;
  };

  PokemonNonVolatile& setSex(unsigned int _sex)
  {
    sex = _sex;
    return *this;
  };

  const Ability& getAbility() const
  {
    assert(abilityExists());
    return *chosenAbility;
  };

  PokemonNonVolatile& setAbility(const Ability& _chosenAbility);

  PokemonNonVolatile& setNoAbility();

  const Nature& getNature() const
  {
    return *chosenNature;
  };

  PokemonNonVolatile& setNoNature();

  PokemonNonVolatile& setNature(const Nature& _chosenNature);

  PokemonNonVolatile& setInitialItem(const Item& _chosenItem);

  PokemonNonVolatile& setNoInitialItem();

  bool hasInitialItem() const;

  const Item& getInitialItem() const;

  PokemonNonVolatile& setIV(size_t type, unsigned int value)
  {
    IV[type] = value;
    return *this;
  };

  unsigned int getIV(size_t type) const
  {
    return IV[type];
  };

  PokemonNonVolatile& setEV(size_t type, unsigned int value)
  {
    EV[type] = value;
    return *this;
  };

  unsigned int getEV(size_t type) const
  {
    return EV[type];
  };

  size_t getNumMoves() const
  {
    return (size_t) numMoves;
  };

  size_t getMaxNumMoves() const
  {
    return actions.size();
  };

  /* is this pokemon allowed to be on the given team according to the current ruleset? */
  bool isLegalAdd(const MoveNonVolatile& cPokemon) const;

  bool isLegalSet(size_t iAction, const MoveNonVolatile& cBase) const;

  bool isLegalAdd(const Move& cPokemon) const;

  bool isLegalSet(size_t iAction, const Move& cBase) const;

  PokemonNonVolatile& addMove(const MoveNonVolatile& cMove);

  /* set a pre-existing move to something else */
  PokemonNonVolatile& setMove(size_t iAction, const MoveNonVolatile& _cMove);

  PokemonNonVolatile& removeMove(size_t iAction);

  /* get the pokemon's move pointed to by this index.
   * normally returns the index action move, but returns
   * move "hurt confusion" if AT_MOVE_CONFUSED,
   * move "struggle" if AT_MOVE_STRUGGLE,
   * NULL if AT_MOVE_NOTHING */
  MoveNonVolatile& getMove(size_t index);

  const Move& getMove_base(size_t index) const { return getMove(index).getBase(); }

  const MoveNonVolatile& getMove(size_t index) const;

  uint32_t getFV_base(size_t type) const
  {
    return FV_base[type][STAGE0];
  };

  void createDigest_impl(std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE>& digest) const;

  /* recursively initializes this pokemon_nonvolatile and all of its moves */
  void initialize();

  void uninitialize();

  /*
   * Initialize the final values and final boosted values of this pokemon
   */
  void initFV();

  /* output a single pokemon */
  void printSummary(std::ostream& out) const;
  void output(std::ostream& oFile, bool printHeader = true) const;

  /* input a single pokemon */
  bool input(
    const std::vector<std::string>& lines, 
    size_t& iLine, 
    OrphanSet* mismatchedPokemon = NULL,
    OrphanSet* mismatchedItems = NULL,
    OrphanSet* mismatchedAbilities = NULL,
    OrphanSet* mismatchedNatures = NULL,
    OrphanSet* mismatchedMoves = NULL);
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV);


#endif	/* POKEMON_NONVOLATILE_H */

