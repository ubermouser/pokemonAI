#ifndef PLANNER_STOCHASTIC_H
#define PLANNER_STOCHASTIC_H

#include "../inc/pkai.h"
#include "../inc/planner.h"

class evaluator;
class pkCU;

class planner_stochastic : public planner
{
private:
	std::string ident;

	std::vector<plannerResult> results;

	pkCU* cu;

	/* evaluator being used by this planner */
	evaluator* eval;

	size_t agentTeam;

	size_t engineAccuracy;

	/* A parameter that tunes how likely the planner will choose an action with better fitness compared to
	 * an action with lower fitness when performing exploration. As temperature approaches zero, the odds 
	 * of selecting a less maximized fitness are reduced. */
	fpType temperature;

	/* probability that the planner will explore the state space instead of choosing
	 * the action that maximizes fitness. 1 implies full stochasticism, 0 implies full determinism */
	fpType exploration;

public:
	planner_stochastic(size_t _engineAccuracy = 1, fpType _temperature = 1.0, fpType _exploration = 0.5);
	planner_stochastic(const evaluator& _eval, size_t _engineAccuracy = 1, fpType _temperature = 1.0, fpType _exploration = 0.5);

	planner_stochastic(const planner_stochastic& other);
	
	~planner_stochastic();

	planner_stochastic* clone() const { return new planner_stochastic(*this); }

	bool isInitialized() const;

	const std::string& getName() const { return ident; };

	void setEvaluator(const evaluator& evalType);
	const evaluator* getEvaluator() const { return eval; }

	void setEnvironment(pkCU& _cu, size_t _agentTeam);

	uint32_t generateSolution(const environment_possible& origin);

	const std::vector<plannerResult>& getDetailedResults() const { return results; };
	void clearResults() { results.clear(); };
};

#endif /* PLANNER_STOCHASTIC_H */
