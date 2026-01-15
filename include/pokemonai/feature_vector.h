#ifndef FEATURE_VECTOR_H
#define FEATURE_VECTOR_H

#include <cstddef>
#include "stdint.h"

class FeatureVector
{
public:
  virtual ~FeatureVector() { };

  virtual size_t inputSize() const = 0;
  virtual size_t outputSize() const = 0;
};

#endif /* FEATURE_VECTOR_H */