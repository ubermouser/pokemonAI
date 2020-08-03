
//#define PKAI_EXPORT
#include "../inc/environment_possible.h"
//#undef PKAI_EXPORT

#include <iostream>

#include "../inc/environment_nonvolatile.h"
#include "../inc/roulette.h"
#include "../inc/signature.h"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(EnvironmentPossible) == (sizeof(uint64_t)*18));

EnvironmentPossible EnvironmentPossible::create(const EnvironmentVolatile& source, bool doHash)
{	
  EnvironmentPossible result = { { source, UINT64_MAX, fixedpoint::create<30>(1.0), 0 } };
  if (doHash) { result.generateHash(); }

  return result;
}





bool EnvironmentPossible::operator <(const EnvironmentPossible& other) const
{
  return data.probability < other.data.probability;
}





void EnvironmentPossible::generateHash()
{
#if defined(_USEFNVHASH)
    data.hash = hashes::hash_fnv(&getEnv(), sizeof(EnvironmentVolatile));
#elif defined(_USEMURMUR2)
    data.hash = hashes::hash_murmur2(&getEnv(), sizeof(EnvironmentVolatile));
#else
    data.hash = hashes::hash_murmur3(&getEnv(), sizeof(EnvironmentVolatile));
#endif
}


void EnvironmentPossible::printState(
    const EnvironmentNonvolatile& envNV, size_t iState, size_t iPly) const {
  // print ply index if we have a valid one:
  if (iPly != SIZE_MAX) { std::cout << "ply " << iPly << ", "; }
  // print state and probability:
  if (iState != SIZE_MAX) { std::cout << "s=" << iState << ", "; }
  // print environment status:
  std::cout << envP_print(envNV, *this);
}


std::ostream& operator <<(std::ostream& os, const envP_print& en)
{
  // print state and probability:
  os << 
    "p=" << en.envP.getProbability().to_double();
  // print status tokens:
  for (unsigned int iTeam = 0; iTeam < 2; iTeam++)
  {
    if (en.envP.hasFreeMove(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Free";
    }
    if (en.envP.hasSwitched(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Switch";
      continue;
    }
    if (en.envP.hasWaited(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Wait";
      continue;
    }
    if (!en.envP.hasHit(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Miss";
    }
    if (en.envP.hasSecondary(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Status";
    }
    if (en.envP.hasCrit(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Crit";
    }
    if (en.envP.wasBlocked(iTeam))
    {
      os << " " << (iTeam==TEAM_A?"A":"B") << "-Blocked";
    }
  } // endof foreach team

  if (en.envP.isMerged())
  {
    os << " (MERGED)";
  }

  if (en.envP.isPruned())
  {
    os << " (PRUNED)";
  }

  // print active pokemon:
  os << "\n";
  size_t otherTeam = (en.agentTeam + 1) % 2;
  os << "\tagent: " << pokemon_print(
    en.envNV.getTeam(en.agentTeam).getPKNV(en.envP.getEnv().getTeam(en.agentTeam)), 
    en.envP.getEnv().getTeam(en.agentTeam),
    en.envP.getEnv().getTeam(en.agentTeam).getPKV());
  os << "\tother: " << pokemon_print(
    en.envNV.getTeam(otherTeam).getPKNV(en.envP.getEnv().getTeam(otherTeam)), 
    en.envP.getEnv().getTeam(otherTeam),
    en.envP.getEnv().getTeam(otherTeam).getPKV());
  return os;
};


class SortByProbability
{
public:
  static fpType getValue (const EnvironmentPossible& cEnvP)
  {
    if (cEnvP.isPruned()) { return std::numeric_limits<fpType>::quiet_NaN(); }
    return cEnvP.getProbability().to_double();
  };
};


const EnvironmentPossible& PossibleEnvironments::stateSelect_roulette(size_t& indexState) const
{
  indexState = roulette<EnvironmentPossible, SortByProbability>::select(
      *this, SortByProbability());

  return at(indexState);
};


const EnvironmentPossible* PossibleEnvironments::stateSelect_index(size_t& indexResult) const
{
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
    return NULL;
  }
  
  if (indexState == -1)
  {
    // choose random state
    const EnvironmentPossible& result = stateSelect_roulette(indexResult);
    
    std::cout << "Randomly chose state " << indexResult << "\n";
    return &result;
  }
  
  // else
  indexResult = indexState;
  return &at(indexState);
} // endOf stateSelect_index


void PossibleEnvironments::printStates(const EnvironmentNonvolatile& envNV, size_t iPly) const {
  std::cout << getNumUnique() << "(" << size() << ") possible states!\n";
  for (size_t iState = 0; iState < size(); iState++)
  {
    const EnvironmentPossible& state = at(iState);
    if (state.isPruned()) { continue; } // don't display pruned states

    state.printState(envNV, iState, iPly);
  }

  std::cout << "\n";
}
