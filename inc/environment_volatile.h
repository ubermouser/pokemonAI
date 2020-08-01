/* 
 * File:   PKAI_environment_volatile.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:08 PM
 */

#ifndef ENVIRONMENT_VOLATILE_H
#define	ENVIRONMENT_VOLATILE_H

#include "../inc/pkai.h"

#include <array>
#include <stdint.h>

#include "../inc/team_volatile.h"

class EnvironmentNonvolatile;

union PKAISHARED EnvironmentVolatile
{
  uint64_t raw[16];
  struct
  {
    TeamVolatile teams[2];
  } data;
  
  /* Compares values of selected environment. Base values are compared by
   * pointer, volatile values are compared by value */
  bool operator==(const EnvironmentVolatile& other) const;
  bool operator!=(const EnvironmentVolatile& other) const;

  const TeamVolatile& getTeam(size_t movesFirst) const ;
  const TeamVolatile& getOtherTeam(size_t movesFirst) const;
  
  TeamVolatile& getTeam(size_t movesFirst);
  TeamVolatile& getOtherTeam(size_t movesFirst);

  static EnvironmentVolatile create(const EnvironmentNonvolatile& envNV);
  
  void initialize(const EnvironmentNonvolatile& envNV);
};

#endif	/* ENVIRONMENT_VOLATILE_H */

