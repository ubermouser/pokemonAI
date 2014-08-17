#include "../inc/ranked_evaluator.h"

#include <ostream>
#include <iomanip>

#include "../inc/evaluator.h"

ranked_evaluator::ranked_evaluator(
	const evaluator& _eval,
	size_t generation, 
	const trueSkillSettings& settings)
	: ranked(generation, settings),
	eval(_eval.clone())
{
};

ranked_evaluator::ranked_evaluator(const ranked_evaluator& other)
	: ranked(other),
	eval(other.eval->clone())
{
};

ranked_evaluator::~ranked_evaluator()
{
	delete eval;
};

const std::string& ranked_evaluator::getName() const
{
	return eval->getName();
};

std::ostream& operator <<(std::ostream& os, const ranked_evaluator& tR)
{
	os << std::setw(45) << std::left << tR.getName().substr(0,45);

	size_t prevPrecision = os.precision();
	os.precision(6);
	os <<
		" g= " << std::setw(3) << std::right << tR.getGeneration() <<
		" m= " << std::setw(7) << tR.getSkill().getMean() <<
		" s= " << std::setw(7) << tR.getSkill().getStdDev() <<
		" w= " << std::setw(7) << std::left << tR.getNumWins() << 
		" / " << std::setw(7) << std::right << (tR.getNumGamesPlayed());
	os.precision(prevPrecision);

	return os;
};
