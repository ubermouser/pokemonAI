#include "../inc/trainer.h"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <boost/format.hpp>
#include <boost/program_options.hpp>

namespace po = boost::program_options;


po::options_description Trainer::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc = Ranker::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "mutation-probability").c_str(),
      po::value<double>(&mutationProbability)->default_value(defaults.mutationProbability),
      "probability of a mutation.")
      ((prefix + "crossover-probability").c_str(),
      po::value<double>(&crossoverProbability)->default_value(defaults.crossoverProbability),
      "probability of a crossover.")
      ((prefix + "seed-probability").c_str(),
      po::value<double>(&seedProbability)->default_value(defaults.seedProbability),
      "probability of a seed.")
      ((prefix + "team-populations").c_str(),
      po::value<std::vector<int64_t>>(&teamPopulationSize)->multitoken(),
      "league populations for teams of each size.")
      ((prefix + "max-generations").c_str(),
      po::value<size_t>(&maxGenerations)->default_value(defaults.maxGenerations),
      "number of league generations.");


  return desc;
}



Trainer::Trainer(
    const Config& cfg)
  : Ranker(cfg),
    cfg_(cfg),
    teamFactory_(TeamFactory()),
    doNothingProbability_(1. - cfg.mutationProbability - cfg.crossoverProbability - cfg.seedProbability) {
}


void Trainer::initialize() {
  if (cfg_.teamPopulationSize.size() != 6) { throw std::invalid_argument("teamPopulationSize"); }
  if (*std::min_element(begin(cfg_.teamPopulationSize), end(cfg_.teamPopulationSize)) < 0) { throw std::invalid_argument("teamPopulationSize count"); }
  if (cfg_.mutationProbability > 1.0 || cfg_.mutationProbability < 0.0) { throw std::invalid_argument("mutationProbability"); }
  if (cfg_.crossoverProbability > 1.0 || cfg_.crossoverProbability < 0.0) { throw std::invalid_argument("crossoverProbability"); }
  if (cfg_.seedProbability > 1.0 || cfg_.seedProbability < 0.0) { throw std::invalid_argument("seedProbability"); }
  if (doNothingProbability_ > 1.0 || doNothingProbability_ < 0.0) { throw std::invalid_argument("doNothingProbability"); }

  Ranker::initialize();
}


LeagueHeat Trainer::evolve() const {
  testInitialized();

  LeagueHeat league = constructLeague();
  for (size_t iGeneration = 0; iGeneration < cfg_.maxGenerations; ++iGeneration) {
    if (iGeneration > 0 ) {
      // perform an evolution step (if this is not the first generation):
      evolveGeneration(league);
      
      // reset league counting:
      resetLeague(league);
    }

    if (cfg_.verbosity >= 1) { printGenerationStart(league, iGeneration); }

    // rank the league:
    runLeague(league);
  }

  if (cfg_.verbosity > 0) { out_.get() << "Evolution Complete!\n"; }
  if (cfg_.saveOnCompletion) { saveTeamPopulation(league); }
  return league;
}


void Trainer::resetLeague(LeagueHeat& league) const {
  // destroy the record of the last played games:
  league.resetStats();
  // existing battlegroups should play existing games:
  for (auto& bg : league.battlegroups) {
    bg.second->record().resetRecord();
  }
}


void Trainer::evolveGeneration(LeagueHeat& league) const {
  {
    // mutate the existing population:
    TeamLeague children = spawnTeamChildren(league);

    // shrink the population to make room for the new children:
    shrinkPopulations(league, children.countTeamLeague());

    // feather the ranks of all remaining teams

    // merge the children into the new population:
    for (auto& child: children) { league.addTeam(child.second); }

    // seed additional teams if we don't have the proper number:
    seedRandomTeamPopulation(league);
  }
  // remove pokemon that are no longer used:
  league.removeUnusedPokemon();
}


size_t Trainer::loadTeamPopulation() {
  size_t result = Ranker::loadTeamPopulation();
  result += seedRandomTeamPopulation(initialLeague_);

  return result;
}


size_t Trainer::seedRandomTeamPopulation(League& league) const {
  size_t numAdded = 0;
  LeagueCount population = league.teams.countTeamLeague();
  for (size_t iLeague = 0; iLeague < 6; ++iLeague) {
    int64_t deltaPopulation = (cfg_.teamPopulationSize[iLeague] - population[iLeague]);
    
    for (int64_t iAdded = 0; iAdded < deltaPopulation; ++iAdded) {
      RankedTeamPtr seeded = std::make_shared<RankedTeam>(teamFactory_.createRandom(iLeague + 1), league.pokemon);
      // insert into league:
      league.addTeam(seeded);
      numAdded += 1;
    }
  }

  return numAdded;
}


TeamLeague Trainer::spawnTeamChildren(League& league) const {
  std::vector<RankedTeamPtr> teams = league.teams.getAll();
  MutationStats stats;
  TeamLeague newChildren;
  std::discrete_distribution<size_t> mutationChoices{
      cfg_.mutationProbability,
      cfg_.crossoverProbability,
      cfg_.seedProbability,
      doNothingProbability_
  };
  auto randomOtherTeam = [&](size_t iTeam) {
    size_t iOTeam = std::uniform_int_distribution<size_t>{1, teams.size() - 1}(rand_);
    size_t iNOTeam = (iTeam + iOTeam) % teams.size();
    return teams[iNOTeam];
  };

  for (size_t iTeam = 0; iTeam != teams.size(); ++iTeam) {
    const RankedTeamPtr& team = teams[iTeam];
    size_t choice = mutationChoices(rand_);
    switch(choice) {
      case 0: // mutation:
      {
        RankedTeamPtr mutated = std::make_shared<RankedTeam>(
            teamFactory_.mutate(team->nv()),
            league.pokemon);
        if (newChildren.insert({mutated->hash(), mutated}).second) {
          stats.numMutations++;
        }
        break;
      }
      case 1: // crossover:
      {
        const RankedTeamPtr& oTeam = randomOtherTeam(iTeam);
        RankedTeamPtr crossover = std::make_shared<RankedTeam>(
            teamFactory_.crossover(team->nv(), oTeam->nv()),
            league.pokemon);
        if(newChildren.insert({crossover->hash(), crossover}).second) {
          stats.numCrossovers++;
        }
        break;
      }
      case 2: // seed:
      {
        RankedTeamPtr seeded = std::make_shared<RankedTeam>(
            teamFactory_.createRandom(team->nv().getNumTeammates()),
            league.pokemon);
        if (newChildren.insert({seeded->hash(), seeded}).second) {
          stats.numSeeds++;
        }
        break;
      }
      default:
      case 3: // do nothing:
        break;
    };
  }

  if (cfg_.verbosity >= 1) { printMutationStats(stats); }
  return newChildren;
}


size_t Trainer::shrinkPopulations(League& league, const LeagueCount& newChildren) const {
  size_t numRemoved = 0;
  for (size_t iLeague = 0; iLeague < 6; ++iLeague) {
    std::vector<RankedTeamPtr> teams = league.teams.getLeague(iLeague + 1);
    int64_t deltaPopulation = (teams.size() + newChildren[iLeague]) - cfg_.teamPopulationSize[iLeague];
    deltaPopulation = std::min(deltaPopulation, int64_t(teams.size()));
    // if instead of removing teams, we need to add them - don't perform any shrink operation
    if (deltaPopulation <= 0) { continue; }
    
    // select the deltaPopulation worst performing teams and remove them:
    std::partial_sort(
        teams.begin(),
        teams.begin() + deltaPopulation,
        teams.end(),
        [](const auto& a, const auto& b) {
          return a->skill() < b->skill();
    });

    // erase the item:
    for (size_t iTeam = 0, iMax = deltaPopulation; iTeam < iMax; ++iTeam) {
      numRemoved += league.removeTeam(teams[iTeam]->hash());
    }
  }
  return numRemoved;
}


void Trainer::printGenerationStart(const League& league, size_t iGeneration) const {
  out_.get() << boost::format("Begin generation %d of %d! %d battlegroups playing %d heats each.\n")
      % (iGeneration + 1)
      % cfg_.maxGenerations
      % league.battlegroups.size()
      % cfg_.minGamesPerBattlegroup;
}


void Trainer::printMutationStats(const MutationStats& stats) const {
  out_.get() << boost::format("Evolution step mutations=% 5d crossovers=% 5d seeds=% 5d total=%d\n")
      % stats.numMutations
      % stats.numCrossovers
      % stats.numSeeds
      % stats.numTotal();
}
