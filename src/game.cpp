//#define PKAI_IMPORT

#include <iostream>
#include <boost/foreach.hpp>

#include "../inc/planner.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/fp_compare.h"

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





Game::Game(size_t _maxPlies, size_t _maxMatches, bool _rollout)
  : agents(),
  gameLog(),
  gameResults(),
  hResult(),
  cu_(NULL),
  nv_(NULL),
  maxPlies(_maxPlies),
  maxMatches(_maxMatches),
  rollout(_rollout),
  allowStateSelection(false),
  isInitialized(false),
  initialState_(EnvironmentVolatileData())
{
  agents.fill(NULL);
}


void Game::cleanUp() {
  gameLog.clear();
  gameResults.clear();
}


void Game::setEnvironment(const EnvironmentNonvolatile& envNV) {
  nv_ = std::make_shared<const EnvironmentNonvolatile>(envNV);
  
  for(auto& planner: agents) {
    if (planner == NULL) { continue; }
    planner->setEnvironment(nv_);
  }
  if (eval_ != NULL) { eval_->setEnvironment(nv_); }
  if (cu_ != NULL) {
    cu_->setEnvironment(nv_);
    initialState_ = cu_->initialState();
  }

  isInitialized = false;
}


void Game::setEngine(const PkCU& cu) {
  cu_ = std::make_shared<PkCU>(cu);

  if (nv_ != NULL) {
    cu_->setEnvironment(nv_);
    initialState_ = cu_->initialState();
  }

  isInitialized = false;
}


void Game::setPlanner(size_t iAgent, const std::shared_ptr<Planner>& cPlanner) {
  assert(iAgent < 2);
  agents[iAgent] = cPlanner;

  if (nv_ != NULL) { agents[iAgent]->setEnvironment(nv_); }
  isInitialized = false;
}


void Game::setEvaluator(const std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;

  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
}

void Game::setInitialState(const EnvironmentVolatileData& rolloutState)
{
  if (rollout != true && verbose >= 5) 
  { 
    std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
      ": An initial rollout state was defined, but rollout mode was not enabled. The state will not be used.\n";
  }
  initialState_ = rolloutState;
}


bool Game::initialize()
{
  // teams must be set before initialize is called
  if (nv_->getTeam(TEAM_A).getNumTeammates() == 0 || nv_->getTeam(TEAM_B).getNumTeammates() == 0)
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

  // initialize pkCU engine
  if (cu_ == NULL) { setEngine(PkCU()); }

  // initialize agents:
  for (size_t iAgent = 0; iAgent < 2; ++iAgent)
  {
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


void Game::run()
{
  assert(isInitialized);

  gameLog.clear();
  gameResults.clear();
  hResult = HeatResult();

  // used to determine if a team wins the bestOf by majority instead of the bestOf running to completion
  bool heatOver = false;
  int32_t matchState;
  std::array<unsigned int, 2> numWins;
  numWins.fill(0);
  
  for (size_t iMatch = 0; iMatch != maxMatches; ++iMatch)
  {
    // initialize match metrics:
    std::vector<Turn>* cLog = NULL;
    if (!rollout) 
    {
      gameLog.push_back(std::vector<Turn>());
      cLog = &gameLog.back(); 
    }

    // initialize pokemon:
    auto environmentData = EnvironmentPossibleData::create(initialState_, true);
    ConstEnvironmentPossible cEnvP{*nv_, environmentData};

    if (verbose >= 3)
    {
      std::cout << 
        "\nBegin game " << (iMatch+1) << 
        " of " << maxMatches <<
        " between teams TA: " << agents[TEAM_A]->getName() <<
        " - " << nv_->getTeam(TEAM_A).getName() <<
        " and TB: " << agents[TEAM_B]->getName() << 
        " - "<< nv_->getTeam(TEAM_B).getName() <<
        ((verbose>=3)?"!\n\n":"!\n");
    }

    // local game variables:
    matchState = MATCH_MIDGAME;
    size_t iPly;
    size_t iLastEnvironment = 0;

    // begin combat:
    for (iPly = 0; iPly < maxPlies && (matchState == MATCH_MIDGAME); ++iPly) {
      
      // determine if the current state is a terminal state, and if so end the game:
      matchState = cu_->isGameOver(cEnvP);

      // determine which move the teams will use:
      std::array<uint32_t, 2> actions;
      if (matchState == MATCH_MIDGAME) {
        for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
          actions[iTeam] = agents[iTeam]->generateSolution(cEnvP);
          if (actions[iTeam] >= AT_ITEM_USE) {
            matchState = (iTeam + 1) % 2; break;
          }
        }
      }

      // if both of the teams have valid moves:
      if (matchState == MATCH_MIDGAME) {
        if (verbose >= 3) {
          for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
            printAction(cEnvP.getEnv().getTeam(iTeam), actions[iTeam], iTeam);
          }
        }
    
        // predict what will occur given these actions and their probabilities
        PossibleEnvironments possibleEnvironments = cu_->updateState(
            cEnvP, actions[TEAM_A], actions[TEAM_B]);
        assert(possibleEnvironments.getNumUnique() > 0);

        ConstEnvironmentPossible nextEnvironment{*nv_};
        if (allowStateSelection) {
          nextEnvironment = possibleEnvironments.stateSelect_index();
        } else {
          nextEnvironment = possibleEnvironments.stateSelect_roulette();
        }

        // perform state transition:
        if (!nextEnvironment.isEmpty()) {
          // create a log of this turn:
          if (!rollout) { digestTurn(*cLog, actions[TEAM_A], actions[TEAM_B], iLastEnvironment, cEnvP); }

          // remove a ply if the transition was a dummy move:
          if (nextEnvironment.hasFreeMove(TEAM_A) || nextEnvironment.hasFreeMove(TEAM_B))
          {
            iPly--;
          }

          // print the state that occurs:
          if (verbose >= 3)
          {
            if (verbose >= 4) { std::cout << "\n"; }
            nextEnvironment.printState(SIZE_MAX, iPly); // TODO(@drendleman) - which state is this?
            std::cout << "\n";
          }

          // perform the state transition:
          environmentData = cEnvP.data();
          cEnvP = ConstEnvironmentPossible{*nv_, environmentData};
          iLastEnvironment = 0; // TODO(@drendleman) - which state is this?
        }
      } // endOf if matchState isn't terminal
      else if (verbose >= 2 && maxMatches > 1) {
        // if a tie occurs: (all pokemon dead)
        if (matchState == MATCH_TIE) {
          std::cout << 
            "Teams TA: " << agents[TEAM_A]->getName() <<
            " - " << nv_->getTeam(TEAM_A).getName() <<
            " and TB: " << agents[TEAM_B]->getName() << 
            " - "<< nv_->getTeam(TEAM_B).getName() <<
            " have tied game " << (iMatch+1) << 
            " of " << maxMatches <<
            "!\n";
        } else if (matchState == MATCH_DRAW) {
          std::cout << 
            "Teams TA: " << agents[TEAM_A]->getName() <<
            " - " << nv_->getTeam(TEAM_A).getName() <<
            " and TB: " << agents[TEAM_B]->getName() << 
            " - "<< nv_->getTeam(TEAM_B).getName() <<
            " have drawn game " << (iMatch+1) << 
            " of " << maxMatches <<
            "!\n";
        } else {
          size_t losingTeam = (matchState + 1) % 2;
          std::cout << 
            "Team " << "T" << (matchState==TEAM_A?"A":"B") <<
            ": " << agents[matchState]->getName() <<
            " - " << nv_->getTeam(matchState).getName() <<
            " has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
            ": " << agents[losingTeam]->getName() <<
            " - " << nv_->getTeam(losingTeam).getName() <<
            " in game " << (iMatch+1) << 
            " of " << maxMatches <<
            " - " << (numWins[matchState]+1) <<
            " to " << numWins[losingTeam] <<
            "!\n";
        }
      } // endOf if matchState IS terminal
    } //endOf game loop

    // digest the last game state:
    BOOST_FOREACH(auto& cPlanner, agents) {
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
        printGameOutline(gameResults.back(), *cLog, *nv_);
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
        " - " << nv_->getTeam(matchState).getName() <<
        " has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
        ": " << agents[losingTeam]->getName() <<
        " - " << nv_->getTeam(losingTeam).getName() <<
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
        " - " << nv_->getTeam(TEAM_A).getName() <<
        " and TB: " << agents[TEAM_B]->getName() << 
        " - "<< nv_->getTeam(TEAM_B).getName() <<
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
        " - " << nv_->getTeam(TEAM_A).getName() <<
        " and TB: " << agents[TEAM_B]->getName() << 
        " - "<< nv_->getTeam(TEAM_B).getName() <<
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
      printMatchOutline(*nv_);
    }
  }
  else
  {
    hResult.score[TEAM_A] = numWins[TEAM_A];
    hResult.score[TEAM_B] = numWins[TEAM_B];
  }
} // endOf run


void Game::digestTurn(
  std::vector<Turn>& cLog, 
  unsigned int actionTeamA, 
  unsigned int actionTeamB, 
  size_t resultingState, 
  const ConstEnvironmentPossible& envP)
{
  // do not create a digest of a dummy move:
  //if (envP.hasFreeMove(TEAM_A) || envP.hasFreeMove(TEAM_B)) { return; }
  
  cLog.push_back(Turn());
  Turn& cTurn = cLog.back();

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    // set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
    fpType simpleFitness = eval_->calculateFitness(envP.getEnv(), iTeam).fitness;
    fpType initialFitness, finalFitness;
    if (agents[iTeam] != NULL) {
      const std::vector<PlannerResult>& results = agents[iTeam]->getDetailedResults();
      // if the agent for this team has been initialized, grab its collected fitnesses:
      if (results.empty()) {
        initialFitness = simpleFitness; finalFitness = simpleFitness;
      } else {
        const PlannerResult& iResult = results.front();
        const PlannerResult& fResult = results.back();

        initialFitness = (iResult.lbFitness + iResult.ubFitness) / 2.0;
        finalFitness = (fResult.lbFitness + fResult.ubFitness) / 2.0;
      }
    } else {
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
  cTurn.env = envP.getEnv().data();
} // endOf digestTurn


void Game::digestGame(const std::vector<Turn>& cLog, int endStatus)
{
  gameResults.push_back(GameResult());
  GameResult& cResult = gameResults.back();

  // initialize collected data:
  std::array<std::array<std::array<uint32_t, 5>, 6>, 2>& moveUse = cResult.moveUse;
  std::array<std::array<uint32_t, 6>, 2>& participation = cResult.participation;
  std::array<std::array<fpType, 6>, 2>& aggregateContribution = cResult.aggregateContribution;
  std::array<std::array<fpType, 6>, 2>& simpleContribution = cResult.simpleContribution;
  std::array<std::array<fpType, 6>, 2>& d0Contribution = cResult.d0Contribution;
  std::array<std::array<fpType, 6>, 2>& dMaxContribution = cResult.dMaxContribution;
  std::array<std::array<uint32_t, 6>, 2>& ranking = cResult.ranking;
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    for (size_t iTeammate = 0; iTeammate < 6; ++iTeammate)
    {
      moveUse[iTeam][iTeammate].fill(0);
    }
    
    participation[iTeam].fill(0);
    aggregateContribution[iTeam].fill(std::numeric_limits<fpType>::quiet_NaN());
    simpleContribution[iTeam].fill(0);
    d0Contribution[iTeam].fill(0);
    dMaxContribution[iTeam].fill(0);
    ranking[iTeam].fill(7);
  }

  // initialize prevFitnesses
  std::array<fpType, 2> prevSimpleFitness;
  std::array<fpType, 2> prev0Fitness;
  std::array<fpType, 2> prevMaxFitness;
  { // first turn:
    const Turn& fTurn = cLog[0];
    for (size_t iTeam = 0; iTeam < 2; ++iTeam)
    {
      // set previous values to fitnesses of the first ply:
      prevSimpleFitness[iTeam] = fTurn.simpleFitness[iTeam];
      prev0Fitness[iTeam] = fTurn.depth0Fitness[iTeam];
      prevMaxFitness[iTeam] = fTurn.depthMaxFitness[iTeam];
  
      // add participation for first moving pokemon: (or lead twice)
      participation[iTeam][fTurn.activePokemon[iTeam]] += 1;
      // add a move increment for the first moving pokemon:
      if (PkCU::isMoveAction(fTurn.action[iTeam]))
      {
        moveUse[iTeam][fTurn.activePokemon[iTeam]][ fTurn.action[iTeam] - AT_MOVE_0 ] += 1;
      }
    }
  }

  // all turns after the first:
  for (size_t iPly = 1; iPly < cLog.size(); ++iPly)
  {
    // the previous turn. The previous turn was responsible for the delta between turn n-1 and n
    const Turn& bTurn = cLog[iPly -1];
    // the current turn. Used for updating delta
    const Turn& cTurn = cLog[iPly];

    for (size_t iTeam = 0; iTeam < 2; ++iTeam)
    {
      // create deltas
      fpType dSimpleFitness = cTurn.simpleFitness[iTeam] - prevSimpleFitness[iTeam];
      fpType d0Fitness = cTurn.depth0Fitness[iTeam] - prev0Fitness[iTeam];
      fpType dMaxFitness = cTurn.depthMaxFitness[iTeam] - prevMaxFitness[iTeam];

      // for every turn a pokemon is in play, this increases a counter for that pokemon by 1
      participation[iTeam][cTurn.activePokemon[iTeam]]+= 1;
      
      // add a move increment for the current pokemon's move
      if (PkCU::isMoveAction(cTurn.action[iTeam]))
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
    for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon)
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
    std::array<bool, 6> rankedPokemon;
    rankedPokemon.fill(false);
    for (size_t iRank = 0; iRank < nv_->getTeam(iTeam).getNumTeammates(); ++iRank)
    {
      fpType currentBestA = -std::numeric_limits<fpType>::infinity();
      fpType currentBestS = -std::numeric_limits<fpType>::infinity();
      size_t iCurrentBest = SIZE_MAX;

      for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon)
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





void Game::digestMatch(
    const std::vector<GameResult>& gLog,
    const std::array<unsigned int, 2>& numWins,
    int matchResult) {
  // initialize heatResult:
  std::array<std::array<fpType, 6>, 2>& participation = hResult.participation;
  std::array<std::array<fpType, 6>, 2>& aggregateContribution = hResult.aggregateContribution;
  std::array<std::array<fpType, 6>, 2> avgRanking;
  std::array<std::array<uint32_t, 6>, 2>& ranking = hResult.ranking;
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    avgRanking[iTeam].fill(0);
    participation[iTeam].fill(0);
    aggregateContribution[iTeam].fill(0);
    ranking[iTeam].fill(7);
  }

  // generate average numPlies:
  hResult.numPlies = 0;
  for (size_t iLog = 0; iLog != gLog.size(); ++iLog)
  {
    const GameResult& cLog = gLog[iLog];

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
        const GameResult& cLog = gLog[iLog];

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
    std::array<bool, 6> rankedPokemon;
    rankedPokemon.fill(false);
    for (size_t iRank = 0; iRank < nv_->getTeam(iTeam).getNumTeammates(); ++iRank)
    {
      fpType currentBestR = std::numeric_limits<fpType>::infinity();
      fpType currentBestA = -std::numeric_limits<fpType>::infinity();
      size_t iCurrentBest = SIZE_MAX;

      for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon)
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





const HeatResult& Game::getResult() const
{
  assert(rollout || !gameResults.empty());
  return hResult;
};

const std::vector<GameResult>& Game::getGameResults() const
{
  assert(!gameResults.empty());
  return gameResults;
};

const std::vector<Turn>& Game::getGameLog(size_t iGame) const
{
  assert(!gameLog.empty() && (iGame < gameLog.size()));
  return gameLog[iGame];
};


void Game::printAction(
    const ConstTeamVolatile& cTeam, unsigned int indexAction, unsigned int iTeam) {
  //const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
  if (indexAction >= AT_MOVE_0 && indexAction <= AT_MOVE_3) {
    std::clog 
      << "T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName() << " used "
      << (indexAction - AT_MOVE_0) << "-" 
      << cTeam.getPKV().getMV(indexAction)
      << "!\n";
  } else if (indexAction >= AT_SWITCH_0 && indexAction <= AT_SWITCH_5) {
    std::clog 
      << "T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName() << " is switching out with "
      << (indexAction - AT_SWITCH_0) << ": "
      << cTeam.teammate(indexAction - AT_SWITCH_0).nv().getName() << "!\n";
  } else if (indexAction == AT_MOVE_NOTHING) {
    std::clog 
      << "T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName() << " waited for a turn!\n";
  } else if (indexAction == AT_MOVE_STRUGGLE) {
    std::clog 
      << "T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName() << " used X-"
      << cTeam.getPKV().getMV(AT_MOVE_STRUGGLE)
      << "!\n";
  } else {
    std::clog 
      << "T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName() << " chose unknown action "
      << indexAction << "!\n";
  }
}





void Game::printGameOutline(const GameResult& gResult, const std::vector<Turn>& gLog, const EnvironmentNonvolatile& cEnv)
{
  std::clog 
    << "--- GAME STATISTICS ---\n "
    << gResult.numPlies << " plies total\n"
    " Leaderboard: (index: name - (rank) aScore sScore participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++)
  {
    const TeamNonVolatile& cTeam = cEnv.getTeam(iTeam);
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





void Game::printMatchOutline(const EnvironmentNonvolatile& cEnv)
{
  std::clog 
    << "--- MATCH STATISTICS ---\n "
    << hResult.matchesPlayed << " out of " << hResult.matchesTotal << " games played\n "
    << "final score: " << hResult.score[0] << " to " << hResult.score[1] << "\n "
    << hResult.numPlies << " average plies per game\n"
    " Leaderboard: (index: name - (rank) avG-score avG-participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++)
  {
    const TeamNonVolatile& cTeam = cEnv.getTeam(iTeam);
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
