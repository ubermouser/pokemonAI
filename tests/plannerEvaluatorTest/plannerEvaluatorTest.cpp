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

int main()
{
  verbose = 5;
  warning = 2;
  srand((unsigned int)time(NULL));
  tinyAI* _pkdex = new tinyAI();
  pkdex = _pkdex;
  bool writeOut = false;
  static const std::string oDirectory = "output.csv";
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

  environment_nonvolatile envNV;
  // SET TEAMS HERE:
  {   std::vector<std::string> lines; size_t iLine = 0; 
    { std::string inputBuffer; INI::loadFileToString("teams/hexTeamD.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
    envNV.getTeam(TEAM_A).input(lines, iLine); }
  {   std::vector<std::string> lines; size_t iLine = 0; 
    { std::string inputBuffer; INI::loadFileToString("teams/hexTeamA.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
    envNV.getTeam(TEAM_B).input(lines, iLine); }

  neuralNet aNet;
  {   std::vector<std::string> lines; size_t iLine = 0; 
    { std::string inputBuffer; INI::loadFileToString("networks/mc_128_48_16_1.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
    aNet.input(lines, iLine); }
  neuralNet bNet;
  {   std::vector<std::string> lines; size_t iLine = 0; 
    { std::string inputBuffer; INI::loadFileToString("networks/td_16_48_16_1.txt", "", inputBuffer); lines = INI::tokenize(inputBuffer, "\n\r"); } 
    bNet.input(lines, iLine); }

  verbose = 0;

  game cGame(MAXPLIES, 20000, 1);
  cGame.setEnvironment(envNV);
  //cGame.setPlanner(TEAM_A, planner_minimax());
  //cGame.setPlanner(TEAM_B, planner_minimax());
  cGame.setPlanner(TEAM_A, planner_directed(experienceNet(1, experienceNetSettings(8, 0.0625, 0.8, EXPERIENCENET_RECENCY)), 1, 0.5));
  //cGame.setPlanner(TEAM_A, planner_max());
  cGame.setPlanner(TEAM_B, planner_max());
  //cGame.setPlanner(TEAM_B, planner_directed(experienceNet(1, experienceNetSettings(24, 1.0f / 18.0f, 0.75f))));
  //cGame.setEvaluator(TEAM_A, evaluator_simple());
  cGame.setEvaluator(TEAM_B, evaluator_simple());
  cGame.setEvaluator(TEAM_A, *boost::scoped_ptr<evaluator_featureVector>(evaluator_featureVector::getEvaluator(aNet)));
  //cGame.setEvaluator(TEAM_B, *boost::scoped_ptr<evaluator_featureVector>(evaluator_featureVector::getEvaluator(aNet)));

  std::cout << "starting game...\n";
  if (cGame.initialize())
  {
    cGame.run();
    std::cout << "game completed!\n";
  }
  else
  {
    std::cerr << "game failed to initialize!\n";
    return false;
  }

  if (writeOut) 
  { 
    // attempt to open an ostream here:
    std::fstream oS(oDirectory, std::fstream::out | std::fstream::trunc);
    if (!oS.is_open() || !oS.good()) { return false; }

    boost::scoped_ptr<evaluator_featureVector> eval(evaluator_featureVector::getEvaluator(aNet));
    if (eval == NULL) { return false; }
    if (eval->outputSize() != 1) { return false; }

    // print header of desired feature vector:
    eval->outputNames(oS);

    eval->resetEvaluator(cGame.getEnvNV());

    if (!eval->isInitialized()) { return false; }
    // output game feature vector:
    for (size_t iGame = 0; iGame != cGame.getGameResults().size(); ++iGame)
    {
      const std::vector<turn>& cGameLog = cGame.getGameLog(iGame);
      for (size_t iTurn = 0; iTurn != cGameLog.size(); ++iTurn)
      {
        const turn& cTurn = cGameLog[iTurn];
        evalResult_t result = eval->calculateFitness(cTurn.env, TEAM_A);
        eval->outputFeatureVector(oS, &result.fitness);
      }
    }

    oS.close();
  }
  evaluator_featureVector::uninitStatic();
  return true;
};
