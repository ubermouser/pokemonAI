#ifndef EVALUATOR_NETWORK_32_H
#define EVALUATOR_NETWORK_32_H

#include "pokemonai/evaluator_network.h"

template <class Base>
class evaluator_network32_impl : public Base {
 public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network32_impl(
      const typename Base::Config& cfg = typename Base::Config{});
  evaluator_network32_impl(const evaluator_network32_impl& other);
  evaluator_network32_impl(
      const neuralNet& cNet,
      const typename Base::Config& cfg = typename Base::Config{});
  virtual ~evaluator_network32_impl() override{};

  evaluator_network32_impl* clone() const override {
    return new evaluator_network32_impl(*this);
  }

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
};

typedef evaluator_network32_impl<EvaluatorNetwork> evaluator_network32;
typedef evaluator_network32_impl<TrainableEvaluatorNetwork>
    trainable_evaluator_network32;

#endif /* EVALUATOR_NETWORK_32_H */
