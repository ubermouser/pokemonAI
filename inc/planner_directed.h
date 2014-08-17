#ifndef PLANNER_DIRECTED_H
#define PLANNER_DIRECTED_H

#include "../inc/pkai.h"

#include "../inc/planner.h"
#include "../inc/experienceNet.h"

class evaluator;
class experienceNet;
class evaluator_featureVector;

class planner_directed : public planner
{
private:
	std::string ident;

	std::vector<plannerResult> results;

	pkCU* cu;

	/* evaluator being used by this planner */
	evaluator_featureVector* eval;

	/* experience accumulator of eval */
	experienceNet exp;

	size_t agentTeam;

	size_t engineAccuracy;

	fpType bias;

	bool updateExperience;

public:
	planner_directed(
		const experienceNet& _exp = experienceNet(1), 
		size_t _engineAccuracy = 1, 
		fpType _bias = 0.5,
		bool _updateExperience = true);
	planner_directed(
		const evaluator_featureVector& _eval, 
		const experienceNet& _exp, 
		size_t _engineAccuracy = 1, 
		fpType _bias = 0.5,
		bool _updateExperience = true);

	planner_directed(const planner_directed& other);
	
	~planner_directed();

	planner_directed* clone() const { return new planner_directed(*this); }

	bool isInitialized() const;

	const std::string& getName() const { return ident; };

	void setEvaluator(const evaluator& evalType);
	const evaluator* getEvaluator() const;

	void setEnvironment(pkCU& _cu, size_t _agentTeam);

	void setExperience(const experienceNet& _exp);
	const experienceNet& getExperience() const { return exp; };
	void clearExperience() { exp.clear(); };

	uint32_t generateSolution(const environment_possible& origin);

	const std::vector<plannerResult>& getDetailedResults() const { return results; };
	void clearResults() { results.clear(); };
};

#endif /* PLANNER_DIRECTED_H */
