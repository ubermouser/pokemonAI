#ifndef AGENTMOVE_H
#define AGENTMOVE_H

#include "../inc/pkai.h"

#include <limits>
#include <vector>

#ifndef NDEBUG
#include <iostream>
#include <iomanip>
#endif

#include <stdint.h>
#include <boost/array.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

#include "vertex.h"

class minimax_threadArg;
class ply;
class otherMove;
union environment_possible;

class agentMove : public vertex
{
private:
#ifndef _DISABLEFINEGRAINEDLOCKING
  /* this prevents this move from being accessed concurrently */
  boost::mutex lock;
#endif

  /* vector containing other moves */
  std::vector<otherMove*> children;
  
  /* vector containing this node's parent */
  ply& parent;
  
  /* (upper bound) fitness the currentAction represents. Take minimum of this */
  fpType betaBound;

  /* (upper bound) fitness the best otherAction represents. Stored because we do comparisons against this */
  fpType ubFitness;

  /* (lower bound) fitness the best otherAction represents. */
  fpType lbFitness;

  /* for partial matches, the probability of the agentMove that has been evaluated */
  fpType probability;

  /* the agent team action this class represents */
  const uint8_t action;
  
  /* index of the other team move with the LEAST fitness 
   * with respect to agent team, maximizing other team fitness */
  int8_t bestAction;
  
  /* the last action that generated a child, or the maximum if 
   all possibilities of this branchexhausted */
  uint8_t lastAction;

  /* status of this node - may be fully evaluated, cutoff evaluated, or not evaluated at all */
  uint8_t status;

  boost::array<uint8_t, AT_ITEM_USE+1> order;
  
  agentMove();
  agentMove(const agentMove& other);
  agentMove& operator=(const agentMove& source);
  
  /* generates ONE set of siblings. Returns true if siblings generated,
   * false if siblings could not be generated. */
  bool generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children);
  
  bool generateChildren_nonLocking(minimax_threadArg& _planner, std::vector<ply*>& _children);

  bool generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children);

  bool generateChildren_backwards(minimax_threadArg& _planner, std::vector<ply*>& _children);

  agentMove(
    const std::vector<environment_possible>& possibleEnvironments, 
    minimax_threadArg& _planner, 
    unsigned int numUnique, 
    ply& _parent, 
    unsigned int agentAction, 
    unsigned int otherAction, 
    std::vector<ply*>& result);

  agentMove(ply& _parent, minimax_threadArg& _planner, uint8_t agentAction);

  /* recursively propagates fitness to lower plys */
  ply& propagateFitness(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children);

  ply& prune(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* recurse to lower nodes, then delete current node if no other nodes in this ply exist */
  ply& recurse(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated);

  /* travel forwards through the tree, deleting all nodes one comes across */
  void deleteTree_recursive();

  /* determine if Apha Beta Pruning would prune this section of othermoves */ 
  bool isPruned() const;

  /* generate the bound of this agentMove from the previous otherMove and agentMove */
  static fpType bound(const ply& _parent);

  ply& getParent() const;

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
  friend class otherMove;
  
};

#endif
