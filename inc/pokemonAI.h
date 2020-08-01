#ifndef POKEMONAI_H
#define	POKEMONAI_H

#include "../inc/pkai.h"

#include <vector>
#include <stdint.h>

// class defines:
#include "../inc/pluggable.h"
#include "../inc/type.h"
#include "../inc/ability.h"
#include "../inc/nature.h"
#include "../inc/item.h"
#include "../inc/move.h"
#include "../inc/pokemon_base.h"
#include "../inc/pokedex.h"
#include "../inc/team_volatile.h"
#include "../inc/trueSkill.h"
#include "../inc/neuralNet.h"
#include "../inc/temporalpropNet.h"
#include "../inc/experienceNet.h"

namespace boost { namespace dll { class shared_library; } }
class PkIO;
class Game;
class Trainer;
class PokemonNonVolatile;
class Evaluator;
class Planner;

class PokemonAI : public Pokedex
{
private:
  friend class PkIO;
  friend class Trainer;
  friend class Game;
  friend class PkCU;
  friend class PokemonNonVolatile;

  int isInitialized;

  class PkIO* pokemonIO;
  class Game* game;
  class Trainer* trainer;

  // invocation:
  /* see pkai.h for a list of permissable values for gameType enum */
  uint32_t gameType;
  
  // search variables:
  size_t numThreads;
  fpType secondsPerMove;
  size_t maxSearchDepth;
  size_t transpositionTableBins;
  size_t transpositionTableBinSize;

  // genetic algorithm variables:
  fpType crossoverProbability;
  fpType mutationProbability;
  fpType seedProbability;
  fpType minimumWorkTime;
  size_t writeOutInterval;
  size_t maxGenerations;
  // team trainer variables:
  std::array<size_t, 6> teamPopulations;
  std::string teamDirectory;
  bool seedTeams;
  bool enforceSameLeague;
  // network trainer variables:
  size_t networkPopulation;
  std::vector<size_t> networkLayers;
  std::string networkDirectory;
  size_t jitterEpoch;
  fpType seedNetworkProbability;
  bool seedNetworks;
  bool seedDumbEvaluator;
  bool seedRandomEvaluator;
  // stochastic planner variables:
  fpType plannerTemperature;
  fpType plannerExploration;
  // directed planner variables:
  experienceNetSettings expSettings;
  // backpropagation and TD variables:
  temporalpropSettings netSettings;
  size_t numRollouts;

  // trueskill variables:
  trueSkillSettings tSettings;

  // engine variables:
  size_t engineAccuracy;
  size_t gameAccuracy;

  // game variables:
  size_t maxPlies;
  size_t maxMatches;

  std::vector<Move> moves; // list of all acceptable moves
  std::vector<Type> types; // list of all acceptable types
  std::vector<PokemonBase> pokemon; // list of all acceptable pokemon
  std::vector<Ability> abilities; // list of all acceptable abilities
  std::vector<Nature> natures; // list of all acceptable natures
  std::vector<Item> items; // list of all acceptable items
  EnginePlugins engineExtensions; // list of engine extensions
  std::vector<TeamNonVolatile> teams; // growable list of team_nonvolatile objects
  std::vector<neuralNet> networks; // growable list of neural network evaluation functions
  std::vector<boost::dll::shared_library*> plugins; // growable list of plugins (need to be closed upon exiting program)

public:

  /* methods returns NULL if no planner/evaluator/team was selected */
  void printTeams(bool printTeammates = false) const;
  const TeamNonVolatile* teamSelect(char playerID);

  void printEvaluators() const;
  /* returns a NEW evaluator */
  Evaluator* evaluatorSelect(char playerID);

  void printPlanners() const;
  /* returns a NEW planner */
  Planner* plannerSelect(char playerID);

  std::vector<Move>& getMoves() { return moves; };
  std::vector<Type>& getTypes() { return types; };
  std::vector<PokemonBase>& getPokemon() { return pokemon; };
  std::vector<Ability>& getAbilities() { return abilities; };
  std::vector<Nature>& getNatures() { return natures; };
  std::vector<Item>& getItems() { return items; };
  EnginePlugins& getExtensions() { return engineExtensions; };
  std::vector<boost::dll::shared_library*>& getPlugins() { return plugins; };
  const std::vector<Move>& getMoves() const { return moves; };
  const std::vector<Type>& getTypes() const { return types; };
  const std::vector<PokemonBase>& getPokemon() const { return pokemon; };
  const std::vector<Ability>& getAbilities() const { return abilities; };
  const std::vector<Nature>& getNatures() const { return natures; };
  const std::vector<Item>& getItems() const { return items; };
  const EnginePlugins& getExtensions() const { return engineExtensions; };

  std::vector<TeamNonVolatile>& getTeams() { return teams; }; 

  PkIO*& getIO() { return pokemonIO; };
  
  bool init();
  bool run();
  PokemonAI();
  ~PokemonAI();
};

#endif	/* POKEMONAI_H */

