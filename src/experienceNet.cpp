#include "pokemonai/experienceNet.h"

#include <assert.h>
#include <cstring>
#include <boost/math/distributions/normal.hpp>
#include <math.h>

#include "pokemonai/fp_compare.h"

#include "pokemonai/evaluator_featureVector.h"
#include "pokemonai/neuralNet.h"

#define ALIGN_SIZE 4
#define ALIGN_OF(size) (size & 3)
#define IS_ALIGNED(size) (ALIGN_OF(size) == 0)

const experienceNetSettings experienceNetSettings::defaultSettings = experienceNetSettings(defaultTaps, defaultExtrapolation, defaultDecay);

const size_t experienceNetSettings::defaultTaps = 32;
const float experienceNetSettings::defaultDecay = 0.8f;
const float experienceNetSettings::defaultExtrapolation = 1.0f / 256.0f;
const experienceNetType_t experienceNetSettings::defaultType = EXPERIENCENET_HISTOGRAM;

experienceNetSettings::experienceNetSettings(
  size_t _numTaps, 
  float _extrapolation, 
  float _decay,
  experienceNetType_t _type)
  : numTaps(std::max((size_t)2,_numTaps)),
  extrapolation(_extrapolation),
  decay(_decay),
  eType(_type)
{
};

experienceNet::experienceNet(size_t _numFeatures, const experienceNetSettings& _cSet)
  : vals(),
  offsets(),
  eSet(_cSet),
  expAdd(_cSet.eType==EXPERIENCENET_HISTOGRAM?frequencyAdd_sse:_cSet.eType==EXPERIENCENET_RECENCY?recencyAdd_sse:NULL),
  numFeatures(_numFeatures),
  partition((1.0f) / ((float)eSet.numTaps - 1)),
  maxNormalRecip(1.0f / boost::math::pdf(boost::math::normal_distribution<float>(0.0f, eSet.extrapolation), 0.0f))
{
  assert(expAdd != NULL);
  assert(numFeatures > 0);
  size_t expandedTapSize = eSet.numTaps;
  if (!IS_ALIGNED(expandedTapSize)) { expandedTapSize += ALIGN_SIZE - ALIGN_OF(expandedTapSize); } 
  // fill val array:
  vals.assign(expandedTapSize * numFeatures, 0.0f);
  // assign offset array:
  offsets.reserve(numFeatures);
  for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature)
  {
    offsets.push_back(iFeature * expandedTapSize);
  }
};

experienceNet::experienceNet(const neuralNet& cNet, const experienceNetSettings& _cSet)
{
  *this = experienceNet(cNet.numInputs(), _cSet);
};

experienceNet::experienceNet(const featureVector& cVec, const experienceNetSettings& _cSet)
{
  *this = experienceNet(cVec.inputSize(), _cSet);
};




void experienceNet::clear()
{
  // zero everything:
  std::memset(&vals.front(), 0, sizeof(float) * vals.size());
};






void experienceNet::resize(size_t _numFeatures)
{
  if (_numFeatures == numFeatures) { return; } // do nothing if sizes are equal
  // obtain previous expanded tap size:
  size_t expandedTapSize = vals.size() / numFeatures;
  // fill val array:
  vals.assign(expandedTapSize * _numFeatures, 0.0f);
  numFeatures = _numFeatures;
  // reassign offset array:
  offsets.clear();
  offsets.reserve(numFeatures);
  for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature)
  {
    offsets.push_back(iFeature * expandedTapSize);
  }
};





void experienceNet::decayExperience()
{
  /*if (mostlyEQ(eSet.decay, 1.0f)) { return; } // exit early if no change

  const __m128 decays = _mm_load_ps1(&eSet.decay);

  for (__m128* cVal = (__m128*)&vals.front(), *endVal = (__m128*)(&vals.front()+vals.size()); cVal != endVal; ++cVal)
  {
    *cVal = _mm_mul_ps(*cVal, decays);
  };*/
  /*// TODO: SIMD
  for (size_t iVal = 0; iVal != vals.size(); ++iVal)
  {
    vals[iVal] *= eSet.decay;
  }*/

};





float experienceNet::maximum() const
{
  float result = 0.0f;
  for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature)
  {
    float cHighest = -std::numeric_limits<float>::infinity();
    constFloatIterator_t cVals = vals.begin() + offsets[iFeature];
    for (size_t iTap = 0; iTap != eSet.numTaps; ++iTap)
    {
      if (cVals[iTap] > cHighest) { cHighest = cVals[iTap]; }
    }
    result += cHighest;
  }
  result /= (float) numFeatures;
  return result;
};





float experienceNet::minimum() const
{
  float result = 0.0f;
  for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature)
  {
    float cLowest = std::numeric_limits<float>::infinity();
    constFloatIterator_t cVals = vals.begin() + offsets[iFeature];
    for (size_t iTap = 0; iTap != eSet.numTaps; ++iTap)
    {
      if (cVals[iTap] < cLowest) { cLowest = cVals[iTap]; }
    }
    result += cLowest;
  }
  result /= (float) numFeatures;
  return result;
};





float experienceNet::entropy() const
{
  float result = 0.0f;
  for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature)
  {
    result += getEntropy_feature(iFeature);
  }
  result /= (float) numFeatures;
  return result;
};

float experienceNet::getEntropy_feature(size_t iFeature) const
{
  constFloatIterator_t cVals = vals.begin() + offsets[iFeature];
  float bitsToRepresent = log((float)eSet.numTaps); // assured minimum 2

  // summate frequency of all taps:
  float frequencySum = 0.0f;
  for (size_t iTap = 0; iTap != eSet.numTaps; ++iTap) { frequencySum += cVals[iTap]; }
  if (mostlyEQ(frequencySum, 0.0f)) { return 0.0f; }
  // determine entropy of each tap:
  float entropy = 0.0f;
  for (size_t iTap = 0; iTap != eSet.numTaps; ++iTap)
  {
    float subsetFrequency = cVals[iTap] / frequencySum;
    if (mostlyEQ(0.0f,subsetFrequency)) { continue; }
    entropy += // the max comparison serves to remove any NaNs that may occur from taking the log of 0
      subsetFrequency *
      (log(subsetFrequency) / bitsToRepresent);
  }

  return -entropy;
};





float experienceNet::getExperience_feature(float cValue, size_t iFeature) const
{
  constFloatIterator_t cVals = vals.begin() + offsets[iFeature];
  assert(cValue >= 0 && cValue <= 1.0);

  // determine bounded lowest and highest tap:
  size_t iUpperBound, iLowerBound;
  float upperBound, lowerBound;
  iLowerBound = (size_t) (cValue / partition); // implicit floor
  iLowerBound = std::min(eSet.numTaps, iLowerBound);
  iLowerBound = (size_t)std::max((int64_t)iLowerBound, (int64_t)0);
  iUpperBound = std::min(eSet.numTaps - 1, iLowerBound + 1);
  // bounds to interpolate around:
  upperBound = partition * iUpperBound;
  lowerBound = partition * iLowerBound;

  // interpolate result:
  float interpolationFactor = scale(cValue, upperBound, lowerBound);
  float interpolatedValue = ((1.0f - interpolationFactor) * cVals[iLowerBound]) + ((interpolationFactor) * cVals[iUpperBound]);
  return interpolatedValue;
};



#define M_PI 3.14159265358979323846
/*void experienceNet::normalPDF(float* result, const float* _partitions, float _mean, float _stdDev)
{
  boost::math::normal_distribution<float> cNormal(_mean, _stdDev);
  result[0] = boost::math::pdf(cNormal, _partitions[0]));
}*/

void experienceNet::normalPDF_sse(float* result, const float* _partitions, float _mean, float _stdDev)
{
  /* 
  CODE ADAPTED FROM boost/math/normal.hpp 

  RealType exponent = x - mean;
  exponent *= -exponent;
  exponent /= 2 * sd * sd;

  result = exp(exponent);
  result /= sd * sqrt(2 * constants::pi<RealType>());

  return result;
  */
  boost::math::normal_distribution<float> cNormal(_mean, _stdDev);
  result[0] = boost::math::pdf(cNormal, _partitions[0]);
  result[1] = boost::math::pdf(cNormal, _partitions[1]);
  result[2] = boost::math::pdf(cNormal, _partitions[2]);
  result[3] = boost::math::pdf(cNormal, _partitions[3]);
};
#undef M_PI

void experienceNet::addExperience_feature_sse(float cVal, float magnitude, size_t iFeature)
{
  /*assert(cVal >= 0 && cVal <= 1.0);

  static const __m128 addOffsets = _mm_set_ps(0.0, 1.0, 2.0, 3.0);
  floatIterator_t cVals = vals.begin() + offsets[iFeature];
  const __m128 partitions = _mm_load_ps1(&partition);
  const __m128 maxNormalRecips = _mm_load_ps1(&maxNormalRecip);
  const __m128 magnitudes = _mm_load_ps1(&magnitude);

  float cPartitions, result;

  for (size_t iTap = 0; iTap < eSet.numTaps; iTap +=4, cVals += 4)
  {
    // set to iTap,iTap,iTap,iTap
    cPartitions = _mm_set1_ps(iTap);
    // iTap+0, iTap+1, iTap+2, iTap+3
    cPartitions = _mm_add_ps(cPartitions, addOffsets);
    // partition[0], partition[1], partition[2], partition[3]
    cPartitions = _mm_mul_ps(cPartitions, partitions);

    // compute PDF of the normal distribution:
    normalPDF((float*)&result, (float*)&cPartitions, cVal, eSet.extrapolation);

    // divide by maxNormal, multiply by magnitude:
    result *= maxNormalRecip;
    result *= magnitude;

    // add recency/histogram:
    expAdd((float*)&*cVals, (float*)&*cVals, (float*)&result);
  }*/
}






void experienceNet::recencyAdd_sse(float* cResult, const float* cVal, const float* addedVal)
{
  /*// increase value bounded to 1.0:
  static const __m128 ones = _mm_set1_ps(1.0f);
  const __m128* addedVals = (__m128*)addedVal;
  const __m128* cVals = (__m128*)cVal;

  // result = cVal + addedVal
  __m128 result = _mm_add_ps(*cVals, *addedVals);
  // result = min(1.0, result)
  result = _mm_min_ps(result, ones);

  // return result:
  _mm_store_ps(cResult, result);*/
}

void experienceNet::frequencyAdd_sse(float* cResult, const float* cVal, const float* addedVal)
{
  /*// increase value:
  const __m128* addedVals = (__m128*)addedVal;
  const __m128* cVals = (__m128*)cVal;

  // result = cVal + addedVal
  __m128 result = _mm_add_ps(*cVals, *addedVals);

  // return result:
  _mm_store_ps(cResult, result);*/
}
