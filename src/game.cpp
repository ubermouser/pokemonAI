#include "pokemonai/game.h"

#include <fmt/color.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <omp.h>

#include <algorithm>
#include <boost/program_options.hpp>
#include <iostream>
#include <memory>
#include <numeric>
#include <sstream>
#include <stdexcept>

#include "pokemonai/engine.h"
#include "pokemonai/evaluator.h"
#include "pokemonai/evaluator_simple.h"
#include "pokemonai/fp_compare.h"
#include "pokemonai/pkCU.h"
#include "pokemonai/planner.h"
#include "pokemonai/planner_max.h"
#include "pokemonai/state_transition_printer.h"

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


Game* Game::clone() const {
  Game* newGame = new Game(*this);
  for (size_t iAgent = 0; iAgent < 2; ++iAgent) {
    if (!agents_[iAgent]) { continue; }
    newGame->agents_[iAgent] = std::shared_ptr<Planner>(agents_[iAgent]->clone());
  }
  if (cu_) {
    newGame->cu_ = std::shared_ptr<PkCU>(cu_->clone());
  }
  if (eval_) {
    newGame->eval_ = std::shared_ptr<Evaluator>(eval_->clone());
  }
  return newGame;
}


Game& Game::setEnvironment(const std::shared_ptr<const EnvironmentNonvolatile>& nv) {
  if (nv_ != nv) {
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
  }
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
  agents_[iAgent]->setTeam((TEAM)iAgent);

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
    SPDLOG_ERROR("One or more teams are undefined!");
    throw std::runtime_error("team(s) undefined");
  }

  // initialize pkCU engine
  if (cu_ == NULL) { setEngine(PkCU()); }

  // initialize evaluator:
  if (eval_ == NULL) {setEvaluator(EvaluatorSimple().setEngine(cu_)); }
  try {
    eval_->initialize();
  } catch (const std::exception& e) {
    SPDLOG_ERROR("Game-state evaluator : \"{}\" failed to initialize!", eval_->getName());
    SPDLOG_ERROR("{}", e.what());
    throw std::runtime_error("game-state evaluator failed to initialize");
  }
  // assign default agents if none exist:
  for (size_t iAgent = 0; iAgent < 2; ++iAgent) {
    auto& agent = agents_[iAgent];
    if (agent == NULL) {
      if (!cfg_.allowUndefinedAgents) { throw std::runtime_error("agent(s) undefined"); }

      setPlanner(iAgent, PlannerMax().setEngine(cu_).setEvaluator(eval_));
      SPDLOG_ERROR("agent {} is undefined! Replaced with {}", iAgent, agent->getName());
    }

    try {
      agent->initialize();
    } catch (const std::exception& e) {

      SPDLOG_ERROR("Agent {}: \"{}\" failed to initialize!", iAgent, agent->getName());
      SPDLOG_ERROR("{}", e.what());
      throw std::runtime_error("agent(s) failed to initialize");
    }
  }
  
  // number of matches to be played must be sane:
  if ((cfg_.maxMatches & 1) == 0) { cfg_.maxMatches += 1; }

  if (cfg_.numThreads == SIZE_MAX) {
    cfg_.numThreads = omp_get_num_procs();
    SPDLOG_WARN("Game thread parallelism set to {}!", cfg_.numThreads);
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
  EnvironmentPossibleData stateData = EnvironmentPossibleData::create(initialState);
  ConstEnvironmentPossible envP{*nv_, stateData};
  std::vector<Turn> turnLog;
  turnLog.reserve(cfg_.maxPlies + nv_->getNumPokemon());
  turnLog.push_back(digestInitialState(envP));
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
        printAction(envP.getEnv(), actions[iTeam].bestAgentAction());
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
      possibleEnvironments.printStates(out, fmt::format("ply={} ", iPly));
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
      if (nextEnvironment.flagsFor(TEAM_A).isFreeMove() ||
          nextEnvironment.flagsFor(TEAM_B).isFreeMove()) {
        iPly--;
      }

      // print the state that occurs:
      if (cfg_.verbosity >= 3) {
        printStateTransition(stateData.env, turnLog.back(), iPly);
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


Turn Game::digestInitialState(const ConstEnvironmentPossible& envP) const {
  Turn initialTurn{};
  initialTurn.env = envP.data();
  return initialTurn;
}


Turn Game::digestTurn(
    const std::array<PlannerResult, 2>& actions,
    size_t resultingState,
    const ConstEnvironmentPossible& envP) const {
  Turn cTurn{};

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    auto& turn = cTurn.teams[iTeam];
    const PlannerResult& action = actions[iTeam];
    // set simple fitness to fitness as it would be evaluated depth 0 by the simple non perceptron evaluation function
    turn.simpleFitness = eval_->evaluate(envP, iTeam).fitness.lowerBound();
    if (!action.atDepth.empty()) {
      // simple, d-0 and d-M fitness at the BEGINNING of the turn:
        turn.depth0Fitness = action.atDepth.front().fitness.value();
        turn.depthMaxFitness = action.atDepth.back().fitness.value();
        turn.timeSpent = action.atDepth.back().timeSpent;
        if (!std::isfinite(turn.depth0Fitness)) {
          turn.depth0Fitness = turn.simpleFitness;
        }
        if (!std::isfinite(turn.depthMaxFitness)) {
          turn.depthMaxFitness = turn.simpleFitness;
        }
    } else {
      turn.depth0Fitness = turn.simpleFitness;
      turn.depthMaxFitness = turn.simpleFitness;
    }
    // sum of all nodes evaluated:
    turn.numNodesEvaluated = std::accumulate(
        std::begin(action.atDepth),
        std::end(action.atDepth),
        0U,
        [](auto a, auto& b) { return a + b.numNodes; });
    // action taken by each team to transition the previous turn to the current turn:
    turn.action = actions[iTeam].bestAgentAction();
  } // endOf foreach team

  // resulting environment:
  cTurn.env = envP.data();
  cTurn.stateSelected = resultingState;
  // was the transition a free one?
  cTurn.freeTurn = envP.flagsFor(TEAM_A).isFreeMove() ||
                   envP.flagsFor(TEAM_B).isFreeMove();

  return cTurn;
} // endOf digestTurn


GameResult Game::digestGame(
    std::vector<Turn>& cLog, const ConstEnvironmentVolatile& initialState, int endStatus) const {
  GameResult cResult{};

  // encounter tracking:
  for (size_t iTurn = 1; iTurn < cLog.size(); ++iTurn) {
    digestGameEncounters(cResult, cLog[iTurn - 1], cLog[iTurn]);
  }

  // foreach team:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];
    // for each turn:
    for (const auto& turn : cLog) {
      const auto& tTurn = turn.teams[iTeam];
      // increment turns evaluated and time spent:
      team.numNodesEvaluated += tTurn.numNodesEvaluated;
      team.timeSpent += tTurn.timeSpent;

      for (const auto& [actor, action] : tTurn.action) {
        auto& pokemon = team.pokemon[actor.iTeammate()];
        // for every turn a pokemon is in play, this increases a counter for
        // that pokemon by 1:
        pokemon.participation += 1;
        // add a move increment for the current pokemon's move:
        if (action.isMove()) { pokemon.moveUse[action.iMove()] += 1; }
      }
    }
  }

  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    auto& team = cResult.teams[iTeam];
    // terminal state fitness:
    ConstEnvironmentVolatile terminalState = cLog.size()>0?ConstEnvironmentVolatile{*nv_, cLog.back().env.env}:initialState;
    team.lastSimpleFitness = eval_->evaluate(terminalState, iTeam).fitness.lowerBound();
    // delta fitness change:
    for (size_t iPly = 1; iPly < cLog.size(); ++iPly) {
      // the previous turn. The previous turn was responsible for the delta between turn n-1 and n
      const auto& pTTurn = cLog[iPly - 1].teams[iTeam];
      // the current turn. Used for updating delta
      const auto& cTTurn = cLog[iPly].teams[iTeam];
      // increase contribution fractionals:
      for (const auto& [actor, action] : pTTurn.action) {
        auto& pokemon = team.pokemon[actor.iTeammate()];
        pokemon.simpleContribution +=
            cTTurn.simpleFitness - pTTurn.simpleFitness;
        pokemon.d0Contribution += cTTurn.depth0Fitness - pTTurn.depth0Fitness;
        pokemon.dMaxContribution +=
            cTTurn.depthMaxFitness - pTTurn.depthMaxFitness;
      }
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
  cResult.numPlies = cLog.size() > 0 ? cLog.size() - 1 : 0;
  if (cfg_.storeSubcomponents) {
    cResult.log = std::move(cLog);
    cResult.log.shrink_to_fit();
  }

  return cResult;
} // endOf digestGame


void Game::digestGameEncounters(
    GameResult& cResult,
    const Turn& previousTurn,
    const Turn& currentTurn) const {
  auto updateEncounters =
      [&](const ActionMap& actions, const ActionMap& oppActions, size_t iTeam) {
        for (const auto& [actor, action] : actions) {
          for (const auto& [oppActor, oppAction] : oppActions) {
            cResult.teams[iTeam]
                .pokemon[actor.iTeammate()]
                .encounters[oppActor.iTeammate()]
                .numTotal++;
          }
        }
      };
  updateEncounters(
      previousTurn.teams[TEAM_A].action,
      previousTurn.teams[TEAM_B].action,
      TEAM_A);
  updateEncounters(
      previousTurn.teams[TEAM_B].action,
      previousTurn.teams[TEAM_A].action,
      TEAM_B);

  ConstEnvironmentPossible env_prev{*nv_, previousTurn.env};
  ConstEnvironmentPossible env_curr{*nv_, currentTurn.env};

  auto updateStats = [&](size_t iTeam) {
    for (const auto& [actor, action] : currentTurn.teams[iTeam].action) {
      size_t pk_start = actor.iTeammate();
      bool wasAlive = env_prev.getTeam(iTeam).teammate(pk_start).isAlive();
      bool isAlive = env_curr.getTeam(iTeam).teammate(pk_start).isAlive();

      if (wasAlive && !isAlive) {
        // Find which opponent KO'd this pokemon? For now just say all active
        // opponents.
        for (const auto& [oppActor, oppAction] :
             currentTurn.teams[1 - iTeam].action) {
          cResult.teams[1 - iTeam]
              .pokemon[oppActor.iTeammate()]
              .encounters[pk_start]
              .numKOs++;
        }
      } else if (action.isSwitch()) {
        // This is a bit simplified, but if ANY action for this actor was a
        // switch, count it.
        cResult.teams[iTeam]
            .pokemon[pk_start]
            .encounters[0]
            .numSwitches++;  // simplified opp index
      }
    }
  };

  updateStats(TEAM_A);
  updateStats(TEAM_B);
}


HeatResult Game::digestMatch(std::vector<GameResult>& gLog) const {
  // initialize heatResult:
  HeatResult hResult{};

  hResult.nv = nv_;
  hResult.numPlies = 0;
  hResult.matchesTotal = cfg_.maxMatches;
  hResult.matchesPlayed = std::count_if(
      begin(gLog), end(gLog), [](auto& log){return log.isPlayed();});

  // generate average team and total values:
  for(const auto& log: gLog) {
    // add a point for the winning team:
    incrementScore(log.endStatus, hResult.score);
    // accumulate average numPlies:
    hResult.numPlies += log.numPlies;

    for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      const auto& source = log.teams[iTeam];
      auto& team = hResult.teams[iTeam];
      team.lastSimpleFitness += source.lastSimpleFitness;
      team.averageNodesEvaluated += source.numNodesEvaluated;
      team.averageTimeSpent += source.timeSpent;
    }
  }
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

        for (size_t iOpponent = 0; iOpponent < 6; ++iOpponent) {
          pokemon.encounters[iOpponent].numKOs +=
              source.encounters[iOpponent].numKOs;
          pokemon.encounters[iOpponent].numSwitches +=
              source.encounters[iOpponent].numSwitches;
          pokemon.encounters[iOpponent].numTotal +=
              source.encounters[iOpponent].numTotal;
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

std::string Game::getGameIdentifier(size_t iMatch) const {
  if (iMatch != SIZE_MAX) {
    return fmt::format("game {} of {}", iMatch + 1, cfg_.maxMatches);
  }
  return "the game";
}

std::string Game::getTeamIdentifier(size_t iTeam) const {
  return fmt::format(
      "T{}: {} - {}",
      (iTeam == TEAM_A ? "A" : "B"),
      agents_[iTeam]->getName(),
      nv_->getTeam(iTeam).getName());
}


std::string Game::getPokemonIdentifier(
    const ConstEnvironmentVolatile& env, const Actor& actor) const {
  return fmt::format(
      "{}: {} - {}",
      fmt::streamed(actor),
      env.getTeam(actor.iTeam()).nv().getName(),
      env.teammate(actor).nv().getName());
}


void Game::printAction(
    const ConstEnvironmentVolatile& env,
    const ActionMap& actionMap) const {
  for (const auto& [actor, action] : actionMap) {
    std::string result;
    if (action.isMove()) {
      result = fmt::format(
          "{} used {}-{}!\n",
          getPokemonIdentifier(env, actor),
          action.iMove() + 1,
          fmt::streamed(env.teammate(actor).getMV(action)));
    } else if (action.isSwitch()) {
      result = fmt::format(
          "{} is switching out with {}: {}!\n",
          getPokemonIdentifier(env, actor),
          action.friendlyTarget() + 1,
          env.teammate(actor.iTeam(), action.iFriendly())
              .nv()
              .getName());
    } else if (action.isWait()) {
      result = fmt::format(
          "{} waited for a turn!\n",
          getPokemonIdentifier(env, actor));
    } else {
      result = fmt::format(
          "{} chose unknown action {}!\n",
          getPokemonIdentifier(env, actor),
          fmt::streamed(action));
    }
    // if the current pokemon is dead and switching out, print their team:
    if (!env.teammate(actor).isAlive()) {
      std::ostringstream team_out;
      env.getTeam(actor.iTeam()).printTeam(team_out, "    ");
      result += team_out.str();
    }
    fmt::print("{}", result);
  }
}


void Game::printStateTransition(
    const EnvironmentVolatileData& oldState,
    const Turn& cTurn,
    size_t iPly) const {
  if (iPly != SIZE_MAX) {
    fmt::print(fmt::emphasis::bold, "--- Turn {} ---\n", iPly + 1);
  }

  StateTransitionPrinter::print(
      std::cout,
      ConstEnvironmentVolatile{*nv_, oldState},
      ConstEnvironmentPossible{*nv_, cTurn.env});

  ConstEnvironmentPossible{*nv_, cTurn.env}.printState(std::cout);
}


void Game::printGameStart(size_t iMatch) const {
  fmt::print(
      "\nBegin {} between teams {} and {}{}",
      getGameIdentifier(iMatch),
      getTeamIdentifier(TEAM_A),
      getTeamIdentifier(TEAM_B),
      ((cfg_.verbosity >= 3) ? "!\n\n" : "!\n"));
}

template <typename ResultType>
void printLeaderboard(
    std::string& out,
    size_t iPokemon,
    const ResultType& pResult,
    const PokemonNonVolatile& cPKNV) {
  std::ostringstream ss;
  ss << cPKNV;
  std::string pk_name = ss.str();

  out += fmt::format(
      "    {}: {:>24.24} r={}  c={:5.3f}  s={:5.3f}  p={:5.3f}  ",
      iPokemon + 1,
      pk_name,
      pResult.ranking + 1,
      pResult.aggregateContribution,
      pResult.simpleContribution,
      pResult.participation);
  for (size_t iMove = 0; iMove < cPKNV.getNumMoves(); ++iMove) {
    out += fmt::format(
        "{}={:5.3f}  ",
        fmt::streamed(Action::move(iMove)),
        pResult.moveUse[iMove]);
  }
  // struggle move:
  out += fmt::format(
      "{}={:5.3f}\n", fmt::streamed(Action::struggle()), pResult.moveUse[4]);
}

void Game::printGameOutline(const GameResult& gResult, size_t iMatch) const {
  std::string result;
  std::string gameIdentifier = getGameIdentifier(iMatch);
  int matchState = gResult.endStatus;

  if (matchState == MATCH_TIE) {
    result += fmt::format(
        "Teams {} and {} have tied {}!\n",
        getTeamIdentifier(TEAM_A),
        getTeamIdentifier(TEAM_B),
        gameIdentifier);
  } else if (matchState == MATCH_DRAW) {
    result += fmt::format(
        "Teams {} and {} have drawn {}!\n",
        getTeamIdentifier(TEAM_A),
        getTeamIdentifier(TEAM_B),
        gameIdentifier);
  } else {
    size_t losingTeam = (gResult.endStatus + 1) % 2;
    result += fmt::format(
        "Team {} has beaten team {} in {}!\n",
        getTeamIdentifier(matchState),
        getTeamIdentifier(losingTeam),
        gameIdentifier);
  }

  result += fmt::format(
      "--- GAME STATISTICS ---\n {} plies total\n"
      " Leaderboard: (index: name  r=rank  c=aggregate-score  s=simple-score  "
      "p=participation)\n",
      gResult.numPlies);

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const auto& teamResult = gResult.teams[iTeam];
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    result += fmt::format(
        "  {}{}\n",
        getTeamIdentifier(iTeam),
        ((int)iTeam == gResult.endStatus ? " (winner)" : ""));
    result += fmt::format(
        "  time={:7.2f}  nnod={}\n",
        teamResult.timeSpent,
        teamResult.numNodesEvaluated);
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      const auto& pResult = teamResult.pokemon[iPokemon];
      const PokemonNonVolatile& cPKNV = cTeam.teammate(iPokemon);
      printLeaderboard(result, iPokemon, pResult, cPKNV);
    }
  }
  fmt::print("{}", result);
}

void Game::printHeatOutline(const HeatResult& result) const {
  std::string out;
  if (result.endStatus == MATCH_TIE) {
    out += fmt::format(
        "{}Teams {} and {} have tied the bo{} series {} to {}{}",
        ((cfg_.verbosity >= 3) ? "\n" : ""),
        getTeamIdentifier(TEAM_A),
        getTeamIdentifier(TEAM_B),
        cfg_.maxMatches,
        result.score[TEAM_A],
        result.score[TEAM_B],
        ((cfg_.verbosity >= 3) ? "!\n\n" : "!\n"));
  } else if (result.endStatus == MATCH_DRAW) {
    out += fmt::format(
        "{}Teams {} and {} have drawn the bo{} series {} to {}{}",
        ((cfg_.verbosity >= 3) ? "\n" : ""),
        getTeamIdentifier(TEAM_A),
        getTeamIdentifier(TEAM_B),
        cfg_.maxMatches,
        result.score[TEAM_A],
        result.score[TEAM_B],
        ((cfg_.verbosity >= 3) ? "!\n\n" : "!\n"));
  } else {
    int matchState = result.endStatus;
    size_t losingTeam = (matchState + 1) % 2;
    out += fmt::format(
        "{}Team {} has beaten team {}, winning the bo{} series {} to {}{}",
        ((cfg_.verbosity >= 3) ? "\n" : ""),
        getTeamIdentifier(matchState),
        getTeamIdentifier(losingTeam),
        cfg_.maxMatches,
        result.score[matchState],
        result.score[losingTeam],
        ((cfg_.verbosity >= 3) ? "!\n\n" : "!\n"));
  }

  out += fmt::format(
      "--- MATCH STATISTICS ---\n {} out of {} games played\n "
      "final score: {} to {}\n "
      "{:g} average plies per game\n"
      " Leaderboard: (index: name  r=rank  aC=avG-score  "
      "aP=avG-participation)\n",
      result.matchesPlayed,
      result.matchesTotal,
      result.score[0],
      result.score[1],
      result.averagePlies());

  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    const auto& teamResult = result.teams[iTeam];
    const TeamNonVolatile& cTeam = nv_->getTeam(iTeam);
    out += fmt::format(
        "  {}{}\n",
        getTeamIdentifier(iTeam),
        (((int)iTeam == result.endStatus) ? " (winner)" : ""));
    out += fmt::format(
        "  aTime={:7.2f}  aNod={}\n",
        teamResult.averageTimeSpent,
        teamResult.averageNodesEvaluated);
    for (size_t iPokemon = 0; iPokemon < cTeam.getNumTeammates(); ++iPokemon) {
      const auto& pResult = teamResult.pokemon[iPokemon];
      const PokemonNonVolatile& cPKNV = cTeam.teammate(iPokemon);
      printLeaderboard(out, iPokemon, pResult, cPKNV);
    }
  }
  fmt::print("{}", out);
}


void Game::printHeatStart() const {
  std::string out;
  for (size_t iTeam = 0; iTeam < 2; iTeam++) {
    out += fmt::format("Team {}:\n", getTeamIdentifier(iTeam));
    std::ostringstream team_summary;
    nv_->getTeam(iTeam).printSummary(team_summary, "    ");
    out += team_summary.str();
  }

  fmt::print("{}", out);
}
