#ifndef OTHERMOVE_H
#define OTHERMOVE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <limits>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include "../inc/vertex.h"

#include "../inc/environment_possible.h"

class minimax_threadArg;
class ply;
class agentMove;

class otherMove : public vertex
{
public:
  static otherMove* dummyRoot;
private:
#ifndef _DISABLEFINEGRAINEDLOCKING
  /* this prevents this move from being accessed concurrently */
  boost::mutex lock;
#endif

  /* vector containing newly developed plys */
  std::vector<ply*> children;

  /* vector containing the actual environments of each of this ply's children */
  std::vector<EnvironmentPossible> possibleEnvironments;
  
  /* vector containing this node's parent */
  agentMove& parent;

  /* fitness accumulator of random elements */
  fpType lbFitness;

  /* bounded from 0..1, the accumulated probability of all nodes that have returned their fitness so far */
  fpType probability;
  
  /* the other team action this class represents */
  const uint8_t action;

  /* index of the other team move with the LEAST fitness 
   * with respect to agent team, maximizing other team fitness */
  uint8_t status;
  
  otherMove();
  otherMove(const otherMove& other);
  otherMove& operator=(const otherMove& source);

  void pushbackChildren_nonLocking(minimax_threadArg& _planner, size_t numUnique, std::vector<ply*>& result);
  
  void pushbackChildren(minimax_threadArg& _planner, size_t numUnique, std::vector<ply*>& result);

  bool generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children);

  otherMove(agentMove& _parent, minimax_threadArg& _planner, unsigned int otherAction);

  /* recursively propagates fitness to lower plys */
  ply& propagateFitness(const ply& child, minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* recurse to lower nodes, then delete current node if no other nodes in this ply exist */
  ply& recurse(const ply& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated);

  /* travel forwards through the tree, deleting all nodes one comes across */
  void deleteTree_recursive();

  ply& getParent() const;

  /* determine if Apha Beta Pruning would prune this section of random moves */ 
  bool isPruned(fpType clbFitness = 0, fpType cProbability = 0) const;

public:

  template <class libraryType> std::vector<libraryType*>& getChildren()
  {
    return children;
  }

  size_t getAction() const
  {
    return action;
  }
  
  friend class ply;
  friend class agentMove;
};

#endif
