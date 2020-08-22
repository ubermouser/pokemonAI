/* 
 * File:   fitness.h
 * Author: ubermouser
 *
 * Created on August 21, 2020, 11:12 AM
 */

#ifndef FITNESS_H
#define FITNESS_H

#include "pkai.h"

#include <assert.h>

#include "fp_compare.h"

template<
    typename PrecisionType,
    int min_fitness_t=0,
    int max_fitness_t=1,
    int fitness_d=1>
class FitnessType {
public:
  using fitness_t = FitnessType<PrecisionType, min_fitness_t, max_fitness_t, fitness_d>;
  static constexpr PrecisionType one() { return PrecisionType(1.0); }
  static constexpr PrecisionType zero() { return PrecisionType(0.0); }
  static constexpr PrecisionType max_fitness() { return PrecisionType(max_fitness_t) / PrecisionType(fitness_d); }
  static constexpr PrecisionType min_fitness() { return PrecisionType(min_fitness_t) / PrecisionType(fitness_d); }

  static constexpr fitness_t worst() { return fitness_t{min_fitness(), one()}; }
  static constexpr fitness_t best() { return fitness_t{max_fitness(), one()}; }

  FitnessType(
      const PrecisionType& value = min_fitness(),
      const PrecisionType& certainty = zero()) :
      value_(value),
      certainty_(certainty) {
    assertValidity();
  };
  ~FitnessType() {};
  FitnessType(const fitness_t& other) = default;

  friend fitness_t operator +(fitness_t lhs, const fitness_t& rhs) {
    lhs += rhs;
    return lhs;
  }
  fitness_t& operator +=(const fitness_t& rhs) {
    value_ = (certainty() * value_) + (rhs.certainty() * rhs.value_);
    certainty_ += rhs.certainty();

    assertValidity();
    return *this;
  }

  bool operator <(const fitness_t& rhs) const { return upperBound() < rhs.lowerBound(); }
  bool operator <=(const fitness_t& rhs) const { return upperBound() <= rhs.lowerBound(); }
  bool operator >(const fitness_t& rhs) const { return lowerBound() > rhs.upperBound(); }
  bool operator >=(const fitness_t& rhs) const { return lowerBound() >= rhs.upperBound(); }

  bool fullyEvaluated() const {
    return mostlyEQ(certainty(), one());
  }

  PrecisionType upperBound() const { return (value_ * certainty()) + (max_fitness() * uncertainty()); }
  PrecisionType lowerBound() const { return (value_ * certainty()) + (min_fitness() * uncertainty()); }
  const PrecisionType& value() const { return value_; }
  const PrecisionType& certainty() const { return certainty_; }
  PrecisionType uncertainty() const { return one() - certainty_; }
protected:
  void assertValidity() const {
    assert(value_ >= min_fitness() && value_ <= max_fitness());
    assert(certainty_ >= zero() && certainty_ <= one());
    assert(upperBound() <= max_fitness() && lowerBound() >= min_fitness());
  }

  PrecisionType value_;

  PrecisionType certainty_;
};

using Fitness = FitnessType<fpType, 0, 1, 1>;

#endif /* FITNESS_H */

