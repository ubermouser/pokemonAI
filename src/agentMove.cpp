//#define PKAI_IMPORT
#include "../inc/environment_possible.h"
#include "../inc/pkCU.h"

#include "../inc/fp_compare.h"

#include "../inc/planner_minimax_thread.h"

#include "../inc/orderHeuristic.h"
#include "../inc/transposition_table.h"

#include "../inc/vertex.h"
#include "../inc/otherMove.h"
#include "../inc/ply.h"

#include "../inc/agentMove.h"





agentMove::agentMove(ply& _parent, minimax_threadArg& _planner, uint8_t agentAction)
  : 
#ifndef _DISABLEFINEGRAINEDLOCKING
  lock(),
#endif
  children(),
  parent(_parent),
#ifndef _DISABLETIGHTBOUNDS
  betaBound((&_parent.parent==otherMove::dummyRoot)?_parent.initialBetaBound:bound(_parent)),
#else
  betaBound(std::numeric_limits<fpType>::infinity()),
#endif
  ubFitness(std::numeric_limits<fpType>::infinity()),
  lbFitness(-std::numeric_limits<fpType>::infinity()),
  probability(0),
  action(agentAction),
  bestAction(-1),
  lastAction((uint8_t)-1),
  status(NODET_NOTEVAL),
  order()
{
  int8_t cBestAgentMove;
  int8_t cBestOtherMove;
  if (_parent.tNode.inUse()) { _parent.tNode.getAgentValues(cBestAgentMove, cBestOtherMove); }

  // since this node is empty and has no fitness, it needs to be given some by a generateMove function
  if (parent.tNode.inUse() && cBestAgentMove == agentAction)
  {
    _planner.getOtherOrder().seedOrdering(
      order, 
      parent.envP.getEnv().getTeam(_planner.getOtherTeam()).getICPKV(),
      parent.envP.getEnv().getOtherTeam(_planner.getOtherTeam()).getICPKV(),
      cBestOtherMove);
  }
  else
  {
    _planner.getOtherOrder().seedOrdering(
      order, 
      parent.envP.getEnv().getTeam(_planner.getOtherTeam()).getICPKV(),
      parent.envP.getEnv().getOtherTeam(_planner.getOtherTeam()).getICPKV());
  }
}





fpType agentMove::bound(const ply& _parent)
{
  // determine lower bounded fitness from otherMove:
  fpType clbFitness = _parent.parent.lbFitness; // lower bound of already evaluated nodes
  fpType cProbability = _parent.envP.getProbability().to_double(); // partition of current action
  fpType oldBetaBound = _parent.parent.parent.betaBound; // previous bound

  fpType dBound = oldBetaBound - clbFitness;
  if (mostlyEQ(dBound, 0.0))
  {
    dBound = 0.0;
  }

  fpType result = dBound / cProbability;

  assert(mostlyGTE(result, 0.0));

  return std::min(result, _parent.initialBetaBound);
}





bool agentMove::generateChildren(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
#ifndef _DISABLEFINEGRAINEDLOCKING
  // don't allow others to modify this node while we add to it
  boost::unique_lock<boost::mutex> _lock(lock);
#endif

  return generateChildren_nonLocking(_planner, _children);
}





bool agentMove::generateChildren_nonLocking(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  // only produce children when we absolutely need them:
  if (children.size() > 0) // TODO: possibility that with n+ worker threads, this could be idle much of the time
  {
    return true; 
  }

  return generateChildren_noExistingCheck(_planner, _children);
} //end of generateChildren - nonlocking





bool agentMove::generateChildren_noExistingCheck(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  // are all of the other team's potential moves pruned by alpha beta pruning?
  if (isPruned()) 
  { 
    // immediately prune this mode upon fitness propagation
    lastAction = (uint8_t) order.size()  + 1;

    // and set the upperbound of the entire agentMove to ubFit, with the lower bound to 0
    ubFitness = std::min(ubFitness, (fpType)1.0);
    lbFitness = std::max(lbFitness, (fpType)0.0);
    probability = probability - lbFitness;
    assert(mostlyLTE(probability, 1.0) && mostlyGTE(probability, 0.0)); // check probability (upperbound)
    if (mostlyEQ(probability, 0.0)) { probability = 0.0; }
    lbFitness = 0.0;

#ifndef NDEBUG
    if (verbose >= 7)
    {
      boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
      std::cout << "PR-A" <<
        " d" << std::setw(2) << std::dec << (unsigned int) parent.getDepth()*3 - 1 <<
        " i" << std::setw(2) << (unsigned int) parent.action <<
        " a" << std::setw(2) << (unsigned int) action <<
        " o x" <<
        " #  x" <<
        " ubfit" << std::setw(10) << ubFitness <<
        " cProb" << std::setw(10) << probability <<
        " fail " << ((mostlyLTE(ubFitness, 0.0))?"agentLoss":((mostlyLTE(ubFitness, parent.alphaBound))?"low":"other")) <<
        "\n";
      std::cout.flush();
    }
#endif
    return false; 
  }

  // if generating for the first time, lastAction will be -1 and increment to 0
  // or the children array is empty, which means our last step was a recursion. Generate new nodes
  for ( lastAction+=1 ; lastAction < order.size() ; lastAction++)
  {
    // is otherTeam's current->child behavior valid?
    if (_planner.getCU().isValidAction(parent.envP.getEnv(), order[lastAction], _planner.getOtherTeam()) == false) { continue; }
    
    // TODO: is otherTeam's behavior something we would ever evaluate?
    
    size_t location;
    bool result;
    if (findChild(children, order[lastAction], location) == true)
    {
      // node already exists
      result = children.at(location)->generateChildren_noExistingCheck(_planner, _children);
    }
    else
    {
      // node does not exist, create it and generate children for it
      otherMove* tempMove = new otherMove(*this, _planner, order[lastAction]);

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
        // increment order status of other team:
        _planner.getOtherOrder().incrementUse(
          getParent().getDepth(), 
          getParent().envP.getEnv().getTeam(_planner.getOtherTeam()).getICPKV(),
          parent.envP.getEnv().getOtherTeam(_planner.getOtherTeam()).getICPKV(),
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
      return true; 
    } 
    else { continue; } // did not generate new nodes, try again
  } // endOf foreach action in move ordering
  
  // if this postcondition is met and the has returned, we weren't able to generate any children
  return false;
} // end of generateChildren - noExistingCheck





bool agentMove::generateChildren_backwards(minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  bool result = false;
  {
    result = generateChildren(_planner, _children);
  }

  if (!result)
  {
    // recurse
    result = parent.generateChildren_backwards(_planner, _children);
  }

  return result;
}





ply& agentMove::propagateFitness(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result;
  bool propagated = false;
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    assert(child.status != NODET_NOTEVAL);

    // sanitize nubFitness
    fpType nubFitness = child.lbFitness + (1.0 - child.probability);
    assert(mostlyLTE(nubFitness, 1.0) && mostlyGTE(nubFitness, child.lbFitness)); // check upperbound fitness
    if (mostlyEQ(nubFitness, 1.0)) { nubFitness = 1.0; probability = 1.0; }

    // is the propagated fitness "worse" than the current "worst" fitness?
    if (mostlyLT(nubFitness, ubFitness) ||
      (mostlyLTE(nubFitness, ubFitness) && mostlyGT(child.probability, probability)))
    {
#ifndef NDEBUG
      if (verbose >= 7)
      {
        boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
        std::cout << "BP-O" <<
          " d" << std::setw(2) << std::dec << (unsigned int) parent.getDepth()*3 - 1 <<
          " i" << std::setw(2) << (unsigned int) parent.action <<
          " a" << std::setw(2) << (unsigned int) action <<
          " o" << std::setw(2) << (unsigned int) child.action <<
          " #  x" <<
          " ubfit" << std::setw(10) << nubFitness <<
          "  oFit" << std::setw(10) << ubFitness <<
          " b-bnd" << std::setw(10) << betaBound <<
          ((mostlyGTE(nubFitness, betaBound))?" fail low":"") <<
          ((mostlyLT(child.probability, 1.0))?" pCut":"") <<
          "\n";
        std::cout.flush();
      }
#endif

      ubFitness = nubFitness;
      lbFitness = child.lbFitness;
      probability = child.probability;
      bestAction = (signed) child.action;
      status = child.status;

      if (mostlyLT(nubFitness, betaBound))
      {
        betaBound = nubFitness;

#ifndef _DISABLEORDERHEURISTIC
#ifndef _DISABLEHISTORYHEURISTIC
        // increment cutoff status of other team:
        _planner.getOtherOrder().incrementCutoff(
          parent.getDepth(), 
          parent.envP.getEnv().getTeam(_planner.getOtherTeam()).getICPKV(),
          parent.envP.getEnv().getOtherTeam(_planner.getOtherTeam()).getICPKV(),
          bestAction);
#endif
#endif

      } // endOf within cutoff
    } // endOf within bound
#ifndef NDEBUG
    else if (verbose >= 7)
    {
      boost::unique_lock<boost::mutex> lock(_planner.getStdioLock());
      std::cout << "NB-O" <<
        " d" << std::setw(2) << std::dec << (unsigned int) parent.getDepth()*3 - 1 <<
        " i" << std::setw(2) << (unsigned int) parent.action <<
        " a" << std::setw(2) << (unsigned int) action <<
        " o" << std::setw(2) << (unsigned int) child.action <<
        " #  x" <<
        " ubfit" << std::setw(10) << nubFitness <<
        "  oFit" << std::setw(10) << ubFitness <<
        " b-bnd" << std::setw(10) << betaBound <<
        " fail low" <<
        ((mostlyLT(child.probability, 1.0))?" pCut":"") <<
        "\n";
      std::cout.flush();
    }
#endif

    result = &recurse(child, _planner, _children, propagated);
  } // implicitly unlock mutex

  if (propagated) { delete this; }
  return *result;
}





ply& agentMove::prune(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children)
{
  ply* result;
  bool propagated = false;
  {
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> _lock(lock);
#endif

    result = &recurse(child, _planner, _children, propagated);
  } // implicitly unlock mutex

  if (propagated) { delete this; }
  return *result;
}





ply& agentMove::recurse(const otherMove& child, minimax_threadArg& _planner, std::vector<ply*>& _children, bool& propagated)
{
  ply* result = &getParent();

  // delete child from array
  size_t location;
  findChild(children, child.action, location); // MUST return true
  children.erase(children.begin() + location);

  if (children.empty())
  {
    if (lastAction < order.size())
    {
      generateChildren_noExistingCheck(_planner, _children);
    }

    if (lastAction >= order.size())
    {
      propagated = true;

      assert(mostlyLTE(lbFitness, ubFitness) && mostlyGTE(lbFitness, 0.0)); // check lowerbound fitness
      assert(mostlyLTE(ubFitness, 1.0) && mostlyGTE(ubFitness, lbFitness)); // check upperbound fitness
      assert(mostlyLTE(probability, 1.0) && mostlyGTE(probability, lbFitness)); // check probability (upperbound)

      /*if (bestAction == (unsigned)-1)
      {
        result = parent->prune(this, _planner, _children);
      }
      else*/
      {
        result = &parent.propagateFitness(*this, _planner, _children);
      }
    }

  }

  return *result;
}





void agentMove::deleteTree_recursive()
{
  for (size_t iChild = 0; iChild < children.size(); iChild++)
  {
    children.at(iChild)->deleteTree_recursive();
  }
  children.clear();

  delete this;
}





bool agentMove::isPruned() const
{
  // is the fitness of this agentMove something the agent would never evaluate?
  // Do not produce siblings of this node if this is the case

  // is it impossible for any other move to have greater (upper bound) fitness than the current other move?
  if (mostlyLTE(ubFitness, 0.0)) // fitness = ubFitness
  {
    return true;
  }
  // if it is impossible for any fully evaluated fitness to fit within the bounds
  else if (mostlyGT(parent.alphaBound, betaBound))
  {
    return true;
  }
  // if it is not possible for the current agentMove to be "better" than the "best" agentMove, and thus the agent player would never choose this action:
  /*else if ((mostlyGTE(parent->probability, 1.0) && mostlyLTE(ubFitness, parent->alphaBound)) ||
    mostlyLT(ubFitness, parent->alphaBound))*/
  else if(mostlyLTE(ubFitness, parent.alphaBound))
  {
    return true;
  }
  else
  {
    // recurse
    return parent.isPruned();
  }
}





ply& agentMove::getParent() const
{
  return parent;
}