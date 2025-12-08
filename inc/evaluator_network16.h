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

  const std::string& getName() const override { return ident; };

  evaluator_network16* clone() const override { return new evaluator_network16(*this); }

  bool isInitialized() const;

  void resetNetwork(const neuralNet& cNet) override;
  void resetEvaluator(const EnvironmentNonvolatile& envNV);
  EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;

  EvalResult calculateFitness(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;

  void seed(float* cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
  const float* getInput() const override;

  void outputNames(std::ostream& oS) const override;
};

#endif /* EVALUATOR_NETWORK_16_H */
