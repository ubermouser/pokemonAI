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


std::istream& operator>>(std::istream& in, NeoPkCU::StateSelectMethod& method) {
  std::string token;
  in >> token;
  if (token == "RANDOM") {
    method = NeoPkCU::StateSelectMethod::RANDOM;
  } else if (token == "MOST_LIKELY") {
    method = NeoPkCU::StateSelectMethod::MOST_LIKELY;
  } else if (token == "ALL") {
    method = NeoPkCU::StateSelectMethod::ALL;
  } else {
    throw po::validation_error(
        po::validation_error::invalid_option_value,
        "state-select-method",
        token);
  }
  return in;
}


std::ostream& operator<<(
    std::ostream& out, const NeoPkCU::StateSelectMethod& method) {
  switch (method) {
    case NeoPkCU::StateSelectMethod::RANDOM: out << "RANDOM"; break;
    case NeoPkCU::StateSelectMethod::MOST_LIKELY: out << "MOST_LIKELY"; break;
    case NeoPkCU::StateSelectMethod::ALL: out << "ALL"; break;
  }
  return out;
}


boost::program_options::options_description NeoPkCU::Config::options(
    const std::string& category, std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  // clang-format off
  desc.add_options()
      ((prefix + "engine-verbosity").c_str(),
      po::value<int>(&verbosity)->default_value(defaults.verbosity),
      "verbosity level, controls status printing.")
      ((prefix + "engine-accuracy").c_str(),
      po::value<size_t>(&numRandomEnvironments)->default_value(defaults.numRandomEnvironments),
      "number of random environments to create per hit/crit 1-16.")
      ((prefix + "num-active-pokemon").c_str(),
      po::value<size_t>(&numActivePokemon)->default_value(defaults.numActivePokemon),
      "number of active Pokemon on each team.")
      ((prefix + "state-select-method").c_str(),
      po::value<StateSelectMethod>(&stateSelectMethod)->default_value(defaults.stateSelectMethod),
      "method used to select resulting environments: RANDOM, MOST_LIKELY, ALL.")
      ((prefix + "allow-invalid-moves").c_str(),
      po::value<bool>(&allowInvalidMoves)->default_value(defaults.allowInvalidMoves),
      "if true, the engine will not throw an exception for invalid moves.")
      ((prefix + "max-num-states").c_str(),
      po::value<size_t>(&maxNumStates)->default_value(defaults.maxNumStates),
      "maximum number of states the engine should return when StateSelectMethod is RANDOM or MOST_LIKELY.");

  // clang-format on

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


NeoPkCU& NeoPkCU::setStateSelectMethod(StateSelectMethod method) {
  cfg_.stateSelectMethod = method;
  return *this;
}


NeoPkCU& NeoPkCU::setAllowInvalidMoves(bool allow) {
  cfg_.allowInvalidMoves = allow;
  return *this;
}


NeoPkCU& NeoPkCU::setMaxNumStates(size_t maxNumStates) {
  cfg_.maxNumStates = maxNumStates;
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
  size_t expected = numPossibleActive(cEnv.getTeam(TEAM_A)) +
                    numPossibleActive(cEnv.getTeam(TEAM_B));
  if (expected != actions.size()) {
    throw std::invalid_argument(fmt::format(
        "wrong number of actions: expected {}, got {}",
        expected,
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
  auto team = envV.getTeam(agentTeam);

  std::vector<std::pair<Actor, ActionVector>> actorActions;
  for (const Actor& actor : team.yieldActiveActors()) {
    actorActions.push_back({actor, getValidActions(envV, actor)});
  }

  std::vector<ActionMap> result;
  result.push_back({});

  // 1. Handle currently active actors
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

  // 2. Handle empty slots (entries from bench)
  size_t numToFill = numRequiredToActivate(team);
  if (numToFill > 0) {
    ActorActionVector entryActions = getValidEntryActions(envV, agentTeam);

    for (size_t iFill = 0; iFill < numToFill; ++iFill) {
      std::vector<ActionMap> expanded;
      for (const auto& partial : result) {
        for (const auto& [actor, action] : entryActions) {
          // must not have picked this pokemon already for this team in this
          // turn
          if (partial.count(actor)) { continue; }

          ActionMap combined = partial;
          combined[actor] = action;
          expanded.push_back(std::move(combined));
        }
      }
      result = std::move(expanded);
    }
  }

  return result;
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV, TEAM iTeam, const Action& action) const {
  return isValidAction(envV, Actor(iTeam, envV.getTeam(iTeam).getICPKV()), action);
}


IsValidResult NeoPkCU::isValidAction(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  guardNonvolatileState(envV);

  bool actorIsActive = envV.teammate(actor).isActive();

  switch (action.type()) {
  case Action::MOVE_0:
  case Action::MOVE_1:
  case Action::MOVE_2:
  case Action::MOVE_3:
    return isValidAction_move(envV, actor, action);
  case Action::MOVE_SWITCH:
    return isValidAction_switch(envV, actor, action);
  case Action::MOVE_ACTIVATE:
    return isValidAction_activate(envV, actor, action);
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
  assert(action.isMove());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // is this a valid move?
  if (action.iMove() >= cPKV.nv().getNumMoves()) {
    return IsValidResult::MOVE_INVALID;
  }

  ConstMoveVolatile cMV = cPKV.getMV(action);

  // cannot move if any pokemon on the field has fainted:
  if (numRequiredToActivate(envV) > 0) {
    return IsValidResult::REPLACEMENT_NEEDED;
  }

  // Resolve the actual targets of this action
  auto targets = envV.getTargets(actor, action);

  // Validate target existence and activity
  bool allowBench = cMV.getBase().allowsBenchTarget();
  bool hasSpecificFriendlyTarget = action.targetedFriendly();
  bool hasSpecificHostileTarget = action.targetedHostile();

  IsValidResult result = IsValidResult::VALID;
  for (const auto& t : targets) {
    size_t teamSize = envV.getTeam(t.iTeam()).nv().getNumTeammates();

    if (t.iTeammate() >= teamSize) {
      result = IsValidResult::INVALID_TARGET;
      break;
    }

    auto tPKV = envV.teammate(t);
    if (!allowBench && !tPKV.isActive()) {
      result = IsValidResult::MOVE_TARGET_NOT_ACTIVE;
      break;
    }

    if (!tPKV.isAlive()) {
      result = IsValidResult::MOVE_TARGET_FAINTED;
      break;
    }
  }

  if (result.reason == IsValidResult::VALID) {
    // Check if targeting mode matches move capability
    bool moveIsTargetedFriendly = cMV.getBase().isTargetedFriendly();
    bool moveIsTargetedHostile = cMV.getBase().isTargetedHostile();

    if (hasSpecificFriendlyTarget && !moveIsTargetedFriendly) {
      result = IsValidResult::MOVE_DOES_NOT_TARGET_FRIENDLY;
    } else if (hasSpecificHostileTarget && !moveIsTargetedHostile) {
      result = IsValidResult::MOVE_DOES_NOT_TARGET_HOSTILE;
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
      } else if (!doAllowMove[VALID_MOVE_HAS_PP]) {
        result = IsValidResult::MOVE_NO_PP;
      } else if (
          hasSpecificFriendlyTarget &&
          !doAllowMove[VALID_MOVE_TARGET_IS_OTHER]) {
        result = IsValidResult::MOVE_FRIENDLY_TARGET_SELF;
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
  assert(action.isSwitch());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // is the pokemon we're switching to a valid teammate?
  if (action.iFriendly() >= cTV.nv().getNumTeammates()) {
    return IsValidResult::SWITCH_INVALID_POKEMON;
  }

  if (!cPKV.isActive()) { return IsValidResult::SWITCH_ACTOR_NOT_ACTIVE; }

  // Build validation flags
  ValidSwapSet doAllowSwitch = getValidSwapFlags(envV, actor, action, cPKV);

  ConstPokemonVolatile fPKV = cTV.teammate(action.iFriendly());

  if (!fPKV.isAlive()) { return IsValidResult::SWITCH_POKEMON_FAINTED; }

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
  if (!doAllowSwitch[VALID_SWAP_MUST_WAIT]) {
    return IsValidResult::REPLACEMENT_NEEDED;
  }
  if (!doAllowSwitch[VALID_SWAP_SCRIPT]) {
    return IsValidResult::SWITCH_LOCKED_BY_SCRIPT;
  }

  return IsValidResult::VALID;
}  // endOf isValidAction_switch


IsValidResult NeoPkCU::isValidAction_activate(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  assert(action.isActivate());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());

  // is the pokemon we're activating a valid teammate?
  if (actor.iTeammate() >= cTV.nv().getNumTeammates()) {
    return IsValidResult::SWITCH_INVALID_POKEMON;
  }

  ConstPokemonVolatile cPKV = envV.teammate(actor);

  if (cPKV.isActive()) { return IsValidResult::SWITCH_ACTIVE_POKEMON; }
  if (!cPKV.isAlive()) { return IsValidResult::SWITCH_POKEMON_FAINTED; }

  if (numRequiredToActivate(envV.getTeam(actor.iTeam())) == 0) {
    return IsValidResult::SWITCH_ACTOR_NOT_ACTIVE;
  }

  return IsValidResult::VALID;
}  // endOf isValidAction_activate


ActorActionVector NeoPkCU::getValidEntryActions(
    const ConstEnvironmentVolatile& envV, size_t iTeam) const {
  ActorActionVector result;
  ConstTeamVolatile cTV = envV.getTeam(iTeam);
  if (numRequiredToActivate(cTV) == 0) { return result; }

  for (const auto& [actor, pkv] : cTV.yieldInactivePokemon()) {
    Action action = Action::activate();
    if (isValidAction_activate(envV, actor, action)) {
      result.push_back(std::make_pair(actor, action));
    }
  }
  return result;
}


size_t NeoPkCU::numPossibleActive(const ConstTeamVolatile& team) const {
  return std::min((size_t)team.numTeammatesAlive(), cfg_.numActivePokemon);
}


size_t NeoPkCU::numRequiredToActivate(const ConstTeamVolatile& team) const {
  int numPossible = (int)numPossibleActive(team);
  int numActive = (int)team.getNumActivePokemon();

  return (size_t)std::max(0, numPossible - numActive);
}


size_t NeoPkCU::numRequiredToActivate(
    const ConstEnvironmentVolatile& envV) const {
  return numRequiredToActivate(envV.getTeam(TEAM_A)) +
         numRequiredToActivate(envV.getTeam(TEAM_B));
}


IsValidResult NeoPkCU::isValidAction_wait(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  assert(action.isWait());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  if (!cPKV.isAlive()) { return IsValidResult::MOVE_ACTOR_NOT_ACTIVE; }

  // in most cases, do not allow not moving
  if (numRequiredToActivate(envV) == 0) {
    return IsValidResult::WAIT_NOT_ALLOWED;
  }


  // are we waiting for the other team to take its free move?
  // We should wait if any active pokemon is dead (until replacement)
  return IsValidResult::VALID;
}  // endOf isValidAction_wait


IsValidResult NeoPkCU::isValidAction_struggle(
    const ConstEnvironmentVolatile& envV,
    const Actor& actor,
    const Action& action) const {
  assert(action.isStruggle());
  ConstTeamVolatile cTV = envV.getTeam(actor.iTeam());
  ConstPokemonVolatile cPKV = cTV.teammate(actor.iTeammate());

  // cannot move if any pokemon on the field has fainted:
  if (numRequiredToActivate(envV) > 0) {
    return IsValidResult::REPLACEMENT_NEEDED;
  }

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

  // Filter targets by move capability
  std::vector<Actor> validTargets;
  for (const auto& t : targets) {
    bool isAlly = (t.iTeam() == actor.iTeam());
    if (isAlly ? cMV.getBase().targetsAlly() : cMV.getBase().targetsEnemy()) {
      validTargets.push_back(t);
    }
  }

  doAllowMove[VALID_MOVE_ACTOR_ACTIVE] = cPKV.isActive();
  doAllowMove[VALID_MOVE_HAS_PP] = cMV.hasPP();

  if (action.targetedFriendly()) {
    size_t fIndex = action.iFriendly();
    bool canTargetSelf = cMV.getBase().canTargetSelf();
    doAllowMove[VALID_MOVE_TARGET_IS_OTHER] =
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

  // is the pokemon we're switching to already on the field?
  doAllowSwitch[VALID_SWAP_TARGET_INACTIVE] = !fPKV.isActive();

  if (!cPKV.isAlive()) {  // if a pokemon is active but fainted, it must swap:
    doAllowSwitch[VALID_SWAP_MUST_WAIT] = true;
  } else {  // otherwise, we can swap if no pokemon on the field has fainted:
    doAllowSwitch[VALID_SWAP_MUST_WAIT] = numRequiredToActivate(envV) == 0;
  }

  return doAllowSwitch;
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
