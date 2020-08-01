#ifndef MINIMAX_THREAD_H
#define MINIMAX_THREAD_H

#include "../inc/pkai.h"

#include <queue>
#include <vector>

class PokemonAI;
class EnvironmentNonvolatile;
class planner_minimax;
class ply;
class PkCU;
class Evaluator;
class transposition_table;
class orderHeuristic;
namespace boost { class mutex; };

union EnvironmentVolatile;
union EnvironmentPossible;

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
  PkCU* cu;

  /* evaluator being used by this thread */
  Evaluator* eval;

  minimax_threadArg(); // default constructor should NOT be called

  bool evaluateVertex_IDDFS_Fitness(ply& current, std::vector<ply*>& _children);
  int evaluateVertex_IDDFS();

public:
  minimax_threadArg(planner_minimax& _base, const Evaluator& eval);
  minimax_threadArg(planner_minimax& _base);
  ~minimax_threadArg();

  minimax_threadArg(const minimax_threadArg& other);

  const EnvironmentNonvolatile& getNV() const;

  PkCU& getCU();

  transposition_table& getTTable();
  const transposition_table& getTTable() const;

  Evaluator& getEval();
  const Evaluator& getEval() const;

  size_t getAgentTeam();
  size_t getOtherTeam();

  void setEvaluator(const Evaluator& evalType);

  orderHeuristic& getAgentOrder();
  orderHeuristic& getOtherOrder();

  const orderHeuristic& getAgentOrder() const;
  const orderHeuristic& getOtherOrder() const;

  size_t getMaxDepth() const;

  boost::mutex& getStdioLock();

  void setEnvironment(PkCU& _cu, size_t _agentTeam);

  void setRoot(const EnvironmentPossible& origin);

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
