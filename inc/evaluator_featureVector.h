#ifndef FEATURE_VECTOR_H
#define FEATURE_VECTOR_H

#include "../inc/pkai.h"

#include <assert.h>
#include <ostream>
#include <map>

//#include <boost/ptr_container/ptr_map.hpp>
#include <array>

#include "../inc/evaluator.h"
#include "../inc/neuralNet.h"

class EnvironmentVolatile;
class evaluator_featureVector;
class featureVector;
class neuralNet;
typedef std::map<size_t, evaluator_featureVector*> evaluatorMap_t;
//typedef boost::ptr_map<size_t, evaluator_featureVector> evaluatorMap_t;
typedef std::array< std::array< std::array< std::array<uint8_t, 4> , 6> , 6>, 2> bestMoveOrders_t;
typedef std::array< std::array< std::array< std::array<float, 4> , 6> , 6>, 2> bestMoveDamages_t;
typedef std::array< std::array< std::array< uint8_t, 6> , 6> , 2> orders_t;

namespace featureVector_impl
{
  void generateBestMoves(const EnvironmentNonvolatile& envNV, bestMoveOrders_t& iBestMoves, bestMoveDamages_t& dBestMoves);
  void generateOrders(const bestMoveDamages_t& dBestMoves, orders_t& orders);
};

class featureVector
{
public:
  /* delete evaluator */
  virtual ~featureVector() { };

  void seed(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam) const
  {
    assert(cNet.numInputs() >= inputSize());
    seed(cNet.inputBegin(), env, iTeam);
  };
  
  /* output feature vector defined by env starting at cInput */
  virtual void seed(float* cInput, const EnvironmentVolatile& env, size_t iTeam) const = 0;

  void seed(floatIterator_t cInput, const EnvironmentVolatile& env, size_t iTeam) const
  {
    seed(&*cInput, env, iTeam);
  };

  /*template< size_t bufferSize >
  void seed(std::array< float, bufferSize >::iterator cInput, const environment_volatile& env, size_t iTeam) const
  {
    assert(bufferSize >= size());
    seed(&*cInput, env, iTeam);
  };*/

  /* return cached feature vector or NULL if it does not exist */
  virtual const float* getInput() const
  {
    return NULL;
  };

  /* sets the neural network of a featureVector type */
  virtual void resetNetwork(const neuralNet& cNet) = 0;

  /* the size in number of elements that the seed operation will output */
  virtual size_t inputSize() const = 0;
  virtual size_t outputSize() const = 0;

  /* output names of each of the feature vector's elements separated by commas. By default, will output the class name followed by an index */
  virtual void outputNames(std::ostream& oS) const;

  template<class constFloatIterator_t>
  void outputFeatureVector(std::ostream& oS, constFloatIterator_t cOutput)
  {
    // print input vector:
    const float* cInput = getInput();
    if (cInput == NULL) { return; }
    for (size_t iInput = 0; iInput < inputSize(); ++iInput)
    {
      oS << *cInput++ << ", ";
    }
    // print output:
    oS << *cOutput++;
    for (size_t iOutput = 1; iOutput < outputSize(); ++iOutput)
    {
      oS << ", " << *cOutput++;
    }
  
    oS << "\n";
    oS.flush();
  };
};

class evaluator_featureVector : public Evaluator, public featureVector
{
private:
  static evaluatorMap_t evaluators;
public:
  static void initStatic();

  static void uninitStatic();

  static const evaluator_featureVector* getEvaluator(size_t numInputNeurons, size_t numOutputNeurons);

  static bool hasEvaluator(const neuralNet& cNet);

  /* returns a NEW ranked_neuralNet's evaluator type, initialized to include the ranked_neuralNet's network.
   * Returns NULL if no evaluator of this size exists.
   */
  static evaluator_featureVector* getEvaluator(const neuralNet& cNet);


  virtual ~evaluator_featureVector() { };
};

#endif /* FEATURE_VECTOR_H */
