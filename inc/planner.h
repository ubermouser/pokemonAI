#ifndef PLANNER_H
#define PLANNER_H

#include "../inc/pkai.h"

#include "../inc/name.h"

class evaluator;
class environment_nonvolatile;
class pkCU;

union environment_possible;





class plannerResult
{
public:
	size_t depth;
	int bestAgentAction;
	int bestOtherAction;
	fpType lbFitness;
	fpType ubFitness;

	plannerResult(size_t _depth, int _bestAgentAction, int _bestOtherAction, fpType _lbFitness, fpType _ubFitness)
		: depth(_depth),
		bestAgentAction(_bestAgentAction),
		bestOtherAction(_bestOtherAction),
		lbFitness(_lbFitness),
		ubFitness(_ubFitness)
	{
	};
};

class planner : public hasName
{
private:
	static const std::vector<plannerResult> emptyResults;
public:
	virtual ~planner() { };

	/* create a copy of the current planner (and its evaluator) */
	virtual planner* clone() const = 0;

	/* does the planner have all acceptable data required to perform planning? */
	virtual bool isInitialized() const { return true; }

	/* if the planner uses an evaluator, set the evaluator to evalType */
	virtual void setEvaluator(const evaluator& evalType) = 0;
	virtual const evaluator* getEvaluator() const = 0;

	/* sets the nonvolatile environment to _cEnvironment, the team being referenced */
	virtual void setEnvironment(pkCU& _cu, size_t _agentTeam) = 0;

	/* generate an action */
	virtual uint32_t generateSolution(const environment_possible& origin) = 0;

	virtual const std::vector<plannerResult>& getDetailedResults() const { return emptyResults; };
	virtual void clearResults() { };
};

#endif /* PLANNER_H */
