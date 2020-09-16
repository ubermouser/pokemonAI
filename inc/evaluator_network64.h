#ifndef EVALUATOR_NETWORK_64_H
#define EVALUATOR_NETWORK_64_H

#include "../inc/evaluator.h"

#include <ostream>

#include "../inc/neuralNet.h"
#include "../inc/evaluator_featureVector.h"

class evaluator_network64: public evaluator_featureVector
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

  ~evaluator_network64();
  evaluator_network64();
  evaluator_network64(const evaluator_network64& other);
  evaluator_network64(const neuralNet& cNet);

  const std::string& getName() const { return ident; };

  evaluator_network64* clone() const { return new evaluator_network64(*this); }

  bool isInitialized() const;

  void resetNetwork(const neuralNet& cNet);
  void resetEvaluator(const EnvironmentNonvolatile& envNV);
  EvalResult calculateFitness(const EnvironmentVolatile& env, size_t iTeam);

  EvalResult calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam);

  void seed(float* cInput, const EnvironmentVolatile& env, size_t iTeam) const;
  size_t inputSize() const { return numInputNeurons; };
  size_t outputSize() const { return numOutputNeurons; };
  const float* getInput() const;

  void outputNames(std::ostream& oS) const;
};

#endif /* EVALUATOR_NETWORK_64_H */
