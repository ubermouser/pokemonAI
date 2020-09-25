#ifndef RANKED_EVALUATOR_H
#define RANKED_EVALUATOR_H

#include "pkai.h"

#include <memory>
#include <unordered_map>

#include "ranked.h"
#include "evaluator.h"

class RankedEvaluator : public Ranked {
public:
  RankedEvaluator(const std::shared_ptr<Evaluator>& eval) : Ranked(), eval_(eval) { identify(); }

  const std::string& getName() const override { return eval_->getName(); }

  const Evaluator& get() const { return *eval_; };

protected:
  virtual Hash generateHash(bool generateSubHashes = true) override;
  virtual std::string defineName() override { return eval_->getName(); }

  std::shared_ptr<Evaluator> eval_;
};


using RankedEvaluatorPtr = std::shared_ptr<RankedEvaluator>;
using EvaluatorLeague = std::unordered_map<RankedEvaluator::Hash, RankedEvaluatorPtr >;

#endif /* RANKED_EVALUATOR_H */
