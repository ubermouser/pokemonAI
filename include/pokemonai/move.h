#ifndef MOVE_H
#define	MOVE_H

#include "pokemonai/pkai.h"

#include <stdint.h>
#include <string>
#include <array>

#include "pokemonai/name.h"
#include "pokemonai/collection.h"
#include "pokemonai/pluggable.h"

class Type;
class Types;
class Pokedex;

class PKAISHARED Move: public Name, public Pluggable
{
public:
  using BuffModArray = std::array<int32_t, 9>;

  static const Move* move_struggle;
  static const Move* move_none;

  /*
   * the index of the move's type.
   * 
   * pointer: index of type
   * NULL: move does not have an explicit type,
   *       or the type isn't referenced in combat.
   *       May also be an orphan type
   */
  const Type* type_ = NULL;
  
  /*
   * base accuracy of this ability. Usually applies to damage, or a buff /
   * debuff / ailment if ability does not cause damage (standardize?)
   * 
   * -1: accuracy varies, or is undefined
   * >0: base accuracy of move
   */
  int32_t primaryAccuracy_ = -1; // base accuracy of this ability. -1 for varies, undefined
  
  /*
   * base power this ability has.
   * 
   * -1: untyped, varies, undefined
   * >0: base power of move
   */
  uint32_t power_ = 0;
  
  /*
   * base number of uses this ability may have. 
   *
   * >0: number moves allowed
   */
  uint32_t PP_ = 0;
  
  /*
   * 0 - does not cause damage
   * 1 - causes physical damage
   * 2 - causes special damage
   * 3 - damage not boosted by atk or spa (exceptional)
   */
  uint32_t damageType_ = 0;

  // clang-format off
  enum TargetType : int {
    UNKNOWN = -1,           /* varies / unknown */
    SELF = 0,               /* may target self only (defense curl, protect) */
    ANY_ADJACENT = 1,       /* may target an adjacent pokemon (tackle, charm) */
    ANY_ADJACENT_ALLY = 2,  /* may target an adjacent ally (helping hand) */
    ANY_ADJACENT_ENEMY = 3, /* may target an adjacent enemy (me first) */
    ANY_ADJACENT_ALLY_SELF = 4, /* may target an adjacent ally or self (acupressure) */
    ANY_ACTIVE = 5,         /* may target any active pokemon (gust, peck) */
    ANY_ALLY = 6,           /* may target any living ally, active or not (baton pass, u-turn) */
    ALL_ADJACENT = 7,       /* targets all adjacent pokemon (earthquake, explosion) */
    ALL_ADJACENT_ENEMY = 8, /* targets all adjacent enemy pokemon (growl, blizzard) */
    ALL_ADJACENT_ALLY = 9,  /* targets all adjacent friendly pokemon */
    ALL_ACTIVE_ALLIES = 10,  /* targets all active teammates (coaching) */
    ALL_ACTIVE_ENEMIES = 11, /* targets all active enemies */
    ALL_ACTIVE = 12,        /* targets all active pokemon (perish song) */
    SIDE_ALLY = 13,         /* targets the allied side (mist, light screen) */
    SIDE_ENEMY = 14,        /* targets the enemy side (spikes, stealth rock) */
    SIDE_ALL = 15,          /* targets the entire battlefield (haze, rain dance) */
    ALL_ALLIES = 16,        /* targets all living teammates, active or not (heal bell, aromatherapy) */
    ALL_ENEMIES = 17,       /* targets all living enemies, active or not */
    ALL_FIELD = 18,         /* targets all living pokemon, including the self, active or not */
  };
  // clang-format on
  static TargetType targetTypeFromString(const std::string& str);
  TargetType target_ = TargetType::UNKNOWN;

  /*
   * the priority ranking of the move.
   * Positive numbers go first, usually ranges from +6 to -7. 
   * Exceptions exist
   */
  int32_t priority_ = 0;
  
  /*
   * secondary accuracy of this ability. -1 for undefined, -2 for varies
   */
  int32_t secondaryAccuracy_ = -1;
  
  /*
   *selfBuff: positive implies a buff
   * includes atk, spa, def, spd, spe, hp, eva, acc, cht (using FV_ defines)
   * 
   * 0: Attack (de)buff
   * 1: Special Attack (de)buff
   * 2: Defense (de)buff
   * 3: Special Defense (de)buff
   * 4: Speed (de)buff
   * 5: Evasion (de)buff
   * 6: Critical Hit (de)buff
   */
  BuffModArray selfBuff_; //TODO: standardize buffs and debuffs
  
  /*
   *targetDebuff: positive implies a debuff
   * includes atk, spa, def, spd, spe, hp, eva, acc, cht (using FV_ defines)
   * 
   * 0: Attack (de)buff
   * 1: Special Attack (de)buff
   * 2: Defense (de)buff
   * 3: Special Defense (de)buff
   * 4: Speed (de)buff
   * 5: Evasion (de)buff
   * 6: Accuracy (de)buff
   */
  BuffModArray targetDebuff_;
  
  /*
   * targetAilment:
   * AIL_NV_NONE: no status effect
   * AIL_NV_BURN: Burn
   * AIL_NV_FREEZE: Freeze
   * AIL_NV_PARALYSIS: Paralysis
   * AIL_NV_POISON: Poison
   * AIL_NV_SLEEP: Sleep
   */
  uint32_t targetAilment_ = AIL_NV_NONE;
  
  /*
   * targetVolatileAilment:
   * AIL_V_NONE : none
   * AIL_V_CONFUSED: Confusion
   * AIL_V_FLINCH: Flinch
   * ?: Identify
   * AIL_V_INFATUATED: Infatuation
   * ?: Leech Seed
   * ?: Lock On
   * ?: Nightmare
   * ?: Partial Trap
   */
  uint32_t targetVolatileAilment_ = AIL_V_NONE;
  
  /*
   * this move's plaintext pokedex description
   */
  std::string description_;

  /*
   * index of this move in the Moves collection
   */
  size_t index_ = 0;

  /*
   * true if this move references variables or plugins that do not exist
   */
  bool lostChild = true;

  static bool input(const std::vector<std::string>& lines, size_t& iLine);

  bool isImplemented() const
  {
    return Pluggable::isImplemented() && !lostChild;
  };

  const Type& getType() const;

  bool targetsSelf() const { return target_ == TargetType::SELF; };
  bool targetsSingleOther() const { return target_ == TargetType::ANY_ADJACENT; };
  bool targetsSingleAlly() const { return target_ == TargetType::ANY_ADJACENT_ALLY; };
  bool targetsSingleOpponent() const { return target_ == TargetType::ANY_ADJACENT_ENEMY; };
  bool targetsAllAllies() const { return target_ == TargetType::ALL_ACTIVE_ALLIES; };
  bool targetsAllOpponents() const { return target_ == TargetType::ALL_ACTIVE_ENEMIES; };
  bool targetsAdjacentOthers() const { return target_ == TargetType::ALL_ADJACENT; };
  bool targetsAllField() const { return target_ == TargetType::ALL_FIELD; };

  bool targetsMultiple() const { return target_ >= TargetType::ALL_ADJACENT; };

  [[deprecated]] bool legacyTargetsEnemy() const {
    return primaryAccuracy_ > 0;
  };
  [[deprecated]] bool legacyTargetsAlly() const { return target_ == ANY_ALLY; }

  bool targetsEnemy() const {
    // TODO - set or bitmask operaton
    return target_ == ANY_ADJACENT_ENEMY || target_ == ANY_ADJACENT ||
           target_ == ANY_ACTIVE || target_ == ALL_ADJACENT_ENEMY ||
           target_ == ALL_ADJACENT || target_ == ALL_ACTIVE_ENEMIES ||
           target_ == ALL_ACTIVE || target_ == ALL_ENEMIES ||
           target_ == ALL_FIELD;
  };
  bool targetsAlly() const {
    // TODO - set or bitmask operaton
    return target_ == SELF || target_ == ANY_ADJACENT_ALLY ||
           target_ == ANY_ADJACENT ||
           target_ == ANY_ADJACENT_ALLY_SELF || target_ == ANY_ACTIVE ||
           target_ == ANY_ALLY || target_ == ALL_ADJACENT_ALLY ||
           target_ == ALL_ADJACENT || target_ == ALL_ACTIVE_ALLIES ||
           target_ == ALL_ACTIVE || target_ == ALL_ALLIES ||
           target_ == ALL_FIELD;
  };

  int32_t getSpeedPriority() const { return priority_; }

  int32_t getSelfBuff(size_t iBuff) const { return selfBuff_[iBuff]; };

  int32_t getTargetDebuff(size_t iBuff) const { return targetDebuff_[iBuff]; };

  uint32_t getTargetAilment() const { return targetAilment_; };

  uint32_t getTargetVolatileAilment() const { return targetVolatileAilment_; };

  uint32_t getPower() const { return power_; };
  
  uint32_t getDamageType() const { return damageType_; };

  const std::string& getDamageTypeName() const;

  const std::string& getDescription() const { return description_; };

  FixType getPrimaryAccuracy() const {
    return FixType((double)primaryAccuracy_ / 100.0);
  };

  FixType getSecondaryAccuracy() const {
    return FixType((double)secondaryAccuracy_ / 100.0);
  };


  Move(
      const std::string& name,
      const Type* type,
      int32_t primaryAccuracy,
      uint32_t power,
      uint32_t PP,
      uint32_t damageType,
      TargetType target,
      int32_t priority,
      int32_t secondaryAccuracy,
      const BuffModArray& selfBuff,
      const BuffModArray& targetDebuff,
      uint32_t targetAilment,
      uint32_t targetVolatileAilment,
      bool hasPlugins,
      const std::string& description);
  Move() = default;
  Move(const Move& source) = default;
  Move& operator=(const Move& source) = default;
  virtual ~Move() = default;
};


class PKAISHARED Moves: public Collection<Move>
{
public:
  bool initialize(const std::string& path, const Types& types);

protected:
  bool loadFromFile(const std::string& path, const Types& types);
  bool loadFromFile_lines(const Types& types, const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* MOVE_H */
