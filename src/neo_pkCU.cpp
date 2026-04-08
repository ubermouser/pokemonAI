#include "pokemonai/neo_pkCU.h"

#include <stdexcept>

#include "pokemonai/environment_volatile.h"
#include "pokemonai/neo_pkCU_engine.h"
#include "pokemonai/pkCU_types.h"
#include "pokemonai/pluggable_types.h"


#ifdef LEGACY_PKAI_CU_H
static_assert(false, "NeoPkCU should not be compiled with LegacyPkCU!");
#endif


namespace po = boost::program_options;


NeoPkCU::NeoPkCU(const Config& cfg) : cfg_(cfg), initialized_(false) {}


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
  if (nv_ != nullptr) { initialState_ = createInitialVolatileState(); }
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
  EnvironmentVolatileData initialState =
      EnvironmentVolatileData::create(*nv_, cfg_.numActivePokemon);

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
  if (!initialized_) throw std::runtime_error("NeoPkCU environment not set");
  return ConstEnvironmentVolatile{*nv_, initialState_};
}


void NeoPkCU::initialize() {
  if (!nv_) throw std::runtime_error("NeoPkCU environment not set");

  // Clear existing plugins
  for (auto& set : pluginSet_) {
    set.clear();
  }

  // Add plugins from all teammates on both teams
  for (const auto& [actor, pokemon] : nv_->yieldPokemon()) {
    // Moves
    for (const auto& [iMove, mNV] : pokemon.yieldMoves()) {
      const auto& move = mNV.getBase();
      for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
        plugin p = move.getPlugin(iPlugin);
        if (p.getFunction()) { pluginSet_[iPlugin].insert(p); }
      }
    }

    // Ability
    if (pokemon.abilityExists()) {
      const auto& ability = pokemon.getAbility();
      for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
        plugin p = ability.getPlugin(iPlugin);
        if (p.getFunction()) { pluginSet_[iPlugin].insert(p); }
      }
    }

    // Items
    if (pokemon.hasInitialItem()) {
      const auto& item = pokemon.getInitialItem();
      for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
        plugin p = item.getPlugin(iPlugin);
        if (p.getFunction()) { pluginSet_[iPlugin].insert(p); }
      }
    }
  }

  // Add global extensions from Pokedex
  if (pkdex) {
    const auto& extensions = pkdex->getExtensions();
    for (size_t iPlugin = 0; iPlugin < PLUGIN_MAXSIZE; ++iPlugin) {
      for (size_t i = 0; i < extensions.getNumPlugins(iPlugin); ++i) {
        pluginSet_[iPlugin].insert(extensions.getPlugin(iPlugin, i));
      }
    }
  }

  initialState_ = createInitialVolatileState();
  initialized_ = true;
}


ActionVector NeoPkCU::getValidActions(
    const ConstEnvironmentVolatile& envV, const Actor& actor) const {
  ActionVector result;
  for (const auto& action : envV.getActions(actor)) {
    if (isValidAction(envV, actor, action)) { result.push_back(action); }
  }
  return result;
}


ActionVector NeoPkCU::getValidActions(
    const ConstEnvironmentVolatile& envV, TEAM iTeam) const {
  return getValidActions(envV, Actor(iTeam, envV.getTeam(iTeam).getICPKV()));
}


ActionVector NeoPkCU::getValidMoveActions(
    const ConstEnvironmentVolatile& envV, const Actor& actor) const {
  ActionVector result;
  for (const auto& action : envV.getMoveActions(actor)) {
    if (isValidAction(envV, actor, action)) { result.push_back(action); }
  }
  return result;
}


ActionVector NeoPkCU::getValidMoveActions(
    const ConstEnvironmentVolatile& envV, TEAM iTeam) const {
  return getValidMoveActions(envV, Actor(iTeam, envV.getTeam(iTeam).getICPKV()));
}


ActionVector NeoPkCU::getValidSwapActions(
    const ConstEnvironmentVolatile& envV, const Actor& actor) const {
  ActionVector result;
  for (const auto& action : envV.getSwapActions(actor)) {
    if (isValidAction(envV, actor, action)) { result.push_back(action); }
  }
  return result;
}


ActionVector NeoPkCU::getValidSwapActions(
    const ConstEnvironmentVolatile& envV, TEAM iTeam) const {
  return getValidSwapActions(envV, Actor(iTeam, envV.getTeam(iTeam).getICPKV()));
}


std::vector<ActionMap> NeoPkCU::getAllValidActions(
    const ConstEnvironmentVolatile& envV, TEAM agentTeam) const {
  guardNonvolatileState(envV);

  std::vector<std::pair<Actor, ActionVector>> actorActions;
  for (const Actor& actor : envV.getTeam(agentTeam).yieldActiveActors()) {
    actorActions.push_back({actor, getValidActions(envV, actor)});
  }

  std::vector<ActionMap> result;
  result.push_back({});

  for (const auto& [actor, actions] : actorActions) {
    std::vector<ActionMap> expanded;
    for (const auto& partial : result) {
      for (const auto& action : actions) {
        ActionMap combined = partial;
        combined[actor] = action;
        expanded.push_back(std::move(combined));
      }
    }
    result = std::move(expanded);
  }

  return result;
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV, TEAM iTeam, const Action& action) const {
  return isValidAction(envV, Actor(iTeam, envV.getTeam(iTeam).getICPKV()), action);
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentPossible& envV, TEAM iTeam, const Action& action) const {
  return isValidAction(envV.getEnv(), iTeam, action);
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentPossible& envP,
    const Actor& actor,
    const Action& action) const {
  return isValidAction(envP.getEnv(), actor, action);
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  guardNonvolatileState(envV);

  switch (action.type()) {
  case Action::MOVE_0:
  case Action::MOVE_1:
  case Action::MOVE_2:
  case Action::MOVE_3:
    return isValidAction_move(envV, actor, action);
  case Action::MOVE_SWITCH:
    return isValidAction_switch(envV, actor, action);
  case Action::MOVE_WAIT:
    return isValidAction_wait(envV, actor, action);
  case Action::MOVE_STRUGGLE:
    return isValidAction_struggle(envV, actor, action);
  default:  // disabled action types (item use):
    return IsValidResult::ACTION_TYPE_DISABLED;
  }
}  // endOf is valid action


IsValidResult NeoPkCU::isValidAction_move(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // is this a valid move?
  if (action.iMove() >= cPKV.nv().getNumMoves()) {
    return IsValidResult::MOVE_INVALID;
  }

  ConstMoveVolatile cMV = cPKV.getMV(action);

  // Resolve the actual targets of this action
  auto targets = envV.getTargets(actor, action);

  // Validate target existence and activity
  bool allowBench = cMV.getBase().allowsBenchTarget();
  bool hasSpecificFriendlyTarget = action.targetedFriendly();
  bool hasSpecificHostileTarget = action.targetedHostile();

  IsValidResult result = IsValidResult::VALID;
  for (const auto& t : targets) {
    bool isAlly = (t.iTeam() == actor.iTeam());
    size_t teamSize = envV.getTeam(t.iTeam()).nv().getNumTeammates();

    if (t.iTeammate() >= teamSize) {
      if (isAlly) {
        result = IsValidResult::INVALID_FRIENDLY_TARGET;
      } else {
        result = IsValidResult::MOVE_TARGET_NOT_ACTIVE;
      }
      break;
    }

    if (!allowBench && !envV.teammate(t).isActive()) {
      if (isAlly && hasSpecificFriendlyTarget) {
        result = IsValidResult::INVALID_FRIENDLY_TARGET;
      } else {
        result = IsValidResult::MOVE_TARGET_NOT_ACTIVE;
      }
      break;
    }
  }

  if (result.reason == IsValidResult::VALID) {
    // Check if targeting mode matches move capability
    bool moveIsTargetedFriendly = cMV.getBase().isTargetedFriendly();
    bool moveIsTargetedHostile = cMV.getBase().isTargetedHostile();

    if (hasSpecificFriendlyTarget && !moveIsTargetedFriendly) {
      result = IsValidResult::INVALID_FRIENDLY_TARGET;
    } else if (hasSpecificHostileTarget && !moveIsTargetedHostile) {
      result = IsValidResult::MOVE_TARGET_NOT_ACTIVE;
    }

    if (result.reason == IsValidResult::VALID) {
      ValidMoveSet doAllowMove =
          getValidMoveFlags(envV, actor, action, cPKV, cMV, targets);

      for (const auto& cPlugin : pluginSet_[PLUGIN_ON_TESTMOVE]) {
        onTestMove_rawType pFunction =
            (onTestMove_rawType)cPlugin.getFunction();
        if (pFunction(cTV, cPKV, cMV, action, doAllowMove) > 1) { break; }
      }

      if (!doAllowMove[VALID_MOVE_ACTOR_ACTIVE]) {
        result = IsValidResult::MOVE_ACTOR_NOT_ACTIVE;
      } else if (!doAllowMove[VALID_MOVE_SELF_ALIVE]) {
        result = IsValidResult::MOVE_SELF_DEAD;
      } else if (!doAllowMove[VALID_MOVE_TARGET_ALIVE]) {
        // Filter targets by move capability to determine failure reason
        std::vector<Actor> validTargets;
        for (const auto& t : targets) {
          bool isAlly = (t.iTeam() == actor.iTeam());
          if (isAlly ? cMV.getBase().targetsAlly()
                     : cMV.getBase().targetsEnemy()) {
            validTargets.push_back(t);
          }
        }
        // If the move hits both sides, check if it's an ally death.
        bool canHitEnemy = std::any_of(
            validTargets.begin(), validTargets.end(), [&](const Actor& t) {
              return t.iTeam() != actor.iTeam();
            });
        bool canHitAlly = std::any_of(
            validTargets.begin(), validTargets.end(), [&](const Actor& t) {
              return t.iTeam() == actor.iTeam();
            });
        if (canHitAlly && !canHitEnemy) {
          result = IsValidResult::MOVE_FRIENDLY_TARGET_DEAD;
        } else {
          result = IsValidResult::MOVE_TARGET_DEAD;
        }
      } else if (!doAllowMove[VALID_MOVE_HAS_PP]) {
        result = IsValidResult::MOVE_NO_PP;
      } else if (hasSpecificFriendlyTarget) {
        if (!doAllowMove[VALID_MOVE_FRIENDLY_ALIVE]) {
          result = IsValidResult::MOVE_FRIENDLY_TARGET_DEAD;
        } else if (!doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER]) {
          result = IsValidResult::MOVE_FRIENDLY_TARGET_SELF;
        }
      } else if (!doAllowMove[VALID_MOVE_SCRIPT]) {
        result = IsValidResult::MOVE_LOCKED_BY_SCRIPT;
      }
    }
  }
  return result;
}  // endOf isValidAction_move


IsValidResult NeoPkCU::isValidAction_switch(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // is the pokemon we're switching to a valid teammate?
  if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
    return IsValidResult::SWITCH_INVALID_POKEMON;
  }

  // Build validation flags
  ValidSwapSet doAllowSwitch = getValidSwapFlags(envV, actor, action, cPKV);

  ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());

  // Are we locked out of switching?
  for (const auto& cPlugin : pluginSet_[PLUGIN_ON_TESTSWITCH]) {
    onTestSwitch_rawType pFunction =
        (onTestSwitch_rawType)cPlugin.getFunction();
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
}  // endOf isValidAction_switch


IsValidResult NeoPkCU::isValidAction_wait(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // are we waiting for the other team to take its free move?
  // We should wait if all of the opponent's active pokemon are dead (until
  // replacement)
  bool anyEnemyActiveAlive = false;
  for (const auto& [oActor, oPKV] :
       envV.getOtherTeam(actor.iTeam()).yieldActivePokemon()) {
    if (oPKV.isAlive()) {
      anyEnemyActiveAlive = true;
      break;
    }
  }

  if (!anyEnemyActiveAlive && cPKV.isAlive()) { return IsValidResult::VALID; }

  // in most cases, do not allow not moving
  return IsValidResult::WAIT_NOT_ALLOWED;
}  // endOf isValidAction_wait


IsValidResult NeoPkCU::isValidAction_struggle(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // is the other team alive?
  if (!(envV.getOtherTeam(actor.iTeam()).isAlive())) {
    return IsValidResult::MOVE_TARGET_DEAD;
  }

  // is the pokemon we're currently using alive?
  if (!cPKV.isAlive()) { return IsValidResult::MOVE_SELF_DEAD; }

  // are all other moves unusable?
  for (const auto& [iMove, mNV] : cPKV.nv().yieldMoves()) {
    for (const auto& moveAction : envV.getActions(actor, mNV)) {
      if (isValidAction(envV, actor, moveAction)) {
        return IsValidResult::STRUGGLE_NOT_ALLOWED;
      }
    }
  }

  // may struggle when all other moves are unusable:
  return IsValidResult::VALID;
}  // endOf isValidAction_struggle


ValidMoveSet NeoPkCU::getValidMoveFlags(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action,
    const ConstPokemonVolatile& cPKV,
    const ConstMoveVolatile& cMV,
    const std::vector<Actor>& targets) const {
  ValidMoveSet doAllowMove((1 << VALID_MOVE_SIZE) - 1);

  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  bool hasSpecificFriendlyTarget = action.targetedFriendly();

  // Filter targets by move capability
  std::vector<Actor> validTargets;
  for (const auto& t : targets) {
    bool isAlly = (t.iTeam() == actor.iTeam());
    if (isAlly ? cMV.getBase().targetsAlly() : cMV.getBase().targetsEnemy()) {
      validTargets.push_back(t);
    }
  }

  // is at least one target alive?
  bool anyTargetAlive = std::any_of(
      validTargets.begin(), validTargets.end(), [&](const Actor& t) {
        return envV.teammate(t).isAlive();
      });
  doAllowMove[VALID_MOVE_TARGET_ALIVE] = anyTargetAlive;
  doAllowMove[VALID_MOVE_SELF_ALIVE] = cPKV.isAlive();
  doAllowMove[VALID_MOVE_ACTOR_ACTIVE] = cPKV.isActive();
  doAllowMove[VALID_MOVE_HAS_PP] = cMV.hasPP();

  if (hasSpecificFriendlyTarget) {
    size_t fIndex = action.iFriendly();
    ConstPokemonVolatile fPKV = cTV.teammate(fIndex);
    doAllowMove[VALID_MOVE_FRIENDLY_ALIVE] = fPKV.isAlive();
    bool canTargetSelf = cMV.getBase().canTargetSelf();
    doAllowMove[VALID_MOVE_FRIENDLY_IS_OTHER] =
        (fIndex != actor.iTeammate()) || canTargetSelf;
  }

  return doAllowMove;
}


ValidSwapSet NeoPkCU::getValidSwapFlags(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action,
    const ConstPokemonVolatile& cPKV) const {
  ValidSwapSet doAllowSwitch((1 << VALID_SWAP_SIZE) - 1);

  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());

  // are we trying to switch to ourself?
  doAllowSwitch[VALID_SWAP_FRIENDLY_IS_OTHER] =
      action.iFriendly() != actor.iTeammate();

  // is the pokemon we're switching to even alive?
  ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());
  doAllowSwitch[VALID_SWAP_FRIENDLY_ALIVE] = fPKV.isAlive();

  // is the pokemon we're switching to already on the field?
  doAllowSwitch[VALID_SWAP_TARGET_INACTIVE] = !fPKV.isActive();

  // are we trying to move during the other team's free move?
  // Can swap if all enemies on the field are alive, AND we ourselves are dead
  // (replacement)
  bool anyEnemyActiveAlive = false;
  for (const auto& [oActor, oPKV] :
       envV.getOtherTeam(actor.iTeam()).yieldActivePokemon()) {
    if (oPKV.isAlive()) {
      anyEnemyActiveAlive = true;
      break;
    }
  }

  doAllowSwitch[VALID_SWAP_MUST_WAIT] = anyEnemyActiveAlive || !cPKV.isAlive();

  return doAllowSwitch;
}


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