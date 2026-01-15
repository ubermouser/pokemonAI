#ifndef EVALUATOR_NETWORK_16_H
#define EVALUATOR_NETWORK_16_H

#include "pokemonai/evaluator_network.h"

class evaluator_network16 : public EvaluatorNetwork
{
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network16(const Config& cfg = Config{});
  evaluator_network16(const evaluator_network16& other);
  evaluator_network16(const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~evaluator_network16() override {};

  evaluator_network16* clone() const override;

  void seed(neuralNet::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };


protected:
  void generateOrders() override;
};

#endif /* EVALUATOR_NETWORK_16_H */
