/* 
 * File:   evaluators.h
 * Author: drendleman
 *
 * Created on September 13, 2020, 3:00 PM
 */

#ifndef EVALUATORS_H
#define EVALUATORS_H

#include <memory>
#include <string>

#include "evaluator.h"

namespace evaluators {

  std::shared_ptr<Evaluator::Config> config(const std::string& _type);
  std::shared_ptr<Evaluator> choose(const std::string& _type, const Evaluator::Config& cfg);

};

#endif /* EVALUATORS_H */

