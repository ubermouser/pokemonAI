#ifndef RANKED_EVALUATOR_H
#define RANKED_EVALUATOR_H

#include "../inc/pkai.h"

#include <memory>

#include "../inc/ranked.h"

class Evaluator;

class RankedEvaluator : public Ranked {
protected:
  std::shared_ptr<Evaluator> eval;
public:
  RankedEvaluator(
    const Evaluator& _eval,
    size_t generation = 0, 
    const trueSkillSettings& settings = trueSkillSettings::defaultSettings);

  RankedEvaluator(const RankedEvaluator& other);

  ~RankedEvaluator();

  const std::string& getName() const;

  const Evaluator& getEvaluator() const { return *eval; };

  friend std::ostream& operator <<(std::ostream& os, const RankedEvaluator& tR);
};

std::ostream& operator <<(std::ostream& os, const RankedEvaluator& tR);

#endif /* RANKED_EVALUATOR_H */
