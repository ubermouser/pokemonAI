#ifndef EVALUATOR_NETWORK_32_H
#define EVALUATOR_NETWORK_32_H

#include "pokemonai/evaluator.h"

#include <ostream>

#include "pokemonai/neuralNet.h"
#include "pokemonai/evaluator_featureVector.h"

class evaluator_network32: public evaluator_featureVector
{
  std::string ident;

  const EnvironmentNonvolatile* envNV;
  neuralNet* network;

  bestMoveOrders_t iBestMoves;
  bestMoveDamages_t dBestMoves;
  orders_t orders;

  void generateBestMoves();
  void generateOrders();
public:
  static const size_t numInputNeurons;
  static const size_t numOutputNeurons;

  ~evaluator_network32();
  evaluator_network32();
  evaluator_network32(const evaluator_network32& other);
  evaluator_network32(const neuralNet& cNet);

  const std::string& getName() const override { return ident; };

  evaluator_network32* clone() const override { return new evaluator_network32(*this); }

  bool isInitialized() const;

  void resetNetwork(const neuralNet& cNet) override;
  void resetEvaluator(const EnvironmentNonvolatile& envNV);
  EvalResult calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const override;

  EvalResult calculateFitness(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const;

  void seed(float* cInput, const ConstEnvironmentVolatile& env, size_t iTeam) const override;
  size_t inputSize() const override { return numInputNeurons; };
  size_t outputSize() const override { return numOutputNeurons; };
  const float* getInput() const override;

  void outputNames(std::ostream& oS) const override;
};

#endif /* EVALUATOR_NETWORK_32_H */
