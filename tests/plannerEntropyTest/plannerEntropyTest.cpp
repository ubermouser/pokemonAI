//#define PKAI_IMPORT
#include "../../inc/pkai.h"

#include <fstream>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <boost/scoped_ptr.hpp>
#include <boost/foreach.hpp>

#include "../../inc/team_nonvolatile.h"
#include "../../inc/environment_nonvolatile.h"
#include "../../inc/environment_volatile.h"
#include "../../inc/environment_possible.h"

#include "../../inc/pokedex.h"
#include "../../inc/plugin.h"
#include "../../inc/pkCU.h"

#include "../../inc/ranked_team.h"

#include "../../inc/planner_max.h"
#include "../../inc/planner_stochastic.h"
#include "../../inc/planner_directed.h"
#include "../../inc/planner_minimax.h"
#include "../../inc/planner_human.h"
#include "../../inc/planner_random.h"
#include "../../inc/evaluator_simple.h"
#include "../../inc/evaluator_featureVector.h"
#include "../../inc/experienceNet.h"

#include "../../inc/init_toolbox.h"
#include "../../inc/orphan.h"
#include "../../inc/roulette.h"

#include "../../inc/game.h"

class tinyAI : public pokedex
{
	std::vector<move> moves; // list of all acceptable moves
	std::vector<type> types; // list of all acceptable types
	std::vector<pokemon_base> pokemon; // list of all acceptable pokemon
	std::vector<ability> abilities; // list of all acceptable abilities
	std::vector<nature> natures; // list of all acceptable natures
	std::vector<item> items; // list of all acceptable items
	enginePlugins engineExtensions; // list of engine extensions
	std::vector<boost::extensions::shared_library*> plugins; // growable list of plugins (need to be closed upon exiting program)

public:

	std::vector<move>& getMoves() { return moves; };
	std::vector<type>& getTypes() { return types; };
	std::vector<pokemon_base>& getPokemon() { return pokemon; };
	std::vector<ability>& getAbilities() { return abilities; };
	std::vector<nature>& getNatures() { return natures; };
	std::vector<item>& getItems() { return items; };
	enginePlugins& getExtensions() { return engineExtensions; };
	std::vector<boost::extensions::shared_library*>& getPlugins() { return plugins; };
	const std::vector<move>& getMoves() const { return moves; };
	const std::vector<type>& getTypes() const { return types; };
	const std::vector<pokemon_base>& getPokemon() const { return pokemon; };
	const std::vector<ability>& getAbilities() const { return abilities; };
	const std::vector<nature>& getNatures() const { return natures; };
	const std::vector<item>& getItems() const { return items; };
	const enginePlugins& getExtensions() const { return engineExtensions; };
};

float findEntropies(const game& cGame)
{
	boost::scoped_ptr<evaluator_featureVector> cEval((evaluator_featureVector*)cGame.getPlanner(TEAM_A)->getEvaluator()->clone());
	experienceNet cExperience(*cEval, experienceNetSettings(100, 1.0 / 64.0, 1.0, EXPERIENCENET_HISTOGRAM));

	cEval->resetEvaluator(cGame.getEnvNV());
	std::vector<float> buffer(cEval->inputSize(), 0.0f);

	{
		if (verbose >= 2) { std::cout << "generating stats for evaluator... \n"; }
		for (size_t iGame = 0; iGame != cGame.getGameResults().size(); ++iGame)
		{
			BOOST_FOREACH(const turn& cTurn, cGame.getGameLog(iGame))
			{
				cEval->seed(buffer.data(), cTurn.env, TEAM_A);
				cExperience.addExperience(buffer.data());
			}
		}
	}
	float averageEntropy = 0.0;
	{
		float cEntropy = cExperience.entropy();
		averageEntropy += cEntropy;
		if (verbose >= 1) 
		{
			std::cout <<
				"stats of " << cEval->getName() << ": "
				<< "\n\tmax     : " << cExperience.maximum()
				<< "\n\tmin     : " << cExperience.minimum()
				<< "\n\tentropy : " << cEntropy
				<< "\n";
		}
	}
	//averageEntropy /= 4.0f;
	if (verbose >= 1) {std::cout << "average entropy: " << averageEntropy << "\n"; }
	return averageEntropy;
};

int main()
{
	verbose = 5;
	warning = 2;
	size_t numGames = 100;
	size_t pkCUAccuracy = 1;
	srand((unsigned int)time(NULL));
	tinyAI* _pkdex = new tinyAI();
	pkdex = _pkdex;
	evaluator_featureVector::initStatic();

	// pokedex input:
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_types.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); }
		if (!_pkdex->inputTypes(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_natures.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputNatures(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_abilities.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputAbilities(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_items.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputItems(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_moves.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputMoves(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_pokemon.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputPokemon(lines, iLine)) { return false; } }
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("data/gen4_movelist.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		if (!_pkdex->inputMovelist(lines, iLine)) { return false; } }
	//{   if (!_pkdex->inputPlugins("plugins")) { return false; } }
	{   if (!_pkdex->registerPlugin(&registerExtensions)) { return false; } }

	move::move_struggle = orphan::orphanCheck_ptr(_pkdex->getMoves(), NULL, "struggle");
	if (move::move_struggle != NULL) { move_nonvolatile::mNV_struggle = new move_nonvolatile(*move::move_struggle); }

	std::vector<team_nonvolatile> teams;

	neuralNet aNet;
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("networks/mc_128_48_16_1.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		aNet.input(lines, iLine); }
	neuralNet bNet;
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("networks/td_16_48_16_1.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		bNet.input(lines, iLine); }
	neuralNet dNet;
	{   std::vector<std::string> lines; size_t iLine = 0; 
		{ std::string inputBuffer; INI::loadFileToString("networks/tt_32_96_32_1.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
		dNet.input(lines, iLine); }

	verbose = 0;

	game cGame(MAXPLIES, 1, pkCUAccuracy);

	teams.reserve(numGames);
	for (size_t iTeam = 0; iTeam != numGames; ++iTeam) { teams.push_back(ranked_team::createRandom(6)); }

	cGame.setPlanner(TEAM_B, planner_max());
	cGame.setEvaluator(TEAM_B, evaluator_simple());

	std::cout << 
		"pkCUAccuracy= " << pkCUAccuracy <<
		" numGames= " << std::setw(3) << numGames <<
		"\n";
	for (size_t iType = 0; iType != 2; ++iType)
	{
		experienceNetType_t cType = iType==0?EXPERIENCENET_RECENCY:EXPERIENCENET_HISTOGRAM;
		for (size_t iNumTaps = 8; iNumTaps < 65; iNumTaps+=8)
		{
			for (size_t iExtrapolation = 4; iExtrapolation < 1025; iExtrapolation*=2)
			{
				float extrapolation = 1.0f / (float)iExtrapolation;
				for (size_t iDecay = 750; iDecay < 1001; iDecay += 25)
				{
					float decay = (float)(iDecay) / 1000.0f;
					for (size_t iBias = 50; iBias < 101; iBias += 5)
					{
						float bias = (float)iBias / 100.0f;


						cGame.setPlanner(TEAM_A, 
							planner_directed(
							experienceNet(1, experienceNetSettings(iNumTaps, extrapolation, decay, cType)), 1, bias));
						cGame.setEvaluator(TEAM_A, *boost::scoped_ptr<evaluator_featureVector>(evaluator_featureVector::getEvaluator(dNet)));

						float cEntropy = 0;
						for (size_t iGame = 0; iGame != numGames; ++iGame)
						{
							cGame.setTeam(TEAM_A, teams[iGame]);
							cGame.setTeam(TEAM_B, teams[(iGame+1)%numGames]);

							if (cGame.initialize()) { cGame.run(); }
							cEntropy += findEntropies(cGame);
						}
						cEntropy /= numGames;
						std::cout << 
							"type= " << (iType==1?"HISTOGRAM":"RECENCY  ") <<
							" numTaps= " << std::setw(4) << iNumTaps << 
							" extrapD= " << std::setw(4) << iExtrapolation <<
							" decay= " << std::setw(4) << decay <<
							" bias= " << std::setw(4) << bias <<
							" entropd= " << std::setw(12) << cEntropy <<
							"\n";
					}
				}
			}
		}
	}

	/*for (size_t iExploration = 0; iExploration < 101; iExploration += 5)
	{
		float exploration = (float)std::max(iExploration,(size_t)1U) / 100.0f;
		for (size_t iTemperature = 0; iTemperature < 1001; iTemperature += 5)
		{
			float temperature = (float)std::max(iTemperature,(size_t)1U) / 100.0f;

			cGame.setPlanner(TEAM_A, 
				planner_stochastic(1, temperature, exploration));
			cGame.setEvaluator(TEAM_A, *boost::scoped_ptr<evaluator_featureVector>(evaluator_featureVector::getEvaluator(dNet)));

			float cEntropy = 0;
			for (size_t iGame = 0; iGame != numGames; ++iGame)
			{
				cGame.setTeam(TEAM_A, teams[iGame]);
				cGame.setTeam(TEAM_B, teams[(iGame+1)%numGames]);

				if (cGame.initialize()) { cGame.run(); }
				cEntropy += findEntropies(cGame);
			}
			cEntropy /= numGames;
			std::cout << 
				"exploration= " << std::setw(4) << exploration << 
				" temperature= " << std::setw(5) << temperature <<
				" entrop= " << std::setw(12) << cEntropy <<
				"\n";
		}
	}*/

	evaluator_featureVector::uninitStatic();
	return true;
};
