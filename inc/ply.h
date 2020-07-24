/* 
 * File:   ply.h
 * Author: Ubermouser
 *
 * Created on July 1, 2011, 6:38 AM
 */

#ifndef PLY_H
#define	PLY_H

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

#include "../inc/transposition_table.h"

#include "../inc/vertex.h"

union environment_possible;
union table_entry;
class minimax_threadArg;
class agentMove;
class otherMove;





class ply : public vertex
{
public:
  static ply* dummyRoot;
private:
#ifndef _DISABLEFINEGRAINEDLOCKING
  /* this prevents this move from being accessed concurrently */
  boost::mutex lock;
#endif
  
  /* vector containing other moves */
  std::vector<agentMove*> children;
  
  /* vector containing this node's parent 
   * TODO: pointer should be const, must not change */
  otherMove& parent;

  /* the current environment this environment possible represents */
  const environment_possible& envP;

  /* transposition table entry that this node refers to, or NULL if so far unique */
  table_entry tNode;

  /* the window this ply needs to score above */
  fpType alphaBound;

  /* the upper bound window of the root node, or the bound of the best agent ply */
  fpType initialBetaBound;

  /* (lower bound) fitness the best agentAction represents */
  fpType lbFitness;

  /* corresponding probability to the lowerBound's fitness */
  fpType probability;

  /* the agent team action this class represents */
  const uint16_t action;
  
  /* depth of this node in the tree
   * equals 0 if root depth */
  uint8_t depth;

  /* status of this node - may be fully evaluated, cutoff evaluated, or not evaluated at all */
  uint8_t status;

  /* the index of the last action that generated a child, or the maximum if 
   all possibilities of this branchexhausted */
  uint8_t lastAction;

  /* priority of this node in the plyQueue */
  uint64_t priority;

  /* index of the agent team move with the GREATEST fitness 
   * with respect to agent team, maximizing agent team fitness */
  int8_t bestAction;

  /* index of the other team move with the LEAST fitness
   * with respect to agent team, maximizing other team fitness */
  int8_t bestChildAction;

  boost::array<uint8_t, AT_ITEM_USE+1> order;
  
  ply();
  ply(const ply& orig);
  ply& operator=(const ply& source);
  
  static uint64_t getPriority(
    uint8_t depth, 
    uint16_t _plyMove, 
    uint8_t agentMove, 
    uint8_t otherMove, 
    uint8_t status,
    fpType minProbability, 
    fpType maxProbability, 
    fpType probability);
  
  /* recursively propagates fitness to lower plys */
  ply& propagateFitness(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* prune a non leaf node */
  ply& prune(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* recurse to lower nodes, then delete current node if no other nodes in this ply exist */
  ply& recurse(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated);

  /* travel forwards through the tree, deleting all nodes one comes across */
  void deleteTree_recursive();
  
  bool generateChildren_backwards(minimax_threadArg& _planner, std::vector<ply*>& _children);

  bool generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockChild);
  bool generateChildren_variableLock(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockChild);
  bool generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockParent);

  unsigned int getLastAction()
  {
#ifndef	_DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif
    return lastAction;
  }

  /* generate the bound of this ply from the previous otherMove and ply */
  static fpType bound(const otherMove& parent, fpType cProbability);

public:

  friend class agentMove;
  friend class otherMove;

  template <class libraryType> std::vector<libraryType*>& getChildren()
  {
    return children;
  }

  uint64_t getPriority() const
  {
    return priority;
  }

  unsigned int getDepth() const
  {
    return depth;
  }

  int getMaxDepth() const
  {
    if (!tNode.inUse() ) { return depth; }
    else { return (signed)depth - (signed)tNode.getDepth(); }
  }

  fpType getAlpha() const
  {
    return alphaBound;
  };

  fpType getBeta() const
  {
    return initialBetaBound;
  }

  fpType getLowerBound() const
  {
    return lbFitness;
  }

  fpType getUpperBound() const
  {
    return lbFitness + (1 - probability);
  }

  fpType getProbability() const
  {
    return probability;
  }

  size_t getAction() const
  {
    return action;
  }

  int getBestAction() const
  {
    return bestAction;
  }

  int getBestChildAction() const
  {
    return bestChildAction;
  }

  unsigned int getStatus() const
  {
    return status;
  };

  const environment_possible& getEnvP() const
  {
    return envP;
  }

  const table_entry& getTNode() const
  {
    return tNode;
  };

  bool isEvaluated() const
  {
    return (lastAction==UINT8_MAX)?false:(lastAction >= order.size());
  };

  bool isFullyEvaluated() const
  {
    return ((status & 1) == NODET_FULLEVAL) /*&& mostlyGTE(probability, 1.0)*/;
  }

  bool isLeaf() const
  {
    return (status < 2)?true:false;
  };
  
  ply& getParent() const;

  bool operator >(const ply& other) const
  {
    return getPriority() > other.getPriority();
  }

  bool operator <(const ply& other) const
  {
    return getPriority() < other.getPriority();
  }
  
  /* generates ONE set of siblings. Returns true if siblings generated,
   * false if siblings could not be generated. */
  bool generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children);

  bool generateSiblings(minimax_threadArg& _planner, std::vector<ply*>& _children);
  
  /* recursively propagate fitness to lower plys, doesn't start at a leaf node */
  ply& propagateFitness(fpType _lbfitness, fpType _probability, int8_t _bestAction, int8_t _bestChildAction, minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* prune this leaf node */
  ply& prune(minimax_threadArg& _planner, std::vector<ply*>& _children);

  /* travel forwards through the tree, deleting all nodes one comes across */
  void deleteTree();

  /* resets a root node to its initial state */
  void reset(
    minimax_threadArg& _planner, 
    fpType _alphaBound = -std::numeric_limits<fpType>::infinity(),
    fpType _betaBound = std::numeric_limits<fpType>::infinity());
  
  /* generates child, no lower level propagation */
  ply(
    const environment_possible& possibleEnvironment, 
    minimax_threadArg& _planner, 
    ply& _randomMove, 
    agentMove& _agentMove, 
    otherMove& _otherMove, 
    uint16_t randomAction, 
    fpType minProbability, 
    fpType maxProbability);

  /* generates a root node */
  ply (
    const environment_possible& rootEnvironment, 
    minimax_threadArg& _planner,
    fpType _alphaBound = -std::numeric_limits<fpType>::infinity(),
    fpType _betaBound = std::numeric_limits<fpType>::infinity());

  void loadTNode(minimax_threadArg& _planner, uint16_t rAction, uint8_t aAction, uint8_t oAction, uint16_t cAction, fpType minProb, fpType maxProb);

  /* determine if Apha Beta Pruning would prune this ply */ 
  bool isPruned(bool recurse = true) const;
  
};

#endif	/* PLY_H */

