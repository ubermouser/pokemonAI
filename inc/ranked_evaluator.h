#ifndef RANKED_EVALUATOR_H
#define RANKED_EVALUATOR_H

#include "pkai.h"

#include <memory>

#include "ranked.h"
#include "evaluator.h"

class RankedEvaluator : public Ranked {
public:
  RankedEvaluator(
    std::shared_ptr<Evaluator>& eval,
    size_t generation = 0);

  const std::string& getName() const { return eval_->getName(); }

  const Evaluator& get() const { return *eval_; };

  std::ostream& print(std::ostream& os) const;

protected:
  std::shared_ptr<Evaluator> eval_;
};


using RankedEvaluatorPtr = std::shared_ptr<RankedEvaluator>;
using EvaluatorLeague = std::unordered_map<RankedEvaluator::Hash, RankedEvaluatorPtr >;

std::ostream& operator <<(std::ostream& os, const RankedEvaluator& tR);

#endif /* RANKED_EVALUATOR_H */
