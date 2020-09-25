#include "../inc/ranked_evaluator.h"

#include <functional>

std::ostream& operator <<(std::ostream& os, const RankedEvaluator& tR) {
  tR.print(os);
  return os;
};


RankedEvaluator::Hash RankedEvaluator::generateHash(bool generateSubHashes) {
  hash_ = std::hash<Evaluator*>()(eval_.get());
  return hash_;
}
