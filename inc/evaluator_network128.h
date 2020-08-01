#ifndef NEURAL_NETWORK_EVALUATOR_H
#define NEURAL_NETWORK_EVALUATOR_H

#include "../inc/evaluator.h"

#include <ostream>

#include "../inc/neuralNet.h"
#include "../inc/evaluator_featureVector.h"

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

  const std::string& getName() const { return ident; };

  evaluator_network128* clone() const { return new evaluator_network128(*this); }

  bool isInitialized() const;

  void resetNetwork(const neuralNet& cNet);
  void resetEvaluator(const EnvironmentNonvolatile& envNV);
  EvalResult_t calculateFitness(const EnvironmentVolatile& env, size_t iTeam);

  EvalResult_t calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam);

  void seed(float* cInput, const EnvironmentVolatile& env, size_t iTeam) const;
  size_t inputSize() const { return numInputNeurons; };
  size_t outputSize() const { return numOutputNeurons; };
  const float* getInput() const;

  void outputNames(std::ostream& oS) const;
};

#endif /* NEURAL_NETWORK_EVALUATOR_H */
