/* 
 * File:   nature.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:54 PM
 */

#ifndef NATURE_H
#define	NATURE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <boost/array.hpp>

#include "../inc/name.h"

class PKAISHARED nature: public name
{
public:
  static const nature* no_nature;

  /*
   * FV_ATTACK - attack
   * FV_SPATTACK - special attack
   * FV_DEFENSE - defense
   * FV_SPDEFENSE - special defense
   * FV_SPEED - speed
   * 
   * other FV values are not allocated
   */
  boost::array<uint8_t, 5> modTable;
};

#endif	/* NATURE_H */

