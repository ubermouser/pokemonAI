#include "pokemonai/environment_possible.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <boost/static_assert.hpp>
#include <iostream>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/roulette.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentPossibleData) == (sizeof(uint64_t)*19));

EnvironmentPossibleData EnvironmentPossibleData::create(
    const EnvironmentVolatileData& source, bool doHash) {
  EnvironmentPossibleData result{source, UINT64_MAX, FixType(1.0f), {0}};
  if (doHash) { result.generateHash(); }

  return result;
}


bool EnvironmentPossibleData::operator <(const EnvironmentPossibleData& other) const {
  return probability < other.probability;
}


void EnvironmentPossibleData::generateHash() {
  hash = env.generateHash();
}


static EnvironmentPossibleData standardEnvironment;
ConstEnvironmentPossible::ConstEnvironmentPossible(
    nonvolatile_t& nv
): impl_t(nv, standardEnvironment) {}


bool ConstEnvironmentPossible::isEmpty() const {
  return &data() == &standardEnvironment;
}


ENV_POSSIBLE_IMPL_TEMPLATE
void ENV_POSSIBLE_IMPL::printState() const {
  printState(std::cout);
}

ENV_POSSIBLE_IMPL_TEMPLATE
void ENV_POSSIBLE_IMPL::printState(std::ostream& os) const {
  // print environment status:
  fmt::print(
      os, "{}\n", fmt::streamed(ConstEnvironmentPossible{nv(), data()}));
}


ENV_POSSIBLE_IMPL_TEMPLATE
void ENV_POSSIBLE_IMPL::printEnvironment(std::ostream& os) const {
  // print state and probability:
  os << fmt::format("p={}", getProbability().to_double());
  // print status tokens:
  for (unsigned int iTeam = 0; iTeam < 2; iTeam++) {
    std::string teamLabel = (iTeam == TEAM_A ? "A" : "B");
    if (hasFreeMove(iTeam)) { os << fmt::format(" {}-Free", teamLabel); }
    if (hasSwitched(iTeam)) {
      os << fmt::format(" {}-Switch", teamLabel);
      continue;
    }
    if (hasWaited(iTeam)) {
      os << fmt::format(" {}-Wait", teamLabel);
      continue;
    }
    if (!hasHit(iTeam)) { os << fmt::format(" {}-Miss", teamLabel); }
    if (hasSecondary(iTeam)) { os << fmt::format(" {}-Status", teamLabel); }
    if (hasCrit(iTeam)) { os << fmt::format(" {}-Crit", teamLabel); }
    if (wasBlocked(iTeam)) { os << fmt::format(" {}-Blocked", teamLabel); }
  }  // endof foreach team

  if (isMerged()) { os << " (MERGED)"; }

  if (isPruned()) { os << " (PRUNED)"; }

  // print active pokemon:
  os << "\n";
}


std::ostream& operator <<(std::ostream& os, const ConstEnvironmentPossible& envP) {
  envP.printEnvironment(os);
  os << envP.getEnv();
  return os;
};


class SortByProbability {
public:
  static fpType getValue(const EnvironmentPossibleData& cEnvP)
  {
    if (cEnvP.isPruned()) { return std::numeric_limits<fpType>::quiet_NaN(); }
    return cEnvP.probability.to_double();
  };
};


ConstEnvironmentPossible PossibleEnvironments::stateSelect_roulette(size_t& indexState) const {
  indexState = roulette<EnvironmentPossibleData, SortByProbability>::select(
      *this, SortByProbability());

  return at(indexState);
};


ConstEnvironmentPossible PossibleEnvironments::stateSelect_index(size_t& indexResult) const {
  std::string input;
  int32_t indexState;

  do {
    fmt::print(
        "Please select the index of the desired state for the player, -1 for a "
        "random state, or -2 to go discard these states\n");
    getline(std::cin, input);
    std::stringstream inputResult(input);

    // determine if state is valid:

    if (!(inputResult >> indexState) ||
        !(indexState < (int32_t)size() && indexState >= -2)) {
      fmt::print("Invalid state \"{}\"!\n", input);

      continue;
    }

    if ((indexState >= 0 && indexState < (int32_t)size()) &&
        at(indexState).isPruned()) {
      fmt::print("State {} was pruned!\n", input);
      continue;
    }

    break;
  } while (true);

  if (indexState == -2)
  {
    return ConstEnvironmentPossible{*nv_};
  }

  if (indexState == -1) {
    // choose random state
    ConstEnvironmentPossible result = stateSelect_roulette(indexResult);

    fmt::print("Randomly chose state {}\n", indexResult);
    return result;
  }

  // else
  indexResult = indexState;
  return at(indexState);
} // endOf stateSelect_index

void PossibleEnvironments::printStates() const { printStates(std::cout, ""); }

void PossibleEnvironments::printStates(std::ostream& os, const std::string& linePrefix) const {
  os << fmt::format("{}({}) possible states!\n", getNumUnique(), size());
  for (size_t iState = 0; iState < size(); iState++) {
    ConstEnvironmentPossible state = at(iState);
    if (state.isPruned()) { continue; }  // don't display pruned states

    os << fmt::format("{}state={} ", linePrefix, iState);
    state.printState(os);
  }

  os << "\n";
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::getValidEnvironments(bool sort) const {
  std::vector<ConstEnvironmentPossible> result; result.reserve(getNumUnique());

  for (size_t iState = 0; iState < size(); ++iState) {
    ConstEnvironmentPossible state = at(iState);
    if (state.isPruned()) { continue; }

    result.push_back(state);
  }

  if (sort) {
    std::sort(std::begin(result), std::end(result), [&](const ConstEnvironmentPossible& a, const ConstEnvironmentPossible& b){
      return a.getProbability() > b.getProbability();
    });
  }

  return result;
}


template class EnvironmentPossibleImpl<ConstEnvironmentVolatile, const EnvironmentPossibleData>;
template class EnvironmentPossibleImpl<EnvironmentVolatile, EnvironmentPossibleData>;
