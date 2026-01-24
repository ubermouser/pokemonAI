#ifndef EVALUATOR_NETWORK_16_H
#define EVALUATOR_NETWORK_16_H

#include "pokemonai/evaluator_network.h"

template <class Base>
class evaluator_network16_impl : public Base {
 public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  evaluator_network16_impl(
      const typename Base::Config& cfg = typename Base::Config{});
  evaluator_network16_impl(const evaluator_network16_impl& other);
  evaluator_network16_impl(
      const neuralNet& cNet,
      const typename Base::Config& cfg = typename Base::Config{});
  virtual ~evaluator_network16_impl() override{};

  evaluator_network16_impl* clone() const override {
    return new evaluator_network16_impl(*this);
  }

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };

 protected:
  void generateOrders() override;
};

typedef evaluator_network16_impl<EvaluatorNetwork> evaluator_network16;
typedef evaluator_network16_impl<TrainableEvaluatorNetwork>
    trainable_evaluator_network16;

#endif /* EVALUATOR_NETWORK_16_H */
