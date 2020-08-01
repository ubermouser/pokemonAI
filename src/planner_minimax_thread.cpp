//#define PKAI_IMPORT
#include "../inc/planner_minimax_thread.h"

#include "../inc/pkCU.h"
#include "../inc/ply.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/planner_minimax.h"

bool plyComparison::operator() (const ply* lhs, const ply* rhs) const
{
  return (lhs[0] > rhs[0]);
}




minimax_threadArg::minimax_threadArg(planner_minimax& _base)
: base(&_base),
plyQueue(),
root(NULL),
cu(NULL),
eval(NULL)
{
};

minimax_threadArg::minimax_threadArg(planner_minimax& _base, const Evaluator& _eval)
: base(&_base),
plyQueue(),
root(NULL),
cu(NULL),
eval(_eval.clone())
{
};

minimax_threadArg::minimax_threadArg(const minimax_threadArg& other)
  : base(other.base),
  plyQueue(),
  root(NULL),
  cu(other.cu!=NULL?new PkCU(*other.cu):NULL),
  eval(other.eval!=NULL?other.eval->clone():NULL)
{
};

const EnvironmentNonvolatile& minimax_threadArg::getNV() const { return cu->getNV(); };

PkCU& minimax_threadArg::getCU() { return *cu; };

transposition_table& minimax_threadArg::getTTable() { return base->ttable; }
const transposition_table& minimax_threadArg::getTTable() const { return base->ttable; }

Evaluator& minimax_threadArg::getEval() { return *eval; }
const Evaluator& minimax_threadArg::getEval() const { return *eval; }

size_t minimax_threadArg::getAgentTeam() { return base->agentTeam; }
size_t minimax_threadArg::getOtherTeam() { return base->otherTeam; }

orderHeuristic& minimax_threadArg::getAgentOrder() { return base->agentOrder; };
orderHeuristic& minimax_threadArg::getOtherOrder() { return base->otherOrder; };

const orderHeuristic& minimax_threadArg::getAgentOrder() const { return base->agentOrder; };
const orderHeuristic& minimax_threadArg::getOtherOrder() const { return base->otherOrder; };

size_t minimax_threadArg::getMaxDepth() const { return base->greatestDepth; }

boost::mutex& minimax_threadArg::getStdioLock() { return base->stdioLock; } 





bool minimax_threadArg::isInitialized() const 
{ 
  if (cu == NULL) { return false; }
  if (eval == NULL) { return false; }
  if (!eval->isInitialized()) { return false; }
  
  return true;
}

void minimax_threadArg::setEvaluator(const Evaluator& evalType)
{
  if (eval != NULL) { delete eval; }
  eval = evalType.clone();
  if (cu != NULL) { eval->resetEvaluator(cu->getNV()); };
};

void minimax_threadArg::setEnvironment(PkCU& _cu, size_t _agentTeam)
{
  if (cu == NULL) { cu = new PkCU(_cu); cu->setAccuracy(base->engineAccuracy); }
  else { cu->setEnvironment(_cu.getNV()); }

  // assign new evaluator of the same type:
  if (eval != NULL) { eval->resetEvaluator(getNV()); }

  // clear IDDFS vars:
  cleanUp();
};

void minimax_threadArg::setRoot(const EnvironmentPossible& origin)
{
  cleanUp();
  root = new ply(origin, *this);
};





void minimax_threadArg::cleanUp()
{
  while (!plyQueue.empty()) { plyQueue.pop(); }
  if (root != NULL) { root->deleteTree(); delete root; root = NULL; }
};
  
minimax_threadArg::~minimax_threadArg()
{
  if (eval != NULL) { delete eval; eval = NULL; }
  if (cu != NULL) { delete cu; cu = NULL; }
  cleanUp();
};





void minimax_thread(minimax_threadArg* _t)
{
  assert(_t != NULL);
  minimax_threadArg& t = *_t;

  if (verbose >= 6) 
  { 
    boost::unique_lock<boost::mutex> lock(t.getStdioLock()); 
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      ": Thread " << boost::this_thread::get_id() << " running!\n";
    std::cerr.flush(); 
  }
  
  t.base->post.signal(); // posting that this thread has been started
  
  
  bool running = true;
  do
  {
    switch(t.base->threadInvocation)
    {
      case THREADS_GENERATE:
        {
          int result = t.evaluateVertex_IDDFS();
          switch(result)
          {
          case 0:
            boost::this_thread::yield();
            break;
          case 1:
            break; // just continue
          case 2:
            // signal master thread, something important happened in a worker thread:
            t.base->solution.signal(); 
            // signal that this thread has completed its task
            //t->solution->signal();
            // wait for new orders
            //t->ready->wait();
            break;
          }
        }
        break;
      case THREADS_WAIT:
        // don't burn through all the cpu time just busy-waiting
        boost::this_thread::sleep(boost::posix_time::milliseconds(1));
        break;
      case THREADS_HALT:
        t.base->post.signal(); // posting that this thread is halting
        if (verbose >= 6) 
        { 
          boost::unique_lock<boost::mutex> lock(t.getStdioLock()); 
          std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
            ": Thread " << boost::this_thread::get_id() << " halted!\n";
          std::cerr.flush(); 
        }
        t.base->halt.wait();
        if (verbose >= 6) 
        { 
          boost::unique_lock<boost::mutex> lock(t.getStdioLock()); 
          std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
            ": Thread " << boost::this_thread::get_id() << " unhalted!\n";
          std::cerr.flush(); 
        }
        t.base->post.signal(); // posting that this thread is unhalting
        break;
      case THREADS_KILL:
      default:
        running = false;
        break;
    }
  }while(running);
  
  if (verbose >= 6) 
  { 
    boost::unique_lock<boost::mutex> lock(t.getStdioLock());
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      ": Thread "  << boost::this_thread::get_id() << " shutdown!\n";
    std::cerr.flush(); 
  }
  
  t.base->post.signal(); // posting that this thread is shutting down
};





bool minimax_threadArg::evaluateVertex_IDDFS_Fitness(ply& current, std::vector<ply*>& _children)
{	
  ply* result;
  
  // alpha beta pruning: would we ever evaluate this node?
  if (current.isPruned(false)) //NOTE: possible race condition, but shouldn't matter
  {
    // if not, prune the node. Do not calculate fitness (WORKAROUND: prune will pass-through values precomputed by ttable)
    result = &current.prune(*this, _children);
  }
  // this node would not be pruned by alpha - beta pruning, evaluate normally
  else if (current.isLeaf())
  {
    // if a ttable value of this node exists (any depth will do, as this is a depth 0 node)
    if (current.getTNode().inUse())
    {
      // don't bother calculating fitness, use tnode value
      result = &current.propagateFitness(
        current.getLowerBound(), 
        current.getProbability(), 
        current.getBestAction(),
        current.getBestChildAction(),
        *this, _children);
    }
    else
    {
      // calculate fitness
      EvalResult_t evalResult;
      if (current.getStatus() != NODET_FULLEVAL)
      {
        // use the heuristic evaluator for cutoff nodes
        evalResult = getEval().calculateFitness(current.getEnvP().getEnv(), getAgentTeam());
      }
      else
      {
        // use the simple evaluator for terminal nodes
        evalResult.fitness = evaluator_simple::calculateFitness(getNV(), current.getEnvP().getEnv(), getAgentTeam());
        evalResult.agentMove = AT_MOVE_NOTHING;
        evalResult.otherMove = AT_MOVE_NOTHING;
        
      }
    
      // add fitness to ply
      result = &current.propagateFitness(
        evalResult.fitness, 
        1.0, 
        evalResult.agentMove,
        evalResult.otherMove,
        *this, 
        _children);
    }
  }
  // has this node been pre-evaluated by the transposition table?
  else
  {
    result = &current.propagateFitness(
        current.getLowerBound(), 
        current.getProbability(), 
        current.getBestAction(),
        current.getBestChildAction(),
        *this, _children);
  }

  if (result == ply::dummyRoot)
  {
    // solution found, done with root
    return true;
  }

  // no solution found, keep going
  return false;
}





int minimax_threadArg::evaluateVertex_IDDFS()
{
  ply* current;

  {
    // acquire lock for the ply queue
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> lock(t.base->plyLock);
#endif

    if (plyQueue.empty()) 
    { 
      return 0; 
    } // try again later; there's nothing in the queue

    // decrement priority queue
    current = plyQueue.top(); // get current vertex
    plyQueue.pop(); // remove current vertex from queue

    // we evaluated 1 total node in this depth
    base->nodesEvaluated += 1;
  } // implicitly destroy lock
  
  // push back a sibling of the current node which evaluates a node on the same level
  std::vector<ply*> currentChildren; // children added to current node
  
  // force leaf node if we are at the current iterative deepening threshold
  bool childrenGenerated = false;
  bool result = false;
  if (current->getDepth() > 0 && !current->isEvaluated())
  {
    // if childrenGenerated returns or is false:
    //  this is either an implicit leaf node - no states can come from it
    //  or the node was pruned through alpha-beta-gamma pruning
    //  or the node was previously evaluated to sufficient depth through the ttable
    childrenGenerated = current->generateChildren(*this, currentChildren);
  }
  
  // if this is a leaf node or at max depth, calculate fitness of this node:
  if (!childrenGenerated)
  {
    result = evaluateVertex_IDDFS_Fitness(*current, currentChildren);
  } // end of maximum depth so must be leaf node

#ifndef NDEBUG
  if (verbose >= 9)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(10));
  }
#endif

  // do we need to add any nodes to the plyQueue?
  if (!currentChildren.empty())
  {
    // acquire lock for the ply queue
#ifndef _DISABLEFINEGRAINEDLOCKING
    boost::unique_lock<boost::mutex> lock(t.base->plyLock);
#endif

    // and generated children.size() children and clones to evaluate
    for (std::vector<ply*>::const_iterator iChild = currentChildren.begin(), iSize = currentChildren.end(); iChild != iSize; ++iChild)
    {
      plyQueue.push(*iChild); // push a pointer of this child to the queue
    }

  } // implicitly unlock the ply queue

  if (result == true)
  {
    return 2; // found solution, await new root
  }

  return 1;
}; // endOf evaluateVertex_IDDFS
