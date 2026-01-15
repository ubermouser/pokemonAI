#ifndef EVALUATOR_NETWORK_64_H
#define EVALUATOR_NETWORK_64_H

#include "pokemonai/evaluator_network.h"

class evaluator_network64 : public EvaluatorNetwork
{
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network64(const Config& cfg = Config{});
  evaluator_network64(const evaluator_network64& other);
  evaluator_network64(const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~evaluator_network64() override {};

  evaluator_network64* clone() const override;

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
};

#endif /* EVALUATOR_NETWORK_64_H */
