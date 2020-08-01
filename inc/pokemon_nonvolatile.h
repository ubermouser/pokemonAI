/* 
 * File:   pokemon_nonvolatile.h
 * Author: Ubermouser
 *
 * Created on June 30, 2011, 10:37 PM
 */

#ifndef POKEMON_NONVOLATILE_H
#define	POKEMON_NONVOLATILE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <ostream>
#include <array>
#include <vector>

#include "../inc/signature.h"

#include "../inc/name.h"
#include "../inc/move_nonvolatile.h"

#define STAGE0 6

class Pokedex;
class Item;
class Ability;
class Nature;
class pokemon_print;
class PokemonBase;

union TeamVolatile;
union PokemonVolatile;

#define POKEMON_NONVOLATILE_DIGESTSIZE (MOVE_NONVOLATILE_DIGESTSIZE*4 + 94)

/* contains metrics that the user can modify, but are not modified in combat */
class PKAISHARED PokemonNonVolatile : public Name, public Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>
{

private:

  /* The pokemon in which this volatile set is based off of*/
  const PokemonBase* base; 
  
  /* Pointer to the ability that the pokemon has currently */
  const Ability* chosenAbility; 
  
  /* Pointer to the nature that the pokemon has */
  const Nature* chosenNature;

  /* pointers to actions the pokemon may perform in combat */
  std::array<MoveNonVolatile, 4> actions;

  uint8_t numMoves;

  /* index to the initial item (may change in combat) */
  uint8_t initialItem;
  
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

public:

  //friend class pkIO;
  friend union PokemonVolatile;
  friend union TeamVolatile;
  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const pokemon_print& combinedPokemon);
  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV);

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

  void setBase(const PokemonBase& _base)
  {
    base = &_base;
  };

  unsigned int getLevel() const
  {
    return level;
  };

  void setLevel(unsigned int _level)
  {
    level = _level;
  };

  unsigned int getSex() const
  {
    return sex;
  };

  void setSex(unsigned int _sex)
  {
    sex = _sex;
  };

  const Ability& getAbility() const
  {
    assert(abilityExists());
    return *chosenAbility;
  };

  void setAbility(const Ability& _chosenAbility);

  void setNoAbility();

  const Nature& getNature() const
  {
    return *chosenNature;
  };

  void setNoNature();

  void setNature(const Nature& _chosenNature);

  void setInitialItem(const Item& _chosenItem);

  void setNoInitialItem();

  bool hasInitialItem() const;

  const Item& getInitialItem() const;

  void setIV(size_t type, unsigned int value)
  {
    IV[type] = value;
  };

  unsigned int getIV(size_t type) const
  {
    return IV[type];
  };

  void setEV(size_t type, unsigned int value)
  {
    EV[type] = value;
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

  void addMove(const MoveNonVolatile& cMove);

  /* set a pre-existing move to something else */
  void setMove(size_t iAction, const MoveNonVolatile& _cMove);

  void removeMove(size_t iAction);

  /* get the pokemon's move pointed to by this index.
   * normally returns the index action move, but returns
   * move "hurt confusion" if AT_MOVE_CONFUSED,
   * move "struggle" if AT_MOVE_STRUGGLE,
   * NULL if AT_MOVE_NOTHING */
  MoveNonVolatile& getMove(size_t index);

  const Move& getMove_base(size_t index) const;

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
  void output(std::ostream& oFile, bool printHeader = true) const;

  /* input a single pokemon */
  bool input(
    const std::vector<std::string>& lines, 
    size_t& iLine, 
    std::vector<std::string>* mismatchedPokemon = NULL,
    std::vector<std::string>* mismatchedItems = NULL,
    std::vector<std::string>* mismatchedAbilities = NULL,
    std::vector<std::string>* mismatchedNatures = NULL,
    std::vector<std::string>* mismatchedMoves = NULL);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV);


class PKAISHARED pokemon_print
{
private:
  const PokemonNonVolatile& cPokemon;
  const TeamVolatile& cTeam;
  const PokemonVolatile& currentPokemon;

public:
  pokemon_print(const PokemonNonVolatile& _cPokemon, const TeamVolatile& _cTeam, const PokemonVolatile& _currentPokemon)
    : cPokemon(_cPokemon),
    cTeam(_cTeam),
    currentPokemon(_currentPokemon)
  {
  };

  friend PKAISHARED std::ostream& operator <<(std::ostream& os, const pokemon_print& combinedPokemon);
};

PKAISHARED std::ostream& operator <<(std::ostream& os, const pokemon_print& combinedPokemon);

#endif	/* POKEMON_NONVOLATILE_H */

