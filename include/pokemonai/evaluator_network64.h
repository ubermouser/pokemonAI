#ifndef EVALUATOR_NETWORK_64_H
#define EVALUATOR_NETWORK_64_H

#include "pokemonai/evaluator_network.h"

template <class Base>
class evaluator_network64_impl : public Base {
 public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network64_impl(
      const typename Base::Config& cfg = typename Base::Config{});
  evaluator_network64_impl(const evaluator_network64_impl& other);
  evaluator_network64_impl(
      const neuralNet& cNet,
      const typename Base::Config& cfg = typename Base::Config{});
  virtual ~evaluator_network64_impl() override{};

  evaluator_network64_impl* clone() const override {
    return new evaluator_network64_impl(*this);
  }

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
};

typedef evaluator_network64_impl<EvaluatorNetwork> evaluator_network64;
typedef evaluator_network64_impl<TrainableEvaluatorNetwork>
    trainable_evaluator_network64;

#endif /* EVALUATOR_NETWORK_64_H */
