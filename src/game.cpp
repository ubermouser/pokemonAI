//#define PKAI_IMPORT

#include <iostream>
#include <boost/foreach.hpp>

#include "../inc/planner.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/fp_compare.h"
#include "../inc/roulette.h"

#include "../inc/pkCU.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/environment_volatile.h"
#include "../inc/environment_possible.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_volatile.h"
#include "../inc/move.h"

#include "../inc/game.h"





game::game(size_t _maxPlies, size_t _maxMatches, size_t _gameAccuracy, bool _rollout)
	: agents(),
	gameLog(),
	gameResults(),
	hResult(),
	cu(NULL),
	cEnvNV(),
	maxPlies(_maxPlies),
	maxMatches(_maxMatches),
	gameAccuracy(_gameAccuracy),
	rollout(_rollout),
	allowStateSelection(false),
	isInitialized(false),
	initialEnvP(environment_possible::create(environment_volatile())),
	cEnvP()
{
	agents.assign(NULL);
}





void game::cleanUp()
{
	isInitialized = false;

	if (agents[TEAM_A] != NULL) { delete agents[TEAM_A]; agents[TEAM_A] = NULL; }
	if (agents[TEAM_B] != NULL) { delete agents[TEAM_B]; agents[TEAM_B] = NULL; }

	if (cu != NULL) { delete cu; cu = NULL; }

	gameLog.clear();
	gameResults.clear();
}





game::~game()
{
	// order in which elements are deleted is important
	for (size_t iAgent = 0; iAgent < 2; iAgent++)
	{
		if (agents[iAgent] != NULL) delete agents[iAgent];
	}

	if (cu != NULL) delete cu;

	// cu and agents reference the nonvolatile environment cEnvNV
}

void game::setEnvironment(const environment_nonvolatile& _envNV)
{
	cEnvNV = _envNV;
	cEnvNV.initialize();
	isInitialized = false;
}

void game::setTeam(size_t iAgent, const team_nonvolatile& cTeam)
{
	assert(iAgent < 2);
	cEnvNV.setTeam(iAgent, cTeam, true);
	isInitialized = false;
}

void game::setEvaluator(size_t iAgent, const evaluator& cEval)
{
	assert(iAgent < 2);
	if (agents[iAgent] == NULL) 
	{ 
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": agent " << iAgent <<
			" has not been initialized, and cannot accept an evaluator argument!\n";
		return; 
	}
	agents[iAgent]->setEvaluator(cEval);
	isInitialized = false;
}

void game::setPlanner(size_t iAgent, const planner& cPlanner)
{
	assert(iAgent < 2);
	if (agents[iAgent] != NULL) { delete agents[iAgent]; }
	agents[iAgent] = cPlanner.clone();
	isInitialized = false;
}

const planner* game::getPlanner(size_t iAgent) const
{
	assert(iAgent < 2);
	return agents[iAgent];
}

void game::setInitialState(const environment_volatile& rolloutState)
{
	if (rollout != true && verbose >= 5) 
	{ 
		std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
			": An initial rollout state was defined, but rollout mode was not enabled. The state will not be used.\n";
	}
	initialEnvP = environment_possible::create(rolloutState, true);
}





bool game::initialize()
{
	// teams must be set before initialize is called
	if (cEnvNV.getTeam(TEAM_A).getNumTeammates() == 0 || cEnvNV.getTeam(TEAM_B).getNumTeammates() == 0) 
	{
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": One or more teams are undefined!\n";
		return false; 
	}

	// agents too:
	if (agents[TEAM_A] == NULL || agents[TEAM_B] == NULL) 
	{ 
		std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
			": One or more agents are undefined!\n";
		return false; 
	}
	
	// number of matches to be played must be sane:
	if ((maxMatches & 1) == 0) { maxMatches += 1; }

	/* initialize cEnvNV: */
	//cEnvNV.initialize(); // initialized in setTeam and setEnvironment

	// generate environment if the initial environment hasn't already been created:
	bool genEnvP = (initialEnvP.getHash() == UINT64_MAX);
	if (!rollout || genEnvP)
	{
		if (rollout && verbose >= 5)
		{
			std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
				": Rollout mode was enabled, but an initial rollout state was not defined.\n";
		}
		initialEnvP = environment_possible::create(environment_volatile::create(cEnvNV), true);
	}

	// initialize pkCU engine
	if (cu == NULL) { cu = new pkCU(cEnvNV, gameAccuracy); }
	else { cu->setEnvironment(cEnvNV); cu->setAccuracy(gameAccuracy); }

	// initialize agents:
	for (size_t iAgent = 0; iAgent < 2; ++iAgent)
	{
		agents[iAgent]->setEnvironment(*cu, iAgent);
		if (!agents[iAgent]->isInitialized()) 
		{
			std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
				": Agent " << iAgent << 
				": \"" << agents[iAgent]->getName() << "\" failed to initialize!\n";
			return false; 
		}
	}

	isInitialized = true;
	return true;
}





void game::run()
{
	assert(isInitialized);

	gameLog.clear();
	gameResults.clear();
	hResult = heatResult();

	// used to determine if a team wins the bestOf by majority instead of the bestOf running to completion
	bool heatOver = false;
	int32_t matchState;
	boost::array<unsigned int, 2> numWins;
	numWins.assign(0);
	
	for (size_t iMatch = 0; iMatch != maxMatches; ++iMatch)
	{
		// initialize match metrics:
		std::vector<turn>* cLog = NULL;
		if (!rollout) 
		{
			gameLog.push_back(std::vector<turn>());
			cLog = &gameLog.back(); 
		}

		// initialize pokemon:
		cEnvP = initialEnvP;

		if (verbose >= 3)
		{
			std::cout << 
				"\nBegin game " << (iMatch+1) << 
				" of " << maxMatches <<
				" between teams TA: " << agents[TEAM_A]->getName() <<
				" - " << cEnvNV.getTeam(TEAM_A).getName() << 
				" and TB: " << agents[TEAM_B]->getName() << 
				" - "<< cEnvNV.getTeam(TEAM_B).getName() << 
				((verbose>=3)?"!\n\n":"!\n");
		}

		// local game variables:
		matchState = MATCH_MIDGAME;
		size_t iPly;
		size_t iLastEnvironment = 0;

		// begin combat:
		for (iPly = 0; iPly < maxPlies && (matchState == MATCH_MIDGAME); ++iPly)
		{

			// determine if the current state is a terminal state, and if so end the game:
			matchState = cu->isGameOver(cEnvP.getEnv());

			// determine which move the teams will use:
			boost::array<uint32_t, 2> actions;
			if (matchState == MATCH_MIDGAME)
			{
				for (size_t iTeam = 0; iTeam != 2; ++iTeam)
				{
					actions[iTeam] = agents[iTeam]->generateSolution(cEnvP);
					if (actions[iTeam] >= AT_ITEM_USE)
					{
						matchState = (iTeam + 1) % 2; break;
					}
				}
			}

			// if both of the teams have valid moves:
			if (matchState == MATCH_MIDGAME)
			{
				if (verbose >= 3)
				{
					for (size_t iTeam = 0; iTeam != 2; ++iTeam)
					{
						printAction(cEnvP.getEnv().getTeam(iTeam), actions[iTeam], iTeam);
					}
				}

				// predict what will occur given these actions and their probabilities
				std::vector<environment_possible> possibleEnvironments;
		
				// determine what will happen and at what probability
				size_t numUnique = cu->updateState(cEnvP.getEnv(), possibleEnvironments, actions[TEAM_A], actions[TEAM_B]);
				assert(numUnique > 0);

				size_t iEnvironment;
				if (allowStateSelection)
				{
					printStates(possibleEnvironments, numUnique);
					iEnvironment = stateSelect_index(possibleEnvironments);
				}
				else
				{
					iEnvironment = stateSelect_Roulette(possibleEnvironments);
				}

				// perform state transition:
				if (iEnvironment != SIZE_MAX) 
				{
					const environment_possible& switchedEnvironment = possibleEnvironments.at(iEnvironment);

					// create a log of this turn:
					if (!rollout) { digestTurn(*cLog, actions[TEAM_A], actions[TEAM_B], iLastEnvironment, cEnvP); }

					// remove a ply if the transition was a dummy move:
					if (switchedEnvironment.hasFreeMove(TEAM_A) || switchedEnvironment.hasFreeMove(TEAM_B))
					{
						iPly--;
					}

					// print the state that occurs:
					if (verbose >= 3)
					{
						if (verbose >= 4) { std::cout << "\n"; }
						printState(switchedEnvironment, iEnvironment, iPly);
						std::cout << "\n";
					}

					// perform the state transition:
					cEnvP = switchedEnvironment;
					iLastEnvironment = iEnvironment;
				}
			} // endOf if matchState isn't terminal
			else if (verbose >= 2 && maxMatches > 1)
			{
				// if a tie occurs: (all pokemon dead)
				if (matchState == MATCH_TIE)
				{
					std::cout << 
						"Teams TA: " << agents[TEAM_A]->getName() <<
						" - " << cEnvNV.getTeam(TEAM_A).getName() << 
						" and TB: " << agents[TEAM_B]->getName() << 
						" - "<< cEnvNV.getTeam(TEAM_B).getName() << 
						" have tied game " << (iMatch+1) << 
						" of " << maxMatches <<
						"!\n";
				}
				else if (matchState == MATCH_DRAW)
				{
					std::cout << 
						"Teams TA: " << agents[TEAM_A]->getName() <<
						" - " << cEnvNV.getTeam(TEAM_A).getName() << 
						" and TB: " << agents[TEAM_B]->getName() << 
						" - "<< cEnvNV.getTeam(TEAM_B).getName() << 
						" have drawn game " << (iMatch+1) << 
						" of " << maxMatches <<
						"!\n";
				}
				else
				{
					size_t losingTeam = (matchState + 1) % 2;
					std::cout << 
						"Team " << "T" << (matchState==TEAM_A?"A":"B") <<
						": " << agents[matchState]->getName() <<
						" - " << cEnvNV.getTeam(matchState).getName() << 
						" has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
						": " << agents[losingTeam]->getName() <<
						" - " << cEnvNV.getTeam(losingTeam).getName() << 
						" in game " << (iMatch+1) << 
						" of " << maxMatches <<
						" - " << (numWins[matchState]+1) <<
						" to " << numWins[losingTeam] <<
						"!\n";
				}
			} // endOf if matchState IS terminal
		} //endOf game loop

		// digest the last game state:
		BOOST_FOREACH(planner* cPlanner, agents)
		{
			cPlanner->clearResults();
		}
		// by clearing these results, we force digestTurn to use only the simple evaluation of this turn (win, loss, tie)
		if (!rollout) 
		{
			// collect statistics about the game:
			digestGame(*cLog, matchState);
			digestTurn(*cLog, AT_MOVE_NOTHING, AT_MOVE_NOTHING, iLastEnvironment, cEnvP); 

			if (verbose >= 2 && maxMatches > 1)
			{
				printGameOutline(gameResults.back(), *cLog, cEnvNV);
			}
		}

		// add a point for the winning team:
		switch (matchState)
		{
		case MATCH_TEAM_A_WINS:
		case MATCH_TEAM_B_WINS:
			numWins[matchState]++;
			break;
		case MATCH_DRAW:
		case MATCH_TIE:
			break;
		}

		// stop the heat if the winning team has won enough wins to secure victory:
		if (std::max(numWins[TEAM_A], numWins[TEAM_B]) > (maxMatches / 2))
		{
			heatOver = true;
		}

		if (heatOver == true)
		{
			break;
		}
	} // endOf foreach match

	// determine winner
	if (numWins[TEAM_A] != numWins[TEAM_B])
	{
		matchState = (numWins[TEAM_A] > numWins[TEAM_B])?MATCH_TEAM_A_WINS:MATCH_TEAM_B_WINS;
		size_t losingTeam = (matchState + 1) % 2;
		if (verbose >= 2)
		{
			std::cout << 
				((verbose>=3)?"\n":"") <<
				"Team " << "T" << (matchState==TEAM_A?"A":"B") <<
				": " << agents[matchState]->getName() <<
				" - " << cEnvNV.getTeam(matchState).getName() << 
				" has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
				": " << agents[losingTeam]->getName() <<
				" - " << cEnvNV.getTeam(losingTeam).getName() << 
				" , winning the bo" << maxMatches <<
				" series " << numWins[matchState] << 
				" to " << numWins[matchState==TEAM_A?TEAM_B:TEAM_A] <<
				((verbose>=3)?"!\n\n":"!\n");
		}
	}
	else if (numWins[TEAM_A] == 0 && numWins[TEAM_B] == 0)
	{
		matchState = MATCH_DRAW;

		if (verbose >= 2)
		{
			std::cout << 
				((verbose>=3)?"\n":"") <<
				"Teams TA: " << agents[TEAM_A]->getName() <<
				" - " << cEnvNV.getTeam(TEAM_A).getName() << 
				" and TB: " << agents[TEAM_B]->getName() << 
				" - "<< cEnvNV.getTeam(TEAM_B).getName() << 
				" have drawn the bo" << maxMatches << 
				" series " << numWins[TEAM_A] << 
				" to " << numWins[TEAM_B] <<
				((verbose>=3)?"!\n\n":"!\n");
		}
	}
	else
	{
		matchState = MATCH_TIE;

		if (verbose >= 2)
		{
			std::cout << 
				((verbose>=3)?"\n":"") <<
				"Teams TA: " << agents[TEAM_A]->getName() <<
				" - " << cEnvNV.getTeam(TEAM_A).getName() << 
				" and TB: " << agents[TEAM_B]->getName() << 
				" - "<< cEnvNV.getTeam(TEAM_B).getName() << 
				" have tied the bo" << maxMatches << 
				" series " << numWins[TEAM_A] << 
				" to " << numWins[TEAM_B] <<
				((verbose>=3)?"!\n\n":"!\n");
		}
	}

	// collect statistics about the heat:
	if (!rollout) 
	{ 
		digestMatch(gameResults, numWins, matchState);
		if (verbose >= 2)
		{
			printMatchOutline(cEnvNV);
		}
	}
	else
	{
		hResult.score[TEAM_A] = numWins[TEAM_A];
		hResult.score[TEAM_B] = numWins[TEAM_B];
	}
} // endOf run





const environment_volatile& game::stateSelect(const std::vector<environment_possible>& possibleEnvironments)
{
	size_t state = stateSelect_index(possibleEnvironments);
	if (state == SIZE_MAX) return cEnvP.getEnv();
	return possibleEnvironments.at(state).getEnv();
}





size_t game::stateSelect_index(const std::vector<environment_possible>& possibleEnvironments)
{
	std::string input = "";
	int32_t state;
	
	do
	{
		std::cout << "Please select the index of the desired state for the player, -1 for a random state, or -2 to go discard these states\n";
		getline(std::cin, input);
		std::stringstream inputResult(input);
		
		// determine if state is valid:
		
		if (!(inputResult >> state) || 
			!(state < (int32_t) possibleEnvironments.size() && state >= -2))
		{
			std::cout << "Invalid state \"" << input << "\"!\n";
			
			continue;
		}

		if ((state >= 0 && state < (int32_t) possibleEnvironments.size()) && possibleEnvironments.at(state).isPruned())
		{
			std::cout << "State " << input << " was pruned!\n";
			continue;
		}
		
		break;
	}while(true);
	
	if (state == -2)
	{
		return SIZE_MAX;
	}
	
	if (state == -1)
	{
		// choose random state
		state = (int32_t) stateSelect_Roulette(possibleEnvironments);
		
		std::cout << "Randomly chose state " << state << "\n";
	}
	
	// else
	return (size_t) state;
} // endOf stateSelect_index





class sortByProbability
{
public:
	static fpType getValue (const environment_possible& cEnvP)
	{
		if (cEnvP.isPruned()) { return std::numeric_limits<fpType>::quiet_NaN(); }
		return cEnvP.getProbability().to_double();
	};
};

size_t game::stateSelect_Roulette(const std::vector<environment_possible>& possibleEnvironments)
{
	size_t result = roulette<environment_possible, sortByProbability>::select(possibleEnvironments, sortByProbability());

	assert((result != SIZE_MAX) && "roulette selection did not produce a reachable state!");

	return result;
};





void game::digestTurn(
	std::vector<turn>& cLog, 
	unsigned int actionTeamA, 
	unsigned int actionTeamB, 
	size_t resultingState, 
	const environment_possible& envP)
{
	// do not create a digest of a dummy move:
	//if (envP.hasFreeMove(TEAM_A) || envP.hasFreeMove(TEAM_B)) { return; }
	
	cLog.push_back(turn());
	turn& cTurn = cLog.back();

	for (size_t iTeam = 0; iTeam < 2; iTeam++)
	{
		// set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
		fpType simpleFitness = evaluator_simple::calculateFitness(cEnvNV, cEnvP.getEnv(), iTeam);
		fpType initialFitness, finalFitness;
		if (agents[iTeam] != NULL)
		{
			const std::vector<plannerResult>& results = agents[iTeam]->getDetailedResults();
			// if the agent for this team has been initialized, grab its collected fitnesses:
			if (results.empty()) { initialFitness = simpleFitness; finalFitness = simpleFitness; }
			else
			{
				const plannerResult& iResult = results.front();
				const plannerResult& fResult = results.back();

				initialFitness = (iResult.lbFitness + iResult.ubFitness) / 2.0;
				finalFitness = (fResult.lbFitness + fResult.ubFitness) / 2.0;
			}
		}
		else
		{
			// or just use the simple evaluation function if no agent was created
			initialFitness = simpleFitness;
			finalFitness = simpleFitness;
		}

		cTurn.activePokemon[iTeam] = (uint32_t) envP.getEnv().getTeam(iTeam).getICPKV();
		cTurn.simpleFitness[iTeam] = simpleFitness;
		cTurn.depth0Fitness[iTeam] = initialFitness;
		cTurn.depthMaxFitness[iTeam] = finalFitness;
	} // endOf foreach team

	cTurn.action[TEAM_A] = actionTeamA;
	cTurn.action[TEAM_B] = actionTeamB;
	cTurn.stateSelected = (uint32_t) resultingState;
	cTurn.probability = envP.getProbability().to_double();
	cTurn.env = envP.getEnv();
} // endOf digestTurn





void game::digestGame(const std::vector<turn>& cLog, int endStatus)
{
	gameResults.push_back(gameResult());
	gameResult& cResult = gameResults.back();

	// initialize collected data:
	boost::array<boost::array<boost::array<uint32_t, 5>, 6>, 2>& moveUse = cResult.moveUse;
	boost::array<boost::array<uint32_t, 6>, 2>& participation = cResult.participation;
	boost::array<boost::array<fpType, 6>, 2>& aggregateContribution = cResult.aggregateContribution;
	boost::array<boost::array<fpType, 6>, 2>& simpleContribution = cResult.simpleContribution;
	boost::array<boost::array<fpType, 6>, 2>& d0Contribution = cResult.d0Contribution;
	boost::array<boost::array<fpType, 6>, 2>& dMaxContribution = cResult.dMaxContribution;
	boost::array<boost::array<uint32_t, 6>, 2>& ranking = cResult.ranking;
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		for (size_t iTeammate = 0; iTeammate < 6; ++iTeammate)
		{
			moveUse[iTeam][iTeammate].assign(0);
		}
		
		participation[iTeam].assign(0);
		aggregateContribution[iTeam].assign(std::numeric_limits<fpType>::quiet_NaN());
		simpleContribution[iTeam].assign(0);
		d0Contribution[iTeam].assign(0);
		dMaxContribution[iTeam].assign(0);
		ranking[iTeam].assign(7);
	}

	// initialize prevFitnesses
	boost::array<fpType, 2> prevSimpleFitness;
	boost::array<fpType, 2> prev0Fitness;
	boost::array<fpType, 2> prevMaxFitness;
	{ // first turn:
		const turn& fTurn = cLog[0];
		for (size_t iTeam = 0; iTeam < 2; ++iTeam)
		{
			// set previous values to fitnesses of the first ply:
			prevSimpleFitness[iTeam] = fTurn.simpleFitness[iTeam];
			prev0Fitness[iTeam] = fTurn.depth0Fitness[iTeam];
			prevMaxFitness[iTeam] = fTurn.depthMaxFitness[iTeam];
	
			// add participation for first moving pokemon: (or lead twice)
			participation[iTeam][fTurn.activePokemon[iTeam]] += 1;
			// add a move increment for the first moving pokemon:
			if (pkCU::isMoveAction(fTurn.action[iTeam]))
			{
				moveUse[iTeam][fTurn.activePokemon[iTeam]][ fTurn.action[iTeam] - AT_MOVE_0 ] += 1;
			}
		}
	}

	// all turns after the first:
	for (size_t iPly = 1; iPly < cLog.size(); ++iPly)
	{
		// the previous turn. The previous turn was responsible for the delta between turn n-1 and n
		const turn& bTurn = cLog[iPly -1];
		// the current turn. Used for updating delta
		const turn& cTurn = cLog[iPly];

		for (size_t iTeam = 0; iTeam < 2; ++iTeam)
		{
			// create deltas
			fpType dSimpleFitness = cTurn.simpleFitness[iTeam] - prevSimpleFitness[iTeam];
			fpType d0Fitness = cTurn.depth0Fitness[iTeam] - prev0Fitness[iTeam];
			fpType dMaxFitness = cTurn.depthMaxFitness[iTeam] - prevMaxFitness[iTeam];

			// for every turn a pokemon is in play, this increases a counter for that pokemon by 1
			participation[iTeam][cTurn.activePokemon[iTeam]]+= 1;
			
			// add a move increment for the current pokemon's move
			if (pkCU::isMoveAction(cTurn.action[iTeam]))
			{
				moveUse[iTeam][cTurn.activePokemon[iTeam]][cTurn.action[iTeam] - AT_MOVE_0] += 1;
			}

			// increase contribution fractionals:
			simpleContribution[iTeam][ bTurn.activePokemon[iTeam] ] += dSimpleFitness;
			d0Contribution[iTeam][ bTurn.activePokemon[iTeam] ] += d0Fitness;
			dMaxContribution[iTeam][ bTurn.activePokemon[iTeam] ] += dMaxFitness;

			// increment previous values
			prevSimpleFitness[iTeam] = cTurn.simpleFitness[iTeam];
			prev0Fitness[iTeam] = cTurn.depth0Fitness[iTeam];
			prev0Fitness[iTeam] = cTurn.depthMaxFitness[iTeam];
		}
	}

	// create scores:
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		for (size_t iPokemon = 0; iPokemon < cEnvNV.getTeam(iTeam).getNumTeammates(); ++iPokemon)
		{
			aggregateContribution[iTeam][iPokemon] =
				(
					simpleContribution[iTeam][iPokemon] * 0.35 
					+
					d0Contribution[iTeam][iPokemon] * 0.05 
					+
					dMaxContribution[iTeam][iPokemon] * 0.6
				) 
				*
				(fpType)((fpType)participation[iTeam][iPokemon] / (fpType)(cLog.size()));
		}
	}

	// create ranking:
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		boost::array<bool, 6> rankedPokemon;
		rankedPokemon.assign(false);
		for (size_t iRank = 0; iRank < cEnvNV.getTeam(iTeam).getNumTeammates(); ++iRank)
		{
			fpType currentBestA = -std::numeric_limits<fpType>::infinity();
			fpType currentBestS = -std::numeric_limits<fpType>::infinity();
			size_t iCurrentBest = SIZE_MAX;

			for (size_t iPokemon = 0; iPokemon < cEnvNV.getTeam(iTeam).getNumTeammates(); ++iPokemon)
			{
				// don't compare if already ranked:
				if (rankedPokemon[iPokemon] == true) { continue; }

				// if score is greater than; or if score is equal, if simple score is greater than:
				if (mostlyGT(aggregateContribution[iTeam][iPokemon], currentBestA) || 
					(mostlyGTE(aggregateContribution[iTeam][iPokemon], currentBestA) &&
					mostlyGTE(simpleContribution[iTeam][iPokemon], currentBestS)))
				{
					currentBestA = aggregateContribution[iTeam][iPokemon];
					currentBestS = simpleContribution[iTeam][iPokemon];
					iCurrentBest = iPokemon;
				}
			} // endOf inner pokemon

			// no more ranked pokemon available
			if (iCurrentBest == SIZE_MAX) { break; }

			ranking[iTeam][iCurrentBest] = (uint32_t) iRank;
			rankedPokemon[iCurrentBest] = true;

		} // endOf foreach rank
	} // endOf foreach team

	// set game status:
	cResult.endStatus = endStatus;
	cResult.numPlies = (uint32_t) cLog.size();
} // endOf digestGame





void game::digestMatch(const std::vector<gameResult>& gLog, const boost::array<unsigned int, 2>& numWins, int matchResult)
{
	// initialize heatResult:
	boost::array<boost::array<fpType, 6>, 2>& participation = hResult.participation;
	boost::array<boost::array<fpType, 6>, 2>& aggregateContribution = hResult.aggregateContribution;
	boost::array<boost::array<fpType, 6>, 2> avgRanking;
	boost::array<boost::array<uint32_t, 6>, 2>& ranking = hResult.ranking;
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		avgRanking[iTeam].assign(0);
		participation[iTeam].assign(0);
		aggregateContribution[iTeam].assign(0);
		ranking[iTeam].assign(7);
	}

	// generate average numPlies:
	hResult.numPlies = 0;
	for (size_t iLog = 0; iLog != gLog.size(); ++iLog)
	{
		const gameResult& cLog = gLog[iLog];

		hResult.numPlies += (fpType) cLog.numPlies;
	}
	hResult.numPlies /= gLog.size();

	// generate average values:
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		// score:
		hResult.score[iTeam] = numWins[iTeam];

		for (size_t iPokemon = 0; iPokemon < 6; ++iPokemon)
		{
			for (size_t iLog = 0; iLog != gLog.size(); ++iLog)
			{
				const gameResult& cLog = gLog[iLog];

				participation[iTeam][iPokemon] += (fpType) ((fpType)cLog.participation[iTeam][iPokemon] / (fpType)cLog.numPlies);
				avgRanking[iTeam][iPokemon] += (fpType)cLog.ranking[iTeam][iPokemon];
				aggregateContribution[iTeam][iPokemon] += cLog.aggregateContribution[iTeam][iPokemon];

			} // endOf forEach log

			participation[iTeam][iPokemon] /= gLog.size();
			avgRanking[iTeam][iPokemon] /= gLog.size();
			aggregateContribution[iTeam][iPokemon] /= gLog.size();
		} // endOf forEach pokemon
	} // endOf forEach team 

	// create aggregate ranking:
	for (size_t iTeam = 0; iTeam < 2; ++iTeam)
	{
		boost::array<bool, 6> rankedPokemon;
		rankedPokemon.assign(false);
		for (size_t iRank = 0; iRank < cEnvNV.getTeam(iTeam).getNumTeammates(); ++iRank)
		{
			fpType currentBestR = std::numeric_limits<fpType>::infinity();
			fpType currentBestA = -std::numeric_limits<fpType>::infinity();
			size_t iCurrentBest = SIZE_MAX;

			for (size_t iPokemon = 0; iPokemon < cEnvNV.getTeam(iTeam).getNumTeammates(); ++iPokemon)
			{
				// don't compare if already ranked:
				if (rankedPokemon[iPokemon] == true) { continue; }

				if (mostlyLT(avgRanking[iTeam][iPokemon], currentBestR) || 
					(mostlyLTE(avgRanking[iTeam][iPokemon], currentBestR) &&
					mostlyGTE(aggregateContribution[iTeam][iPokemon], currentBestA)))
				{
					currentBestR = avgRanking[iTeam][iPokemon];
					currentBestA = aggregateContribution[iTeam][iPokemon];
					iCurrentBest = iPokemon;
				}
			} // endOf inner pokemon

			// no more ranked pokemon available
			if (iCurrentBest == SIZE_MAX) { break; }

			ranking[iTeam][iCurrentBest] = (uint32_t) iRank;
			rankedPokemon[iCurrentBest] = true;

		} // endOf foreach rank
	} // endOf foreach team

	// variables that do not require loops:
	hResult.endStatus = matchResult;
	hResult.matchesPlayed = gLog.size();
	hResult.matchesTotal = maxMatches;

} // endOf digestMatch





const heatResult& game::getResult() const
{
	assert(rollout || !gameResults.empty());
	return hResult;
};

const std::vector<gameResult>& game::getGameResults() const
{
	assert(!gameResults.empty());
	return gameResults;
};

const std::vector<turn>& game::getGameLog(size_t iGame) const
{
	assert(!gameLog.empty() && (iGame < gameLog.size()));
	return gameLog[iGame];
};

const environment_nonvolatile& game::getEnvNV() const
{
	return cEnvNV;
};





void game::printStates(const std::vector<environment_possible>& possibleEnvironments, size_t numUnique)
{
	std::cout << numUnique << "(" << possibleEnvironments.size() << ") possible states!\n";
	for (size_t iState = 0; iState < possibleEnvironments.size(); iState++)
	{
		if (possibleEnvironments.at(iState).isPruned()) { continue; } // don't display pruned states

		printState(possibleEnvironments.at(iState), iState);
	}

	std::cout << "\n";
}





void game::printState(const environment_possible& possible, size_t iState, size_t iPly)
{
	// print ply index if we have a valid one:
	if (iPly != SIZE_MAX) { std::cout << "ply " << iPly << ", "; }
	// print state and probability:
	std::cout << 
		"s=" << iState << 
		", p=" << possible.getProbability().to_double();
	// print status tokens:
	for (unsigned int iTeam = 0; iTeam < 2; iTeam++)
	{
		if (possible.hasFreeMove(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Free";
		}
		if (possible.hasSwitched(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Switch";
			continue;
		}
		if (possible.hasWaited(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Wait";
			continue;
		}
		if (!possible.hasHit(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Miss";
		}
		if (possible.hasSecondary(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Status";
		}
		if (possible.hasCrit(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Crit";
		}
		if (possible.wasBlocked(iTeam))
		{
			std::cout << " " << (iTeam==TEAM_A?"A":"B") << "-Blocked";
		}
	} // endof foreach team

	if (possible.isMerged())
	{
		std::cout << " (MERGED)";
	}

	if (possible.isPruned())
	{
		std::cout << " (PRUNED)";
	}

	// print active pokemon:
	std::cout << "\n";
	std::cout << "\ttA: " << pokemon_print(
		cEnvNV.getTeam(0).getPKNV(possible.getEnv().getTeam(0)), 
		possible.getEnv().getTeam(0),
		possible.getEnv().getTeam(0).getPKV());
	std::cout << "\ttB: " << pokemon_print(
		cEnvNV.getTeam(1).getPKNV(possible.getEnv().getTeam(1)), 
		possible.getEnv().getTeam(1),
		possible.getEnv().getTeam(1).getPKV());
}





void game::printTeam(const team_volatile& currentTeam, unsigned int iTeam, unsigned int verbosity)
{
	const team_nonvolatile& cTeam = cEnvNV.getTeam(iTeam);
	for (unsigned int indexPokemon = 0; indexPokemon < cTeam.getNumTeammates(); indexPokemon++)
	{
		const pokemon_volatile& currentPokemon = currentTeam.teammate(indexPokemon);
		
		// print out index of pokemon
		if (verbosity >= 1) { std::cout << indexPokemon << "-"; }
		
		std::cout << pokemon_print(cTeam.getPKNV(currentTeam), currentTeam, currentPokemon);
	}
} // end of printPokemon





void game::printAction(const team_volatile& currentTeam, unsigned int indexAction, unsigned int iTeam)
{
	const team_nonvolatile& cTeam = cEnvNV.getTeam(iTeam);
	if (indexAction >= AT_MOVE_0 && indexAction <= AT_MOVE_3)
	{
		std::clog 
			<< "T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << " - "
			<< currentTeam.getICPKV() << ": "
			<< cTeam.getPKNV(currentTeam).getName() << " used " 
			<< (indexAction - AT_MOVE_0) << "-" 
			<< move_print(cTeam.getPKNV(currentTeam).getMove_base(indexAction), currentTeam.getPKV().getMV(indexAction))
			//<< cTeam.getCurrentPokemon(currentTeam).getMove_base(indexAction).getName() 
			<< "!\n";
	}
	else if (indexAction >= AT_SWITCH_0 && indexAction <= AT_SWITCH_5)
	{
		std::clog 
			<< "T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << " - "
			<< currentTeam.getICPKV() << ": "
			<< cTeam.getPKNV(currentTeam).getName() << " is switching out with " 
			<< (indexAction - AT_SWITCH_0) << ": "
			<< cTeam.teammate(indexAction - AT_SWITCH_0).getName() << "!\n";
	}
	else if (indexAction == AT_MOVE_NOTHING)
	{
		std::clog 
			<< "T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << " - "
			<< currentTeam.getICPKV() << ": "
			<< cTeam.getPKNV(currentTeam).getName() << " waited for a turn!\n";
	}
	else if (indexAction == AT_MOVE_STRUGGLE)
	{
		std::clog 
			<< "T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << " - "
			<< currentTeam.getICPKV() << ": "
			<< cTeam.getPKNV(currentTeam).getName() << " used X-" 
			<< move_print(cTeam.getPKNV(currentTeam).getMove_base(AT_MOVE_STRUGGLE), currentTeam.getPKV().getMV(AT_MOVE_STRUGGLE))
			//<< cTeam.getCurrentPokemon(currentTeam).getMove_base(indexAction).getName() 
			<< "!\n";
	}
	else
	{
		std::clog 
			<< "T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << " - "
			<< currentTeam.getICPKV() << ": "
			<< cTeam.getPKNV(currentTeam).getName() << " chose unknown action " 
			<< indexAction << "!\n";
	}
}





void game::printGameOutline(const gameResult& gResult, const std::vector<turn>& gLog, const environment_nonvolatile& cEnv)
{
	std::clog 
		<< "--- GAME STATISTICS ---\n "
		<< gResult.numPlies << " plies total\n"
		" Leaderboard: (index: name - (rank) aScore sScore participation)\n";

	for (size_t iTeam = 0; iTeam < 2; iTeam++)
	{
		const team_nonvolatile& cTeam = cEnv.getTeam(iTeam);
		std::clog 
			<< "  T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << ":"
			<< (((int)iTeam==gResult.endStatus)?" (winner)":"")
			<< "\n";
		for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon)
		{
			std::clog 
				<< "    "
				<< iPokemon << ": "
				<< cTeam.teammate(iPokemon).getName() << " - ("
				<< (gResult.ranking[iTeam][iPokemon] + 1) << ") "
				<< gResult.aggregateContribution[iTeam][iPokemon] << " "
				<< gResult.simpleContribution[iTeam][iPokemon] << " "
				<< ((fpType)gResult.participation[iTeam][iPokemon] / (fpType)gResult.numPlies )
				<< "\n";

		}
	}
}





void game::printMatchOutline(const environment_nonvolatile& cEnv)
{
	std::clog 
		<< "--- MATCH STATISTICS ---\n "
		<< hResult.matchesPlayed << " out of " << hResult.matchesTotal << " games played\n "
		<< "final score: " << hResult.score[0] << " to " << hResult.score[1] << "\n "
		<< hResult.numPlies << " average plies per game\n"
		" Leaderboard: (index: name - (rank) avG-score avG-participation)\n";

	for (size_t iTeam = 0; iTeam < 2; iTeam++)
	{
		const team_nonvolatile& cTeam = cEnv.getTeam(iTeam);
		std::clog 
			<< "  T" << (iTeam==TEAM_A?"A":"B") <<": " 
			<< cTeam.getName() << ":"
			<< (((int)iTeam==hResult.endStatus)?" (winner)":"")
			<< "\n";
		for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon)
		{
			std::clog
				<< "    "
				<< iPokemon << ": "
				<< cTeam.teammate(iPokemon).getName() << " - ("
				<< (hResult.ranking[iTeam][iPokemon] + 1) << ") "
				<< hResult.aggregateContribution[iTeam][iPokemon] << " "
				<< hResult.participation[iTeam][iPokemon]
				<< "\n";

		}
	}
}
