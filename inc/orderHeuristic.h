/* 
 * File:   item.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:37 PM
 */

#ifndef ORDERHEURISTIC_H
#define	ORDERHEURISTIC_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <algorithm>
#include <vector>

//#include <boost/interprocess/detail/atomic.hpp>
#include <array>
#include <boost/thread/locks.hpp>
#include <boost/smart_ptr/detail/spinlock.hpp>

typedef boost::detail::spinlock spinlock;

class path
{
public:
  path()
    : cutoffCount(0),
    useCount(1),
    place((uint8_t)-1)
  {
  }

  uint64_t cutoffCount;
  uint64_t useCount;
  uint8_t place;

  bool operator <(const path& other) const
  {
    uint64_t thisCount = cutoffCount * other.useCount;
    uint64_t otherCount = other.cutoffCount * useCount;

    assert(thisCount >= cutoffCount || thisCount >= other.useCount);
    assert(otherCount >= other.cutoffCount || thisCount >= useCount);

    return (thisCount) < (otherCount);
  }
};

class orderHeuristic
{
private:
  std::array<spinlock, 36> locks;

  std::array<std::array<path, AT_ITEM_USE + 1>, 36> paths;
  std::array<std::array<uint8_t, AT_ITEM_USE + 1>, 36> orders;

private:
  /* call sortIniital_perPath on all paths */
  void sortInitial();

  /* performs o(N^2) sort on the given assumed unsorted path, based on its count values */
  void sortInitial_perPath(size_t iPath);
public:
  orderHeuristic(bool randomize = false);
  void incrementCutoff(uint8_t depth, size_t cPokemon, size_t oPokemon, uint8_t iAction);
  void incrementUse(uint8_t depth, size_t cPokemon, size_t oPokemon, uint8_t iAction);
  void reset(bool randomize = false);
  void seedOrdering(std::array<uint8_t, AT_ITEM_USE+1>& ordering, size_t cPokemon, size_t oPokemon, int8_t killerMove = -1);
  bool isValidOrder(std::array<uint8_t, AT_ITEM_USE+1>& ordering);
};

#endif
