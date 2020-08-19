#include "../inc/game.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>

#include "../inc/planner.h"
#include "../inc/planner_max.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/fp_compare.h"

#include "../inc/pkCU.h"
#include "../inc/engine.h"


Game::Game(const Config& cfg):
    cfg_(cfg),
    nv_(std::make_shared<EnvironmentNonvolatile>()),
    isInitialized_(false) {
}


Game& Game::setEnvironment(const std::shared_ptr<EnvironmentNonvolatile>& nv) {
  nv_ = nv;
  
  for(auto& planner: agents_) {
    if (planner == NULL) { continue; }
    planner->setEnvironment(nv_);
  }
  if (eval_ != NULL) { eval_->setEnvironment(nv_); }
  if (cu_ != NULL) {
    cu_->setEnvironment(nv_);
    initialState_ = cu_->initialState();
  }

  isInitialized_ = false;
  return *this;
}


Game& Game::setTeam(size_t iAgent, const TeamNonVolatile& tNV) {
  nv_->setTeam(iAgent, tNV, true);
  return setEnvironment(nv_);
}


Game& Game::setEngine(const std::shared_ptr<PkCU>& cu) {
  cu_ = cu;

  if (nv_ != NULL) {
    cu_->setEnvironment(nv_);
    initialState_ = cu_->initialState();
  }

  isInitialized_ = false;
  return *this;
}


Game& Game::setPlanner(size_t iAgent, const std::shared_ptr<Planner>& cPlanner) {
  assert(iAgent < 2);
  agents_[iAgent] = cPlanner;

  if (nv_ != NULL) { agents_[iAgent]->setEnvironment(nv_); }
  isInitialized_ = false;
  return *this;
}


Game& Game::setEvaluator(const std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;

  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
  return *this;
}


bool Game::initialize() {
  // teams must be set before initialize is called
  if (nv_ == NULL ||
      nv_->getTeam(TEAM_A).getNumTeammates() == 0 ||
      nv_->getTeam(TEAM_B).getNumTeammates() == 0) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": One or more teams are undefined!\n";
    throw std::runtime_error("team(s) undefined");
  }

  // initialize pkCU engine
  if (cu_ == NULL) { setEngine(PkCU()); }

  // initialize evaluator:
  if (eval_ == NULL) {setEvaluator(evaluator_simple()); }
  if (!eval_->isInitialized()) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": Game-state evaluator " <<
      ": \"" << eval_->getName() << "\" failed to initialize!\n";
  }

  // assign default agents if none exist:
  for (size_t iAgent = 0; iAgent < 2; ++iAgent) {
    if (agents_[iAgent] != NULL) { continue; }
    if (!cfg_.allowUndefinedAgents) { throw std::runtime_error("agent(s) undefined"); }

    setPlanner(iAgent, PlannerMax(iAgent).setEngine(cu_).setEvaluator(eval_));
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": agent " << iAgent << 
      " is undefined! Replaced with " << agents_[iAgent]->getName() << "\n";

    if (agents_[iAgent]->isInitialized()) { continue; }

    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": Agent " << iAgent <<
      ": \"" << agents_[iAgent]->getName() << "\" failed to initialize!\n";
    throw std::runtime_error("agent(s) failed to initialize");
  }
  
  // number of matches to be played must be sane:
  if ((cfg_.maxMatches & 1) == 0) { cfg_.maxMatches += 1; }

  isInitialized_ = true;
  return true;
}


HeatResult Game::rollout(const EnvironmentVolatileData& initialState) {
  std::vector<GameResult> gameLog;
  if (!isInitialized_) { initialize(); }
  
  if (cfg_.verbosity > 1) { printHeatStart(); }
  
  for (size_t iMatch = 0; iMatch != cfg_.maxMatches; ++iMatch) {
    GameResult gResult = rollout_game(initialState, iMatch);
    gameLog.push_back(gResult);
  }

  HeatResult result = digestMatch(gameLog);
  if (cfg_.verbosity > 1) { printHeatOutline(result); }
  return result;
}


GameResult Game::rollout_game(const EnvironmentVolatileData& initialState, size_t iMatch) {
  std::vector<Turn> turnLog;
  if (!isInitialized_) { initialize(); }
  EnvironmentPossibleData stateData = EnvironmentPossibleData::create(initialState);
  ConstEnvironmentPossible envP{*nv_, stateData};
  int32_t matchState = cu_->isGameOver(envP);
  size_t iPly, iLastEnvironment = 0;

  if (cfg_.verbosity >= 2) { printGameStart(iMatch); }
  
  for (iPly = 0; iPly < cfg_.maxPlies && (matchState == MATCH_MIDGAME); ++iPly) {
    // determine which move the teams will use:
    std::array<uint32_t, 2> actions;
    for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
      actions[iTeam] = agents_[iTeam]->generateSolution(envP);
    }

    // print out the agent's moves:
    if (cfg_.verbosity >= 3) {
      for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
        printAction(envP.getEnv().getTeam(iTeam), actions[iTeam], iTeam);
      }
    }

    // predict what will occur given these actions and their probabilities
    PossibleEnvironments possibleEnvironments = cu_->updateState(
        envP, actions[TEAM_A], actions[TEAM_B]);
    assert(possibleEnvironments.getNumUnique() > 0);

    // select the next environment, either by user choice or by random chance:
    ConstEnvironmentPossible nextEnvironment{*nv_};
    if (cfg_.allowStateSelection) {
      nextEnvironment = possibleEnvironments.stateSelect_index();
    } else {
      nextEnvironment = possibleEnvironments.stateSelect_roulette();
    }

    // perform state transition:
    if (!nextEnvironment.isEmpty()) {
      size_t iNextEnvironment = &nextEnvironment.data() - &possibleEnvironments.front();
      // determine if the current state is a terminal state, and if so end the game:
      matchState = cu_->isGameOver(envP);

      // create a log of this turn:
      turnLog.push_back(digestTurn(actions[TEAM_A], actions[TEAM_B], iLastEnvironment, envP));

      // remove a ply if the transition was a dummy move:
      if (nextEnvironment.hasFreeMove(TEAM_A) || nextEnvironment.hasFreeMove(TEAM_B)) {
        iPly--;
      }

      // print the state that occurs:
      if (cfg_.verbosity >= 3) {
        nextEnvironment.printState(iNextEnvironment, iPly); // TODO(@drendleman) - which state is this?
        std::cout << "\n";
      }

      // perform the state transition:
      stateData = nextEnvironment.data();
      iLastEnvironment = iNextEnvironment;
    } // endof state transition
  } // endOf foreach turn

  // print terminal state:
  GameResult result = digestGame(turnLog, matchState);
  if (cfg_.verbosity >= 2) { printGameOutline(result, iMatch); }

  return result;
} // endof rollout_game


Turn Game::digestTurn(
    unsigned int actionTeamA,
    unsigned int actionTeamB,
    size_t resultingState,
    const ConstEnvironmentPossible& envP) {
  Turn cTurn;

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    // set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
    fpType simpleFitness = eval_->calculateFitness(envP.getEnv(), iTeam).fitness;
    fpType initialFitness, finalFitness;
    if (agents_[iTeam] != NULL) {
      const std::vector<PlannerResult>& results = agents_[iTeam]->getDetailedResults();
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

  return cTurn;
} // endOf digestTurn


GameResult Game::digestGame(const std::vector<Turn>& cLog, int endStatus) {
  GameResult cResult;
  cResult.log = cLog;

  // initialize collected data:
  std::array<std::array<std::array<uint32_t, 5>, 6>, 2>& moveUse = cResult.moveUse;
  std::array<std::array<uint32_t, 6>, 2>& participation = cResult.participation;
  std::array<std::array<fpType, 6>, 2>& aggregateContribution = cResult.aggregateContribution;
  std::array<std::array<fpType, 6>, 2>& simpleContribution = cResult.simpleContribution;
  std::array<std::array<fpType, 6>, 2>& d0Contribution = cResult.d0Contribution;
  std::array<std::array<fpType, 6>, 2>& dMaxContribution = cResult.dMaxContribution;
  std::array<std::array<uint32_t, 6>, 2>& ranking = cResult.ranking;
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    for (size_t iTeammate = 0; iTeammate < 6; ++iTeammate) {
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
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      // set previous values to fitnesses of the first ply:
      prevSimpleFitness[iTeam] = fTurn.simpleFitness[iTeam];
      prev0Fitness[iTeam] = fTurn.depth0Fitness[iTeam];
      prevMaxFitness[iTeam] = fTurn.depthMaxFitness[iTeam];
  
      // add participation for first moving pokemon: (or lead twice)
      participation[iTeam][fTurn.activePokemon[iTeam]] += 1;
      // add a move increment for the first moving pokemon:
      if (PkCU::isMoveAction(fTurn.action[iTeam])) {
        moveUse[iTeam][fTurn.activePokemon[iTeam]][ fTurn.action[iTeam] - AT_MOVE_0 ] += 1;
      }
    }
  }

  // all turns after the first:
  for (size_t iPly = 1; iPly < cLog.size(); ++iPly) {
    // the previous turn. The previous turn was responsible for the delta between turn n-1 and n
    const Turn& bTurn = cLog[iPly -1];
    // the current turn. Used for updating delta
    const Turn& cTurn = cLog[iPly];

    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
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
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon) {
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

  return cResult;
} // endOf digestGame


HeatResult Game::digestMatch(const std::vector<GameResult>& gLog) {
  // initialize heatResult:
  HeatResult hResult;
  std::array<std::array<fpType, 6>, 2>& participation = hResult.participation;
  std::array<std::array<fpType, 6>, 2>& aggregateContribution = hResult.aggregateContribution;
  std::array<std::array<fpType, 6>, 2> avgRanking;
  std::array<std::array<uint32_t, 6>, 2>& ranking = hResult.ranking;
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    avgRanking[iTeam].fill(0);
    participation[iTeam].fill(0);
    aggregateContribution[iTeam].fill(0);
    ranking[iTeam].fill(7);
  }

  // compute final score:
  for(const auto& log: gLog) {
    // add a point for the winning team:
    switch (log.endStatus) {
    case MATCH_TEAM_A_WINS:
    case MATCH_TEAM_B_WINS:
      hResult.score[log.endStatus]++;
      break;
    case MATCH_DRAW:
    case MATCH_TIE:
      break;
    }
  }

  // generate average numPlies:
  hResult.numPlies = 0;
  for (size_t iLog = 0; iLog != gLog.size(); ++iLog) {
    const GameResult& cLog = gLog[iLog];

    hResult.numPlies += (fpType) cLog.numPlies;
  }
  hResult.numPlies /= gLog.size();

  // generate average values:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    for (size_t iPokemon = 0; iPokemon < 6; ++iPokemon) {
      for (size_t iLog = 0; iLog != gLog.size(); ++iLog) {
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
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
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
  if (hResult.score[TEAM_A] > hResult.score[TEAM_B]) {
    hResult.endStatus = MATCH_TEAM_A_WINS;
  } else if (hResult.score[TEAM_A] < hResult.score[TEAM_B]) {
    hResult.endStatus = MATCH_TEAM_B_WINS;
  } else if (hResult.score[TEAM_A] == 0 && hResult.score[TEAM_B] == 0) {
    hResult.endStatus = MATCH_DRAW;
  } else {
    hResult.endStatus = MATCH_TIE;
  }
  hResult.matchesPlayed = gLog.size();
  hResult.matchesTotal = cfg_.maxMatches;

  return hResult;
} // endOf digestMatch


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


void Game::printGameStart(size_t iMatch) {
  std::ostringstream gameIdentifier;
  
  std::cout <<
    "\nBegin " << gameIdentifier.str() << 
    " between teams TA: " << agents_[TEAM_A]->getName() <<
    " - " << nv_->getTeam(TEAM_A).getName() <<
    " and TB: " << agents_[TEAM_B]->getName() <<
    " - "<< nv_->getTeam(TEAM_B).getName() <<
    ((cfg_.verbosity>=3)?"!\n\n":"!\n");
}


void Game::printGameOutline(const GameResult& gResult, size_t iMatch) {
  std::ostringstream gameIdentifier;
  int matchState = gResult.endStatus;
  if (iMatch != SIZE_MAX) {
    gameIdentifier << "game " << (iMatch+1) << " of " << cfg_.maxMatches;
  } else {
    gameIdentifier << "the game";
  }

  if (matchState == MATCH_TIE) {
    std::cout <<
      "Teams TA: " << agents_[TEAM_A]->getName() <<
      " - " << nv_->getTeam(TEAM_A).getName() <<
      " and TB: " << agents_[TEAM_B]->getName() <<
      " - "<< nv_->getTeam(TEAM_B).getName() <<
      " have tied " << gameIdentifier.str() <<
      "!\n";
  } else if (matchState == MATCH_DRAW) {
    std::cout <<
      "Teams TA: " << agents_[TEAM_A]->getName() <<
      " - " << nv_->getTeam(TEAM_A).getName() <<
      " and TB: " << agents_[TEAM_B]->getName() <<
      " - "<< nv_->getTeam(TEAM_B).getName() <<
      " have drawn game " << gameIdentifier.str() <<
      "!\n";
  } else {
    size_t losingTeam = (gResult.endStatus + 1) % 2;
    std::cout <<
      "Team " << "T" << (matchState==TEAM_A?"A":"B") <<
      ": " << agents_[matchState]->getName() <<
      " - " << nv_->getTeam(matchState).getName() <<
      " has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
      ": " << agents_[losingTeam]->getName() <<
      " - " << nv_->getTeam(losingTeam).getName() <<
      " in game " << gameIdentifier.str() <<
      "!\n";
  }

  std::clog 
    << "--- GAME STATISTICS ---\n "
    << gResult.numPlies << " plies total\n"
    " Leaderboard: (index: name - (rank) aScore sScore participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    std::clog 
      << "  T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.getName() << ":"
      << (((int)iTeam==gResult.endStatus)?" (winner)":"")
      << "\n";
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
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


void Game::printHeatStart() {
  
}


void Game::printHeatOutline(const HeatResult& result) {
  if (result.endStatus == MATCH_TIE) {
    std::cout <<
      ((cfg_.verbosity>=3)?"\n":"") <<
      "Teams TA: " << agents_[TEAM_A]->getName() <<
      " - " << nv_->getTeam(TEAM_A).getName() <<
      " and TB: " << agents_[TEAM_B]->getName() <<
      " - "<< nv_->getTeam(TEAM_B).getName() <<
      " have tied the bo" << cfg_.maxMatches <<
      " series " << result.score[TEAM_A] <<
      " to " << result.score[TEAM_B] <<
      ((verbose>=3)?"!\n\n":"!\n");
  } else if (result.endStatus == MATCH_DRAW) {
    std::cout <<
      ((verbose>=3)?"\n":"") <<
      "Teams TA: " << agents_[TEAM_A]->getName() <<
      " - " << nv_->getTeam(TEAM_A).getName() <<
      " and TB: " << agents_[TEAM_B]->getName() <<
      " - "<< nv_->getTeam(TEAM_B).getName() <<
      " have drawn the bo" << cfg_.maxMatches <<
      " series " << result.score[TEAM_A] <<
      " to " << result.score[TEAM_B] <<
      ((verbose>=3)?"!\n\n":"!\n");
  } else {
    int matchState = result.endStatus;
    size_t losingTeam = (matchState + 1) % 2;
    std::cout <<
      ((verbose>=3)?"\n":"") <<
      "Team " << "T" << (matchState==TEAM_A?"A":"B") <<
      ": " << agents_[matchState]->getName() <<
      " - " << nv_->getTeam(matchState).getName() <<
      " has beaten team " << "T" << (matchState==TEAM_A?"B":"A") <<
      ": " << agents_[losingTeam]->getName() <<
      " - " << nv_->getTeam(losingTeam).getName() <<
      " , winning the bo" << cfg_.maxMatches <<
      " series " << result.score[matchState] <<
      " to " << result.score[matchState==TEAM_A?TEAM_B:TEAM_A] <<
      ((verbose>=3)?"!\n\n":"!\n");
  }

  std::clog 
    << "--- MATCH STATISTICS ---\n "
    << result.matchesPlayed << " out of " << result.matchesTotal << " games played\n "
    << "final score: " << result.score[0] << " to " << result.score[1] << "\n "
    << result.numPlies << " average plies per game\n"
    " Leaderboard: (index: name - (rank) avG-score avG-participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    std::clog 
      << "  T" << (iTeam==TEAM_A?"A":"B") <<": " 
      << cTeam.getName() << ":"
      << (((int)iTeam==result.endStatus)?" (winner)":"")
      << "\n";
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      std::clog
        << "    "
        << iPokemon << ": "
        << cTeam.teammate(iPokemon).getName() << " - ("
        << (result.ranking[iTeam][iPokemon] + 1) << ") "
        << result.aggregateContribution[iTeam][iPokemon] << " "
        << result.participation[iTeam][iPokemon]
        << "\n";
    }
  }
}
