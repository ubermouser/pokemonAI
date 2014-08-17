#ifndef PLANNER_RANDOM_H
#define PLANNER_RANDOM_H

#include "../inc/planner.h"

class pokemonAI;
class pkCU;

class planner_random : public planner
{
private:
	static const std::string ident;

	pkCU* cu;

	size_t agentTeam;

public:
	planner_random();

	planner_random(const planner_random& other);
	
	~planner_random() { };

	planner_random* clone() const { return new planner_random(*this); }

	bool isInitialized() const;

	const std::string& getName() const { return ident; };

	void setEvaluator(const evaluator& evalType) { /* we're not going to use the evaluator, so do nothing with it */ };
	const evaluator* getEvaluator() const { return NULL; }

	void setEnvironment(pkCU& _cu, size_t _agentTeam);

	uint32_t generateSolution(const environment_possible& origin);
};

#endif /* PLANNER_RANDOM_H */
