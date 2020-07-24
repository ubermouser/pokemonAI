#ifndef MOVE_H
#define	MOVE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <string>
#include <boost/array.hpp>

#include "../inc/name.h"
#include "../inc/pluggable.h"

class type;
class pokedex;

class PKAISHARED move: public name, public pluggable
{
public:
  static const move* move_struggle;
  static const move* move_none;

  /*
   * the index of the move's type.
   * 
   * 0-n: index of type 
   * UINT_MAX: move does not have an explicit type, 
   *       or the type isn't referenced in combat.
   *       May also be an orphan type
   */
  const type* cType;
  
  /*
   * base accuracy of this ability. Usually applies to damage, or a buff /
   * debuff / ailment if ability does not cause damage (standardize?)
   * 
   * -1: accuracy varies, or is undefined
   * >0: base accuracy of move
   */
  uint8_t primaryAccuracy; // base accuracy of this ability. -1 for varies, undefined
  
  /*
   * base power this ability has.
   * 
   * -1: untyped, varies, undefined
   * >0: base power of move
   */
  uint8_t power;
  
  /*
   * base number of uses this ability may have. 
   *
   * >0: number moves allowed
   */
  uint8_t PP;
  
  /*
   * 0 - does not cause damage
   * 1 - causes physical damage
   * 2 - causes special damage
   * 3 - damage not boosted by atk or spa (exceptional)
   */
  uint8_t damageType;
  
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
  int8_t target;
  
  /*
   * the priority ranking of the move.
   * Positive numbers go first, usually ranges from +6 to -7. 
   * Exceptions exist
   */
  int8_t priority;
  
  /*
   * secondary accuracy of this ability. -1 for undefined, -2 for varies
   */
  int8_t secondaryAccuracy;
  
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
  boost::array<int8_t, 9> selfBuff; //TODO: standardize buffs and debuffs
  
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
  boost::array<int8_t, 9> targetDebuff;
  
  /*
   * targetAilment:
   * AIL_NV_NONE: no status effect
   * AIL_NV_BURN: Burn
   * AIL_NV_FREEZE: Freeze
   * AIL_NV_PARALYSIS: Paralysis
   * AIL_NV_POISON: Poison
   * AIL_NV_SLEEP: Sleep
   */
  uint8_t targetAilment;
  
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
  uint8_t targetVolatileAilment;
  
  /*
   * this move's plaintext pokedex description
   */
  std::string description;

  bool lostChild;

  static bool input(const std::vector<std::string>& lines, size_t& iLine);

  bool isImplemented() const
  {
    return pluggable::isImplemented() && !lostChild;
  };

  const type& getType() const;

  bool targetsEnemy() const { return primaryAccuracy > 100; };

  int8_t getSpeedPriority() const { return priority; }

  int8_t getSelfBuff(size_t iBuff) const { return selfBuff[iBuff]; };

  int8_t getTargetDebuff(size_t iBuff) const { return targetDebuff[iBuff]; };

  uint8_t getTargetAilment() const { return targetAilment; };

  uint8_t getTargetVolatileAilment() const { return targetVolatileAilment; };

  uint8_t getPower() const { return power; };
  
  uint8_t getDamageType() const { return damageType; };

  fpType getPrimaryAccuracy() const { return (fpType)primaryAccuracy / 100.0; };

  fpType getSecondaryAccuracy() const { return (fpType)secondaryAccuracy / 100.0; };


  void init(const move& source);
  move();
  move(const move& source);
  move& operator=(const move& source);
  ~move();
};

#endif	/* MOVE_H */

