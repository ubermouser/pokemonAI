#ifndef TRAINER_H
#define TRAINER_H

#include "pokemonai/teambuilder.h"
#include "pokemonai/trainer_regress_fitness.h"

class Trainer : public TeamBuilder {
 public:
  struct Config : public TeamBuilder::Config {
    TrainerRegressFitness::Config training;

    Config() : TeamBuilder::Config() {}

    boost::program_options::options_description options(
        const std::string& category = "trainer configuration",
        std::string prefix = "");
  };

 Trainer(const Config& cfg);

 virtual void initialize() override;

protected:
 struct TrainablePair {
   std::shared_ptr<class TrainableEvaluatorNetwork> evaluator;
   std::shared_ptr<class TrainableNeuralNet> network;
   std::shared_ptr<class TrainerRegressFitness> trainer;
 };
 virtual void postGenerationHook(LeagueHeat& league) const override;
 virtual void postEvolveHook(LeagueHeat& league) const override;

 void train(LeagueHeat& league) const;
 void saveTrainedNetworks() const;
 void printTrainingResults(const LeagueHeat& league) const;

 Config cfg_;
 std::vector<TrainablePair> trainableNetworks_;
};

#endif // TRAINER_H
