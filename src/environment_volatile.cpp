#include "pokemonai/environment_volatile.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <boost/static_assert.hpp>
#include <cstring>
#include <ostream>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/move.h"
#include "pokemonai/signature.h"
#include "pokemonai/team_nonvolatile.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatileData) == (sizeof(uint64_t)*25));


void EnvironmentVolatile::initialize(size_t numActivePokemon) {
  // zero datastructure:
  std::memset(data_, 0, sizeof(EnvironmentVolatileData));
  // initialize:
  getTeam(0).initialize(numActivePokemon);
  getTeam(1).initialize(numActivePokemon);
}


uint64_t EnvironmentVolatileData::generateHash() const {
#if defined(_USEFNVHASH)
    return hashes::hash_fnv(this, sizeof(EnvironmentVolatileData));
#elif defined(_USEMURMUR2)
    return hashes::hash_murmur2(this, sizeof(EnvironmentVolatileData));
#else
    return hashes::hash_murmur3(this, sizeof(EnvironmentVolatileData));
#endif
}


EnvironmentVolatileData EnvironmentVolatileData::create(
    const EnvironmentNonvolatile& envNV, size_t numActivePokemon) {
  EnvironmentVolatileData result;
  EnvironmentVolatile{envNV, result}.initialize(numActivePokemon);
  return result;
};


bool EnvironmentVolatileData::operator ==(const EnvironmentVolatileData& other) const {
  return std::memcmp(this, &other, sizeof(EnvironmentVolatileData)) == 0;
}


bool EnvironmentVolatileData::operator !=(const EnvironmentVolatileData& other) const {
  return !(*this == other);
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Actor> ENV_VOLATILE_IMPL::getActiveActors() const {
  std::vector<Actor> result;
  for (const Actor& actor : yieldActiveActors()) { result.push_back(actor); }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
size_t ENV_VOLATILE_IMPL::getNumActivePokemon() const {
  size_t result = 0;
  for (const Actor& actor : yieldActiveActors()) {
    (void)actor;
    ++result;
  }
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
void ENV_VOLATILE_IMPL::printActivePokemon(
    std::ostream& os, size_t first) const {
  for (const auto& [actor, teammate] : yieldActivePokemon((TEAM)first)) {
    os << fmt::format("\t{}: {}\n", fmt::streamed(actor), fmt::streamed(teammate));
  }
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Action> ENV_VOLATILE_IMPL::getActions(
    const Actor& actor, const MoveNonVolatile& move) const {
  std::vector<Action> actions;
  const ConstPokemonVolatile current = teammate(actor);
  const Move& base = move.getBase();

  uint32_t iMove = move.getIMove();
  if (iMove == 4) { return {Action::struggle()}; }

  if (iMove >= teammate(actor).nv().getNumMoves()) {
    throw std::runtime_error(fmt::format(
        "Invalid move index: {} when pokemon has {} moves!",
        iMove,
        teammate(actor).nv().getNumMoves()));
  }

  switch (base.target_) {
  // unambiguous cases:
  case Move::ALL_ACTIVE:
    actions.push_back(Action::moveActive(iMove));
    break;
  case Move::ALL_ACTIVE_ALLIES:
    actions.push_back(Action::moveActiveAlly(iMove));
    break;
  case Move::ALL_ACTIVE_ENEMIES:
    actions.push_back(Action::moveActiveEnemy(iMove));
    break;
  case Move::ALL_ADJACENT:
    actions.push_back(Action::moveAdjacent(iMove));
    break;
  case Move::ALL_ADJACENT_ENEMY:
    actions.push_back(Action::moveAdjacentEnemy(iMove));
    break;
  case Move::ALL_ADJACENT_ALLY:
    actions.push_back(Action::moveAdjacentAlly(iMove));
    break;
  case Move::ALL_FIELD:
    actions.push_back(Action::moveAll(iMove));
    break;
  case Move::ALL_ALLIES:
    actions.push_back(Action::moveAllAllies(iMove));
    break;
  case Move::ALL_ENEMIES:
    actions.push_back(Action::moveAllEnemies(iMove));
    break;
  case Move::SELF:
    actions.push_back(Action::moveAlly(iMove, actor.iTeammate()));
    break;
  case Move::SIDE_ALLY:
    actions.push_back(Action::moveSideAlly(iMove));
    break;
  case Move::SIDE_ENEMY:
    actions.push_back(Action::moveSideEnemy(iMove));
    break;
  case Move::SIDE_ALL:
    actions.push_back(Action::moveSideAll(iMove));
    break;
  case Move::UNKNOWN:
    actions.push_back(Action::move(iMove));
    break;

  // ambiguous cases:
  case Move::ANY_ACTIVE:
    for (const auto& other : yieldActiveActors()) {
      if (other == actor) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  case Move::ANY_ADJACENT:
    for (const auto& [other, teammate] : yieldActivePokemon()) {
      if (other == actor) { continue; }
      if (!current.isAdjacent(teammate)) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  case Move::ANY_ADJACENT_ENEMY:
    for (const auto& [other, teammate] :
         getOtherTeam(actor.iTeam()).yieldActivePokemon()) {
      if (!current.isAdjacent(teammate)) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  case Move::ANY_ADJACENT_ALLY_SELF:
    actions.push_back(Action::moveTarget(iMove, actor, actor));
  case Move::ANY_ADJACENT_ALLY:
    for (const auto& [other, teammate] :
         getTeam(actor.iTeam()).yieldActivePokemon()) {
      if (other == actor) { continue; }
      if (!current.isAdjacent(teammate)) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  case Move::ANY_ALLY_SELF:
    actions.push_back(Action::moveTarget(iMove, actor, actor));
  case Move::ANY_ALLY:
    for (const auto& [other, teammate] :
         getTeam(actor.iTeam()).yieldPokemon()) {
      if (other == actor) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  default:
    throw std::runtime_error(
        fmt::format("Invalid move TargetType {}!", (int)base.target_));
  }

  return actions;
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Action> ENV_VOLATILE_IMPL::getActions(const Actor& actor) const {
  std::vector<Action> result = getMoveActions(actor);
  std::vector<Action> swaps = getSwapActions(actor);
  result.insert(result.end(), swaps.begin(), swaps.end());
  return result;
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Action> ENV_VOLATILE_IMPL::getMoveActions(
    const Actor& actor) const {
  std::vector<Action> candidates;
  auto current = teammate(actor);

  for (const auto& [iMove, mNV] : current.nv().yieldMoves()) {
    for (const auto& action : getActions(actor, mNV)) {
      candidates.push_back(action);
    }
  }

  candidates.push_back(Action::struggle());
  candidates.push_back(Action::wait());
  return candidates;
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Action> ENV_VOLATILE_IMPL::getSwapActions(
    const Actor& actor) const {
  std::vector<Action> candidates;
  auto currentTeam = getTeam(actor.iTeam());
  for (size_t i = 0; i < currentTeam.nv().getNumTeammates(); ++i) {
    candidates.push_back(Action::swap(i));
  }
  return candidates;
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Actor> ENV_VOLATILE_IMPL::getTargets(
    const Actor& actor, const Action& action) const {
  std::vector<Actor> targets;
  auto current = teammate(actor);
  size_t iTeam = actor.iTeam();
  size_t iOtherTeam = iTeam ^ 1;

  if (action.isSwitch()) {
    targets.push_back(Actor(iTeam, action.iFriendly()));
    return targets;
  }

  if (action.isStruggle()) {
    targets.push_back(defaultTarget(actor, action));
    return targets;
  }

  // undefined, wait, and unknown move may return 0 targets:
  if (!action.isMove()) { return targets; }

  // Resolve Hostile Targets
  if (action.targetsHostiles()) {
    if (action.targetsMultipleHostile()) {
      switch (action.enemyTarget()) {
      case Action::HOSTILE_ACTIVE:  // target all active pokemon:
        for (const auto& other : getOtherTeam(iTeam).yieldActiveActors()) {
          targets.push_back(other);
        }
        break;
      case Action::HOSTILE_ADJACENT:  // target all adjacent pokemon:
        for (const auto& [other, teammate] :
             getOtherTeam(iTeam).yieldPokemon()) {
          if (!current.isAdjacent(teammate)) { continue; }
          targets.push_back(other);
        }
        break;
      case Action::HOSTILE_ALL:  // target every hostile pokemon:
        for (const auto& other : getOtherTeam(iTeam).yieldActors()) {
          targets.push_back(other);
        }
        break;
      case Action::HOSTILE_SIDE:  // does not target a pokemon
        targets.push_back(defaultTarget(actor, action));
        break;
      default:
        throw std::runtime_error(fmt::format(
            "Invalid Enemy Action Target {}!", (int)action.enemyTarget()));
      }
    } else if (action.targetedHostile()) {  // single-target and default target:
      targets.push_back(Actor(iOtherTeam, action.iEnemy()));
    } else {  // default target, side target:
      targets.push_back(defaultTarget(actor, action));
    }
  }  // end of resolve hostile targets

  // Resolve Friendly Targets
  if (action.targetsFriendlies()) {
    if (action.targetsMultipleFriendly()) {
      switch (action.friendlyTarget()) {
      case Action::FRIENDLY_ACTIVE:  // target all active pokemon:
        for (const auto& other : getTeam(iTeam).yieldActiveActors()) {
          targets.push_back(other);
        }
        break;
      case Action::FRIENDLY_ADJACENT_SELF:  // target all adjacent + self:
        targets.push_back(actor);
      case Action::FRIENDLY_ADJACENT:  // target all adjacent pokemon:
        for (const auto& [other, teammate] : getTeam(iTeam).yieldPokemon()) {
          if (actor == other) { continue; }
          if (!current.isAdjacent(teammate)) { continue; }
          targets.push_back(other);
        }
        break;
      case Action::FRIENDLY_ALL:  // target every friendly pokemon:
        for (const auto& other : getTeam(iTeam).yieldActors()) {
          targets.push_back(other);
        }
        break;
      case Action::FRIENDLY_SIDE:  // does not target a pokemon
        if (action.enemyTarget() != Action::HOSTILE_SIDE) {
          // don't add the default target twice if we are targeting both sides:
          targets.push_back(defaultTarget(actor, action));
        }
        break;
      default:
        throw std::runtime_error(fmt::format(
            "Invalid Friendly Action Target {}!",
            (int)action.friendlyTarget()));
      }
    } else if (action
                   .targetedFriendly()) {  // single-target and default target:
      targets.push_back(Actor(iTeam, action.iFriendly()));
    } else {  // default target, side target:
      targets.push_back(defaultTarget(actor, action));
    }
  }  // end of resolve friendly targets

  // If no specific targets found, raise:
  if (targets.empty()) {
    throw std::runtime_error(fmt::format(
        "No targets found for Action {}:{}!",
        fmt::streamed(actor),
        fmt::streamed(action)));
  }

  return targets;
}


ENV_VOLATILE_IMPL_TEMPLATE
Actor ENV_VOLATILE_IMPL::defaultTarget(
    const Actor& actor, const Action& action) const {
  if (action.targetsHostiles()) {
    return defaultEnemy(actor, action);
  } else if (action.targetsFriendlies()) {
    return defaultFriendly(actor, action);
  } else {
    throw std::runtime_error(fmt::format(
        "No default target available for for Action {}:{}!",
        fmt::streamed(actor),
        fmt::streamed(action)));
  }
}


ENV_VOLATILE_IMPL_TEMPLATE
Actor ENV_VOLATILE_IMPL::defaultEnemy(
    const Actor& actor, const Action& action) const {
  size_t iOtherTeam = actor.iTeam() ^ 1;
  const auto& current = teammate(actor);

  Actor bestTarget = Actor(iOtherTeam, 0);
  int minDistance = 100;

  for (const auto& [other, teammate] :
       getOtherTeam(actor.iTeam()).yieldActivePokemon()) {
    int dist = current.distance(teammate);
    if (dist < minDistance) {
      minDistance = dist;
      bestTarget = other;
    }
  }
  return bestTarget;
}


ENV_VOLATILE_IMPL_TEMPLATE
Actor ENV_VOLATILE_IMPL::defaultFriendly(
    const Actor& actor, const Action& action) const {
  return actor;
}


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  const auto& status = env.getTeam(TEAM_A).status();
  if (status.weather_type != WEATHER_NORMAL) {
    std::string weatherStr;
    // clang-format off
    switch (status.weather_type) {
      case WEATHER_SUNNY: weatherStr = "Sunny"; break;
      case WEATHER_RAIN: weatherStr = "Rain"; break;
      case WEATHER_SAND: weatherStr = "Sandstorm"; break;
      case WEATHER_HAIL: weatherStr = "Hail"; break;
      case WEATHER_FOG: weatherStr = "Fog"; break;
      case WEATHER_SHADOWSKY: weatherStr = "ShadowSky"; break;
      default: weatherStr = "Unknown"; break;
    }
    // clang-format on
    os << fmt::format(
        "Weather: {} ({} turns left)\n", weatherStr, status.weather_duration);
  }
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
