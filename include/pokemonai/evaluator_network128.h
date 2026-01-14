#ifndef NEURAL_NETWORK_EVALUATOR_H
#define NEURAL_NETWORK_EVALUATOR_H

#include "pokemonai/evaluator.h"

#include <ostream>

#include "pokemonai/neuralNet.h"
#include "pokemonai/evaluator_featureVector.h"

class evaluator_network128: public evaluator_featureVector
{
private:
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

  ~evaluator_network128();
  evaluator_network128();
  evaluator_network128(const evaluator_network128& other);
  evaluator_network128(const neuralNet& cNet);

  const std::string& getName() const override { return ident; };

  evaluator_network128* clone() const override { return new evaluator_network128(*this); }

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

#endif /* NEURAL_NETWORK_EVALUATOR_H */
