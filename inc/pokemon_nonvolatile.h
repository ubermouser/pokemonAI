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
#include <iosfwd>
#include <array>
#include <vector>

#include "signature.h"
#include "serializable.h"

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
class PKAISHARED PokemonNonVolatile
  : public Name,
    public Signature<PokemonNonVolatile, POKEMON_NONVOLATILE_DIGESTSIZE>,
    public Serializable<PokemonNonVolatile> {
public:
  using OrphanSet = orphan::OrphanSet;
  using Orphanage = orphan::Orphanage;

  /* The pokemon in which this volatile set is based off of*/
  const PokemonBase* base_;
  
  /* Pointer to the ability that the pokemon has currently */
  const Ability* chosenAbility_;
  
  /* Pointer to the nature that the pokemon has */
  const Nature* chosenNature_;

  /* Pointer to the initial item (may change in combat) */
  const Item* initialItem_;

  /* pointers to actions the pokemon may perform in combat */
  std::vector<MoveNonVolatile> actions_;
  
  /* Pokemon's current level, from 1..100 */
  uint8_t level_;
  
  /*
   * Pokemon's sex. Used for some damage calculations and moves
   * SEX_MALE: for male, 
   * SEX_FEMALE: for female, 
   * SEX_NEUTER: for other or unspecified
   */
  uint8_t sex_;
  
  /* Pokemon's individual values (never referenced in combat) */
  std::array<uint8_t, 6> IV_;
  
  /* Pokemon's effort values (never referenced in combat) */
  std::array<uint8_t, 6> EV_;
  
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
  std::array< std::array<uint16_t, 13>, 6> FV_base_;

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

  PokemonNonVolatile();
  PokemonNonVolatile(const PokemonNonVolatile& orig) = default;
  PokemonNonVolatile& operator=(const PokemonNonVolatile& other) = default;
  ~PokemonNonVolatile() = default;

  bool pokemonExists() const;

  bool abilityExists() const;

  bool natureExists() const;
  
  const PokemonBase& getBase() const {
    return *base_;
  };

  PokemonNonVolatile& setNoBase();
  PokemonNonVolatile& setBase(const PokemonBase& _base);

  unsigned int getLevel() const {
    return level_;
  };

  PokemonNonVolatile& setLevel(unsigned int _level);

  unsigned int getSex() const {
    return sex_;
  };

  PokemonNonVolatile& setSex(unsigned int _sex);

  const Ability& getAbility() const
  {
    return *chosenAbility_;
  };

  PokemonNonVolatile& setAbility(const Ability& _chosenAbility);

  PokemonNonVolatile& setNoAbility();

  const Nature& getNature() const {
    return *chosenNature_;
  };

  PokemonNonVolatile& setNoNature();

  PokemonNonVolatile& setNature(const Nature& _chosenNature);

  PokemonNonVolatile& setInitialItem(const Item& _chosenItem);

  PokemonNonVolatile& setNoInitialItem();

  bool hasInitialItem() const;

  const Item& getInitialItem() const;

  PokemonNonVolatile& setIV(size_t type, unsigned int value);

  unsigned int getIV(size_t type) const
  {
    return IV_[type];
  };

  PokemonNonVolatile& setZeroEV();
  PokemonNonVolatile& setEV(size_t type, unsigned int value);

  unsigned int getEV(size_t type) const
  {
    return EV_[type];
  };

  size_t getNumMoves() const { return actions_.size(); };
  size_t getMaxNumMoves() const { return 4; };

  bool isLegalAbility(const Ability& ability) const;

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
  MoveNonVolatile& getMove(const Action& action) {
    return getMove(action.iMove());
  }

  const Move& getMove_base(size_t index) const { return getMove(index).getBase(); }
  const Move& getMove_base(const Action& action) const { return getMove(action).getBase(); }

  const MoveNonVolatile& getMove(size_t index) const;
  const MoveNonVolatile& getMove(const Action& action) const {
    return getMove(action.iMove());
  }

  uint32_t getFV_base(size_t type) const
  {
    return FV_base_[type][STAGE0];
  };

  void createDigest_impl(std::array<uint8_t, POKEMON_NONVOLATILE_DIGESTSIZE>& digest) const;

  /* recursively initializes this pokemon_nonvolatile and all of its moves */
  void initialize();

  void uninitialize();

  /*
   * Initialize the final values and final boosted values of this pokemon
   */
  void initFV();


  const std::string& defineName();

  /* output a single pokemon */
  void printSummary(std::ostream& out) const;


  void input(const boost::property_tree::ptree& ptree) override;
  void input(const boost::property_tree::ptree& ptree, Orphanage& orphanage);
  boost::property_tree::ptree output(bool printHeader = true) const override;
};


PKAISHARED std::ostream& operator <<(std::ostream& os, const PokemonNonVolatile& cPKNV);


#endif	/* POKEMON_NONVOLATILE_H */

