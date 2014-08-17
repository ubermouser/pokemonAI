#ifndef SIMPLE_EVALUATOR_H
#define SIMPLE_EVALUATOR_H

#include "../inc/evaluator.h"

class evaluator_simple: public evaluator
{
private:
	std::string ident;

	fpType bias;

	const environment_nonvolatile* envNV;
public:
	~evaluator_simple() { };
	evaluator_simple(fpType _bias = 0.5);
	evaluator_simple(const evaluator_simple& other);

	static fpType fitness_team(const team_volatile& tV, const team_nonvolatile& tNV);
	static fpType calculateFitness(const environment_nonvolatile& envNV, const environment_volatile& env, size_t iTeam, fpType bias = 0.5);

	const std::string& getName() const { return ident; };

	evaluator_simple* clone() const { return new evaluator_simple(*this); }

	bool isInitialized() const;

	void resetEvaluator(const environment_nonvolatile& _envNV)
	{
		envNV = &_envNV;
	};

	evalResult_t calculateFitness(const environment_volatile& env, size_t iTeam);
};

#endif /* SIMPLE_EVALUATOR_H */
