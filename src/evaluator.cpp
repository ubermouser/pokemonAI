//#define PKAI_IMPORT
#include "../inc/evaluator.h"


bool Evaluator::isInitialized() const {
  if (nv_ == NULL) { return false; }

  return true;
}