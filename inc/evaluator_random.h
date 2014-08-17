#ifndef RANDOM_EVALUATOR_H
#define RANDOM_EVALUATOR_H

#include "../inc/evaluator.h"

class evaluator_random: public evaluator
{
private:
	static const std::string ident;
public:
	~evaluator_random() { };
	evaluator_random() { };

	const std::string& getName() const { return ident; };

	evaluator_random* clone() const { return new evaluator_random(*this); }

	evalResult_t calculateFitness(const environment_volatile& env, size_t iTeam);
};

#endif /* RANDOM_EVALUATOR_H */
