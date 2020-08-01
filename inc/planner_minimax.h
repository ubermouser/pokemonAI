#ifndef planner_minimax_H
#define	planner_minimax_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <queue>
#include <cerrno>
#include <limits>

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/locks.hpp>
#include <boost/bind/bind.hpp>
#include <boost/timer.hpp>

#include "../inc/planner.h"
#include "../inc/orderHeuristic.h"
#include "../inc/transposition_table.h"

#include "../inc/planner_minimax_thread.h"

class PkCU;
class ply;
class EnvironmentNonvolatile;
class TeamNonVolatile;
class Evaluator;

union EnvironmentPossible;
union EnvironmentVolatile;
union TeamVolatile;





class semaphore
{
  //The current semaphore count.
  unsigned int count_;

  //mutex_ protects count_.
  //Any code that reads or writes the count_ data must hold a lock on
  //the mutex.
  boost::mutex mutex_;

  //Code that increments count_ must notify the condition variable.
  boost::condition_variable condition_;

public:
  explicit semaphore(unsigned int initial_count) 
     : count_(initial_count),
     mutex_(), 
     condition_()
  {
  }

  unsigned int get_count() //for debugging/testing only
  {
    //The "lock" object locks the mutex when it's constructed,
    //and unlocks it when it's destroyed.
    boost::unique_lock<boost::mutex> lock(mutex_);
    return count_;
  }

  void resetCount()//for debugging/testing only
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    count_ = 0;
  }

  void signal() //called "release" in Java
  {
    boost::unique_lock<boost::mutex> lock(mutex_);

    ++count_;

    //Wake up any waiting threads. 
    //Always do this, even if count_ wasn't 0 on entry. 
    //Otherwise, we might not wake up enough waiting threads if we 
    //get a number of signal() calls in a row.
    condition_.notify_one(); 
  }

  void wait() //called "acquire" in Java
  {
    boost::unique_lock<boost::mutex> lock(mutex_);
    while (count_ == 0)
    {
       condition_.wait(lock);
    }
    --count_;
  }

};





class planner_minimax : public Planner
{
private:
  std::string ident;

  // mutexes and locking mechanisms:
  /* locked when thread wishes to access root ply queue */
  boost::mutex plyLock;
  
  /* locks when the thread wishes to print something (for debugging) */
  boost::mutex stdioLock;

  /* used to determine if all running threads have halted, and to halt threads */
  semaphore halt;
  
  /* used to communicate to main thread that running threads have completed work,
   * or are ready for more work */
  semaphore post; 
  
  /* true if any one of the threads has returned a solution from its subtree */
  semaphore solution;

  // search result variables:
  /* result vector */
  std::vector<PlannerResult> plannerResults;

  /* number of nodes evaluated by generateSolution */
  uint64_t nodesEvaluated;

  /* the greatest depth seen; used for finding solutions */
  size_t greatestDepth;

  /* if set to THREADS_KILL, all threads should halt immediately if not blocked */
  uint32_t threadInvocation;
  
  // thread variables:
  /* worker threads. Main thread acts as a timer */
  boost::thread_group threads;

  /* arguments for the specific threads */
  std::vector<minimax_threadArg> threadArgs;

  size_t numThreads;

  /* 
   * set to either TEAM_A or TEAM_B, which team the planner should produce
   * solutions for
   */
  size_t agentTeam;
  size_t otherTeam;

  /* accuracy of pkCU engine */
  size_t engineAccuracy;
  
  /* for iterative deepening */
  size_t maxDepth;

  /* maximum number of seconds allowed for finding a solution */
  fpType maxSeconds;

  /* have all nodes been created within the tree? */
  bool treeCompleted;

  // search extensions:
  orderHeuristic agentOrder;

  orderHeuristic otherOrder;

  transposition_table ttable;

  /* makes sure all worker threads are running and awaiting input */
  bool startThreads();
  
  /* Kills all running worker threads */
  void killThreads(bool waitForPost = true);
  
  /* wait until all threads return with an action, or until msec is up */
  bool waitForPost(unsigned int msec);
  
  /* unblocks all running worker threads  */
  bool unhaltThreads(unsigned int invocation);
  /* blocks all running worker threads*/
  bool haltThreads();

  /* drives alpha-beta. Returns the root ply of the solution */
  ply* generateSolution_singleThread();
  ply* generateSolution_multiThread(const boost::timer& totalTime);
  
public:
  planner_minimax(const planner_minimax& source);
  planner_minimax(
    const Evaluator& eval,
    size_t _numThreads = 1, 
    size_t _engineAccuracy = 1,
    size_t _maxDepth = 31,
    fpType _maxSeconds = 20,
    size_t _numTableBins = 23,
    size_t _binSize = 2);
  planner_minimax(
    size_t _numThreads = 1, 
    size_t _engineAccuracy = 1,
    size_t _maxDepth = 31,
    fpType _maxSeconds = 20,
    size_t _numTableBins = 23,
    size_t _binSize = 2);
  ~planner_minimax();

  void setEvaluator(const Evaluator& evalType);
  const Evaluator* getEvaluator() const;

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  bool isInitialized() const;

  planner_minimax* clone() const { return new planner_minimax(*this); };
  
  const std::string& getName() const { return ident; };

  /* determine the best choice of action for agentTeam from the
   * given environment. Spend MAX seconds time doing it. */
  uint32_t generateSolution(const EnvironmentPossible& origin);

  const std::vector<PlannerResult>& getDetailedResults() const;
  void clearResults();

  friend class ply;
  friend class agentMove;
  friend class otherMove;
  friend class minimax_threadArg;
  friend void minimax_thread(minimax_threadArg* t);
};

#endif	/* planner_minimax_H */

