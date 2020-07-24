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
class pkIO;
class game;
class trainer;
class pokemon_nonvolatile;
class evaluator;
class planner;

class pokemonAI : public pokedex
{
private:
	friend class pkIO;
	friend class trainer;
	friend class game;
	friend class pkCU;
	friend class pokemon_nonvolatile;

	int isInitialized;

	class pkIO* pokemonIO;
	class game* Game;
	class trainer* Trainer;

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
	boost::array<size_t, 6> teamPopulations;
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

	std::vector<move> moves; // list of all acceptable moves
	std::vector<type> types; // list of all acceptable types
	std::vector<pokemon_base> pokemon; // list of all acceptable pokemon
	std::vector<ability> abilities; // list of all acceptable abilities
	std::vector<nature> natures; // list of all acceptable natures
	std::vector<item> items; // list of all acceptable items
	enginePlugins engineExtensions; // list of engine extensions
	std::vector<team_nonvolatile> teams; // growable list of team_nonvolatile objects
	std::vector<neuralNet> networks; // growable list of neural network evaluation functions
	std::vector<boost::dll::shared_library*> plugins; // growable list of plugins (need to be closed upon exiting program)

public:

	/* methods returns NULL if no planner/evaluator/team was selected */
	void printTeams(bool printTeammates = false) const;
	const team_nonvolatile* teamSelect(char playerID);

	void printEvaluators() const;
	/* returns a NEW evaluator */
	evaluator* evaluatorSelect(char playerID);

	void printPlanners() const;
	/* returns a NEW planner */
	planner* plannerSelect(char playerID);

	std::vector<move>& getMoves() { return moves; };
	std::vector<type>& getTypes() { return types; };
	std::vector<pokemon_base>& getPokemon() { return pokemon; };
	std::vector<ability>& getAbilities() { return abilities; };
	std::vector<nature>& getNatures() { return natures; };
	std::vector<item>& getItems() { return items; };
	enginePlugins& getExtensions() { return engineExtensions; };
	std::vector<boost::dll::shared_library*>& getPlugins() { return plugins; };
	const std::vector<move>& getMoves() const { return moves; };
	const std::vector<type>& getTypes() const { return types; };
	const std::vector<pokemon_base>& getPokemon() const { return pokemon; };
	const std::vector<ability>& getAbilities() const { return abilities; };
	const std::vector<nature>& getNatures() const { return natures; };
	const std::vector<item>& getItems() const { return items; };
	const enginePlugins& getExtensions() const { return engineExtensions; };

	std::vector<team_nonvolatile>& getTeams() { return teams; }; 

	pkIO*& getIO() { return pokemonIO; };
	
	bool init();
	bool run();
	pokemonAI();
	~pokemonAI();
};

#endif	/* POKEMONAI_H */

