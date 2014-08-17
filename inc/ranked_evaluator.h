#ifndef RANKED_EVALUATOR_H
#define RANKED_EVALUATOR_H

#include "../inc/pkai.h"

#include "../inc/ranked.h"

class evaluator;

class ranked_evaluator : public ranked
{
private:
	evaluator* eval;
public:
	ranked_evaluator(
		const evaluator& _eval,
		size_t generation = 0, 
		const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

	ranked_evaluator(const ranked_evaluator& other);

	~ranked_evaluator();

	const std::string& getName() const;

	const evaluator& getEvaluator() const { return *eval; };

	friend std::ostream& operator <<(std::ostream& os, const ranked_evaluator& tR);
};

std::ostream& operator <<(std::ostream& os, const ranked_evaluator& tR);

#endif /* RANKED_EVALUATOR_H */
