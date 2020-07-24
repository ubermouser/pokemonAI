#ifndef EXPERIENCENET_H
#define EXPERIENCENET_H

#include "../inc/pkai.h"

#include <vector>
#include <boost/array.hpp>
#include <assert.h>

typedef std::vector<float>::const_iterator constFloatIterator_t;
typedef std::vector<float>::iterator floatIterator_t;
typedef void (*experienceAddSSE_rawType) (float*, const float*, const float* );
typedef enum
{
  EXPERIENCENET_HISTOGRAM,
  EXPERIENCENET_RECENCY
} experienceNetType_t;

class featureVector;
class neuralNet;
class experienceNet;
class experienceNetSettings;

class experienceNetSettings
{
public:
  static const experienceNetSettings defaultSettings;

  static const float defaultExtrapolation;
  static const float defaultDecay;
  static const size_t defaultTaps;
  static const experienceNetType_t defaultType;

  /* number of taps per feature vector */
  size_t numTaps;

  /* value controlling how much extrapolation each datapoint receives */
  float extrapolation;

  /* how much decay per new input added */
  float decay;

  experienceNetType_t eType;

  experienceNetSettings(
    size_t _numTaps = defaultTaps, 
    float _extrapolation = defaultExtrapolation, 
    float _decay = defaultDecay,
    experienceNetType_t _type = defaultType);
};

class experienceNet
{
private:
  // double array of [features][taps]
  std::vector<float> vals;
  // starting indecies of each tap array
  std::vector<size_t> offsets;

  experienceNetSettings eSet;

  // function pointer to experienceNet's add experience function
  experienceAddSSE_rawType expAdd;
  
  // dimensions of val: 
  /* "length" of vector, size in features */
  size_t numFeatures;

  /* the delta value between each tap */
  float partition;

  /* maximum value the normal distribution will take */
  float maxNormalRecip;

  void normalPDF_sse(float* result, const float* _partitions, float _mean, float _stdDev);
  void addExperience_feature_sse(float cVal, float magnitude, size_t iFeature);

  float getExperience_feature(float cVal, size_t iFeature) const;

  /* entropy of a given feature */
  float getEntropy_feature(size_t iFeature) const;

public:

  experienceNet(size_t numFeatures, const experienceNetSettings& _cSet = experienceNetSettings::defaultSettings);
  experienceNet(const neuralNet& cNet, const experienceNetSettings& _cSet = experienceNetSettings::defaultSettings);
  experienceNet(const featureVector& cVec, const experienceNetSettings& _cSet = experienceNetSettings::defaultSettings);

  const experienceNetSettings& getSettings() const { return eSet; };

  float getDecay() const { return eSet.decay; };
  void setDecay(float _decay) { assert(_decay >= 0.0 && _decay <= 1.0); eSet.decay = _decay; };

  float getExtrapolation() const { return eSet.extrapolation; };
  void setExtrapolation(float _extrapolation) { assert(_extrapolation >= 0.0); eSet.extrapolation = _extrapolation; }

  size_t inputSize() const { return numFeatures; };

  /* zero all experience counters within the array */
  void clear();

  /* grow or shrink the number of features the experienceNet uses for each feature */
  void resize(size_t numFeatures);

  float entropy() const;

  /* returns maximum possible experience value in experience array */
  float maximum() const;

  /* minimum possible experience value in experience array */
  float minimum() const;

  /* returns the interpolated amount of experience at the given fVec indecies */
  template<class constIterator_t>
  float getExperience(constIterator_t cVals) const
  {
    float experienceTotal = 0.0f;
    for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature, ++cVals)
    {
      experienceTotal += getExperience_feature(*cVals, iFeature);
    }
    experienceTotal /= (float) numFeatures;
    return experienceTotal;
  };

  /* adds a given amount of experience to the histogram at fVec indecies */
  template<class constIterator_t>
  void addExperience(constIterator_t cVals, float magnitude = 1.0f)
  {
    // decay existing experience:
    decayExperience();
    
    for(size_t iFeature = 0; iFeature != numFeatures; ++iFeature, ++cVals)
    {
      addExperience_feature_sse(*cVals, magnitude, iFeature);
    }
  };

  /* decays the amount of experience in the vector by the decay constant */
  void decayExperience();

private:
  static void recencyAdd_sse(float* cResult, const float* cVal, const float* addedVal);
  static void frequencyAdd_sse(float* cResult, const float* cVal, const float* addedVal);
};

#endif /* EXPERIENCENET_H */
