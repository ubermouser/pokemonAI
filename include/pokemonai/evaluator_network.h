#ifndef EVALUATOR_NETWORK_H
#define EVALUATOR_NETWORK_H

#include <array>
#include <string>

#include "pokemonai/evaluator.h"
#include "pokemonai/feature_vector.h"
#include "pokemonai/neuralNet.h"
#include "pokemonai/trainable_neural_net.h"


typedef std::array< std::array< std::array< std::array<uint8_t, 4> , 6> , 6>, 2> bestMoveOrders_t;
typedef std::array< std::array< std::array< std::array<float, 4> , 6> , 6>, 2> bestMoveDamages_t;
typedef std::array< std::array< std::array< uint8_t, 6> , 6> , 2> orders_t;

namespace featureVector_impl
{
  void generateBestMoves(const EnvironmentNonvolatile& envNV, bestMoveOrders_t& iBestMoves, bestMoveDamages_t& dBestMoves);
  void generateOrders(const bestMoveDamages_t& dBestMoves, orders_t& orders);
};

class EvaluatorNetwork : public Evaluator, public FeatureVector {
 public:
  struct Config : public Evaluator::Config {
    std::string modelPath;
    TrainableNeuralNet::Config netConfig;

    Config() : Evaluator::Config() {}
    virtual ~Config() {}

    virtual boost::program_options::options_description options(
        const std::string& category = "evaluator options",
        std::string prefix = "") override;
  };

  EvaluatorNetwork(const Config& cfg, size_t inputSize, size_t outputSize);
  EvaluatorNetwork(const neuralNet& network, const Config& cfg);
  EvaluatorNetwork(const EvaluatorNetwork& other);
  virtual ~EvaluatorNetwork() override;

  virtual EvaluatorNetwork& initialize() override;

  const Config& getConfig() const { return cfg_; }

  virtual std::shared_ptr<neuralNet>& getNetwork() { return network_; }

  virtual void setNetwork(const std::shared_ptr<neuralNet>& network);
  virtual void setNetwork(const neuralNet& network) {
    setNetwork(std::make_shared<neuralNet>(network));
  }
  virtual EvaluatorNetwork& setEnvironment(
      const std::shared_ptr<const EnvironmentNonvolatile>& env) override;

  virtual EvalResult calculateFitness(
      const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  virtual EvalResult calculateFitness(
      neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;

  void seed(
      neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;

  virtual size_t inputSize() const override = 0;
  virtual size_t outputSize() const override = 0;


 protected:
  Config cfg_;
  std::shared_ptr<neuralNet> network_;

  bestMoveOrders_t iBestMoves_;
  bestMoveDamages_t dBestMoves_;
  orders_t orders_;

  virtual void generateBestMoves();
  virtual void generateOrders();

  virtual std::string baseName() const override { return "Network"; }
  void updateIdent();

  virtual void seed(
      floatIterator_t cInput,
      const ConstEnvironmentVolatile& env,
      size_t iTeam) const override = 0;
};

#endif // EVALUATOR_NETWORK_H
