#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "../inc/pkai.h"

#include "../inc/name.h"

class environment_nonvolatile;
class team_nonvolatile;
class pokemon_nonvolatile;
union environment_volatile;
union team_volatile;
union pokemon_volatile;

struct evalResult_t
{
	fpType fitness;
	int8_t agentMove;
	int8_t otherMove;
};

class evaluator: public hasName
{
public:
	/* delete evaluator */
	virtual ~evaluator() { };

	/* returns a NEW copy of the current evaluator */
	virtual evaluator* clone() const = 0;

	/* does the evaluator have all acceptable data required to perform evaluation? */
	virtual bool isInitialized() const { return true; };

	/* reset the nonvolatile team held by the evaluator. envNV is guaranteed to remain until next reset */
	virtual void resetEvaluator(const environment_nonvolatile& envNV) { };

	/* evaluate the fitness of a given environment for team iTeam */
	virtual evalResult_t calculateFitness(const environment_volatile& env, size_t iTeam) = 0;
};

#endif /* EVALUATOR_H */
