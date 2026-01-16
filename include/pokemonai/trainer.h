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

 virtual LeagueHeat evolve() const;

protected:
  void train(LeagueHeat& league) const;
  void saveTrainedNetworks(const LeagueHeat& league) const;

  Config cfg_;
};

#endif // TRAINER_H
