#ifndef NEURAL_NETWORK_EVALUATOR_H
#define NEURAL_NETWORK_EVALUATOR_H

#include "pokemonai/evaluator_network.h"

class evaluator_network128 : public EvaluatorNetwork
{
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network128(const Config& cfg = Config{});
  evaluator_network128(const evaluator_network128& other);
  evaluator_network128(const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~evaluator_network128() override {};

  evaluator_network128* clone() const override { return new evaluator_network128(*this); }

  void seed(float* cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };

  void outputNames(std::ostream& oS) const override;
};

#endif /* NEURAL_NETWORK_EVALUATOR_H */
