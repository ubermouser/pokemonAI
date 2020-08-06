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
#include <array>
#include <vector>

#include "../inc/name.h"

class PKAISHARED Nature: public Name
{
public:
  static const Nature* no_nature;

  /*
   * FV_ATTACK - attack
   * FV_SPATTACK - special attack
   * FV_DEFENSE - defense
   * FV_SPDEFENSE - special defense
   * FV_SPEED - speed
   * 
   * other FV values are not allocated
   */
  std::array<uint8_t, 5> modTable_;
};


class PKAISHARED Natures: public std::vector<Nature>
{
public:
  bool initialize(const std::string& path);

protected:
  bool loadFromFile(const std::string& path);
  bool loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* NATURE_H */

