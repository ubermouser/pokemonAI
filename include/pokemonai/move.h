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
  
  /*
   * -1: varies / unknown
   *  0: may target self only
   *  1: may target a pokemon other than the self
   *  2: may target one of the currently deployed friendly team
   *  3: may target one of the currently deployed enemy team
   *  4: targets the currently deployed friendly team
   *  5: targets the currently deployed enemy team
   *  6: targets all currently deployed pokemon except for the self
   *  7: targets all pokemon, including the self
   */
  int32_t target_ = -1;
  
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

  bool lostChild = true;

  static bool input(const std::vector<std::string>& lines, size_t& iLine);

  bool isImplemented() const
  {
    return Pluggable::isImplemented() && !lostChild;
  };

  const Type& getType() const;

  bool targetsEnemy() const { return primaryAccuracy_ > 0; };
  bool targetsAlly() const { return target_ == 8; } // TODO(@drendleman) make targeting sane

  int32_t getSpeedPriority() const { return priority_; }

  int32_t getSelfBuff(size_t iBuff) const { return selfBuff_[iBuff]; };

  int32_t getTargetDebuff(size_t iBuff) const { return targetDebuff_[iBuff]; };

  uint32_t getTargetAilment() const { return targetAilment_; };

  uint32_t getTargetVolatileAilment() const { return targetVolatileAilment_; };

  uint32_t getPower() const { return power_; };
  
  uint32_t getDamageType() const { return damageType_; };

  fpType getPrimaryAccuracy() const { return (fpType)primaryAccuracy_ / 100.0; };

  fpType getSecondaryAccuracy() const { return (fpType)secondaryAccuracy_ / 100.0; };


  Move(
    const std::string& name,
    const Type* type,
    int32_t primaryAccuracy,
    uint32_t power,
    uint32_t PP,
    uint32_t damageType,
    int32_t target,
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

