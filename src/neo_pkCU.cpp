#include "pokemonai/neo_pkCU.h"

#include <stdexcept>

#include "pokemonai/environment_volatile.h"
#include "pokemonai/neo_pkCU_engine.h"
#include "pokemonai/pkCU_types.h"
#include "pokemonai/pluggable_types.h"


namespace po = boost::program_options;


NeoPkCU::NeoPkCU(const Config& cfg) : cfg_(cfg) {}


NeoPkCU::NeoPkCU(const NeoPkCU& other) = default;


NeoPkCU::~NeoPkCU() {}


NeoPkCU* NeoPkCU::clone() const {
    return new NeoPkCU(*this);
}


boost::program_options::options_description NeoPkCU::Config::options(
    const std::string& category, std::string prefix) {
    po::options_description desc{category};
    // Add stub options if needed
    return desc;
}


NeoPkCU& NeoPkCU::setEnvironment(
    const std::shared_ptr<const EnvironmentNonvolatile>& cEnv) {
  nv_ = cEnv;
  initialState_ = createInitialVolatileState();
  initialize();
  return *this;
}


NeoPkCU& NeoPkCU::setEnvironment(const EnvironmentNonvolatile& cEnv) {
    return setEnvironment(std::make_shared<const EnvironmentNonvolatile>(cEnv));
}


NeoPkCU& NeoPkCU::setAccuracy(size_t engineAccuracy) {
    cfg_.numRandomEnvironments = engineAccuracy;
    return *this;
}


NeoPkCU& NeoPkCU::setNumActivePokemon(size_t numActivePokemon) {
  cfg_.numActivePokemon = numActivePokemon;
  return *this;
}


NeoPkCU& NeoPkCU::setReturnAllStates(bool returnAllStates) {
  cfg_.returnAllStates = returnAllStates;
  return *this;
}


NeoPkCU& NeoPkCU::setAllowInvalidMoves(bool allow) {
  cfg_.allowInvalidMoves = allow;
  return *this;
}


void NeoPkCU::guardNonvolatileState(
    const ConstEnvironmentVolatile& cEnv) const {
  if (cEnv.nv_ != nv_.get()) {
    throw std::invalid_argument(
        "mismatched nonvolatile state - call setEnvironment first");
  }
}


void NeoPkCU::guardCorrectActionCount(
    const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const {
  size_t numActivePokemon = cEnv.getNumActivePokemon();
  if (numActivePokemon != actions.size()) {
    throw std::invalid_argument(fmt::format(
        "wrong number of actions: expected {}, got {}",
        numActivePokemon,
        actions.size()));
  }
}


void NeoPkCU::guardInvalidActions(
    const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const {
  if (cfg_.allowInvalidMoves) { return; }
  for (auto& action : actions) {
    auto reason = isValidAction(cEnv, action.first, action.second);
    if (!reason) {
      throw std::invalid_argument(fmt::format(
          "Invalid Action {} for {} : {}",
          fmt::streamed(action.second),
          fmt::streamed(action.first),
          invalidActionReasonToString(reason)));
    }
  }
}


EnvironmentVolatileData NeoPkCU::createInitialVolatileState() const {
  if (!nv_) throw std::runtime_error("NeoPkCU environment not set");
  EnvironmentVolatileData initialState = EnvironmentVolatileData::create(*nv_);
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    size_t numTeammatesToActivate =
        std::min(cfg_.numActivePokemon, nv_->getTeam(iTeam).getNumTeammates());

    for (size_t iTeammate = 0; iTeammate < numTeammatesToActivate;
         ++iTeammate) {
      initialState.teams[iTeam].teammates[iTeammate].active = 1;
    }
  }

  // TODO - beginning of game plugins

  return initialState;
}


PossibleEnvironments NeoPkCU::updateState(
    const ConstEnvironmentVolatile& cEnv,
    const Action& actionA,
    const Action& actionB) const {
  ActionMap actions{
      {{TEAM_A, cEnv.getTeam(0).getICPKV()}, actionA},
      {{TEAM_B, cEnv.getTeam(1).getICPKV()}, actionB}};
  return updateState(cEnv, actions);
}


PossibleEnvironments NeoPkCU::updateState(
    const ConstEnvironmentPossible& cEnvP,
    const Action& actionA,
    const Action& actionB) const {
  return updateState(cEnvP.getEnv(), actionA, actionB);
}


PossibleEnvironments NeoPkCU::updateState(
    const ConstEnvironmentVolatile& cEnv,
    const ActionMap& actionsA,
    const ActionMap& actionsB) const {
  ActionMap actions;
  actions.insert(actionsA.begin(), actionsA.end());
  actions.insert(actionsB.begin(), actionsB.end());
  return updateState(cEnv, actions);
}


PossibleEnvironments NeoPkCU::updateState(
    const ConstEnvironmentVolatile& cEnv, const ActionMap& actions) const {
  guardNonvolatileState(cEnv);
  guardCorrectActionCount(cEnv, actions);
  guardInvalidActions(cEnv, actions);

  NeoPkCUEngine engine(*this, cEnv.data(), actions);
  PossibleEnvironments result = engine.updateState();

  return std::move(result);
}


ConstEnvironmentVolatile NeoPkCU::initialState() const {
    if (!nv_) throw std::runtime_error("NeoPkCU environment not set");
    return ConstEnvironmentVolatile{*nv_, initialState_};
}


void NeoPkCU::initialize() {
  if (!nv_) return;

  // Clear existing plugins
  for (auto& set : pluginSet_) {
    set.clear();
  }

  // Add plugins from all teammates on both teams
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    const auto& team = nv_->getTeam(iTeam);
    for (size_t iTeammate = 0; iTeammate < team.getNumTeammates(); ++iTeammate) {
      const auto& pokemon = team.teammate(iTeammate);

      // Moves
      for (size_t iMove = 0; iMove < pokemon.getNumMoves(); ++iMove) {
        const auto& move = pokemon.getMove_base(iMove);
        for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
          plugin_t p = move.getPlugin(iPlugin);
          if (p.pFunction) {
            pluginSet_[iPlugin].push_back(p);
          }
        }
      }

      // Ability
      if (pokemon.abilityExists()) {
        const auto& ability = pokemon.getAbility();
        for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
          plugin_t p = ability.getPlugin(iPlugin);
          if (p.pFunction) {
            pluginSet_[iPlugin].push_back(p);
          }
        }
      }

      // Items
      if (pokemon.hasInitialItem()) {
        const auto& item = pokemon.getInitialItem();
        for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
          plugin_t p = item.getPlugin(iPlugin);
          if (p.pFunction) {
            pluginSet_[iPlugin].push_back(p);
          }
        }
      }
    }
  }

  // Add global extensions from Pokedex
  if (pkdex) {
    const auto& extensions = pkdex->getExtensions();
    for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
      for (size_t i = 0; i < extensions.getNumPlugins(iPlugin); ++i) {
        pluginSet_[iPlugin].push_back(extensions.getPlugin(iPlugin, i));
      }
    }
  }

  // Sort by priority
  for (auto& set : pluginSet_) {
    std::sort(set.begin(), set.end());
  }
}


ActionVector NeoPkCU::getValidActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
  ActionVector result;
  for (size_t iMove = 0; iMove < 4; ++iMove) {
    Action action = Action::move(iMove);
    if (isValidAction(envV, action, iTeam)) { result.push_back(action); }
  }
  for (size_t iTeammate = 0; iTeammate < 6; ++iTeammate) {
    Action action = Action::swap(iTeammate);
    if (isValidAction(envV, action, iTeam)) { result.push_back(action); }
  }
  if (isValidAction(envV, Action::wait(), iTeam)) {
    result.push_back(Action::wait());
  }
  if (result.empty()) { result.push_back(Action::struggle()); }
  return result;
}


ActionVector NeoPkCU::getValidMoveActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
  ActionVector result;
  for (size_t iMove = 0; iMove < 4; ++iMove) {
    Action action = Action::move(iMove);
    if (isValidAction(envV, action, iTeam)) { result.push_back(action); }
  }
  return result;
}


ActionVector NeoPkCU::getValidSwapActions(const ConstEnvironmentVolatile& envV, size_t iTeam) const {
  ActionVector result;
  for (size_t iTeammate = 0; iTeammate < 6; ++iTeammate) {
    Action action = Action::swap(iTeammate);
    if (isValidAction(envV, action, iTeam)) { result.push_back(action); }
  }
  return result;
}


ActionPairVector NeoPkCU::getAllValidActions(const ConstEnvironmentVolatile& envV, size_t agentTeam) const {
    return ActionPairVector{};
}


IsValidResult NeoPkCU::isValidAction(const ConstEnvironmentVolatile& envV, const Action& action, size_t iTeam) const {
  return isValidAction(envV, {iTeam, envV.getTeam(iTeam).getICPKV()}, action);
}


IsValidResult NeoPkCU::isValidAction(const ConstEnvironmentPossible& envV, const Action& action, size_t iTeam) const {
    return isValidAction(envV.getEnv(), action, iTeam);
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  guardNonvolatileState(envV);
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstTeamVolatile oTV = envV.getOtherTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());
  ConstPokemonVolatile tPKV = oTV.getPKV();

  switch (action.type()) {
  case Action::MOVE_0:
  case Action::MOVE_1:
  case Action::MOVE_2:
  case Action::MOVE_3: {
    // is this a valid move?
    if (action.iMove() >= cPKV.nv().getNumMoves()) {
      return IsValidResult::MOVE_INVALID;
    }

    // By default, allow moves
    ValidMoveSet doAllowMove((1 << VALID_MOVE_SIZE) - 1);

    // TODO - respect targeting. Does this target enemy, adjacent, or all?
    // is the other pokemon alive?
    doAllowMove[VALID_MOVE_TARGET_ALIVE] = tPKV.isAlive();

    // is the pokemon we're currently using alive?
    doAllowMove[VALID_MOVE_SELF_ALIVE] = cPKV.isAlive();

    // is the pokemon we're currently using on the field?
    doAllowMove[VALID_MOVE_ACTOR_ACTIVE] = cPKV.isActive();

    // does the move we're using have any PP left?
    ConstMoveVolatile cMV = cPKV.getMV(action);
    doAllowMove[VALID_MOVE_HAS_PP] = cMV.hasPP();

    // if the target of the move is friendly, is the friendly pokemon alive?
    if (cMV.getBase().targetsAlly()) {
      // is this a valid friendly-targeting move?
      if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
        return IsValidResult::INVALID_FRIENDLY_TARGET;
      }
      ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());

      // is the friendly target alive?
      doAllowMove[VALID_MOVE_FRIENDLY_ALIVE] = fPKV.isAlive();

      // is the friendly target us?
      doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER] =
          action.iFriendly() != actor.iTeammate();
    } else if (action.friendlyTarget() != Action::FRIENDLY_DEFAULT) {
      return IsValidResult::INVALID_FRIENDLY_TARGET;
    }

    // Are we locked out of the current move?
    for (const auto& cPlugin : pluginSet_[PLUGIN_ON_TESTMOVE]) {
      onTestMove_rawType pFunction = (onTestMove_rawType)cPlugin.pFunction;
      if (pFunction(cTV, cPKV, cMV, action, doAllowMove) > 1) { break; }
    }

    if (!doAllowMove[VALID_MOVE_TARGET_ALIVE]) {
      return IsValidResult::MOVE_TARGET_DEAD;
    }
    if (!doAllowMove[VALID_MOVE_SELF_ALIVE]) {
      return IsValidResult::MOVE_SELF_DEAD;
    }
    if (!doAllowMove[VALID_MOVE_ACTOR_ACTIVE]) {
      return IsValidResult::MOVE_ACTOR_NOT_ACTIVE;
    }
    if (!doAllowMove[VALID_MOVE_HAS_PP]) { return IsValidResult::MOVE_NO_PP; }
    if (cMV.getBase().targetsAlly()) {
      if (!doAllowMove[VALID_MOVE_FRIENDLY_ALIVE]) {
        return IsValidResult::MOVE_FRIENDLY_TARGET_DEAD;
      }
      if (!doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER]) {
        return IsValidResult::MOVE_FRIENDLY_TARGET_SELF;
      }
    }
    if (!doAllowMove[VALID_MOVE_SCRIPT]) {
      return IsValidResult::MOVE_LOCKED_BY_SCRIPT;
    }

    return IsValidResult::VALID;
  }
  case Action::MOVE_SWITCH: {
    // is the pokemon we're switching to a valid teammate?
    if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
      return IsValidResult::SWITCH_INVALID_POKEMON;
    }

    // By default, allow switches
    ValidSwapSet doAllowSwitch((1 << VALID_SWAP_SIZE) - 1);

    // are we trying to switch to ourself?
    doAllowSwitch[VALID_SWAP_FRIENDLY_IS_OTHER] =
        action.iFriendly() != actor.iTeammate();

    // is the pokemon we're switching to even alive?
    ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());
    doAllowSwitch[VALID_SWAP_FRIENDLY_ALIVE] = fPKV.isAlive();

    // is the pokemon we're switching to already on the field?
    doAllowSwitch[VALID_SWAP_TARGET_INACTIVE] = !fPKV.isActive();

    // are we trying to move during the other team's free move?
    doAllowSwitch[VALID_SWAP_MUST_WAIT] = tPKV.isAlive() || !cPKV.isAlive();

    // Are we locked out of switching?
    for (const auto& cPlugin : pluginSet_[PLUGIN_ON_TESTSWITCH]) {
      onTestSwitch_rawType pFunction = (onTestSwitch_rawType)cPlugin.pFunction;
      if (pFunction(cPKV, fPKV, action, doAllowSwitch) > 1) { break; }
    }

    if (!doAllowSwitch[VALID_SWAP_FRIENDLY_IS_OTHER]) {
      return IsValidResult::SWITCH_TO_SELF;
    }
    if (!doAllowSwitch[VALID_SWAP_TARGET_INACTIVE]) {
      return IsValidResult::SWITCH_ACTIVE_POKEMON;
    }
    if (!doAllowSwitch[VALID_SWAP_FRIENDLY_ALIVE]) {
      return IsValidResult::SWITCH_POKEMON_DEAD;
    }
    if (!doAllowSwitch[VALID_SWAP_MUST_WAIT]) {
      return IsValidResult::SWITCH_MUST_WAIT;
    }
    if (!doAllowSwitch[VALID_SWAP_SCRIPT]) {
      return IsValidResult::SWITCH_LOCKED_BY_SCRIPT;
    }

    return IsValidResult::VALID;
  }
  case Action::MOVE_WAIT:
    // are we waiting for the other team to take its free move?
    if (!(tPKV.isAlive()) && cPKV.isAlive()) { return IsValidResult::VALID; }

    // in most cases, do not allow not moving
    return IsValidResult::WAIT_NOT_ALLOWED;
  case Action::MOVE_STRUGGLE:
    // is the other pokemon alive?
    if (!(tPKV.isAlive())) { return IsValidResult::MOVE_TARGET_DEAD; }

    // is the pokemon we're currently using alive?
    if (!cPKV.isAlive()) { return IsValidResult::MOVE_SELF_DEAD; }

    // are all other moves unusable?
    for (size_t iMove = 0, iSize = cPKV.nv().getNumMoves(); iMove != iSize;
         ++iMove) {
      const Move& move = cPKV.nv().getMove_base(iMove);

      if (move.targetsAlly()) {
        for (size_t iFriendly = 0, numTeammates = cTV.nv().getNumTeammates();
             iFriendly != numTeammates;
             ++iFriendly) {
          if (isValidAction(envV, actor, Action::moveAlly(iMove, iFriendly))) {
            return IsValidResult::STRUGGLE_NOT_ALLOWED;
          }
        }
      } else {
        if (isValidAction(envV, actor, Action::move(iMove))) {
          return IsValidResult::STRUGGLE_NOT_ALLOWED;
        }
      }
    }

    // may struggle when all other moves are unusable:
    return IsValidResult::VALID;
  default:  // disabled action types (item use):
    return IsValidResult::ACTION_TYPE_DISABLED;
  }
}  // endOf is valid action


bool NeoPkCU::isGameOver(const ConstEnvironmentPossible& envV) const {
  return isGameOver(envV.getEnv());
}


bool NeoPkCU::isGameOver(const ConstEnvironmentVolatile& envV) const {
  return getGameState(envV) != MATCH_MIDGAME;
}


MatchState NeoPkCU::getGameState(const ConstEnvironmentVolatile& envV) const {
  guardNonvolatileState(envV);
  bool teamAisDead = !envV.getTeam(TEAM_A).isAlive();
  bool teamBisDead = !envV.getTeam(TEAM_B).isAlive();
  int status = (teamAisDead * 1) + (teamBisDead * 2);

  switch (status) {
  case 0:  // game isn't over, neither team dead
    return MATCH_MIDGAME;
  case 1:  // game is over, team A is dead
    return MATCH_TEAM_B_WINS;
  case 2:  // game is over, team B is dead
    return MATCH_TEAM_A_WINS;
  default:
    assert(false && "isGameOver returned an unacceptable terminal game value!");
  case 3:  // game is over, tie
    return MATCH_TIE;
  };
}


MatchState NeoPkCU::getGameState(const ConstEnvironmentPossible& envV) const {
  return getGameState(envV.getEnv());
}
