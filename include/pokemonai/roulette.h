#ifndef ROULETTE_H
#define ROULETTE_H

#include <stdint.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

#include "pokemonai/pkai.h"

struct CStdRandURBG {
  using result_type = unsigned int;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return RAND_MAX; }
  result_type operator()() const {
    return static_cast<result_type>(std::rand());
  }
};
static_assert(std::uniform_random_bit_generator<CStdRandURBG>);

template <class unknownType>
class basicSortBySize {
 public:
  static double getValue(const unknownType& cValue) {
    return static_cast<double>(cValue);
  }
};

template <class unknownType, class typeValue = basicSortBySize<unknownType>>
class roulette {
 public:
  template <class UnknownTypeCollection>
  static size_t select(
      const UnknownTypeCollection& values,
      const typeValue& comparator = typeValue()) {
    double sum = 0.0;
    size_t lastValid = SIZE_MAX;
    size_t idx = 0;
    for (const auto& v : values) {
      double w = static_cast<double>(comparator.getValue(v));
      if (!std::isnan(w) && w > 0.0) {
        sum += w;
        lastValid = idx;
      }
      ++idx;
    }
    if (lastValid == SIZE_MAX) { return SIZE_MAX; }

    CStdRandURBG rng;
    double target = std::uniform_real_distribution<double>(0.0, sum)(rng);
    idx = 0;
    for (const auto& v : values) {
      double w = static_cast<double>(comparator.getValue(v));
      if (!std::isnan(w) && w > 0.0) {
        if (target < w) { return idx; }
        target -= w;
      }
      ++idx;
    }
    return lastValid;
  }

  template <class UnknownTypeCollection>
  static std::vector<size_t> selectDynamic(
      const UnknownTypeCollection& values,
      size_t numValues,
      const typeValue& comparator = typeValue()) {
    if (numValues == 0) { return {}; }
    if (numValues == 1) {
      size_t chosen = select(values, comparator);
      return (chosen == SIZE_MAX) ? std::vector<size_t>{}
                                  : std::vector<size_t>{chosen};
    }

    const size_t totalSize =
        static_cast<size_t>(std::distance(std::begin(values), std::end(values)));
    std::vector<std::pair<double, size_t>> candidates;
    candidates.reserve(totalSize);

    size_t idx = 0;
    for (const auto& v : values) {
      double w = static_cast<double>(comparator.getValue(v));
      if (!std::isnan(w) && w > 0.0) {
        candidates.emplace_back(w, idx);
      }
      ++idx;
    }

    if (candidates.size() > numValues) {
      CStdRandURBG rng;
      for (auto& [weightOrKey, i] : candidates) {
        // Efraimidis-Spirakis: independent Exp(w_i) arrival times; smallest N win
        weightOrKey = std::exponential_distribution<double>(weightOrKey)(rng);
      }
      std::nth_element(
          candidates.begin(),
          candidates.begin() + numValues,
          candidates.end(),
          [](const auto& a, const auto& b) { return a.first < b.first; });
      candidates.resize(numValues);
    }

    std::vector<size_t> result;
    result.reserve(candidates.size());
    for (const auto& [key, i] : candidates) {
      result.push_back(i);
    }
    return result;
  }
};  // endOf roulette

#endif /* ROULETTE_H */
