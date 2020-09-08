#include "../inc/environment_possible.h"

#include <iostream>
#include <boost/format.hpp>
#include <boost/static_assert.hpp>

#include "../inc/environment_nonvolatile.h"
#include "../inc/roulette.h"
#include "../inc/signature.h"


BOOST_STATIC_ASSERT(sizeof(EnvironmentPossibleData) == (sizeof(uint64_t)*18));

EnvironmentPossibleData EnvironmentPossibleData::create(
    const EnvironmentVolatileData& source, bool doHash) {
  EnvironmentPossibleData result{ source, UINT64_MAX, fixedpoint::create<30>(1.0), 0 };
  if (doHash) { result.generateHash(); }

  return result;
}


bool EnvironmentPossibleData::operator <(const EnvironmentPossibleData& other) const {
  return probability < other.probability;
}


void EnvironmentPossibleData::generateHash() {
#if defined(_USEFNVHASH)
    hash = hashes::hash_fnv(&env, sizeof(EnvironmentVolatileData));
#elif defined(_USEMURMUR2)
    hash = hashes::hash_murmur2(&env, sizeof(EnvironmentVolatileData));
#else
    hash = hashes::hash_murmur3(&env, sizeof(EnvironmentVolatileData));
#endif
}


static EnvironmentPossibleData standardEnvironment;
ConstEnvironmentPossible::ConstEnvironmentPossible(
    nonvolatile_t& nv
): impl_t(nv, standardEnvironment) {}


bool ConstEnvironmentPossible::isEmpty() const {
  return &data() == &standardEnvironment;
}


ENV_POSSIBLE_IMPL_TEMPLATE
void ENV_POSSIBLE_IMPL::printState(std::ostream& os) const {
  // print environment status:
  os << ConstEnvironmentPossible{nv(), data()};
}


ENV_POSSIBLE_IMPL_TEMPLATE
void ENV_POSSIBLE_IMPL::printEnvironment(std::ostream& os) const {
  // print state and probability:
  os <<
    "p=" << getProbability().to_double();
  // print status tokens:
  for (unsigned int iTeam = 0; iTeam < 2; iTeam++)
  {
    if (hasFreeMove(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Free";
    }
    if (hasSwitched(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Switch";
      continue;
    }
    if (hasWaited(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Wait";
      continue;
    }
    if (!hasHit(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Miss";
    }
    if (hasSecondary(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Status";
    }
    if (hasCrit(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Crit";
    }
    if (wasBlocked(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Blocked";
    }
  } // endof foreach team

  if (isMerged())
  {
    os << " (MERGED)";
  }

  if (isPruned())
  {
    os << " (PRUNED)";
  }

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
    std::cout << "Please select the index of the desired state for the player, -1 for a random state, or -2 to go discard these states\n";
    getline(std::cin, input);
    std::stringstream inputResult(input);
    
    // determine if state is valid:
    
    if (!(inputResult >> indexState) || 
      !(indexState < (int32_t) size() && indexState >= -2))
    {
      std::cout << "Invalid state \"" << input << "\"!\n";
      
      continue;
    }

    if ((indexState >= 0 && indexState < (int32_t) size()) && at(indexState).isPruned())
    {
      std::cout << "State " << input << " was pruned!\n";
      continue;
    }
    
    break;
  }while(true);
  
  if (indexState == -2)
  {
    return ConstEnvironmentPossible{*nv_};
  }
  
  if (indexState == -1)
  {
    // choose random state
    ConstEnvironmentPossible result = stateSelect_roulette(indexResult);
    
    std::cout << "Randomly chose state " << indexResult << "\n";
    return result;
  }
  
  // else
  indexResult = indexState;
  return at(indexState);
} // endOf stateSelect_index


void PossibleEnvironments::printStates(std::ostream& os, const std::string& linePrefix) const {
  os << getNumUnique() << "(" << size() << ") possible states!\n";
  for (size_t iState = 0; iState < size(); iState++)
  {
    ConstEnvironmentPossible state = at(iState);
    if (state.isPruned()) { continue; } // don't display pruned states

    os << boost::format("%sstate=%d ") % linePrefix % iState;
    state.printState(os);
  }

  os << "\n";
}


std::vector<ConstEnvironmentPossible> PossibleEnvironments::getValidEnvironments() const {
  std::vector<ConstEnvironmentPossible> result; result.reserve(getNumUnique());

  for (size_t iState = 0; iState < size(); ++iState) {
    ConstEnvironmentPossible state = at(iState);
    if (state.isPruned()) { continue; }

    result.push_back(state);
  }

  return result;
}


template class EnvironmentPossibleImpl<ConstEnvironmentVolatile, const EnvironmentPossibleData>;
template class EnvironmentPossibleImpl<EnvironmentVolatile, EnvironmentPossibleData>;
