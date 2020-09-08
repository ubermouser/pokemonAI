#include "../inc/game.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <omp.h>

#include "../inc/planner.h"
#include "../inc/planner_max.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/fp_compare.h"

#include "../inc/pkCU.h"
#include "../inc/engine.h"

namespace po = boost::program_options;


po::options_description Game::Config::options(
    Config& cfg, const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "game-verbosity").c_str(),
      po::value<int>(&cfg.verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls status printing.")
      ((prefix + "max-plies").c_str(),
      po::value<size_t>(&cfg.maxPlies)->default_value(defaults.maxPlies),
      "maximum number of turns allowed before a draw occurs")
      ((prefix + "max-matches").c_str(),
      po::value<size_t>(&cfg.maxMatches)->default_value(defaults.maxMatches),
      "maximum number of matches, in best of N format")
      ((prefix + "num-threads").c_str(),
      po::value<size_t>(&cfg.numThreads)->default_value(defaults.numThreads),
      "number of threads to use when performing multiple matches")
      ((prefix + "allow-state-selection").c_str(),
      po::value<bool>(&cfg.allowStateSelection)->default_value(defaults.allowStateSelection),
      "when true, manual state selection is used.");

  return desc;

}


Game::Game(const Config& cfg):
    cfg_(cfg),
    nv_(std::make_shared<EnvironmentNonvolatile>()),
    isInitialized_(false) {
}


Game& Game::setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
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
  return setEnvironment(EnvironmentNonvolatile(*nv_).setTeam(iAgent, tNV, true));
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
  agents_[iAgent]->setTeam(iAgent);

  if (nv_ != NULL) { agents_[iAgent]->setEnvironment(nv_); }
  isInitialized_ = false;
  return *this;
}


Game& Game::setEvaluator(const std::shared_ptr<Evaluator>& eval) {
  eval_ = eval;

  if (nv_ != NULL) { eval_->setEnvironment(nv_); }
  return *this;
}


Game& Game::initialize() {
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
  if (eval_ == NULL) {setEvaluator(EvaluatorSimple()); }
  try {
    eval_->initialize();
  } catch (const std::exception& e) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
      ": Game-state evaluator " <<
      ": \"" << eval_->getName() << "\" failed to initialize!\n";
    std::cerr << e.what() << "\n";
    throw std::runtime_error("game-state evaluator failed to initialize");
  }
  // assign default agents if none exist:
  for (size_t iAgent = 0; iAgent < 2; ++iAgent) {
    auto& agent = agents_[iAgent];
    if (agent == NULL) {
      if (!cfg_.allowUndefinedAgents) { throw std::runtime_error("agent(s) undefined"); }

      setPlanner(iAgent, PlannerMax().setEngine(cu_).setEvaluator(eval_));
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": agent " << iAgent << 
        " is undefined! Replaced with " << agent->getName() << "\n";
    }

    try {
      agent->initialize();
    } catch (const std::exception& e) {
      std::cerr << e.what() << "\n";
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": Agent " << iAgent <<
        ": \"" << agent->getName() << "\" failed to initialize!\n";
      throw std::runtime_error("agent(s) failed to initialize");
    }
  }
  
  // number of matches to be played must be sane:
  if ((cfg_.maxMatches & 1) == 0) { cfg_.maxMatches += 1; }

  if (cfg_.numThreads == SIZE_MAX) {
    cfg_.numThreads = omp_get_num_threads();
    std::cerr << "Game thread parallelism set to " << cfg_.numThreads << "!\n";
  }

  isInitialized_ = true;
  return *this;
}


HeatResult Game::rollout(const EnvironmentVolatileData& initialState) {
  std::vector<GameResult> gameLog(cfg_.maxMatches);
  std::array<uint32_t, 2> score{0,0};

  if (!isInitialized_) { initialize(); }
  if (cfg_.verbosity >= 1) { printHeatStart(); }

  #pragma omp parallel for if (cfg_.numThreads > 0) num_threads(cfg_.numThreads)
  for (size_t iMatch = 0; iMatch < cfg_.maxMatches; ++iMatch) {
    // end early if one player clearly dominates the other (we cannot break out of a parallel loop)
    if (*std::max_element(begin(score), end(score)) > (cfg_.maxMatches / 2)) { continue; }

    // perform the rollout:
    GameResult& gResult = gameLog[iMatch];
    gResult = rollout_game(initialState, iMatch);

    // increment score:
    incrementScore(gResult.endStatus, score);
  }

  HeatResult result = digestMatch(gameLog);
  if (cfg_.verbosity >= 1 && cfg_.maxMatches > 1) { printHeatOutline(result); }
  return result;
}


GameResult Game::rollout_game(const EnvironmentVolatileData& initialState, size_t iMatch) {
  std::vector<Turn> turnLog;
  if (!isInitialized_) { initialize(); }
  EnvironmentPossibleData stateData = EnvironmentPossibleData::create(initialState);
  ConstEnvironmentPossible envP{*nv_, stateData};
  int32_t matchState = cu_->getGameState(envP);
  size_t iPly;

  if (cfg_.verbosity >= 2) { printGameStart(iMatch); }
  
  for (iPly = 0; iPly < cfg_.maxPlies && (matchState == MATCH_MIDGAME); ++iPly) {
    // determine which move the teams will use:
    std::array<PlannerResult, 2> actions;
    for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
      actions[iTeam] = agents_[iTeam]->generateSolution(envP);
    }

    // print out the agent's moves:
    if (cfg_.verbosity >= 3) {
      for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
        printAction(envP.getEnv().getTeam(iTeam), actions[iTeam].bestAgentAction(), iTeam);
      }
    }

    // predict what will occur given these actions and their probabilities
    PossibleEnvironments possibleEnvironments = cu_->updateState(
        envP, actions[TEAM_A].bestAgentAction(), actions[TEAM_B].bestAgentAction());
    assert(possibleEnvironments.getNumUnique() > 0);

    // select the next environment, either by user choice or by random chance:
    ConstEnvironmentPossible nextEnvironment{*nv_};
    size_t iNextEnvironment;
    if (cfg_.allowStateSelection) {
      std::stringstream out;
      possibleEnvironments.printStates(out, (boost::format("ply=%d ") % iPly).str());
      std::cout << out.str();
      nextEnvironment = possibleEnvironments.stateSelect_index(iNextEnvironment);
    } else {
      nextEnvironment = possibleEnvironments.stateSelect_roulette(iNextEnvironment);
    }

    // perform state transition:
    if (!nextEnvironment.isEmpty()) {
      // determine if the current state is a terminal state, and if so end the game:
      matchState = cu_->getGameState(nextEnvironment);

      // create a log of this turn:
      turnLog.push_back(digestTurn(actions, iNextEnvironment, nextEnvironment));

      // remove a ply if the transition was a dummy move:
      if (nextEnvironment.hasFreeMove(TEAM_A) || nextEnvironment.hasFreeMove(TEAM_B)) {
        iPly--;
      }

      // print the state that occurs:
      if (cfg_.verbosity >= 3) {
        printStateTransition(turnLog.back(), iPly);
      }

      // perform the state transition:
      stateData = nextEnvironment.data();
    } else { // NO state transition was chosen! Redo the current state:
      iPly--;
    } // endof state transition
  } // endOf foreach turn

  // print terminal state:
  GameResult result = digestGame(turnLog, matchState);
  if (cfg_.verbosity >= 2) { printGameOutline(result, iMatch); }

  return result;
} // endof rollout_game


Turn Game::digestTurn(
    const std::array<PlannerResult, 2>& actions,
    size_t resultingState,
    const ConstEnvironmentPossible& envP) {
  Turn cTurn{};

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const PlannerResult& action = actions[iTeam];
    // set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
    fpType simpleFitness = eval_->calculateFitness(envP.getEnv(), iTeam).fitness;
    fpType initialFitness, finalFitness;
    if (action.hasSolution()) {
        initialFitness = action.atDepth.front().fitness.value();
        finalFitness = action.atDepth.back().fitness.value();
    } else {
      initialFitness = simpleFitness;
      finalFitness = simpleFitness;
    }

    cTurn.activePokemon[iTeam] = (uint32_t) envP.getEnv().getTeam(iTeam).getICPKV();
    cTurn.simpleFitness[iTeam] = simpleFitness;
    cTurn.depth0Fitness[iTeam] = initialFitness;
    cTurn.depthMaxFitness[iTeam] = finalFitness;
  } // endOf foreach team

  cTurn.action[TEAM_A] = actions[TEAM_A].bestAgentAction();
  cTurn.action[TEAM_B] = actions[TEAM_B].bestAgentAction();
  cTurn.stateSelected = (uint32_t) resultingState;
  cTurn.env = envP.data();

  return cTurn;
} // endOf digestTurn


GameResult Game::digestGame(const std::vector<Turn>& cLog, int endStatus) {
  GameResult cResult{};
  if (cfg_.storeSubcomponents) { cResult.log = cLog; }

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
  std::array<fpType, 2> prevSimpleFitness{};
  std::array<fpType, 2> prev0Fitness{};
  std::array<fpType, 2> prevMaxFitness{};
  if (cLog.size() > 0) { // first turn:
    const Turn& fTurn = cLog.at(0); // TODO(@drendleman) - what if match begins at a terminal state?
    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      // set previous values to fitnesses of the first ply:
      prevSimpleFitness[iTeam] = fTurn.simpleFitness[iTeam];
      prev0Fitness[iTeam] = fTurn.depth0Fitness[iTeam];
      prevMaxFitness[iTeam] = fTurn.depthMaxFitness[iTeam];
  
      // add participation for first moving pokemon: (or lead twice)
      participation[iTeam][fTurn.activePokemon[iTeam]] += 1;
      // add a move increment for the first moving pokemon:
      if (fTurn.action[iTeam].isMove()) {
        moveUse[iTeam][fTurn.activePokemon[iTeam]][ fTurn.action[iTeam].iMove() ] += 1;
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
      if (cTurn.action[iTeam].isMove()) {
        moveUse[iTeam][cTurn.activePokemon[iTeam]][cTurn.action[iTeam].iMove()] += 1;
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
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    std::array<bool, 6> rankedPokemon{};
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
  HeatResult hResult{};
  if (cfg_.storeSubcomponents) { hResult.gameResults = gLog; }
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

  hResult.numPlies = 0;
  hResult.matchesTotal = cfg_.maxMatches;
  hResult.matchesPlayed = std::count_if(
      begin(gLog), end(gLog), [](auto& log){return log.isPlayed();});

  // compute final score:
  for(const auto& log: gLog) {
    // add a point for the winning team:
    incrementScore(log.endStatus, hResult.score);
    // accumulate average numPlies:
    hResult.numPlies += (fpType) log.numPlies;
  }
  hResult.numPlies /= hResult.matchesPlayed;

  // generate average values:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    for (size_t iPokemon = 0; iPokemon < 6; ++iPokemon) {
      for(const auto& cLog: gLog) {
        if (!cLog.isPlayed()) { continue; }
        participation[iTeam][iPokemon] += (fpType) ((fpType)cLog.participation[iTeam][iPokemon] / (fpType)cLog.numPlies);
        avgRanking[iTeam][iPokemon] += (fpType)cLog.ranking[iTeam][iPokemon];
        aggregateContribution[iTeam][iPokemon] += cLog.aggregateContribution[iTeam][iPokemon];

      } // endOf forEach log

      participation[iTeam][iPokemon] /= hResult.matchesPlayed;
      avgRanking[iTeam][iPokemon] /= hResult.matchesPlayed;
      aggregateContribution[iTeam][iPokemon] /= hResult.matchesPlayed;
    } // endOf forEach pokemon
  } // endOf forEach team 

  // create aggregate ranking:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    std::array<bool, 6> rankedPokemon;
    rankedPokemon.fill(false);
    for (size_t iRank = 0; iRank < nv_->getTeam(iTeam).getNumTeammates(); ++iRank) {
      fpType currentBestR = std::numeric_limits<fpType>::infinity();
      fpType currentBestA = -std::numeric_limits<fpType>::infinity();
      size_t iCurrentBest = SIZE_MAX;

      for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon) {
        // don't compare if already ranked:
        if (rankedPokemon[iPokemon] == true) { continue; }

        if (mostlyLT(avgRanking[iTeam][iPokemon], currentBestR) || 
          (mostlyLTE(avgRanking[iTeam][iPokemon], currentBestR) &&
          mostlyGTE(aggregateContribution[iTeam][iPokemon], currentBestA))) {
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

  return hResult;
} // endOf digestMatch


void Game::incrementScore(int matchState, std::array<uint32_t, 2>& score) {
  switch (matchState) {
    case MATCH_TEAM_A_WINS:
    case MATCH_TEAM_B_WINS:
      #pragma omp atomic
      score[matchState]++;
      break;
    default:
    case MATCH_DRAW:
    case MATCH_TIE:
      break;
  }
}


void Game::printAction(
    const ConstTeamVolatile& cTeam, const Action& action, unsigned int iTeam) const {
  std::stringstream out;
  if (action.isMove()) {
    out
      << getPokemonIdentifier(cTeam, iTeam) << " used "
      << action.iMove() << "-"
      << cTeam.getPKV().getMV(action)
      << "!\n";
  } else if (action.isSwitch()) {
    out
      << getPokemonIdentifier(cTeam, iTeam) << " is switching out with "
      << action.friendlyTarget() << ": "
      << cTeam.teammate(action.friendlyTarget() - Action::FRIENDLY_0).nv().getName() << "!\n";
  } else if (action.isWait()) {
    out
      << getPokemonIdentifier(cTeam, iTeam) << " waited for a turn!\n";
  } else {
    out
      << getPokemonIdentifier(cTeam, iTeam) << " chose unknown action "
      << action << "!\n";
  }
  // if the current pokemon is dead and switching out, print their team:
  if (!cTeam.getPKV().isAlive()) {
    cTeam.printTeam(out, "    ");
  }
  std::cout << out.str();
}


std::string Game::getGameIdentifier(size_t iMatch) const {
  std::ostringstream gameIdentifier;
  if (iMatch != SIZE_MAX) {
    gameIdentifier << "game " << (iMatch+1) << " of " << cfg_.maxMatches;
  } else {
    gameIdentifier << "the game";
  }

  return gameIdentifier.str();
}


std::string Game::getTeamIdentifier(size_t iTeam) const {
  std::ostringstream teamIdentifier;
  teamIdentifier <<
      "T" << (iTeam==TEAM_A?"A":"B") <<
      ": " << agents_[iTeam]->getName() <<
      " - " << nv_->getTeam(iTeam).getName();

  return teamIdentifier.str();
}


std::string Game::getPokemonIdentifier(const ConstTeamVolatile& cTeam, size_t iTeam) const {
  std::ostringstream pkIdentifier;
  pkIdentifier
      << "T" << (iTeam==TEAM_A?"A":"B") <<": "
      << cTeam.nv().getName() << " - "
      << cTeam.getICPKV() << ": "
      << cTeam.getPKV().nv().getName();

  return pkIdentifier.str();
}


void Game::printStateTransition(const Turn& cTurn, size_t iPly) const {
  std::stringstream out;
  out << "ply " << iPly << ", "
      << "s=" << cTurn.stateSelected << ", ";

  ConstEnvironmentPossible{*nv_, cTurn.env}.printState(out);
  out << "\n";

  std::cout << out.str();
}


void Game::printGameStart(size_t iMatch) const {
  std::stringstream out;
  std::string gameIdentifier = getGameIdentifier(iMatch);
  
  out <<
    "\nBegin " << gameIdentifier << 
    " between teams " << getTeamIdentifier(TEAM_A) <<
    " and " << getTeamIdentifier(TEAM_B) <<
    ((cfg_.verbosity>=3)?"!\n\n":"!\n");
  std::cout << out.str();
}


void Game::printGameOutline(const GameResult& gResult, size_t iMatch) const {
  std::stringstream out;
  std::string gameIdentifier = getGameIdentifier(iMatch);
  int matchState = gResult.endStatus;

  if (matchState == MATCH_TIE) {
    out <<
      "Teams " << getTeamIdentifier(TEAM_A) <<
      " and " << getTeamIdentifier(TEAM_B) <<
      " have tied " << gameIdentifier <<
      "!\n";
  } else if (matchState == MATCH_DRAW) {
    out <<
      "Teams " << getTeamIdentifier(TEAM_A) <<
      " and " << getTeamIdentifier(TEAM_B) <<
      " have drawn " << gameIdentifier <<
      "!\n";
  } else {
    size_t losingTeam = (gResult.endStatus + 1) % 2;
    out <<
      "Team " << getTeamIdentifier(matchState) <<
      " has beaten team " << getTeamIdentifier(losingTeam) <<
      " in " << gameIdentifier <<
      "!\n";
  }

  out
    << "--- GAME STATISTICS ---\n "
    << gResult.numPlies << " plies total\n"
    " Leaderboard: (index: name - (rank) aScore sScore participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    out
      << "  " << getTeamIdentifier(iTeam)
      << (((int)iTeam==gResult.endStatus)?" (winner)":"")
      << "\n";
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      out
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
  std::cout << out.str();
}


void Game::printHeatStart() const {
  std::stringstream out;
  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    out << "Team " << getTeamIdentifier(iTeam) << ":\n";
    ConstEnvironmentVolatile{*nv_, initialState_}.getTeam(iTeam).printTeam(out, "    ");
  }

  std::cout << out.str();
}


void Game::printHeatOutline(const HeatResult& result) const {
  std::stringstream out;
  if (result.endStatus == MATCH_TIE) {
    out <<
      ((cfg_.verbosity>=3)?"\n":"") <<
      "Teams " << getTeamIdentifier(TEAM_A) <<
      " and " << getTeamIdentifier(TEAM_B) <<
      " have tied the bo" << cfg_.maxMatches <<
      " series " << result.score[TEAM_A] <<
      " to " << result.score[TEAM_B] <<
      ((cfg_.verbosity>=3)?"!\n\n":"!\n");
  } else if (result.endStatus == MATCH_DRAW) {
    out <<
      ((cfg_.verbosity>=3)?"\n":"") <<
      "Teams " << getTeamIdentifier(TEAM_A) <<
      " and " << getTeamIdentifier(TEAM_B) <<
      " have drawn the bo" << cfg_.maxMatches <<
      " series " << result.score[TEAM_A] <<
      " to " << result.score[TEAM_B] <<
      ((cfg_.verbosity>=3)?"!\n\n":"!\n");
  } else {
    int matchState = result.endStatus;
    size_t losingTeam = (matchState + 1) % 2;
    out <<
      ((cfg_.verbosity>=3)?"\n":"") <<
      "Team " << getTeamIdentifier(matchState) <<
      " has beaten team " << getTeamIdentifier(losingTeam) <<
      " , winning the bo" << cfg_.maxMatches <<
      " series " << result.score[matchState] <<
      " to " << result.score[losingTeam] <<
      ((cfg_.verbosity>=3)?"!\n\n":"!\n");
  }

  out
    << "--- MATCH STATISTICS ---\n "
    << result.matchesPlayed << " out of " << result.matchesTotal << " games played\n "
    << "final score: " << result.score[0] << " to " << result.score[1] << "\n "
    << result.numPlies << " average plies per game\n"
    " Leaderboard: (index: name - (rank) avG-score avG-participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    out
      << "  " << getTeamIdentifier(iTeam)
      << (((int)iTeam==result.endStatus)?" (winner)":"")
      << "\n";
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      out
        << "    "
        << iPokemon << ": "
        << cTeam.teammate(iPokemon).getName() << " - ("
        << (result.ranking[iTeam][iPokemon] + 1) << ") "
        << result.aggregateContribution[iTeam][iPokemon] << " "
        << result.participation[iTeam][iPokemon]
        << "\n";
    }
  }
  std::cout << out.str();
}
