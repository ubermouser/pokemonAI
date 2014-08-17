#ifndef PLANNER_MAX_H
#define PLANNER_MAX_H

#include "../inc/pkai.h"
#include "../inc/planner.h"

class evaluator;
class pkCU;

class planner_max : public planner
{
private:
	std::string ident;

	std::vector<plannerResult> results;

	pkCU* cu;

	/* evaluator being used by this planner */
	evaluator* eval;

	size_t agentTeam;

	size_t engineAccuracy;

public:
	planner_max(size_t engineAccuracy = 1);
	planner_max(const evaluator& evalType, size_t engineAccuracy = 1);

	planner_max(const planner_max& other);
	
	~planner_max();

	planner_max* clone() const { return new planner_max(*this); }

	bool isInitialized() const;

	const std::string& getName() const { return ident; };

	void setEvaluator(const evaluator& evalType);
	const evaluator* getEvaluator() const { return eval; }

	void setEnvironment(pkCU& _cu, size_t _agentTeam);

	uint32_t generateSolution(const environment_possible& origin);

	static uint32_t generateSolution(pkCU& _cu, evaluator& eval, const environment_possible& origin, size_t _agentTeam, size_t* nodesEvaluated = NULL, std::vector<plannerResult>* results = NULL);

	const std::vector<plannerResult>& getDetailedResults() const { return results; };
	void clearResults() { results.clear(); };
};

#endif /* PLANNER_MAX_H */
