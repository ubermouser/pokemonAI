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

BOOST_STATIC_ASSERT(sizeof(EnvironmentVolatileData) == (sizeof(uint64_t)*17));


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
void ENV_VOLATILE_IMPL::printActivePokemon(std::ostream& os, size_t first) const {
  // TODO: print actor id, only print agent/other once
  for (const Actor& actor : yieldActiveActors()) {
    if (actor.iTeam() == first) {
      os << fmt::format(
          "\tagent: {}\n", fmt::streamed(teammate(actor)));
    } else {
      os << fmt::format(
          "\tother: {}\n", fmt::streamed(teammate(actor)));
    }
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
  case Move::UNKNOWN:
  case Move::SIDE_ALLY:
  case Move::SIDE_ENEMY:
  case Move::SIDE_ALL:
    actions.push_back(Action::move(iMove));
    break;

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

  case Move::ANY_ADJACENT_ALLY:
    for (const auto& [other, teammate] :
         getTeam(actor.iTeam()).yieldActivePokemon()) {
      if (other == actor) { continue; }
      if (!current.isAdjacent(teammate)) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;

  case Move::ANY_ADJACENT_ALLY_SELF:
    for (const auto& [other, teammate] :
         getTeam(actor.iTeam()).yieldActivePokemon()) {
      if (!current.isAdjacent(teammate)) { continue; }

      actions.push_back(Action::moveTarget(iMove, actor, other));
    }
    break;
  case Move::ANY_ALLY:
    for (const auto& [other, teammate] :
         getTeam(actor.iTeam()).yieldPokemon()) {
      if (other == actor) { continue; }

      actions.push_back(Action::moveAlly(iMove, other.iTeammate()));
    }
    break;

  default:
    throw std::runtime_error(
        fmt::format("Invalid move TargetType {}!", (int)base.target_));
  }

  return actions;
}


ENV_VOLATILE_IMPL_TEMPLATE
std::vector<Actor> ENV_VOLATILE_IMPL::getTargets(
    const Actor& actor, const Action& action) const {
  std::vector<Actor> targets;
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

  if (!action.isMove()) { return targets; }

  // Resolve Hostile Targets
  if (action.enemyTarget() != Action::HOSTILE_DEFAULT) {
    size_t eTarget = action.enemyTarget();
    if (eTarget <= Action::HOSTILE_5) {
      targets.push_back(Actor(iOtherTeam, eTarget - Action::HOSTILE_0));
    } else if (
        eTarget == Action::HOSTILE_ADJACENT ||
        eTarget == Action::HOSTILE_ACTIVE) {
      for (const auto& other : getOtherTeam(iTeam).yieldActiveActors()) {
        targets.push_back(other);
      }
    } else if (eTarget == Action::HOSTILE_ALL) {
      for (const auto& [other, teammate] : getOtherTeam(iTeam).yieldPokemon()) {
        targets.push_back(other);
      }
    }
    return targets;
  }

  // Resolve Friendly Targets
  if (action.friendlyTarget() != Action::FRIENDLY_DEFAULT) {
    size_t fTarget = action.friendlyTarget();
    if (fTarget <= Action::FRIENDLY_5) {
      targets.push_back(Actor(iTeam, fTarget - Action::FRIENDLY_0));
    } else if (fTarget == Action::FRIENDLY_ADJACENT) {
      for (const auto& other : getTeam(iTeam).yieldActiveActors()) {
        if (other != actor) { targets.push_back(other); }
      }
    } else if (fTarget == Action::FRIENDLY_ACTIVE) {
      for (const auto& other : getTeam(iTeam).yieldActiveActors()) {
        targets.push_back(other);
      }
    } else if (fTarget == Action::FRIENDLY_ALL) {
      for (const auto& [other, teammate] : getTeam(iTeam).yieldPokemon()) {
        targets.push_back(other);
      }
    }
    return targets;
  }

  // Default:
  targets.push_back(defaultTarget(actor, action));
  return targets;
}


ENV_VOLATILE_IMPL_TEMPLATE
Actor ENV_VOLATILE_IMPL::defaultTarget(
    const Actor& actor, const Action& action) const {
  // friendly default always chooses self
  if (action.isMove()) {
    if (action.friendlyTarget() != Action::FRIENDLY_DEFAULT ||
        action.enemyTarget() == Action::HOSTILE_DEFAULT) {
      return actor;
    }
  }

  // Struggle or Hostile default chooses closest enemy
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


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentVolatile& env) {
  env.printActivePokemon(os);
  return os;
};


template class EnvironmentVolatileImpl<ConstTeamVolatile, const EnvironmentVolatileData>;
template class EnvironmentVolatileImpl<TeamVolatile, EnvironmentVolatileData>;
