#include "../inc/ranked_evaluator.h"

#include <ostream>
#include <iomanip>

#include "../inc/evaluator.h"

RankedEvaluator::RankedEvaluator(
  const Evaluator& _eval,
  size_t generation, 
  const trueSkillSettings& settings)
  : Ranked(generation, settings),
  eval(_eval.clone())
{
};

RankedEvaluator::RankedEvaluator(const RankedEvaluator& other)
  : Ranked(other),
  eval(other.eval->clone())
{
};

const std::string& RankedEvaluator::getName() const
{
  return eval->getName();
};

std::ostream& operator <<(std::ostream& os, const RankedEvaluator& tR)
{
  os << std::setw(45) << std::left << tR.getName().substr(0,45);

  size_t prevPrecision = os.precision();
  os.precision(6);
  os <<
    " g= " << std::setw(3) << std::right << tR.getGeneration() <<
    " m= " << std::setw(7) << tR.skill().getMean() <<
    " s= " << std::setw(7) << tR.skill().getStdDev() <<
    " w= " << std::setw(7) << std::left << tR.getNumWins() << 
    " / " << std::setw(7) << std::right << (tR.getNumGamesPlayed());
  os.precision(prevPrecision);

  return os;
};
