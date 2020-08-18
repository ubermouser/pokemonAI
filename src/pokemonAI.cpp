#include <memory>

#include <boost/dll/shared_library.hpp>

#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>

#include "../inc/pkIO.h"
#include "../inc/game.h"
#include "../inc/trainer.h"
#include "../inc/init_toolbox.h"

#include "../inc/planner_max.h"
//#include "../inc/planner_directed.h"
#include "../inc/planner_random.h"
#include "../inc/planner_human.h"
//#include "../inc/planner_stochastic.h"
//#include "../inc/planner_minimax.h"

#include "../inc/ranked_team.h"
#include "../inc/evaluator_simple.h"
#include "../inc/evaluator_random.h"
//#include "../inc/evaluator_featureVector.h"

//#define PKAI_EXPORT
#include "../inc/pokemonAI.h"
//#undef PKAI_EXPORT

//#define PKAI_STATIC
#include "../inc/pokedex.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/orphan.h"
//#undef PKAI_STATIC

//using namespace boost::extensions;

// globals:


PokemonAI::PokemonAI()
{
  verbose = 2;
  warning = 0;
  
  isInitialized = false;
  
  pokemonIO = new PkIO(this);
  trainer = NULL;
  game = NULL;

  // invocation:
  gameType = GT_DIAG_HUVSHU;
  
  // planner defaults:
  numThreads = 0;
  secondsPerMove = 6;
  maxSearchDepth = 2;
  transpositionTableBins = 23;
  transpositionTableBinSize = 2;

  // genetic algorithm defaults:
  crossoverProbability = 0.035;
  mutationProbability = 0.55;
  seedProbability = 0.035;
  minimumWorkTime = 120;
  writeOutInterval = 0;
  maxGenerations = 12;

  // team trainer defaults:
  teamPopulations[0] = 362;
  teamPopulations[1] = 141;
  teamPopulations[2] = 60;
  teamPopulations[3] = 31;
  teamPopulations[4] = 15;
  teamPopulations[5] = 12;
  seedTeams = false;
  enforceSameLeague = false;

  // network trainer defaults:
  networkLayers.push_back(16); networkLayers.push_back(1);
  networkPopulation = 20;
  seedNetworks = false;
  jitterEpoch = 2500;
  seedNetworkProbability = 0.0001;
  seedDumbEvaluator = false;
  seedRandomEvaluator = false;

  // stochastic planner defaults:
  plannerTemperature = 1.0;
  plannerExploration = 0.5;

  // directed planner defaults:
  expSettings = experienceNetSettings::defaultSettings;

  // learning defaults:
  netSettings = temporalpropSettings::defaultSettings;
  numRollouts = 101;

  // game defaults:
  maxPlies = 50;
  maxMatches = 1;

  // engine defaults:
  engineAccuracy = RANDOMENVIRONMENTS;
  gameAccuracy = SIZE_MAX;

  //trueskill defaults:
  tSettings = trueSkillSettings::defaultSettings;

  evaluator_featureVector::initStatic();
}

PokemonAI::~PokemonAI()
{
  evaluator_featureVector::uninitStatic();

  if (game != NULL) { delete game; }
  if (trainer != NULL) { delete trainer; }
  
  if (pokemonIO != NULL) delete pokemonIO;
}

bool PokemonAI::init()
{
  bool result = 0;

  pokedex = std::make_unique<PokedexDynamic>(pokedex_config);
  
  //Teams:
  if (pokemonIO->input_teams.size() > 0)
  {
    if (verbose >= 1) std::cout << " Loading Pokemon teams...\n";
    for (size_t indexTeam = 0; indexTeam < pokemonIO->input_teams.size(); indexTeam++)
    {
      TeamNonVolatile cTeam;
      result = pokemonIO->inputPlayerTeam(pokemonIO->input_teams.at(indexTeam), cTeam);
      if (result != true)
      {
        std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
                ": inputTeam failed to create an acceptable team.\n";
      }
      else
      {
        // determine if name is duplicate:
        bool isDuplicate = false;
        for (size_t iPTeam = 0; iPTeam < teams.size(); iPTeam++)
        {
          if (cTeam.getName().compare(teams.at(iPTeam).getName()) == 0)
          {
            std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
              ": Duplicate team named \"" << cTeam.getName() << "\"!\n";
            isDuplicate = true;
            break;
          }
        }
        if (!isDuplicate) { teams.push_back(cTeam); };
      }
    }
  }
  //std::sort(teams.begin(), teams.end());

  //Networks:
  if (pokemonIO->input_networks.size() > 0)
  {
    if (verbose >= 1) std::cout << " Loading Evaluation networks...\n";
    networks.reserve(pokemonIO->input_networks.size());
    for (size_t iNetwork = 0; iNetwork < pokemonIO->input_networks.size(); iNetwork++)
    {
      neuralNet cNet;
      result = pokemonIO->inputPlayerNetwork(pokemonIO->input_networks.at(iNetwork), cNet);
      if (result != true)
      {
        std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
                ": inputNetwork failed to create an acceptable network.\n";
      }
      else
      {
        // determine if name is duplicate:
        bool isDuplicate = false;
        for (size_t iPNetwork = 0; iPNetwork < networks.size(); iPNetwork++)
        {
          if (cNet.getName().compare(networks.at(iPNetwork).getName()) == 0)
          {
            std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
              ": Duplicate network named \"" << cNet.getName() << "\"!\n";
            isDuplicate = true;
            break;
          }
        }
        if (!isDuplicate) { networks.push_back(cNet); };
      }
    }
  }
  //std::sort(networks.begin(), networks.end());
  
  // search values:
  checkRangeE(numThreads, 0, MAXTHREADS);
  checkRangeE(maxSearchDepth, 1, MAXSEARCHDEPTH);
  checkRangeE(transpositionTableBins, 1, 32);
  checkRangeE(transpositionTableBinSize, 1, 32);

  // engine values:
  checkRangeE(engineAccuracy, 1, 16);
  if ((gameAccuracy == 0) || (gameAccuracy > 16)) { gameAccuracy = engineAccuracy; }

  // genetic algorithm values:
  checkRangeE(crossoverProbability, 0.0, 1.0);
  checkRangeE(mutationProbability, 0.0, 1.0);
  checkRangeE(seedProbability, 0.0, 1.0);
  checkRangeE(crossoverProbability + mutationProbability + seedProbability, 0, 1.0);
  checkRangeE(seedNetworkProbability, 0, 1.0);
  checkRangeE(maxGenerations, 1, MAXGENERATIONS);
  // learning values:
  checkRangeE(netSettings.learningRate, 0.0, 2.0);
  checkRangeE(netSettings.lambda, 0.0, 1.0);
  checkRangeE(netSettings.gamma, 0.0, 1.0);
  checkRangeE(netSettings.jitterMax, 0.0, 2.0);
  checkRangeE(netSettings.momentum, 0.0, 2.0);
  // planner values:

  
  // team builder values:
  {
    for (size_t iLeague = 0; iLeague < teamPopulations.size(); ++iLeague)
    {
      if (teamPopulations[iLeague] == 0) { continue; }
      checkRangeE(teamPopulations[iLeague], 2 , MAXLEAGUEPOPULATION );
    }
  }

  // network builder values:
  if (networkPopulation != 0)
  {
    checkRangeE(networkPopulation, (seedDumbEvaluator?1:2), MAXNETWORKPOPULATION);
  }

  for (size_t iLayer = 0; iLayer < networkLayers.size(); ++iLayer)
  {
    checkRangeE(networkLayers[iLayer], 1 , MAXNETWORKWIDTH );
  }

  // game values:
  checkRangeE(maxPlies, 1, MAXPLIES);
  checkRangeE(maxMatches, 1, MAXBESTOF);

  return true;
} // end of init





void PokemonAI::printTeams(bool printTeammates) const
{
  for (size_t iTeam = 0; iTeam != teams.size(); ++iTeam)
  {
    const TeamNonVolatile& cTNV = teams.at(iTeam);
    
    // print out index and name of team
    std::cout 
      << iTeam
      << "-\"" << cTNV.getName() << 
      "\"\n";
    
    if (printTeammates)
    {
      // print out the name followed by the species of pokemon on this team
      for (size_t iPokemon = 0; iPokemon != cTNV.getNumTeammates(); ++iPokemon)
      {
        const PokemonNonVolatile& cPKNV = cTNV.teammate(iPokemon);
        std::cout << cPKNV;
      }
    }
  }
} // end of printTeams

const TeamNonVolatile* PokemonAI::teamSelect(char playerID)
{
  std::string input;
  int32_t iTeam;
  bool teamsPrinted = false;
  do
  {
    if (!teamsPrinted) { printTeams(verbose>5); teamsPrinted = true; }

    std::cout << "Please select index of the team for player " << playerID << ", or -1 to load a new team by path into memory:\n> ";
    getline(std::cin, input);
    std::stringstream inputResult(input);

    if (!(inputResult >> iTeam)) 
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " teamSelect recieved invalid input parameter \"" << inputResult.str() << "\"!\n";
      continue;
    }

    if (iTeam == -1)
    {
      std::cout << "Please enter a path:\npath> ";
      input.clear();
      getline(std::cin, input);
      TeamNonVolatile cTeam;
      if (pokemonIO->inputPlayerTeam(input, cTeam)) { teams.push_back(cTeam); teamsPrinted = false; }
      continue;
    }
    
    // determine if team is valid:
    else if((size_t)iTeam < teams.size())
    {
      return &teams[iTeam];
    }
    else
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " index team out of bounds: " << iTeam << "\n";
      continue;
    }
  }while(true);
  
  // hopefully unreachable
  return NULL;
} // endOf teamSelect





void PokemonAI::printEvaluators() const
{
  // print neural evaluators:
  for (size_t iNetwork = 0; iNetwork != networks.size(); ++iNetwork)
  {
    const neuralNet& cNet = networks[iNetwork];
    if (!cNet.isInitialized()) { continue; }
    const evaluator_featureVector* cEval = evaluator_featureVector::getEvaluator(cNet.numInputs(), cNet.numOutputs());
    if (cEval == NULL) { continue; }
    std::cout 
      << iNetwork
      << "-\"" << cNet.getName()
      << "\"-\"" << cEval->getName()
      << "\"\n";
  }
  // dumb and random evals:
  std::cout 
    << networks.size()
    << "-\"simpleEval\"\n"
    << (networks.size() + 1)
    << "-\"randomEval\"\n";
};

Evaluator* PokemonAI::evaluatorSelect(char playerID)
{
  std::string input;
  int32_t iEvaluator;
  bool evaluatorsPrinted = false;
  do
  {
    if (!evaluatorsPrinted) { printEvaluators(); evaluatorsPrinted = true; }

    std::cout 
      << "Please select index of the evaluator for player " 
      << playerID 
      << ", or -1 to load a new neuralNet evaluator by path into memory:\n> ";
    getline(std::cin, input);
    std::stringstream inputResult(input);

    if (!(inputResult >> iEvaluator)) 
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " evaluatorSelect recieved invalid input parameter \"" << inputResult.str() << "\"!\n";
      continue;
    }

    if (iEvaluator == -1)
    {
      std::cout << "Please enter a path:\npath> ";
      input.clear();
      getline(std::cin, input);
      neuralNet cNet;
      if (pokemonIO->inputPlayerNetwork(input, cNet)) { networks.push_back(cNet); evaluatorsPrinted = false; }
      continue;
    }
    
    // determine if network is valid:
    else if (iEvaluator == (networks.size())) // simpleEval:
    {
      return new evaluator_simple();
    }
    else if (iEvaluator == (networks.size()+1))
    {
      return new evaluator_random();
    }
    else if((size_t)iEvaluator < networks.size())
    {
      return evaluator_featureVector::getEvaluator(networks[iEvaluator]);
      //return &networks[iEvaluator];
    }
    else
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " index network out of bounds: " << iEvaluator << "\n";
      continue;
    }
  }while(true);
  
  // hopefully unreachable
  return NULL;
};





void PokemonAI::printPlanners() const
{
  std::cout
    << "0-\"planner_max\"\n"
    << "1-\"planner_stochastic\"\n"
    << "2-\"planner_random\"\n"
    << "3-\"planner_directed\"\n"
    << "4-\"planner_human\"\n"
    << "5-\"planner_minimax\"\n";
};

Planner* PokemonAI::plannerSelect(char playerID)
{
  std::string input;
  int32_t iPlanner;
  bool plannersPrinted = false;
  do
  {
    if (!plannersPrinted) { printPlanners(); plannersPrinted = true; }

    std::cout 
      << "Please select index of the planner for player " 
      << playerID 
      << ":\n> ";
    getline(std::cin, input);
    std::stringstream inputResult(input);

    if (!(inputResult >> iPlanner)) 
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " plannerSelect recieved invalid input parameter \"" << inputResult.str() << "\"!\n";
      continue;
    }

    switch(iPlanner)
    {
    case 0:
      return new PlannerMax();
    case 1:
      return new planner_stochastic(engineAccuracy, plannerTemperature, plannerExploration);
    case 2:
      return new planner_random();
    case 3:
      return new planner_directed(experienceNet(1, expSettings), engineAccuracy, plannerExploration);
    case 4:
      return new planner_human();
    case 5:
      return new planner_minimax(
        numThreads, 
        engineAccuracy, 
        maxSearchDepth, 
        secondsPerMove, 
        transpositionTableBins, 
        transpositionTableBinSize);
    default:
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " index network out of bounds: " << iPlanner << "\n";
      break;
    };
  }while(true);
  
  // hopefully unreachable
  return NULL;
};





bool PokemonAI::run()
{
  // run game (or drop to console):
  if (gameType == GT_OTHER_CONSOLE)
  {
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " The PokemonAI LUA console has been removed from pokemonAI as of version 477.\n";
      return false;
    }
  }
  else if (gameType >= GT_OTHER_EVOTEAMS && gameType <= GT_OTHER_GAUNTLET_BOTH)
  {
    trainer = new Trainer(gameType,
      maxPlies,
      maxMatches,
      gameAccuracy,
      engineAccuracy,
      tSettings,
      expSettings,
      maxGenerations,
      minimumWorkTime,
      mutationProbability,
      crossoverProbability,
      seedProbability,
      enforceSameLeague,
      plannerExploration,
      plannerTemperature,
      networkPopulation,
      netSettings,
      networkLayers,
      teamPopulations,
      seedNetworkProbability,
      jitterEpoch,
      numRollouts,
      writeOutInterval,
      teamDirectory,
      networkDirectory);

    if (seedDumbEvaluator) { trainer->seedEvaluator(evaluator_simple()); }
    if (seedRandomEvaluator) { trainer->seedEvaluator(evaluator_random()); }

    // TODO: gauntlet mode

    if (seedTeams)
    {
      BOOST_FOREACH(const TeamNonVolatile& cTeam, teams) { trainer->seedTeam(cTeam); }
    }

    if (seedNetworks)
    {
      BOOST_FOREACH(const neuralNet& cNet, networks) { trainer->seedNetwork(cNet); }
    }

    //Trainer->outputFeaturesToStream(new std::ofstream("output.csv", std::ios::out | std::ios::trunc));

    if (!trainer->initialize()) 
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " The PokemonAI genetic trainer engine was not successfully initialized.\n";
      return false;
    }

    if (verbose >= 0) std::cout << "Starting Pokemon trainer...\n";
    trainer->evolve();
  }
  else if (gameType >= GT_DIAG_HUVSHU && gameType <= GT_NORM_CPUVSHU)
  {
    if (gameType == GT_DIAG_BENCHMARK) { maxPlies = 1; maxMatches = 1; }

    game = new Game(maxPlies, maxMatches, gameAccuracy);
    // set teams using user data:
    if (teams.size() != 2) 
    {
      for (size_t iTeam = 0; iTeam != 2; ++iTeam)
      {
        const TeamNonVolatile* cTeam = teamSelect('A' + iTeam);
        if (cTeam != NULL) { game->setTeam(iTeam, *cTeam); }
        else { return false; }
      }
    }
    else
    {
      game->setTeam(TEAM_A, teams.front());
      game->setTeam(TEAM_B, teams.back());
    }

    
    // set planners using user data:
    std::array<bool, 2> humanPlanner =
    {{ (gameType == GT_DIAG_HUVSCPU || 
      gameType == GT_DIAG_HUVSHU || 
      gameType == GT_DIAG_HUVSHU_UNCERTAIN ||
      gameType == GT_NORM_HUVSCPU),

      (gameType == GT_DIAG_HUVSHU ||
      gameType == GT_NORM_CPUVSHU ||
      gameType == GT_DIAG_HUVSHU_UNCERTAIN)
    }};
    for (size_t iTeam = 0; iTeam != 2; ++iTeam)
    {
      if (humanPlanner[iTeam]) { game->setPlanner(iTeam, planner_human()); }
      else
      {
        boost::scoped_ptr<Planner> cPlanner(plannerSelect('A' + iTeam));
        if (cPlanner != NULL) { game->setPlanner(iTeam, *cPlanner); }
        else { return false; }
      }
    }

    // make sure we know where the human planners are (in case the user selected a human planner without choosing a human gamemode
    humanPlanner[TEAM_A] = (dynamic_cast<const planner_human*>(game->getPlanner(TEAM_A)) != NULL);
    humanPlanner[TEAM_B] = (dynamic_cast<const planner_human*>(game->getPlanner(TEAM_B)) != NULL);
    bool oneHumanPlanner = humanPlanner[0] ^ humanPlanner[1];
    // set evaluators using user data:
    if (humanPlanner[TEAM_A] && humanPlanner[TEAM_B]) // no evaluators needed, two humans playing
    {
    }
    if (oneHumanPlanner && (networks.size() == 1)) // one evaluator needed, one human playing
    {
      boost::scoped_ptr<evaluator_featureVector> hEval(evaluator_featureVector::getEvaluator(networks.front()));
      game->setEvaluator(humanPlanner[TEAM_A]?TEAM_B:TEAM_A, *hEval);
    }
    else if (!oneHumanPlanner && networks.size() == 2) // two evaluators needed, no humans playing (but two evaluators defined)
    {
      boost::scoped_ptr<evaluator_featureVector> evalA(evaluator_featureVector::getEvaluator(networks.front()));
      boost::scoped_ptr<evaluator_featureVector> evalB(evaluator_featureVector::getEvaluator(networks.back()));
      game->setEvaluator(TEAM_A, *evalA); 
      game->setEvaluator(TEAM_B, *evalB);
    }
    else // two evaluators needed, no humans playing (no evaluators defined)
    {
      for (size_t iTeam = 0; iTeam != 2; ++iTeam)
      {
        if (humanPlanner[iTeam]) { continue; } // don't bother adding an evaluator if none will be used
        boost::scoped_ptr<Evaluator> cEval(evaluatorSelect('A' + iTeam));
        if (cEval != NULL) { game->setEvaluator(iTeam, *cEval); }
        else { return false; }
      }
    }

    if (!game->initialize()) 
    { 
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        " The PokemonAI game engine was not successfully initialized.\n";
      return false;
    }

    if (verbose >= 0) std::cout << "Starting Pokemon game...\n";
    game->run();
  }
  else
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      " PokemonAI received unknown gameType " << gameType << "!\n";
    return false;
  }
  return true;
}
