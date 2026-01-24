#ifndef EVALUATOR_NETWORK_LARGE_H
#define EVALUATOR_NETWORK_LARGE_H

#include "pokemonai/evaluator_network.h"

template <class Base>
class EvaluatorNetworkLarge_impl : public Base {
 public:
  struct Config : public Base::Config {};

  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  EvaluatorNetworkLarge_impl(const Config& cfg = Config{});
  EvaluatorNetworkLarge_impl(const EvaluatorNetworkLarge_impl& other);
  EvaluatorNetworkLarge_impl(
      const neuralNet& cNet, const Config& cfg = Config{});
  virtual ~EvaluatorNetworkLarge_impl() override{};

  EvaluatorNetworkLarge_impl* clone() const override {
    return new EvaluatorNetworkLarge_impl(*this);
  }
  EvaluatorNetworkLarge_impl& setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

  void seed(FeatureVector::floatIterator_t cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };

 protected:
  std::array<std::vector<float>, 2> precomputedNV_;
};

typedef EvaluatorNetworkLarge_impl<EvaluatorNetwork> EvaluatorNetworkLarge;
typedef EvaluatorNetworkLarge_impl<TrainableEvaluatorNetwork>
    TrainableEvaluatorNetworkLarge;

#endif /* EVALUATOR_NETWORK_LARGE_H */
