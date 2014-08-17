#ifndef TRAINER_H
#define TRAINER_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <boost/array.hpp>
#include <vector>
#include <string>

#include "../inc/trueSkill.h"
#include "../inc/ranked.h"

#include "../inc/experienceNet.h"

#include "../inc/ranked_team.h"
#include "../inc/ranked_neuralNet.h"
#include "../inc/ranked_evaluator.h"

class game;
class evaluator;

struct teamTrainerResult
{
	// averages: (rank, mean, stdDev, plies, games, wins, draws)
	boost::array<fpType, 7> averages;
	// stdDevs:
	boost::array<fpType, 7> stdDevs;
	// minimums:
	boost::array<fpType, 7> mins;
	// maximums:
	boost::array<fpType, 7> maxes;
	// highest counts:
	size_t iHighestPokemon, highestPokemonCount;
	size_t iHighestAbility, highestAbilityCount;
	size_t iHighestItem, highestItemCount;
	size_t iHighestType, highestTypeCount;
	size_t iHighestNature, highestNatureCount;
	size_t iHighestMove, highestMoveCount;
};

struct networkTrainerResult
{
	// averages: (rank, mean, stdDev, plies, wins, draws, games)
	boost::array<fpType, 7> averages;
	// stdDevs:
	boost::array<fpType, 7> stdDevs;
	// minimums:
	boost::array<fpType, 7> mins;
	// maximums:
	boost::array<fpType, 7> maxes;
	// meansquared error: (average, stdDev, min, max)
	boost::array<fpType, 4> meanSquaredError;
};

class trainer
{
	/* what do we intend for trainer to do? */
	uint32_t gameType;

	/* trueskill settings used for all rank calculations */
	trueSkillSettings tSettings;

	/* backpropagation and temporal-difference settings used for all neural network training calculations */
	networkSettings_t netSettings;

	/* settings for each network's persistent experience table */
	experienceNetSettings expSettings;

	/* number of generations to complete, maximum */
	size_t maxGenerations;

	/* if a team population is to be loaded to memory from a directory, this is where it is */
	std::string teamPath;

	/* if a network population is to be loaded to memory from a directory, this is where it is */
	std::string networkPath;

	/* if value is nonzero, the number of generations between writeOuts to disk. Otherwise, do not write out */
	size_t writeOutEvery;

	/* minimum amount of time to work on a given league */
	fpType minimumWorkTime;

	/* probability that a pokemon will undergo a mutation */
	fpType mutationProbability;

	/* probability that two pokemon will crossover to create another */
	fpType crossoverProbability;

	/* probability that an entirely new pokemon will spontaneously find its self in the population */
	fpType seedProbability;

	/* probability that an entirely new network will be seeded */
	fpType seedNetworkProbability;

	/* jitters the network after epoch games has been completed */
	size_t jitterEpoch;

	/* number of random rollouts to be performed by monte-carlo simulation */
	size_t numRollouts;

	/* accuracy that planner_stochastic will use. Game accuracy is always the same */
	size_t plannerAccuracy;

	/* probability that planner_stochastic will psuedorandomly explore the search space */
	fpType plannerExploration;

	/* parameter controlling planner_stochastic's psuedorandom search. Values close to 0 are deterministic */
	fpType plannerTemperature;

	/* sizes of the six populations aka "leagues" */
	boost::array<size_t, 6> teamPopulationSize;

	/* size of the network population */
	size_t networkPopulationSize;

	/* the topology of newly created networks */
	std::vector<size_t> networkLayerSize;

	/* do we allow teams to rank against teams of different leagues? Useful for small population */
	bool enforceSameLeague;

	size_t generationsCompleted;

	/* amount of heats performed in each league */
	boost::array<size_t, 6> heatsCompleted;

	/* population of teams to be run on. 0 implies no work will be done on the league */
	boost::array<std::vector<ranked_team>, 6> leagues;

	/* population of networks to be run on. May possibly be 0 */
	std::vector<ranked_neuralNet> networks;

	/* population of evaluators to be run on. If networks is 0, this MUST be greater than 0. */
	std::vector<ranked_evaluator> evaluators;

	/* if performing ranking, trialTeam and/or trialNet are set */
	ranked_team* trialTeam;
	ranked_neuralNet* trialNet;

	/* game instance used for evaluation */
	game* cGame;

	/* select two parents, weighted by fitness */
	boost::array<size_t, 2> selectParent_Roulette(const std::vector<const ranked_team>& cLeague) const;

	/* generate an array of teams which are contained within the current team. Will always return an empty set if the team contains one pokemon */
	void findSubteams(trueSkillTeam& cTeam, size_t iTeam);

	/* generates a random population from previous leagues, or from random functions if at single pokemon league */
	size_t seedRandomTeamPopulation(size_t iLeague, size_t targetSize);

	size_t seedRandomNetworkPopulation(size_t targetSize);

	void spawnTeamChildren(size_t iLeague, size_t& numMutated, size_t& numCrossed, size_t& numSeeded);

	void spawnNetworkChildren(size_t& numMutated, size_t& numCrossed, size_t& numSeeded);

	/* determine the league that should receive the next work cycle, giving precedence to lower leagues */
	size_t determineWorkingLeague() const;

	/* stochastically find a match of ideal skill for a given team. If enforceSameLeague is false, allow matches from nearby leagues */
	trueSkillTeam findMatch( const trueSkillTeam& oTeam );

	/* finds an evaluator, or returns NULL if we are to use the dumb evaluator */
	size_t findEvaluator();

	/* determine if a given ranked_team is already in the population, and if so, return its index. SIZE_MAX if not */
	size_t findInPopulation(size_t iLeague, uint64_t teamHash) const;

	/* return true if findInPopulation returns a value */
	bool isInPopulation(const ranked_team& cRankTeam) const;

	/* determine if a given ranked_neuralNet is already in the network list, and if so, return its index. SIZE_MAX if not */
	size_t findInNetworks(uint64_t teamHash) const;

	/* return true if findInNetworks returns a value */
	bool isInNetworks(const ranked_neuralNet& cRankNet) const;

	/* destroy the elements with the lowest rank from the league */
	void shrinkTeamPopulation(size_t iLeague, size_t targetSize);

	void shrinkNetworkPopulation(size_t targetSize);

	/* calculates interesting things about the given league */
	void calculateDescriptiveStatistics(size_t iLeague, teamTrainerResult& cResult) const;

	void calculateDescriptiveStatistics(networkTrainerResult& cResult) const;

	/* print information about the top n members of league iLeague */
	void printLeagueStatistics(size_t iLeague, size_t numMembers, const teamTrainerResult& cResult) const;

	void printNetworkStatistics(size_t numMembers, const networkTrainerResult& cResult) const;
	
	/*load a population of pokemon and their rankings from a filepath */
	bool loadTeamPopulation();
	bool loadNetworkPopulation();

	bool saveTeamPopulation();
	bool saveNetworkPopulation();
	
public:

	static const boost::array< size_t, 6 > defaultTeamPopulations;

	trainer(
		uint32_t gameType = GT_OTHER_EVOBOTH, 
		size_t _maxPlies = 1560, 
		size_t _maxMatches = 1, 
		size_t _gameAccuracy = 1,
		size_t _engineAccuracy = 1,
		const trueSkillSettings& _tSettings = trueSkillSettings::defaultSettings,
		const experienceNetSettings& _expSettings = experienceNetSettings::defaultSettings,
		size_t _maxGenerations = 12,
		fpType _workTime = 120,
		fpType _mutationProbability = 0.55,
		fpType _crossoverProbability = 0.035,
		fpType _seedProbability = 0.035,
		bool _enforceSameLeague = false,
		fpType _exploration = 0.5,
		fpType _temperature = 1.0,
		size_t networkPopulations = 5,
		const networkSettings_t& _netSettings = networkSettings_t::defaultSettings,
		const std::vector<size_t>& networkLayerSize = std::vector<size_t>(),
		const boost::array<size_t, 6>& teamPopulations = defaultTeamPopulations,
		fpType _seedNetworkProbability = 0.0,
		size_t _jitterEpoch = 2500,
		size_t _numRollouts = 1000,
		size_t _writeoutInterval = 0,
		const std::string& teamPath = std::string(),
		const std::string& networkPath = std::string());
	~trainer();

	void setGauntletTeam(const team_nonvolatile& cTeam);
	void setGauntletNetwork(const neuralNet& cNet);

	bool seedEvaluator(const evaluator& _eval);

	bool seedTeam(const team_nonvolatile& cTeam);
	bool seedNetwork(const neuralNet& cNet);

	/* create all variables, prepare trainer for running */
	bool initialize();

	/* begin evolution process */
	void evolve();
};

#endif /* TRAINER_H */
