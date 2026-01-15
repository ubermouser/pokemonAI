#ifndef EVALUATOR_NETWORK_32_H
#define EVALUATOR_NETWORK_32_H

#include "pokemonai/evaluator_network.h"

class evaluator_network32 : public EvaluatorNetwork
{
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network32(const Config& cfg = Config{});
  evaluator_network32(const evaluator_network32& other);
  evaluator_network32(const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~evaluator_network32() override {};

  evaluator_network32* clone() const override;

  void seed(neuralNet::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
};

#endif /* EVALUATOR_NETWORK_32_H */
