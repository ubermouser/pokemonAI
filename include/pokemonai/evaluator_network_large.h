#ifndef EVALUATOR_NETWORK_LARGE_H
#define EVALUATOR_NETWORK_LARGE_H

#include "pokemonai/evaluator_network.h"

class EvaluatorNetworkLarge : public EvaluatorNetwork {
 public:
  struct Config : public EvaluatorNetwork::Config {

  };

  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  EvaluatorNetworkLarge(const Config& cfg = Config{});
  EvaluatorNetworkLarge(const EvaluatorNetworkLarge& other);
  EvaluatorNetworkLarge(const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~EvaluatorNetworkLarge() override{};

  EvaluatorNetworkLarge* clone() const override;

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
protected:
};

#endif /* EVALUATOR_NETWORK_LARGE_H */
