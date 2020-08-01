//#define PKAI_IMPORT
#include "../inc/environment_possible.h"
#include "../inc/pkCU.h"

#include "../inc/fp_compare.h"

#include "../inc/planner_minimax_thread.h"

#include "../inc/orderHeuristic.h"
#include "../inc/transposition_table.h"

#include "../inc/vertex.h"
#include "../inc/agentMove.h"
#include "../inc/otherMove.h"

#include "../inc/ply.h"

ply* ply::dummyRoot = NULL;





ply::ply(
  const EnvironmentPossible& possibleEnvironment, 
  minimax_threadArg& _planner, 
  ply& _randomMove, 
  agentMove& _agentMove, 
  otherMove& _otherMove, 
  uint16_t randomAction, 
  fpType minProbability, 
  fpType maxProbability)
  :
#ifndef _DISABLEFINEGRAINEDLOCKING
  lock(),
#endif
  children(), 
  parent(_otherMove), 
  envP(possibleEnvironment), 
#ifndef _DISABLETTABLE
  tNode(_planner.getTTable().findElement(possibleEnvironment)),
#else
  tNode(),
#endif
#ifndef _DISABLETIGHTBOUNDS
  alphaBound(bound(_otherMove, possibleEnvironment.getProbability().to_double())),
#else
  alphaBound(-std::numeric_limits<fpType>::infinity()),
#endif
  initialBetaBound(std::numeric_limits<fpType>::infinity()),
  lbFitness(-std::numeric_limits<fpType>::infinity()), // TODO: can we change this to 0.0?
  probability(0.0),
  action(randomAction), 
  depth(_randomMove.depth - 1),
  status(
    (_planner.getCU().isGameOver(possibleEnvironment.getEnv()) != MATCH_MIDGAME)
      ?NODET_FULLEVAL
      :((depth==0)
        ?NODET_CUTOFF
        :NODET_NOTEVAL)
    ),
  lastAction((status!=NODET_NOTEVAL)?((uint8_t)order.size() + 1):(UINT8_MAX)),
  priority(getPriority(
    depth, 
    _randomMove.action, 
    _agentMove.action, 
    _otherMove.action, 
    status,
    minProbability, 
    maxProbability, 
    possibleEnvironment.getProbability().to_double())),
  bestAction(-1),
  bestChildAction(-1),
  order()
{
  assert(mostlyLTE(possibleEnvironment.getProbability(), fixedpoint::create<30>(1.0)) && mostlyGT(possibleEnvironment.getProbability(), fixedpoint::create<30>(0.0))); // check probability sanity

  loadTNode(_planner, _randomMove.action, _agentMove.action, _otherMove.action, action, minProbability, maxProbability);
} // endOf ply constructor

ply::ply (
  const EnvironmentPossible& rootEnvironment, 
  minimax_threadArg& _planner,
  fpType _alphaBound,
  fpType _betaBound)
  :
#ifndef _DISABLEFINEGRAINEDLOCKING
  lock(),
#endif
  children(),
  parent(*otherMove::dummyRoot),
  envP(rootEnvironment),
#ifndef _DISABLETTABLE
  tNode(_planner.getTTable().findElement(rootEnvironment)),
#else
  tNode(),
#endif
#ifndef _DISABLETIGHTBOUNDS
  alphaBound(_alphaBound),
  initialBetaBound(_betaBound),
#else
  alphaBound(-std::numeric_limits<fpType>::infinity()),
  initialBetaBound(std::numeric_limits<fpType>::infinity()),
#endif
  lbFitness(-std::numeric_limits<fpType>::infinity()),
  probability(0),
  action(0),
  depth((uint8_t)_planner.getMaxDepth()),
  status((_planner.getCU().isGameOver(rootEnvironment.getEnv()) != MATCH_MIDGAME)
      ?NODET_FULLEVAL
      :((depth==0)
        ?NODET_CUTOFF
        :NODET_NOTEVAL)
    ),
  lastAction((status!=NODET_NOTEVAL)?((uint8_t)order.size() + 1):(UINT8_MAX)),
  priority(getPriority(depth, 0, 0, 0, status, 0.0, 0.0, 0.0)),
  bestAction(-1),
  bestChildAction(-1),
  order()
{
  assert(mostlyLTE(rootEnvironment.getProbability(), fixedpoint::create<30>(1.0)) && mostlyGT(rootEnvironment.getProbability(), fixedpoint::create<30>(0.0))); // check probability sanity
  
  loadTNode(_planner, 0, 0, 0, 0, 0.0, 1.0);
}// endOf ply constructor

void ply::reset(minimax_threadArg& _planner, fpType _alphaBound, fpType _betaBound)
{
  assert(children.empty());
  assert(&parent == otherMove::dummyRoot);

  alphaBound = _alphaBound;
  initialBetaBound = _betaBound;

  lbFitness = -std::numeric_limits<fpType>::infinity();
  probability = 0.0;

  depth = (uint8_t)_planner.getMaxDepth();

  status = (_planner.getCU().isGameOver(envP.getEnv()) != MATCH_MIDGAME)
      ?NODET_FULLEVAL
      :((depth==0)
        ?NODET_CUTOFF
        :NODET_NOTEVAL);

  lastAction = (status!=NODET_NOTEVAL)?((uint8_t)order.size() + 1):(UINT8_MAX);

  priority = getPriority(depth, 0, 0, 0, status, 0.0, 0.0, 0.0);

  bestAction = -1;
  bestChildAction = -1;

#ifndef _DISABLETTABLE
  tNode = _planner.getTTable().findElement(envP);
#endif

  loadTNode(_planner, 0, 0, 0, 0, 0.0, 1.0);
};





void ply::loadTNode(minimax_threadArg& _planner, uint16_t rAction, uint8_t aAction, uint8_t oAction, uint16_t cAction, fpType minProb, fpType maxProb)
{
  if (!tNode.inUse())
  {
    _planner.getAgentOrder().seedOrdering(
      order, 
      envP.getEnv().getTeam(_planner.getAgentTeam()).getICPKV(),
      envP.getEnv().getOtherTeam(_planner.getAgentTeam()).getICPKV());
  }
  else
  {
    fpType cProbability;
    fpType clbFitness;
    uint8_t cDepth;
    int8_t cBestAgentMove;
    int8_t cBestOtherMove;
    uint8_t cStatus;
    tNode.getValues(cBestAgentMove, cBestOtherMove, cDepth, clbFitness, cProbability, cStatus);

    // if the transposition table data has not been fully evaluated 
    //  and is at a depth shallower than the depth we need to search:
    if (((cStatus & 1) != NODET_FULLEVAL) && cDepth < depth)
    {
      // our evaluation is insufficiently bound, so take the best guess
      _planner.getAgentOrder().seedOrdering(
        order, 
        envP.getEnv().getTeam(_planner.getAgentTeam()).getICPKV(),
        envP.getEnv().getOtherTeam(_planner.getAgentTeam()).getICPKV(),
        cBestAgentMove);
#ifndef NDEBUG
        if (verbose >= 7)
        {
          boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
          std::cout << "TR-K" <<
            " d" << std::setw(2) << std::dec << (unsigned int) depth*3 <<
            " i" << std::setw(2) << (unsigned int) rAction <<
            " a" << std::setw(2) << (unsigned int) aAction <<
            " o" << std::setw(2) << (unsigned int) oAction <<
            " #" << std::setw(3) << (unsigned int) cAction <<
            " 0x" << std::setfill('0') << std::setw(16) << std::hex << envP.getHash() << std::setfill(' ') <<
            " k-D" << std::setw(2) << std::dec << (unsigned int) cDepth*3 <<
            " k-A" << std::setw(2) << (int) cBestAgentMove <<
            " k-O" << std::setw(2) << (int) cBestOtherMove <<
            "\n";
          std::cout.flush();
        }
#endif
    }
    // if the transposition table data has either been fully evaluated
    //  or has been evaluated to a depth at or deeper than the depth we need to search:
    else
    {
      bestAction = cBestAgentMove;
      bestChildAction = cBestOtherMove;
      lbFitness = clbFitness;
      probability = cProbability;
      status = cStatus;

      if (mostlyEQ(cProbability, 1.0))
      {
        lastAction = (uint8_t) order.size() + 1;
        // nodes that are fully evaluated are computed first
        priority = getPriority( // TODO: can we just replace the status portion of this?
          depth, 
          rAction, 
          aAction, 
          oAction, 
          status,
          minProb, 
          maxProb, 
          envP.getProbability().to_double());
#ifndef NDEBUG
        if (verbose >= 7)
        {
          boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
          std::cout << "TRAN" <<
            " d" << std::setw(2) << std::dec << (unsigned int) depth*3 <<
            " i" << std::setw(2) << (unsigned int) rAction <<
            " a" << std::setw(2) << (unsigned int) aAction <<
            " o" << std::setw(2) << (unsigned int) oAction <<
            " #" << std::setw(3) << (unsigned int) cAction <<
            " 0x" << std::setfill('0') << std::setw(16) << std::hex << envP.getHash() << std::setfill(' ') <<
            " k-D" << std::setw(2) << std::dec << (unsigned int) cDepth*3 <<
            " k-F" << std::setw(10) << clbFitness <<
            " k-P" << std::setw(10) << cProbability <<
            "\n";
          std::cout.flush();
        }
#endif
      }
      else
      {
        alphaBound = std::max(clbFitness, alphaBound);
        initialBetaBound = std::min(clbFitness + (1.0 - cProbability),initialBetaBound);

        // we did not perform a full evaluation, so we may need to evaluate this node normally
        _planner.getAgentOrder().seedOrdering(
          order, 
          envP.getEnv().getTeam(_planner.getAgentTeam()).getICPKV(),
          envP.getEnv().getOtherTeam(_planner.getAgentTeam()).getICPKV(),
          cBestAgentMove);
#ifndef NDEBUG
        if (verbose >= 7)
        {
          boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
          std::cout << "TR-K" <<
            " d" << std::setw(2) << std::dec << depth*3 <<
            " i" << std::setw(2) << (unsigned int) rAction <<
            " a" << std::setw(2) << (unsigned int) aAction <<
            " o" << std::setw(2) << (unsigned int) oAction <<
            " #" << std::setw(3) << (unsigned int) cAction <<
            " 0x" << std::setfill('0') << std::setw(16) << std::hex << envP.getHash() << std::setfill(' ') <<
            " k-D" << std::setw(2) << std::dec << (unsigned int) cDepth*3 <<
            " k-A" << std::setw(2) << (int) cBestAgentMove <<
            " k-O" << std::setw(2) << (int) cBestOtherMove <<
            "\n";
          std::cout.flush();
        }
#endif
      } //endOf bounds and not solution
    } // endOf depth not great enough
  } // endOf tNode exists
}





fpType ply::bound(const otherMove& _parent, fpType cProbability)
{
  // determine lower bounded fitness from otherMove:
  fpType cubFitness = _parent.lbFitness + (1 - _parent.probability); // upper bound of previously evaluated fitness
  fpType oldAlphaBound = _parent.parent.parent.alphaBound; // previous bound

  fpType dBound = (oldAlphaBound - (cubFitness - cProbability));
  if (mostlyEQ(dBound, 1.0))
  {
    dBound = 1.0;
  }

  fpType result = dBound / cProbability;

  assert(mostlyLTE(result, 1.0));
  //assert(result <= oldAlphaBound);
  //assert(result <= _parent->parent->betaBound);
  return result;
}





ply& ply::prune(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result = dummyRoot; // this statement defined because we always propagate leaf nodes
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    // attempt to lock the current node, as we want to know if we shoud prune it
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    // set plys with uninitialized fitnesses to something sane
    if (mostlyEQ(probability, 0.0))
    {
      lbFitness = 0.0;
      probability = 0.0;
    }
    else
    {
      // add node to transposition table if it doesn't exist, or update if it exists
      _planner.getTTable().updateElement(
        envP,
        bestAction,
        bestChildAction,
        depth,
        lbFitness,
        probability,
        status);
    }

    // this node is implicitly a PV node but has not been evaluated
    //bestBetaBound = 0.0;
    //alphaBound = 1.0;

    // tell parent to prune this node (and impicilty all of its siblings)
    //result = parent->prune(this, _planner, _children);
    if (&parent != otherMove::dummyRoot) 
    {
      result = &parent.propagateFitness(*this, _planner, _children);
    }
  }

  // it is implied that this node is not a root node by this being called as the leaf node prune function
  if (&parent != otherMove::dummyRoot) { delete this; }

  return *result;
}





ply& ply::propagateFitness(fpType _lbfitness, fpType _probability, int8_t _bestAction, int8_t _bestChildAction, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result = dummyRoot; // this statement defined because we always propagate leaf nodes
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    lbFitness = _lbfitness;
    probability = _probability;
    bestAction = _bestAction;
    bestChildAction = _bestChildAction;

    assert(status != NODET_NOTEVAL);
    assert(mostlyLTE(lbFitness, probability) && mostlyGTE(lbFitness, 0.0)); // check lowerbound fitness
    assert(mostlyLTE(probability, 1.0) && mostlyGTE(probability, lbFitness)); // check probability (upperbound)

#ifndef _DISABLETTABLE
    // add node to transposition table if it doesn't exist, or update if it exists
    _planner.getTTable().updateElement(
      envP,
      bestAction,
      bestChildAction,
      depth,
      lbFitness,
      probability,
      status);
#endif
    
    if (&parent != otherMove::dummyRoot) 
    { 
      result = &parent.propagateFitness(*this, _planner, _children); // continue to recurse
    }
  } // implicitly unlock mutex

  // it is implied that this node is not a root node by this being called as the leaf node prune function
  if (&parent != otherMove::dummyRoot) { delete this; }
  
  return *result;
}





ply& ply::propagateFitness(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result;
  bool propagated = false;
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    assert(child.status != NODET_NOTEVAL);
    assert(!(child.bestAction < 0 && mostlyGT(child.probability, 0.0))); // if a value is propagated (not pruned), it has an action that represented it

    //sanitize nlbFitness (WARNING - taking child->lbFitness nearly halves perfromance in number of nodes evaluated. Why?
    /*fpType nlbFitness = std::max(child->ubFitness - (1 - child->probability), (fpType)0.0);*/
    fpType nlbFitness = child.lbFitness;

    assert(mostlyLTE(nlbFitness, child.ubFitness) && mostlyGTE(nlbFitness, 0.0)); // check lowerbound fitness
    assert(mostlyLTE(child.ubFitness, 1.0) && mostlyGTE(child.ubFitness, nlbFitness)); // check upperbound fitness
    assert(mostlyLTE(child.probability, 1.0) && mostlyGTE(child.probability, nlbFitness)); // check probability (upperbound)

    // is the propagated fitness better than the current best fitness?
    if (mostlyGT(nlbFitness, lbFitness) ||
      (mostlyGTE(nlbFitness, lbFitness) && mostlyLT(child.probability, probability)))
    {
#ifndef NDEBUG
      if (verbose >= 7)
      {
        boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
        std::cout << "BP-A" <<
          " d" << std::setw(2) << std::dec << (unsigned int) getDepth()*3 <<
          " i" << std::setw(2) << (unsigned int) action <<
          " a" << std::setw(2) << (unsigned int) child.action <<
          " o x" <<
          " #  x" <<
          " lbfit" << std::setw(10) << nlbFitness <<
          "  ofit" << std::setw(10) << lbFitness <<
          " a-bnd" << std::setw(10) << alphaBound <<
          ((mostlyLTE(nlbFitness, alphaBound))?" fail high":"") <<
          ((mostlyLT(child.probability, 1.0))?" pCut":"") <<
          "\n";
        std::cout.flush();
      }
#endif
      
      lbFitness = nlbFitness;
      probability = child.probability;

      bestAction = child.action;
      bestChildAction = child.bestAction;

      status = child.status;

      //bestBetaBound = child->betaBound;

      if (mostlyGT(nlbFitness, alphaBound))
      {
        alphaBound = nlbFitness;

#ifndef _DISABLEORDERHEURISTIC
#ifndef _DISABLEHISTORYHEURISTIC
        // increment cutoff status of agent team:
        _planner.getAgentOrder().incrementCutoff(
          depth, 
          envP.getEnv().getTeam(_planner.getAgentTeam()).getICPKV(),
          envP.getEnv().getOtherTeam(_planner.getAgentTeam()).getICPKV(),
          bestAction);
#endif
#endif

      }

      assert(status != NODET_NOTEVAL);
      assert(bestAction >= 0 && bestAction < AT_ITEM_USE + 1);
      assert(bestChildAction >= 0 && bestChildAction < AT_ITEM_USE + 1);
    }
#ifndef NDEBUG
    else if (verbose >= 7)
    {
      boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
      std::cout << "NB-A" <<
        " d" << std::setw(2) << std::dec << (unsigned int) getDepth()*3 <<
        " i" << std::setw(2) << (unsigned int) action <<
        " a" << std::setw(2) << (unsigned int) child.action <<
        " o x" <<
        " #  x" <<
        " lbfit" << std::setw(10) << nlbFitness <<
        "  ofit" << std::setw(10) << lbFitness <<
        " a-bnd" << std::setw(10) << alphaBound <<
        " fail high" <<
        ((mostlyLT(child.probability, 1.0))?" pCut":"") <<
        "\n";
      std::cout.flush();
    }
#endif

    result = &recurse(child, _planner, _children, propagated);
  } // implicitly unlock mutex

  if (propagated) { delete this; } // don't delete root node
  return *result;
}





ply& ply::prune(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result;
  bool propagated = false;
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    result = &recurse(child, _planner, _children, propagated);
  } // implicitly unlock mutex

  if (propagated) { delete this; } // don't delete root node
  return *result;
}





ply& ply::recurse(const agentMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated)
{
  ply* result = this;

  // delete child from array
  size_t location;
  findChild(children, child.action, location); // MUST return true
  children.erase(children.begin() + location);

  if (children.empty())
  {
    if (lastAction < order.size())
    {
      // the child won't be locked regardless of whether we set true or false here, because we just created it
      generateChildren_noExistingCheck(_planner, _children, true);
    }

    if (lastAction >= order.size())
    {
      assert(status != NODET_NOTEVAL);
      assert(mostlyLTE(lbFitness, probability) && mostlyGTE(lbFitness, 0.0)); // check lowerbound fitness
      assert(mostlyLTE(probability, 1.0) && mostlyGTE(probability, lbFitness)); // check probability (upperbound)

#ifndef _DISABLETTABLE
      // add node to transposition table if it doesn't exist, or update if it exists
      _planner.getTTable().updateElement(
        envP,
        bestAction,
        bestChildAction,
        depth,
        lbFitness,
        probability,
        status);
#endif

      if (&parent == otherMove::dummyRoot) 
      { 
        // we've found root! This is the solution. Don't return a node to propagate to
        result = dummyRoot; 
      }
      // this ply is complete, propagate to otherMove
      else
      {
        propagated = true;

        result = &parent.propagateFitness(*this, _planner, _children);
      }
    }
  }

  return *result;
}





void ply::deleteTree()
{
  for (size_t iChild = 0; iChild < children.size(); iChild++)
  {
    children.at(iChild)->deleteTree_recursive();
  }
  children.clear();
}

void ply::deleteTree_recursive()
{
  for (size_t iChild = 0; iChild < children.size(); iChild++)
  {
    children.at(iChild)->deleteTree_recursive();
  }
  children.clear();

  delete this;
}





bool ply::generateChildren_backwards(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  bool result;
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    // noExistingCheck doesn't lock by default
    boost::unique_lock<boost::mutex> _lock(lock);
#endif
    result = generateChildren_noExistingCheck(_planner, _children, true);
  }

  if (!result)
  {
    // recurse
    if (&parent != otherMove::dummyRoot)
    {
      result = parent.parent.generateChildren_backwards(_planner, _children);
    }
    else
    {
      // we've reached the root node, we cannot generate any more children
      result = false;
    }
    
  }

  return result;
}





bool ply::generateSiblings(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  return parent.parent.generateChildren_backwards(_planner, _children);
}





bool ply::generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  return generateChildren(_planner, _children, true);
}





bool ply::generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockParent)
{
  if (lockParent)
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    // don't allow others to modify this node while we add to it
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    return generateChildren_variableLock(_planner, _children, true);
  }
  else
  {
    return generateChildren_variableLock(_planner, _children, true);
  }
} // endOf generateChildren






bool ply::generateChildren_variableLock(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockChild)
{

  // only produce children when we absolutely need them, and prefer to traverse existing nodes:
  if ( !children.empty() ) 
  { 
    size_t location;
    bool result = false;
    // TODO: is this functionally equivalent to children.at(0)?
    if (findChild(children, order[lastAction], location) == true)
    {
      result = children.at(location)->generateChildren(_planner, _children);
    }
    // if this successfully created more children or if the agent moveset has enough children, stop here
    if (result == true) { return true; }
  }

  return generateChildren_noExistingCheck(_planner, _children, lockChild);
} // end of generatechildren - variableLock





bool ply::generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children, bool lockChild)
{
  // are all of the agent team's potential moves pruned by alpha beta pruning?
  if (isPruned()) 
  {
    // immediately prune this mode upon fitness propagation
    lastAction =  (uint8_t) order.size() + 1;

    // and set the lowerbound of the entire ply to lbFit, with the upper bound to 1
    lbFitness = std::max(lbFitness, (fpType)0.0);
    probability = lbFitness;

#ifndef NDEBUG
    if (verbose >= 7)
    {
      boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
      std::cout << "PR-P" <<
        " d" << std::setw(2) << std::dec << (unsigned int) getDepth()*3 <<
        " i" << std::setw(2) << (unsigned int) action <<
        " a x" <<
        " o x" <<
        " #  x" <<
        " lbfit" << std::setw(10) << lbFitness <<
        " cProb" << std::setw(10) << probability <<
        " fail " << ((mostlyGTE(lbFitness, 1.0))?"agentWin":"other") <<
        "\n";
      std::cout.flush();
    }
#endif
    return false; 
  }

  // if generating for the first time, lastAction will be -1 and increment to 0
  // the children array is empty, which means our last step was a recursion. Generate new nodes
  for ( lastAction+=1 ; lastAction < order.size() ; lastAction++)
  {
    // is team 1's current->child behavior valid?
    if (_planner.getCU().isValidAction(envP.getEnv(), order[lastAction], _planner.getAgentTeam()) == false) { continue; } 
    
    // TODO: will this always return false because this method is only called when children is empty?
    size_t location;
    bool result;
    if (findChild(children, order[lastAction], location) == true)
    {
      // node already exists
      if (lockChild) result = children.at(location)->generateChildren(_planner, _children);
      else result = children.at(location)->generateChildren_nonLocking(_planner, _children);
    }
    else
    {
      // node does not exist, create it and generate children for it
      agentMove* tempMove = new agentMove(*this, _planner, order[lastAction]);

      // there's no way the child would need to be locked, because we just created it
      result = tempMove->generateChildren_noExistingCheck(_planner, _children);

      if ( result != true)
      {
        // this move had no valid options to evaluate
        delete tempMove;
      }
      else
      {

#ifndef _DISABLEORDERHEURISTIC
#ifndef _DISABLEBUTTERFLYHEURISTIC
        // increment order status of agent team:
        _planner.getAgentOrder().incrementUse(
          getDepth(), 
          envP.getEnv().getTeam(_planner.getAgentTeam()).getICPKV(),
          envP.getEnv().getOtherTeam(_planner.getAgentTeam()).getICPKV(),
          order[lastAction]);
#endif
#endif

        // push back pre-completed move
        children.insert(children.begin()+location, tempMove);
      }
    }

    if (result == true) 
    {
      // successfully generated nodes, stop here
      assert(!isLeaf());
      return true; 
    } 
    else { continue; } // did not generate new nodes, try again
  }
  
  // if this postcondition is met, we weren't able to generate any children
  assert(!isLeaf());
  lastAction = (uint8_t) order.size() + 1; // set to prune immediately upon fitness evaluation
  return false;
}





bool ply::isPruned(bool recurse) const
{
  // is the fitness of this ply something the agent would never evaluate?
  // Do not produce siblings of this node if this is the case

  // is it impossible for any agent move to have greater (lower bound) fitness than the current agent move?
  if (mostlyGTE(lbFitness, 1.0))
  {
    return true;
  }
  // if it is impossible for any fully evaluated fitness to fit within the bounds
  else if (mostlyGT(alphaBound, initialBetaBound))
  {
    return true;
  }
  // we've reached the root node, this node is within the window and thus should not be pruned
  else if (&parent == otherMove::dummyRoot) { return false; }
  else if (recurse && mostlyGT(lbFitness, 0.0))
  {
    // recurse the lower bound only, and make the upper bound 1.0
    fpType clbFitness = std::max(lbFitness, (fpType)0.0);
    fpType cProbability = envP.getProbability().to_double();
    return parent.isPruned(clbFitness * cProbability, clbFitness * cProbability);
  }
  else
  {
    // recurse
    return parent.isPruned();
  }
}





uint64_t ply::getPriority(
  uint8_t _depth, 
  uint16_t _plyMove, 
  uint8_t _agentMove, 
  uint8_t _otherMove, 
  uint8_t _status,
  fpType min, 
  fpType max, 
  fpType value)
{
  unsigned int ranking;
  // scale probability from min..max to 0..1:
  if (mostlyEQ(max, min)) 
  {
    ranking = 1;
  }
  else
  {
    fpType ratio = (value - min) / (max - min);
    ranking = (unsigned int) (127.0 * ratio);
  }
  
  uint64_t priority =
      ((uint64_t)_depth << 34) | // size 5, 32
      ((uint64_t)_plyMove << 18) | // size 16, 65536
      ((uint64_t)_agentMove << 14) | // size 4, 16
      ((uint64_t)_otherMove << 10) | // size 4, 16
      ((uint64_t)(_status & ~(0x02)) << 7) | // size 3, 8
      ((uint64_t)(127 - ranking) << 0); // size 7, 128

  return priority;
}





ply& ply::getParent() const // WARNING: causes problems if this is the root node
{
  assert(&parent != otherMove::dummyRoot);
  return parent.parent.parent;
}
