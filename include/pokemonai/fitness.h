/* 
 * File:   fitness.h
 * Author: ubermouser
 *
 * Created on August 21, 2020, 11:12 AM
 */

#ifndef FITNESS_H
#define FITNESS_H

#include "pkai.h"

#include <iosfwd>
#include <limits>

#include "fp_compare.h"


#define FITNESS_TEMPLATE template<typename PrecisionType, int min_fitness_t, int max_fitness_t, int fitness_d>
#define FITNESS_IMPL FitnessType<PrecisionType, min_fitness_t, max_fitness_t, fitness_d>

FITNESS_TEMPLATE
class FitnessType {
public:
  using fitness_t = FITNESS_IMPL;
  using precision_t = PrecisionType;

  static constexpr PrecisionType one() { return PrecisionType(1.0); }
  static constexpr PrecisionType zero() { return PrecisionType(0.0); }
  static constexpr PrecisionType max_fitness() { 
    return PrecisionType(max_fitness_t) / PrecisionType(fitness_d);
  }
  static constexpr PrecisionType min_fitness() { 
    return PrecisionType(min_fitness_t) / PrecisionType(fitness_d);
  }

  static constexpr fitness_t worst() { 
    return fitness_t{-std::numeric_limits<PrecisionType>::infinity(), one(), false};
  }
  static constexpr fitness_t best() { 
    return fitness_t{std::numeric_limits<PrecisionType>::infinity(), one(), false};
  }

  explicit FitnessType(
      const PrecisionType& value = min_fitness(),
      const PrecisionType& certainty = one()) :
      FitnessType(value, certainty, true) {}
  ~FitnessType() {};
  FitnessType(const fitness_t& other) = default;

  friend fitness_t operator +(fitness_t lhs, const fitness_t& rhs) {
    lhs += rhs;
    return lhs;
  }
  fitness_t& operator +=(const fitness_t& rhs) {
    // average the two values together in accordance to their certainty:
    value_ = (certainty() * value_) + (rhs.certainty() * rhs.value_);
    // combine certainty:
    certainty_ += rhs.certainty();
    // normalize value by certainty:
    // TODO(@drendleman) - unstable when certainty_ is small!
    value_ /= certainty_;

    assertValidity();
    return *this;
  }

  fitness_t expand(const PrecisionType& probability)const {
    return fitness_t{value_, certainty_ * probability, false};
  }

  bool operator <(const fitness_t& rhs) const { return upperBound() < rhs.lowerBound(); }
  bool operator <=(const fitness_t& rhs) const { return upperBound() <= rhs.lowerBound(); }
  bool operator >(const fitness_t& rhs) const { return lowerBound() > rhs.upperBound(); }
  bool operator >=(const fitness_t& rhs) const { return lowerBound() >= rhs.upperBound(); }
  bool operator ==(const fitness_t& rhs) const { return mostlyEQ(lowerBound(), rhs.upperBound()); }
  bool operator !=(const fitness_t& rhs) const { return !(*this == rhs); }

  bool fullyEvaluated() const {
    return mostlyEQ(certainty(), one());
  }

  PrecisionType upperBound() const { return (value_ * certainty()) + (max_fitness() * uncertainty()); }
  PrecisionType lowerBound() const { return (value_ * certainty()) + (min_fitness() * uncertainty()); }
  const PrecisionType& value() const { return value_; }
  const PrecisionType& certainty() const { return certainty_; }
  PrecisionType uncertainty() const { return one() - certainty_; }

  void print() const;
  std::ostream& print(std::ostream& os) const;
protected:
  explicit FitnessType(
      const PrecisionType& value,
      const PrecisionType& certainty,
      bool doAssertValidity) : value_(value), certainty_(certainty) {
    if (doAssertValidity) { assertValidity(); }
  }

  void assertValidity() const;

  PrecisionType value_;

  PrecisionType certainty_;
};

using Fitness = FitnessType<fpType, 0, 1, 1>;


std::ostream& operator <<(std::ostream& os, const Fitness& fitness);


#endif /* FITNESS_H */

