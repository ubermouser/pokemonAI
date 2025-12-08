#ifndef ROULETTE_H
#define ROULETTE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <array>
#include "fp_compare.h"
#include <boost/math/special_functions/fpclassify.hpp>

template<class unknownType>
class basicSortBySize
{
public:
  static fpType getValue (const unknownType& cValue)
  {
    return (fpType) cValue;
  };
};

template <class unknownType,
class typeValue = basicSortBySize<unknownType> >
class roulette
{
private:

  template<class UnknownTypeCollection>
  static void generatePartitionTable(
    std::vector<fpType>& partition, 
    std::vector<bool>& isValid,
    size_t& numValid,
    fpType& accumulator,
    const UnknownTypeCollection& values,
    const typeValue& comparator)
  {
    // set array of allowable values:
    size_t values_size = std::end(values) - std::begin(values);
    isValid.assign(values_size, true);
    partition.assign(values_size, std::numeric_limits<fpType>::quiet_NaN());
    numValid = values_size;
    auto partition_ptr = std::begin(partition);
    auto value_ptr = std::begin(values);
    auto isValid_ptr = std::begin(isValid);
    for (; value_ptr != std::end(values); ++value_ptr, ++partition_ptr, ++isValid_ptr) {
      fpType cValue = comparator.getValue(*value_ptr);

      if (boost::math::isnan(cValue)) { *isValid_ptr = false; numValid--; continue; }
      if (mostlyLTE(cValue, (fpType)0.0)) { *isValid_ptr = false; numValid--; continue; }

      accumulator += cValue;
      *partition_ptr = cValue;
    }
  };

public:
  template<class UnknownTypeCollection> static size_t select(
    const UnknownTypeCollection& values,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    std::array<size_t, 1> result;
    result.fill(SIZE_MAX);
    _selectN< std::array<size_t, 1> >(values, result, comparator);

    return result[0];
  };

  template<class UnknownTypeCollection> static std::vector<size_t> selectDynamic(
    const UnknownTypeCollection& values,
    size_t numValues,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    std::vector<size_t> result(numValues, SIZE_MAX);

    _selectN< std::vector<size_t> >(values, result, comparator);

    return result;
  };

private:
  template<class ResultCollectionType, class UnknownCollectionType>
  static void _selectN(
    const UnknownCollectionType& values,
    ResultCollectionType& results,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    fpType accumulator = 0;
    std::vector<fpType> partition;
    std::vector<bool> isValid;
    size_t numValid;
    size_t numSelections = results.size();
    // set partition table, accumulator, numValid, isValid
    generatePartitionTable(partition, isValid, numValid, accumulator, values, comparator);
    assert(accumulator < RAND_MAX);

    for (size_t iResult = 0, maxResults = std::min(numSelections, numValid); iResult != maxResults; ++iResult)
    {
      size_t cResult = SIZE_MAX;
      fpType cValue = 0.0;
      fpType target = (double)rand()/(double)RAND_MAX;
      target *= accumulator;
      size_t iValue = rand() % values.size();

      for (size_t iNValue = 0, iSize = values.size() * 2; iNValue != iSize; ++iNValue)
      {
        // ignore partitions with values we've already used / no value / string values, etc:
        if (!isValid[iValue]) { iValue = (iValue + 1) % values.size() ; continue; }

        cValue = partition[iValue];
        if (target < cValue) { cResult = iValue; break; }
        target -= cValue;

        iValue = (iValue + 1) % values.size();
      }

      assert(cResult != SIZE_MAX);

      results[iResult] = cResult;
      accumulator -= cValue;
      isValid[cResult] = false;
      numValid--;
    } // endOf per result

    // return a completed array of results:
    return;
  };

}; //endOf roulette

#endif /* ROULETTE_H */
