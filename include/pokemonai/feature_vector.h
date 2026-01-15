#ifndef FEATURE_VECTOR_H
#define FEATURE_VECTOR_H

#include <stdint.h>

#include <cstddef>
#include <vector>

#include "pokemonai/environment_volatile.h"

class FeatureVector {
 public:
  typedef std::vector<float>::iterator floatIterator_t;
  typedef std::vector<float>::const_iterator constFloatIterator_t;

  virtual ~FeatureVector() { };

  virtual size_t inputSize() const = 0;
  virtual size_t outputSize() const = 0;

  virtual void seed(
      floatIterator_t cInput,
      const ConstEnvironmentVolatile& env,
      size_t iTeam) const = 0;
};

#endif /* FEATURE_VECTOR_H */