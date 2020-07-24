//#define PKAI_IMPORT
#include "../inc/planner_minimax.h"
#include "../inc/planner_minimax_thread.h"

#include <boost/foreach.hpp>

#include "../inc/fp_compare.h"
#include "../inc/ply.h"
#include "../inc/evaluator.h"

#include "../inc/pkCU.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/environment_possible.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/team_volatile.h"





planner_minimax::planner_minimax(
    const evaluator& _eval,
    size_t _numThreads, 
    size_t _engineAccuracy,
    size_t _maxDepth,
    fpType _maxSeconds,
    size_t _numTableBins,
    size_t _binSize)
  : ident(),
  plyLock(), 
  stdioLock(),
  halt(0),
  post(0),
  solution(0),
  plannerResults(),
  nodesEvaluated(0),
  greatestDepth(0),
  threadInvocation(THREADS_KILL),
  threads(),
  threadArgs(),
  numThreads(_numThreads),
  agentTeam(SIZE_MAX),
  otherTeam(SIZE_MAX),
  engineAccuracy(_engineAccuracy),
  maxDepth(_maxDepth),
  maxSeconds(_maxSeconds),
  treeCompleted(false),
  agentOrder(false),
  otherOrder(false),
  ttable(_numTableBins,_binSize)
{
  size_t numThreadArgs = std::max(_numThreads, (size_t)1U);
  threadArgs.reserve(numThreadArgs);
  for (size_t iThread = 0; iThread != numThreadArgs; ++iThread)
  {
    threadArgs.push_back(minimax_threadArg(*this, _eval));
  }
  {
    std::ostringstream name;
    name << "minimax_planner-" << threadArgs.front().getEval().getName();
    ident = name.str();
  }
}

planner_minimax::planner_minimax(
    size_t _numThreads, 
    size_t _engineAccuracy,
    size_t _maxDepth,
    fpType _maxSeconds,
    size_t _numTableBins,
    size_t _binSize)
  : ident("max_planner-NULLEVAL"),
  plyLock(), 
  stdioLock(),
  halt(0),
  post(0),
  solution(0),
  plannerResults(),
  nodesEvaluated(0),
  greatestDepth(0),
  threadInvocation(THREADS_KILL),
  threads(),
  threadArgs(),
  numThreads(_numThreads),
  agentTeam(SIZE_MAX),
  otherTeam(SIZE_MAX),
  engineAccuracy(_engineAccuracy),
  maxDepth(_maxDepth),
  maxSeconds(_maxSeconds),
  treeCompleted(false),
  agentOrder(false),
  otherOrder(false),
  ttable(_numTableBins,_binSize)
{
  size_t numThreadArgs = std::max(_numThreads, (size_t)1U);
  threadArgs.reserve(numThreadArgs);
  for (size_t iThread = 0; iThread != numThreadArgs; ++iThread)
  {
    threadArgs.push_back(minimax_threadArg(*this));
  }
}

planner_minimax::planner_minimax(const planner_minimax& other)
  : ident(other.ident),
  plyLock(), 
  stdioLock(),
  halt(0),
  post(0),
  solution(0),
  plannerResults(),
  nodesEvaluated(0),
  greatestDepth(0),
  threadInvocation(THREADS_KILL),
  threads(),
  threadArgs(other.threadArgs),
  numThreads(other.numThreads),
  agentTeam(other.agentTeam),
  otherTeam(other.otherTeam),
  engineAccuracy(other.engineAccuracy),
  maxDepth(other.maxDepth),
  maxSeconds(other.maxSeconds),
  treeCompleted(false),
  agentOrder(false),
  otherOrder(false),
  ttable(other.ttable.getNumBits(),other.ttable.getBinSize())
{
  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    cThread.base = this;
  }
}



planner_minimax::~planner_minimax()
{
  // forcibly take all threads offline, if they're not offline now
  killThreads(false);
  threads.join_all(); 

  // threadArgs destroyed implicitly
};





bool planner_minimax::isInitialized() const 
{ 
  if (agentTeam >= 2 || otherTeam >= 2) { return false; }
  BOOST_FOREACH(const minimax_threadArg& cThread, threadArgs)
  {
    if (!cThread.isInitialized()) { return false; }
  }
  
  return true; 
}

void planner_minimax::setEvaluator(const evaluator& evalType)
{
  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    cThread.setEvaluator(evalType);
  }
  {
    std::ostringstream name;
    name << "minimax_planner-" << threadArgs.front().getEval().getName();
    ident = name.str();
  }
};

const evaluator* planner_minimax::getEvaluator() const
{
  if (threadArgs.empty()) { return NULL; }
  return &threadArgs.front().getEval();
}

void planner_minimax::setEnvironment(pkCU& _cu, size_t _agentTeam)
{
  // set agent
  agentTeam = _agentTeam;
  otherTeam = (agentTeam==TEAM_A)?TEAM_B:TEAM_A;

  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    cThread.setEnvironment(_cu, _agentTeam);
  }

  // reset (but do not delete) ttable:
  ttable.clear();
#ifdef _HTCOLLECTSTATISTICS
  ttable.resetStatistics();
#endif

  // reset (but do not delete) orderings:
  agentOrder.reset(false);
  otherOrder.reset(false);

  // clear all stored results:
  plannerResults.clear();
};





bool planner_minimax::startThreads()
{
  threadInvocation = THREADS_HALT;
  for (size_t iThread = 0; iThread != numThreads; ++iThread)
  {
    try
    {
      threads.create_thread(boost::bind(minimax_thread, &threadArgs[iThread]));
    }
    catch(boost::thread_resource_error e)
    {
      boost::unique_lock<boost::mutex> lock(stdioLock); 
      
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << ": Thread " << iThread << " failed to run with thread error " << e.what() << "!\n";
      std::cerr.flush();
      return false;
    }
    
  }
  
  // wait for threads to assert that they have been started:
  waitForPost(500);

  // wait for threads to assert that they have halted:
  waitForPost(500);
  
  return true;
}





void planner_minimax::killThreads(bool doWaitForPost)
{
  if (verbose >= 6) 
  {
    boost::unique_lock<boost::mutex> lock(stdioLock); 
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      ": Master thread polling for SHUTDOWN ALL THREADS...\n";
    std::cerr.flush();
  }

  bool wasHalted = threadInvocation == THREADS_HALT;
  
  if (doWaitForPost)
  {
    // unhalt all threads
    unhaltThreads(THREADS_KILL);

    if (wasHalted)
    {
      // wait for threads to assert that they've been shut down:
      waitForPost(500);
    }
  }
  else
  {
    // specify behavior
    threadInvocation = THREADS_KILL;
    // unhalt all threads if they need to be halted
    for (size_t iThread = 0; iThread != numThreads; ++iThread)
    {
      halt.signal();
    }
  }
}





bool planner_minimax::unhaltThreads(unsigned int invocation)
{
  assert(invocation != THREADS_HALT);
  // specify behavior
  threadInvocation = invocation;

  // unhalt all threads
  for (size_t iThread = 0; iThread != numThreads; ++iThread)
  {
    // TODO: only wait a few seconds. Something bad happened otherwise
    halt.signal();
#ifndef NDEBUG
    if (verbose >= 9) { boost::this_thread::sleep(boost::posix_time::milliseconds(950 / numThreads)); }
#endif
  }

  // wait for threads to assert that they have been unhalted:
  return waitForPost(500);
}





bool planner_minimax::haltThreads()
{
  threadInvocation = THREADS_HALT;
  // wait for threads to assert that they have been halted:
  return waitForPost(500);
}





bool planner_minimax::waitForPost(unsigned int msec)
{
  for (size_t iThread = 0; iThread != numThreads; ++iThread)
  {
    // TODO: only wait a few seconds. Something bad happened otherwise
    post.wait();
  }
  
  return true;
}





uint32_t planner_minimax::generateSolution(const environment_possible& _origin)
{
  // reset result vector:
  plannerResults.clear();

  // seed origin to each thread:
  const environment_possible origin = environment_possible::create(_origin.getEnv(), true);
  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    cThread.setRoot(origin);
  }

  // attempt to start execution threads, and if successful pause them in preparation for bootstrapping
  if (threadInvocation == THREADS_KILL)
  {
    if (startThreads() != true) { return -1; }
  }
  assert(threadInvocation == THREADS_HALT);

  // best possible move for agent player (return value)
  uint32_t bestAction = UINT32_MAX;
  // elapsed time:
  boost::timer totalTime;
  
  for (size_t iDepth = 0; iDepth <= maxDepth; iDepth++)
  {
    // make sure root node and plyQueue are allocated and correct:
    {
      boost::unique_lock<boost::mutex> lock(plyLock); // enter critical region
      // reset collected statistics:
      nodesEvaluated = 0;
      //reset logic:
      greatestDepth = iDepth; // this depth counts backwards - iDepth is root node, 0 are leaf nodes
      treeCompleted = false;
    }

    ply* result = NULL;
    if ((iDepth <= 2) || (numThreads == 0)) { result = generateSolution_singleThread(); }
    else { result = generateSolution_multiThread(totalTime); }
    
    // post-search logic:
    if (result != NULL)
    {
      // this implies root was evaluated, meaning it was probably evaluated from the ttable
      if (result->getBestAction() >= 0) 
      { 
        greatestDepth = greatestDepth - result->getMaxDepth();
        if (greatestDepth > iDepth) { iDepth = greatestDepth; }
      }

      bestAction = (unsigned) result->getBestAction();
      plannerResults.push_back(
        plannerResult(
        iDepth, 
        result->getBestAction(), 
        result->getBestChildAction(), 
        result->getLowerBound(), result->getUpperBound())
        );

      // determine if our solution is good enough:
      if (result->isFullyEvaluated() ||
        mostlyGTE(totalTime.elapsed(),maxSeconds))
      {
        if (verbose >= 4)
        {
          boost::unique_lock<boost::mutex> lock(stdioLock);
          std::clog << "----T" << (agentTeam==TEAM_A?"A":"B") <<
            ": ply=" << std::setw(2) << (2*result->getDepth()) << 
            " act=" << std::setw(2) << result->getBestAction() <<
            " oact=" << std::setw(2) << result->getBestChildAction() <<
            " lbFit=" << std::setw(9) << result->getLowerBound() <<
            " ubFit=" << std::setw(9) << result->getUpperBound() <<
            " elaps=" << std::setw(7) << totalTime.elapsed() << "s" <<
            " nnod=" << std::dec << nodesEvaluated << 
            "\n"; 
        }
        // stop search
        break; 
      } // all solutions were found before the time-limit
      else if (verbose >= 5)
      {
        boost::unique_lock<boost::mutex> lock(stdioLock); 
        std::cout << "    T" << (agentTeam==TEAM_A?"A":"B") <<
          ": ply=" << std::setw(2) << (2*result->getDepth()) << 
          " act=" << std::setw(2) << result->getBestAction() <<
          " oact=" << std::setw(2) << result->getBestChildAction() <<
          " lbFit=" << std::setw(9) << result->getLowerBound() <<
          " ubFit=" << std::setw(9) << result->getUpperBound() <<
          " elaps=" << std::setw(7) << totalTime.elapsed() << "s" <<
          " nnod=" << std::dec << nodesEvaluated << 
          "\n"; 
      }
    } // endOf solution found
    else
    {
      // we reached the time-limit, do not continue
      if (verbose >= 4)
      {
        if (plannerResults.empty())
        {
          bestAction = UINT32_MAX;

          boost::unique_lock<boost::mutex> lock(stdioLock); 
          std::clog << "~~~~T" << (agentTeam==TEAM_A?"A":"B") <<
            ": NO SOLUTIONS FOUND FOR ANY DEPTH!\n";
        }
        else
        {
          const plannerResult& cResult = plannerResults.back();

          boost::unique_lock<boost::mutex> lock(stdioLock); 
          std::clog << "~~~~T" << (agentTeam==TEAM_A?"A":"B") <<
            ": ply=" << std::setw(2) << (2*cResult.depth) << 
            " act=" << std::setw(2) << cResult.bestAgentAction <<
            " oact=" << std::setw(2) << cResult.bestOtherAction <<
            " lbFit=" << std::setw(9) << cResult.lbFitness <<
            " ubFit=" << std::setw(9) << cResult.ubFitness <<
            " elaps=" << std::setw(7) << totalTime.elapsed() << "s" <<
            " nnod=" << std::dec << nodesEvaluated << 
            "\n";
        }
      }
      break;
    } // endOf solution was not found
  } // end of iterative deepening main loop
  
  // threads should hopefully be halted at the end of their execution
  return bestAction;
}

ply* planner_minimax::generateSolution_singleThread()
{
  minimax_threadArg& cThreadArg = threadArgs.front();

  // reset for full window:
  cThreadArg.root->reset(cThreadArg);

  // push root ply back onto plyQueue:
  cThreadArg.plyQueue.push(cThreadArg.root);

  // call evaluateVertex_IDDFS repeatedly until a solution is created
  int result;
  do
  {
    result = cThreadArg.evaluateVertex_IDDFS();
  }
  while (result == 1);

  // output:
  return cThreadArg.root;
}

ply* planner_minimax::generateSolution_multiThread(const boost::timer& totalTime)
{
  // checkpoint vars:
  size_t lastNodesEvaluated = 0;
  boost::timer checkPointTime;
  bool running = true;
  bool solutionFound = false;

  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    // reset for full window:
    cThread.root->reset(cThread);

    // push root ply back onto plyQueue:
    cThread.plyQueue.push(cThread.root);
  }

  unhaltThreads(THREADS_GENERATE); // start threads!
    
  // wait for threads to complete, or wait for timelimit to be reached - whichever comes first
  do
  {
    // don't burn too much cpu time on this
    boost::this_thread::sleep(boost::posix_time::milliseconds(1));
    // every few seconds we reach a "checkpoint", where we periodically report on the status of the IDDFS algorithm
    if (checkPointTime.elapsed() >= 3)
    {
      // checkpoint reached!
      checkPointTime.restart();
      if (verbose >= 5)
      {
        boost::unique_lock<boost::mutex> lock(stdioLock); 
        std::cout << 
          "        ply=" << std::setw(2) << (2*greatestDepth) << 
          " n_sec=" << std::setw(7) << std::dec << ((nodesEvaluated - lastNodesEvaluated) / 3) << 
          " n_tot=" << std::setw(11) << std::dec << nodesEvaluated << 
          " elaps=" << std::setw(7) << totalTime.elapsed() << "s" <<
          " tTabl=" << std::setw(8) << ttable.load() << "%" <<
#ifdef _HTCOLLECTSTATISTICS
          " nHits=" << std::setw(8) << ttable.percentProbeHits() << "%" <<
#endif
          "\n";
      }

#ifdef _HTCOLLECTSTATISTICS
      ttable.resetStatistics();
#endif

      lastNodesEvaluated = nodesEvaluated;
    }
        
    // has the time limit been reached?
    if (mostlyGTE(totalTime.elapsed(),maxSeconds)) 
    {
      running = false;
    }

    // has IDDFS returned a root node solution?
    if (solution.get_count() > 0) 
    { 
      // one of the threads has completed its work
      solutionFound = true;
      running = false;
    } 
  }while(running);
    
  // causes all threads to halt after completing the last node they touched
  if (!haltThreads())
  {
    // force shut down threads (shouldn't have to do this normally)
    threads.join_all(); 
    threadInvocation = THREADS_KILL;
  }

  // consume thread completed work
  solution.resetCount();

  // clean up threadArg data:
  BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
  {
    // destroy the entire tree except for the root node:
    while (!cThread.plyQueue.empty()) { cThread.plyQueue.pop(); }
    cThread.root->deleteTree();
  }

  // parse a result, if a result exists:
  if (solutionFound)
  {
    BOOST_FOREACH(minimax_threadArg& cThread, threadArgs)
    {
      if (cThread.root->isEvaluated()) { return cThread.root; }
    }
    assert(false && "threads returned result found, but no result stored!");
    return NULL;
  }
  else
  {
    return NULL;
  }
};





const std::vector<plannerResult>& planner_minimax::getDetailedResults() const
{
  return plannerResults;
};

void planner_minimax::clearResults()
{
  // reset result vector:
  plannerResults.clear();
};