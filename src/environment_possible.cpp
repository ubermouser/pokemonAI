#include "pokemonai/environment_possible.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <algorithm>
#include <boost/static_assert.hpp>
#include <iostream>

#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/roulette.h"

BOOST_STATIC_ASSERT(sizeof(EnvironmentPossibleData) == (sizeof(uint64_t)*21));


EnvironmentPossibleData EnvironmentPossibleData::create(
    const EnvironmentVolatileData& source, bool doHash) {
  EnvironmentPossibleData result{source, UINT64_MAX, FixType(1.0f), {0}};
  if (doHash) { result.generateHash(); }

  return result;
}


bool EnvironmentPossibleData::operator <(const EnvironmentPossibleData& other) const {
  return probability < other.probability;
}


uint64_t EnvironmentPossibleData::generateHash() {
  hash = env.generateHash();
  return hash;
}


static EnvironmentPossibleData standardEnvironment;
ConstEnvironmentPossible::ConstEnvironmentPossible(
    nonvolatile_t& nv
): impl_t(nv, standardEnvironment) {}


ENV_POSSIBLE_IMPL_TEMPLATE
bool ENV_POSSIBLE_IMPL::isEmpty() const {
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
  os << fmt::format("p={:.4f}", getProbability().to_double());
  // print status tokens:
  for (const auto& actor : this->getEnv().yieldActiveActors()) {
    std::string actorLabel =
        fmt::format("{}{}", (actor.iTeam() == TEAM_A ? "A" : "B"), actor.iTeammate());
    auto proxy = this->flagsFor(actor);
    if (proxy.isFreeMove()) { os << fmt::format(" {}-Free", actorLabel); }
    if (proxy.isSwitched()) {
      os << fmt::format(" {}-Switch", actorLabel);
      continue;
    }
    if (proxy.isWaited()) {
      os << fmt::format(" {}-Wait", actorLabel);
      continue;
    }
    if (!proxy.isHit()) { os << fmt::format(" {}-Miss", actorLabel); }
    if (proxy.isSecondary()) { os << fmt::format(" {}-Status", actorLabel); }
    if (proxy.isCrit()) { os << fmt::format(" {}-Crit", actorLabel); }
    if (proxy.isBlocked()) { os << fmt::format(" {}-Blocked", actorLabel); }
  }  // endof foreach actor

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


template <typename ResultType, typename ThisType>
static std::vector<ResultType> whereImpl(
    ThisType& self,
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate) {
  std::vector<ResultType> results;
  for (size_t i = 0; i < self.size(); ++i) {
    auto state = self.at(i);
    if (state.isPruned()) continue;
    if (predicate(state)) { results.push_back(state); }
  }
  return results;
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::where(
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate)
    const {
  return whereImpl<ConstEnvironmentPossible>(*this, predicate);
}


std::vector<EnvironmentPossible> PossibleEnvironments::where(
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate) {
  return whereImpl<EnvironmentPossible>(*this, predicate);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::where(
    EnvironmentBitfield mask, EnvironmentBitfield expected) const {
  return where([mask, expected, this](const ConstEnvironmentPossible& state) {
    EnvironmentBitfield collapsed = state.data().getBitmask().collapseTeams();
    EnvironmentBitfield collapsed_mask = mask.collapseTeams();
    EnvironmentBitfield collapsed_expected = expected.collapseTeams();
    return (collapsed & collapsed_mask) == collapsed_expected;
  });
}


std::vector<EnvironmentPossible> PossibleEnvironments::where(
    EnvironmentBitfield mask, EnvironmentBitfield expected) {
  return where([mask, expected](const ConstEnvironmentPossible& state) {
    EnvironmentBitfield collapsed = state.data().getBitmask().collapseTeams();
    EnvironmentBitfield collapsed_mask = mask.collapseTeams();
    EnvironmentBitfield collapsed_expected = expected.collapseTeams();
    return (collapsed & collapsed_mask) == collapsed_expected;
  });
}


template <typename ResultType, typename ThisType>
static std::vector<ResultType> whereFlagsImpl(
    ThisType& self, size_t iTeam, uint8_t mask, uint8_t expected) {
  return self.where([iTeam, mask, expected](const auto& state) {
    return (state.data().getBitmask().getTeamOr(iTeam) & mask) == expected;
  });
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereHit(
    size_t iTeam) const {
  return whereFlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_HIT, FLAG_HIT);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereHit(size_t iTeam) {
  return whereFlagsImpl<EnvironmentPossible>(*this, iTeam, FLAG_HIT, FLAG_HIT);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereCrit(
    size_t iTeam) const {
  return whereFlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_CRIT, FLAG_CRIT);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereCrit(size_t iTeam) {
  return whereFlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_CRIT, FLAG_CRIT);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereStatus(
    size_t iTeam) const {
  return whereFlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_SECONDARY, FLAG_SECONDARY);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereStatus(
    size_t iTeam) {
  return whereFlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_SECONDARY, FLAG_SECONDARY);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereMiss(
    size_t iTeam) const {
  uint8_t avoid = FLAG_HIT | FLAG_SWITCHED | FLAG_WAITED;
  return whereFlagsImpl<ConstEnvironmentPossible>(*this, iTeam, avoid, 0);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereMiss(size_t iTeam) {
  uint8_t avoid = FLAG_HIT | FLAG_SWITCHED | FLAG_WAITED;
  return whereFlagsImpl<EnvironmentPossible>(*this, iTeam, avoid, 0);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereSwitch(
    size_t iTeam) const {
  return whereFlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_SWITCHED, FLAG_SWITCHED);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereSwitch(
    size_t iTeam) {
  return whereFlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_SWITCHED, FLAG_SWITCHED);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereHitNoCrit(
    size_t iTeam) const {
  uint8_t mask = FLAG_HIT | FLAG_CRIT;
  return whereFlagsImpl<ConstEnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereHitNoCrit(
    size_t iTeam) {
  uint8_t mask = FLAG_HIT | FLAG_CRIT;
  return whereFlagsImpl<EnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::whereHitNoStatus(
    size_t iTeam) const {
  uint8_t mask = FLAG_HIT | FLAG_SECONDARY;
  return whereFlagsImpl<ConstEnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


std::vector<EnvironmentPossible> PossibleEnvironments::whereHitNoStatus(
    size_t iTeam) {
  uint8_t mask = FLAG_HIT | FLAG_SECONDARY;
  return whereFlagsImpl<EnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


template <typename ResultType, typename ThisType>
static ResultType where1Impl(
    ThisType& self,
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate) {
  std::vector<ResultType> results = self.where(predicate);
  if (results.empty()) {
    throw std::runtime_error(
        "PossibleEnvironments::where1: No matching state found");
  }

  auto best = std::max_element(
      results.begin(),
      results.end(),
      [](const ResultType& a, const ResultType& b) {
        return a.getProbability() < b.getProbability();
      });

  return *best;
}


ConstEnvironmentPossible PossibleEnvironments::where1(
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate)
    const {
  return where1Impl<ConstEnvironmentPossible>(*this, predicate);
}


EnvironmentPossible PossibleEnvironments::where1(
    const std::function<bool(const ConstEnvironmentPossible&)>& predicate) {
  return where1Impl<EnvironmentPossible>(*this, predicate);
}


ConstEnvironmentPossible PossibleEnvironments::where1(
    EnvironmentBitfield mask, EnvironmentBitfield expected) const {
  return where1([mask, expected](const ConstEnvironmentPossible& state) {
    EnvironmentBitfield collapsed = state.data().getBitmask().collapseTeams();
    EnvironmentBitfield collapsed_mask = mask.collapseTeams();
    EnvironmentBitfield collapsed_expected = expected.collapseTeams();
    return (collapsed & collapsed_mask) == collapsed_expected;
  });
}


EnvironmentPossible PossibleEnvironments::where1(
    EnvironmentBitfield mask, EnvironmentBitfield expected) {
  return where1([mask, expected](const ConstEnvironmentPossible& state) {
    EnvironmentBitfield collapsed = state.data().getBitmask().collapseTeams();
    EnvironmentBitfield collapsed_mask = mask.collapseTeams();
    EnvironmentBitfield collapsed_expected = expected.collapseTeams();
    return (collapsed & collapsed_mask) == collapsed_expected;
  });
}


template <typename ResultType, typename ThisType>
static ResultType where1FlagsImpl(
    ThisType& self, size_t iTeam, uint8_t mask, uint8_t expected) {
  return self.where1([iTeam, mask, expected](const auto& state) {
    return (state.data().getBitmask().getTeamOr(iTeam) & mask) == expected;
  });
}


ConstEnvironmentPossible PossibleEnvironments::where1Hit(size_t iTeam) const {
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_HIT, FLAG_HIT);
}


EnvironmentPossible PossibleEnvironments::where1Hit(size_t iTeam) {
  return where1FlagsImpl<EnvironmentPossible>(*this, iTeam, FLAG_HIT, FLAG_HIT);
}


ConstEnvironmentPossible PossibleEnvironments::where1Crit(size_t iTeam) const {
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_CRIT, FLAG_CRIT);
}


EnvironmentPossible PossibleEnvironments::where1Crit(size_t iTeam) {
  return where1FlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_CRIT, FLAG_CRIT);
}


ConstEnvironmentPossible PossibleEnvironments::where1Status(
    size_t iTeam) const {
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_SECONDARY, FLAG_SECONDARY);
}


EnvironmentPossible PossibleEnvironments::where1Status(size_t iTeam) {
  return where1FlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_SECONDARY, FLAG_SECONDARY);
}


ConstEnvironmentPossible PossibleEnvironments::where1Miss(size_t iTeam) const {
  uint8_t avoid = FLAG_HIT | FLAG_SWITCHED | FLAG_WAITED;
  return where1FlagsImpl<ConstEnvironmentPossible>(*this, iTeam, avoid, 0);
}


EnvironmentPossible PossibleEnvironments::where1Miss(size_t iTeam) {
  uint8_t avoid = FLAG_HIT | FLAG_SWITCHED | FLAG_WAITED;
  return where1FlagsImpl<EnvironmentPossible>(*this, iTeam, avoid, 0);
}


ConstEnvironmentPossible PossibleEnvironments::where1Switch(
    size_t iTeam) const {
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, FLAG_SWITCHED, FLAG_SWITCHED);
}


EnvironmentPossible PossibleEnvironments::where1Switch(size_t iTeam) {
  return where1FlagsImpl<EnvironmentPossible>(
      *this, iTeam, FLAG_SWITCHED, FLAG_SWITCHED);
}


ConstEnvironmentPossible PossibleEnvironments::where1HitNoCrit(
    size_t iTeam) const {
  uint8_t mask = FLAG_HIT | FLAG_CRIT;
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, mask, FLAG_HIT);
}


EnvironmentPossible PossibleEnvironments::where1HitNoCrit(size_t iTeam) {
  uint8_t mask = FLAG_HIT | FLAG_CRIT;
  return where1FlagsImpl<EnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


ConstEnvironmentPossible PossibleEnvironments::where1HitNoStatus(
    size_t iTeam) const {
  uint8_t mask = FLAG_HIT | FLAG_SECONDARY;
  return where1FlagsImpl<ConstEnvironmentPossible>(
      *this, iTeam, mask, FLAG_HIT);
}


EnvironmentPossible PossibleEnvironments::where1HitNoStatus(size_t iTeam) {
  uint8_t mask = FLAG_HIT | FLAG_SECONDARY;
  return where1FlagsImpl<EnvironmentPossible>(*this, iTeam, mask, FLAG_HIT);
}


template class EnvironmentPossibleImpl<ConstEnvironmentVolatile, const EnvironmentPossibleData>;
template class EnvironmentPossibleImpl<EnvironmentVolatile, EnvironmentPossibleData>;
