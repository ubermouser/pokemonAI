#ifndef ROULETTE_H
#define ROULETTE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <boost/array.hpp>
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

  static void generatePartitionTable(
    std::vector<fpType>& partition, 
    std::vector<bool>& isValid,
    size_t& numValid,
    fpType& accumulator,
    const std::vector<unknownType>& values,
    const typeValue& comparator)
  {
    // set array of allowable values:
    isValid.assign(values.size(), true);
    numValid = values.size();
    partition.assign(values.size(), std::numeric_limits<fpType>::quiet_NaN());
    for (size_t iValue = 0, iSize = values.size(); iValue != iSize; ++iValue)
    {
      fpType cValue = comparator.getValue(values[iValue]);

      if (boost::math::isnan(cValue)) { isValid[iValue] = false; numValid--; continue; }
      if (mostlyLTE(cValue, 0.0)) { isValid[iValue] = false; numValid--; continue; }

      accumulator += cValue;
      partition[iValue] = cValue;
    }

  };
public:
  static size_t select(
    const std::vector<unknownType>& values,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    boost::array<size_t, 1> result;
    result.assign(SIZE_MAX);
    _selectN< boost::array<size_t, 1> >(values, result, comparator);

    return result[0];
  };

  static boost::array<size_t, 2> selectTwo(
    const std::vector<unknownType>& values,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    boost::array<size_t, 2> result;
    result.assign(SIZE_MAX);

    _selectN< boost::array<size_t, 2> >(values, result, comparator);

    return result;
  };

  template<size_t numValues>
  static boost::array<size_t, numValues> selectN(
    const std::vector<unknownType>& values,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    boost::array<size_t, numValues> result;
    result.assign(SIZE_MAX);

    _selectN< boost::array<size_t, numValues> >(values, result, comparator);

    return result;
  };

  static std::vector<size_t> selectDynamic(
    const std::vector<unknownType>& values,
    size_t numValues,
    const typeValue& comparator = basicSortBySize<unknownType>())
  {
    std::vector<size_t> result(numValues, SIZE_MAX);

    _selectN< std::vector<size_t> >(values, result, comparator);

    return result;
  };

private:
  template<class collectionType>
  static void _selectN(
    const std::vector<unknownType>& values,
    collectionType& results,
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
        // ignore partitions with values we've already used / no value / strang values, etc:
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
