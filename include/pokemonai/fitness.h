/*
 * File:   fitness.h
 * Author: ubermouser
 *
 * Created on August 21, 2020, 11:12 AM
 */

#ifndef FITNESS_H
#define FITNESS_H

#include <algorithm>
#include <iosfwd>
#include <limits>

#include "fp_compare.h"
#include "pkai.h"


#define FITNESS_TEMPLATE        \
  template <                    \
      typename PrecisionType,   \
      typename ProbabilityType, \
      int min_fitness_t,        \
      int max_fitness_t,        \
      int fitness_d>
#define FITNESS_IMPL   \
  FitnessType<         \
      PrecisionType,   \
      ProbabilityType, \
      min_fitness_t,   \
      max_fitness_t,   \
      fitness_d>

FITNESS_TEMPLATE
class FitnessType {
public:
  using fitness_t = FITNESS_IMPL;
  using precision_t = PrecisionType;
  using probability_t = ProbabilityType;

  static constexpr PrecisionType one() { return PrecisionType(1.0); }
  static constexpr PrecisionType zero() { return PrecisionType(0.0); }
  static constexpr ProbabilityType prob_one() { return ProbabilityType(1.0); }
  static constexpr ProbabilityType prob_zero() { return ProbabilityType(0.0); }

  static constexpr PrecisionType max_fitness() {
    return PrecisionType(max_fitness_t) / PrecisionType(fitness_d);
  }
  static constexpr PrecisionType min_fitness() {
    return PrecisionType(min_fitness_t) / PrecisionType(fitness_d);
  }

  static constexpr fitness_t worst() {
    return fitness_t{
        -std::numeric_limits<PrecisionType>::infinity(), prob_one(), false};
  }
  static constexpr fitness_t best() {
    return fitness_t{
        std::numeric_limits<PrecisionType>::infinity(), prob_one(), false};
  }

  explicit FitnessType(
      const PrecisionType& value = min_fitness(),
      const ProbabilityType& certainty = prob_one())
      : FitnessType(value, certainty, true) {}
  ~FitnessType() {};
  FitnessType(const fitness_t& other) = default;

  friend fitness_t operator +(fitness_t lhs, const fitness_t& rhs) {
    lhs += rhs;
    return lhs;
  }
  fitness_t& operator +=(const fitness_t& rhs) {
    // average the two values together in accordance to their certainty:
    // using double for intermediate math prevents rounding errors from pushing
    // the value out of bounds when PrecisionType is float.
    double new_value =
        (static_cast<double>(certainty_) * static_cast<double>(value_)) +
        (static_cast<double>(rhs.certainty_) * static_cast<double>(rhs.value_));
    // combine certainty:
    certainty_ = std::min(prob_one(), certainty_ + rhs.certainty_);
    // normalize value by certainty:
    // TODO(@drendleman) - unstable when certainty_ is small!
    new_value /= certainty_.to_double();
    value_ = std::clamp(PrecisionType(new_value), zero(), one());

    assertValidity();
    return *this;
  }

  fitness_t expand(const ProbabilityType& probability) const {
    return fitness_t{value_, certainty_ * probability, false};
  }

  bool operator <(const fitness_t& rhs) const { return upperBound() < rhs.lowerBound(); }
  bool operator <=(const fitness_t& rhs) const { return upperBound() <= rhs.lowerBound(); }
  bool operator >(const fitness_t& rhs) const { return lowerBound() > rhs.upperBound(); }
  bool operator >=(const fitness_t& rhs) const { return lowerBound() >= rhs.upperBound(); }
  bool operator ==(const fitness_t& rhs) const { return mostlyEQ(lowerBound(), rhs.upperBound()); }
  bool operator !=(const fitness_t& rhs) const { return !(*this == rhs); }

  /* @brief when true, this fitness score includes only leaf evaluations. */
  bool fullyEvaluated() const { return mostlyEQ(certainty(), prob_one()); }

  PrecisionType upperBound() const {
    return (value_ * static_cast<PrecisionType>(certainty_)) +
           (max_fitness() * static_cast<PrecisionType>(uncertainty()));
  }
  PrecisionType lowerBound() const {
    return (value_ * static_cast<PrecisionType>(certainty_)) +
           (min_fitness() * static_cast<PrecisionType>(uncertainty()));
  }
  const PrecisionType& value() const { return value_; }
  const ProbabilityType& certainty() const { return certainty_; }
  ProbabilityType uncertainty() const { return prob_one() - certainty_; }

  void print() const;
  std::ostream& print(std::ostream& os) const;
protected:
 explicit FitnessType(
     const PrecisionType& value,
     const ProbabilityType& certainty,
     bool doAssertValidity)
     : value_(value), certainty_(certainty) {
   if (doAssertValidity) { assertValidity(); }
 }

  void assertValidity() const;

  PrecisionType value_;

  ProbabilityType certainty_;
};

using Fitness = FitnessType<fpType, FixType, 0, 1, 1>;


std::ostream& operator <<(std::ostream& os, const Fitness& fitness);


#endif /* FITNESS_H */

