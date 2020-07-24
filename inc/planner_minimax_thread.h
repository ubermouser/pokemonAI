#ifndef MINIMAX_THREAD_H
#define MINIMAX_THREAD_H

#include "../inc/pkai.h"

#include <queue>
#include <vector>

class pokemonAI;
class environment_nonvolatile;
class planner_minimax;
class ply;
class pkCU;
class evaluator;
class transposition_table;
class orderHeuristic;
namespace boost { class mutex; };

union environment_volatile;
union environment_possible;

class plyComparison
{
public:
  bool operator() (const class ply* lhs, const class ply* rhs) const;
};

class minimax_threadArg
{
private:
  /* minimax planner base */
  planner_minimax* base;

  /* this thread's plyQueue */
  std::priority_queue<class ply*, std::vector<class ply*>, plyComparison> plyQueue;

  /* root node */
  ply* root;

  /* this threads CU engine */
  pkCU* cu;

  /* evaluator being used by this thread */
  evaluator* eval;

  minimax_threadArg(); // default constructor should NOT be called

  bool evaluateVertex_IDDFS_Fitness(ply& current, std::vector<ply*>& _children);
  int evaluateVertex_IDDFS();

public:
  minimax_threadArg(planner_minimax& _base, const evaluator& eval);
  minimax_threadArg(planner_minimax& _base);
  ~minimax_threadArg();

  minimax_threadArg(const minimax_threadArg& other);

  const environment_nonvolatile& getNV() const;

  pkCU& getCU();

  transposition_table& getTTable();
  const transposition_table& getTTable() const;

  evaluator& getEval();
  const evaluator& getEval() const;

  size_t getAgentTeam();
  size_t getOtherTeam();

  void setEvaluator(const evaluator& evalType);

  orderHeuristic& getAgentOrder();
  orderHeuristic& getOtherOrder();

  const orderHeuristic& getAgentOrder() const;
  const orderHeuristic& getOtherOrder() const;

  size_t getMaxDepth() const;

  boost::mutex& getStdioLock();

  void setEnvironment(pkCU& _cu, size_t _agentTeam);

  void setRoot(const environment_possible& origin);

  void cleanUp();
  
  bool isInitialized() const;

  friend class planner_minimax;
  friend class ply;
  friend class agentMove;
  friend class otherMove;
  friend void minimax_thread(minimax_threadArg* t);
};

/* static driver for all threads. This is a thread's main method */
void minimax_thread(minimax_threadArg* t);

#endif /* MINIMAX_THREAD_H */
