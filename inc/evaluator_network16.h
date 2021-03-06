#ifndef EVALUATOR_NETWORK_16_H
#define EVALUATOR_NETWORK_16_H

#include "../inc/evaluator.h"

#include <ostream>

#include "../inc/neuralNet.h"
#include "../inc/evaluator_featureVector.h"

class evaluator_network16: public evaluator_featureVector
{
  std::string ident;

  const EnvironmentNonvolatile* envNV;
  neuralNet* network;

  orders_t orders;

  void generateOrders();
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  ~evaluator_network16();
  evaluator_network16();
  evaluator_network16(const evaluator_network16& other);
  evaluator_network16(const neuralNet& cNet);

  const std::string& getName() const { return ident; };

  evaluator_network16* clone() const { return new evaluator_network16(*this); }

  bool isInitialized() const;

  void resetNetwork(const neuralNet& cNet);
  void resetEvaluator(const EnvironmentNonvolatile& envNV);
  EvalResult calculateFitness(const EnvironmentVolatile& env, size_t iTeam);

  EvalResult calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam);

  void seed(float* cInput, const EnvironmentVolatile& env, size_t iTeam) const;
  size_t inputSize() const { return numInputNeurons; };
  size_t outputSize() const { return numOutputNeurons; };
  const float* getInput() const;

  void outputNames(std::ostream& oS) const;
};

#endif /* EVALUATOR_NETWORK_16_H */
