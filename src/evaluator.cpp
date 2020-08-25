//#define PKAI_IMPORT
#include "../inc/evaluator.h"

#include <stdexcept>


Evaluator& Evaluator::initialize() {
  if (nv_ == NULL) { throw std::invalid_argument("evaluator nonvolatile environment undefined"); }

  return *this;
}