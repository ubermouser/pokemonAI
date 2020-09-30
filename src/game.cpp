#include "../inc/game.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <omp.h>
#include <numeric>

#include "../inc/planner.h"
#include "../inc/planner_max.h"
#include "../inc/evaluator.h"
#include "../inc/evaluator_simple.h"
#include "../inc/fp_compare.h"

#include "../inc/pkCU.h"
#include "../inc/engine.h"

namespace po = boost::program_options;


po::options_description Game::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "game-verbosity").c_str(),
      po::value<int>(&verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls status printing.")
      ((prefix + "max-plies").c_str(),
      po::value<size_t>(&maxPlies)->default_value(defaults.maxPlies),
      "maximum number of turns allowed before a draw occurs")
      ((prefix + "max-matches").c_str(),
      po::value<size_t>(&maxMatches)->default_value(defaults.maxMatches),
      "maximum number of matches, in best of N format")
      ((prefix + "num-threads").c_str(),
      po::value<size_t>(&numThreads)->default_value(defaults.numThreads),
      "number of threads to use when performing multiple matches")
      ((prefix + "allow-state-selection").c_str(),
      po::value<bool>(&allowStateSelection)->default_value(defaults.allowStateSelection),
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
  isInitialized_ = false;
  return *this;
}


Game& Game::clear() {
  eval_.reset();
  cu_.reset();
  nv_.reset();
  agents_[0].reset();
  agents_[1].reset();
  isInitialized_ = false;
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
    cfg_.numThreads = omp_get_num_procs();
    std::cerr << "Game thread parallelism set to " << cfg_.numThreads << "!\n";
  }

  isInitialized_ = true;
  return *this;
}


HeatResult Game::rollout(const EnvironmentVolatileData& initialState) const {
  if (!isInitialized_) { throw std::runtime_error("game not initialized"); }
  if (cfg_.verbosity >= 1) { printHeatStart(); }

  std::vector<GameResult> gameLog(cfg_.maxMatches, GameResult());
  std::array<uint32_t, 2> score{0,0};
  bool shouldContinue = true;

  auto rollout_fn = [&](size_t iMatch){
    // end early if one player clearly dominates the other (we cannot break out of a parallel loop)
    if (*std::max_element(begin(score), end(score)) > (cfg_.maxMatches / 2)) { 
      shouldContinue = false;
      return;
    }

    // perform the rollout:
    GameResult& gResult = gameLog[iMatch];
    gResult = rollout_game(initialState, iMatch);

    // increment score:
    incrementScore(gResult.endStatus, score);
  };

  if (cfg_.numThreads > 0) {
    #pragma omp parallel for num_threads(cfg_.numThreads)
    for (size_t iMatch = 0; iMatch < cfg_.maxMatches; ++iMatch) { rollout_fn(iMatch); }
  } else {
    for (size_t iMatch = 0; shouldContinue && iMatch < cfg_.maxMatches; ++iMatch) { rollout_fn(iMatch); }
  }

  HeatResult result = digestMatch(gameLog);
  if (cfg_.verbosity >= 1) { printHeatOutline(result); }
  return result;
}


GameResult Game::rollout_game(const EnvironmentVolatileData& initialState, size_t iMatch) const {
  if (!isInitialized_) { throw std::runtime_error("game not initialized"); }
  std::vector<Turn> turnLog; turnLog.reserve(cfg_.maxPlies + nv_->getNumPokemon());
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
  GameResult result = digestGame(turnLog, ConstEnvironmentVolatile{*nv_, initialState}, matchState);
  if (cfg_.verbosity >= 2) { printGameOutline(result, iMatch); }

  return result;
} // endof rollout_game


Turn Game::digestTurn(
    const std::array<PlannerResult, 2>& actions,
    size_t resultingState,
    const ConstEnvironmentPossible& envP) const {
  Turn cTurn{};

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    auto& turn = cTurn.teams[iTeam];
    const PlannerResult& action = actions[iTeam];
    // set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
    turn.simpleFitness = eval_->calculateFitness(envP.getEnv(), iTeam).fitness.lowerBound();
    if (!action.atDepth.empty()) {
      // simple, d-0 and d-M fitness at the BEGINNING of the turn:
        turn.depth0Fitness = action.atDepth.front().fitness.value();
        turn.depthMaxFitness = action.atDepth.back().fitness.value();
        turn.timeSpent = action.atDepth.back().timeSpent;
    } else {
      turn.depth0Fitness = turn.simpleFitness;
      turn.depthMaxFitness = turn.simpleFitness;
    }
    // sum of all nodes evaluated:
    turn.numNodesEvaluated = std::accumulate(
        std::begin(action.atDepth), std::end(action.atDepth), 0U, [](auto& a, auto& b) {
      return a + b.numNodes;
    });
    // active pokemon at the END of the turn:
    turn.activePokemon = envP.getEnv().getTeam(iTeam).getICPKV();
    // action taken by each team to transition the previous turn to the current turn:
    turn.action = actions[iTeam].bestAgentAction();
  } // endOf foreach team

  // resulting environment:
  cTurn.env = envP.data();
  cTurn.stateSelected = resultingState;
  // was the transition a free one?
  cTurn.freeTurn = envP.hasFreeMove(TEAM_A) || envP.hasFreeMove(TEAM_B);

  return cTurn;
} // endOf digestTurn


GameResult Game::digestGame(
    std::vector<Turn>& cLog, const ConstEnvironmentVolatile& initialState, int endStatus) const {
  GameResult cResult{};

  // foreach team:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];
    // starter pokemon participation:
    team.pokemon[initialState.getTeam(iTeam).getICPKV()].participation += 1;
    // for each turn:
    for (const auto& turn: cLog) {
      const auto& tTurn = turn.teams[iTeam];
      auto& pokemon = team.pokemon[tTurn.activePokemon];
      // increment turns evaluated and time spent:
      team.numNodesEvaluated += tTurn.numNodesEvaluated;
      team.timeSpent += tTurn.timeSpent;
      // for every turn a pokemon is in play, this increases a counter for that pokemon by 1:
      pokemon.participation += 1;
      // add a move increment for the current pokemon's move:
      if (tTurn.action.isMove()) {
        pokemon.moveUse[tTurn.action.iMove()] += 1;
      }
    }
  }

  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];
    // terminal state fitness:
    ConstEnvironmentVolatile terminalState = cLog.size()>0?ConstEnvironmentVolatile{*nv_, cLog.back().env.env}:initialState;
    team.lastSimpleFitness = eval_->calculateFitness(terminalState, iTeam).fitness.lowerBound();
    // delta fitness change:
    for (size_t iPly = 1; iPly < cLog.size(); ++iPly) {
      // the previous turn. The previous turn was responsible for the delta between turn n-1 and n
      const auto& pTTurn = cLog[iPly - 1].teams[iTeam];
      // the current turn. Used for updating delta
      const auto& cTTurn = cLog[iPly].teams[iTeam];
      auto& pokemon = team.pokemon[pTTurn.activePokemon];

      // increase contribution fractionals:
      pokemon.simpleContribution += cTTurn.simpleFitness - pTTurn.simpleFitness;
      pokemon.d0Contribution += cTTurn.depth0Fitness - pTTurn.depth0Fitness;
      pokemon.dMaxContribution += cTTurn.depthMaxFitness - pTTurn.depthMaxFitness;
    }
  }

  // create scores:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];
    for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon) {
      auto& pokemon = team.pokemon[iPokemon];

      // normalize move usage by pokemon participation
      for (size_t iMove = 0; iMove < 5 && pokemon.participation > 0; ++iMove) {
        pokemon.moveUse[iMove] /= pokemon.participation;
      }
      // normalize pokemon participation by game size
      if (cLog.size() > 0) { pokemon.participation /= (double)cLog.size(); }

      pokemon.aggregateContribution = (
          pokemon.simpleContribution * 0.35 +
          pokemon.d0Contribution * 0.05 +
          pokemon.dMaxContribution * 0.6
        ) * pokemon.participation;
    }
  }

  // create ranking:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];

    std::array<size_t, 6> ranking = {0, 1, 2, 3, 4, 5};
    std::sort(begin(ranking), end(ranking), [&](size_t a, size_t b){
      return team.pokemon[a].aggregateContribution > team.pokemon[b].aggregateContribution;
    });

    for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon) {
      team.pokemon[iPokemon].ranking = ranking[iPokemon];
    }
  } // endOf foreach team

  // set game status:
  cResult.endStatus = endStatus;
  cResult.numPlies = cLog.size();
  if (cfg_.storeSubcomponents) {
    cResult.log = std::move(cLog);
    cResult.log.shrink_to_fit();
  }

  return cResult;
} // endOf digestGame


HeatResult Game::digestMatch(std::vector<GameResult>& gLog) const {
  // initialize heatResult:
  HeatResult hResult{};

  hResult.numPlies = 0;
  hResult.matchesTotal = cfg_.maxMatches;
  hResult.matchesPlayed = std::count_if(
      begin(gLog), end(gLog), [](auto& log){return log.isPlayed();});

  // generate average team and total values:
  for(const auto& log: gLog) {
    // add a point for the winning team:
    incrementScore(log.endStatus, hResult.score);
    // accumulate average numPlies:
    hResult.numPlies += (fpType) log.numPlies;

    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      const auto& source = log.teams[iTeam];
      auto& team = hResult.teams[iTeam];
      team.lastSimpleFitness += source.lastSimpleFitness;
      team.averageNodesEvaluated += source.numNodesEvaluated;
      team.averageTimeSpent += source.timeSpent;
    }
  }
  hResult.numPlies /= hResult.matchesPlayed;
  for (auto& team: hResult.teams) {
    team.lastSimpleFitness /= hResult.matchesPlayed;
    team.averageNodesEvaluated /= hResult.matchesPlayed;
    team.averageTimeSpent /= hResult.matchesPlayed;
  }

  // generate average pokemon values:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = hResult.teams[iTeam];
    for (size_t iPokemon = 0; iPokemon < 6; ++iPokemon) {
      auto& pokemon = team.pokemon[iPokemon];
      for (const auto& cLog: gLog) {
        if (!cLog.isPlayed()) { continue; }
        const auto& source = cLog.teams[iTeam].pokemon[iPokemon];
        pokemon.participation += source.participation;
        pokemon.aggregateContribution += source.aggregateContribution;
        pokemon.simpleContribution += source.simpleContribution;

        for (size_t iMove = 0; iMove < 5; ++iMove) {
          pokemon.moveUse[iMove] += source.moveUse[iMove];
        }
      } // endOf forEach log

      if (hResult.matchesPlayed > 0) {
        for (size_t iMove = 0; iMove < 5; ++iMove) {
          pokemon.moveUse[iMove] /= hResult.matchesPlayed;
        }
        pokemon.participation /= hResult.matchesPlayed;
        pokemon.aggregateContribution /= hResult.matchesPlayed;
        pokemon.simpleContribution /= hResult.matchesPlayed;
      }
    } // endOf forEach pokemon
  } // endOf forEach team 

  // create aggregate ranking:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = hResult.teams[iTeam];

    std::array<size_t, 6> ranking = {0, 1, 2, 3, 4, 5};
    std::sort(begin(ranking), end(ranking), [&](size_t a, size_t b){
      return team.pokemon[a].aggregateContribution > team.pokemon[b].aggregateContribution;
    });

    for (size_t iPokemon = 0; iPokemon < nv_->getTeam(iTeam).getNumTeammates(); ++iPokemon) {
      team.pokemon[iPokemon].ranking = ranking[iPokemon];
    }
  } // endOf foreach team

  // variables that do not require loops:
  if (cfg_.storeSubcomponents) { 
    hResult.gameResults = std::move(gLog);
    hResult.gameResults.shrink_to_fit();
  }
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


void Game::incrementScore(int matchState, std::array<uint32_t, 2>& score) const {
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
      << (action.iMove()+1) << "-"
      << cTeam.getPKV().getMV(action)
      << "!\n";
  } else if (action.isSwitch()) {
    out
      << getPokemonIdentifier(cTeam, iTeam) << " is switching out with "
      << (action.friendlyTarget()+1) << ": "
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


template<typename ResultType> void printLeaderboard(
    std::ostream& out, size_t iPokemon, const ResultType& pResult, const PokemonNonVolatile& cPKNV) {
  out << boost::format("    %d: %24.24s r=%d  c=% 5.3f  s=% 5.3f  p=%5.3f  ")
      % (iPokemon+1)
      % cPKNV
      % (pResult.ranking + 1)
      % pResult.aggregateContribution
      % pResult.simpleContribution
      % pResult.participation;
  for (size_t iMove = 0; iMove < cPKNV.getNumMoves(); ++iMove) {
    out << boost::format("%s=%5.3f  ") % Action::move(iMove) % pResult.moveUse[iMove];
  }
  // struggle move:
  out << boost::format("%s=%5.3f\n") % Action::struggle() % pResult.moveUse[4];

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
    " Leaderboard: (index: name  r=rank  c=aggregate-score  s=simple-score  p=participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const auto& teamResult = gResult.teams[iTeam];
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    out
      << "  " << getTeamIdentifier(iTeam)
      << (((int)iTeam==gResult.endStatus)?" (winner)":"")
      << "\n";
    out << boost::format("  time=%7.2f  nnod=%d\n")
        % teamResult.timeSpent
        % teamResult.numNodesEvaluated;
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      const auto& pResult = teamResult.pokemon[iPokemon];
      const PokemonNonVolatile& cPKNV = cTeam.teammate(iPokemon);
      printLeaderboard(out, iPokemon, pResult, cPKNV);
    }
  }
  std::cout << out.str();
}


void Game::printHeatStart() const {
  std::stringstream out;
  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    out << "Team " << getTeamIdentifier(iTeam) << ":\n";
    nv_->getTeam(iTeam).printSummary(out, "    ");
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
      ", winning the bo" << cfg_.maxMatches <<
      " series " << result.score[matchState] <<
      " to " << result.score[losingTeam] <<
      ((cfg_.verbosity>=3)?"!\n\n":"!\n");
  }

  out
    << "--- MATCH STATISTICS ---\n "
    << result.matchesPlayed << " out of " << result.matchesTotal << " games played\n "
    << "final score: " << result.score[0] << " to " << result.score[1] << "\n "
    << result.numPlies << " average plies per game\n"
    " Leaderboard: (index: name  r=rank  aC=avG-score  aP=avG-participation)\n";

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const auto& teamResult = result.teams[iTeam];
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    out
      << "  " << getTeamIdentifier(iTeam)
      << (((int)iTeam==result.endStatus)?" (winner)":"")
      << "\n";
    out << boost::format("  aTime=%7.2f  aNod=%d\n")
        % teamResult.averageTimeSpent
        % teamResult.averageNodesEvaluated;
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      const auto& pResult = teamResult.pokemon[iPokemon];
      const PokemonNonVolatile& cPKNV = cTeam.teammate(iPokemon);
      printLeaderboard(out, iPokemon, pResult, cPKNV);
    }
  }
  std::cout << out.str();
}
