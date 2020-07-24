
//#define PKAI_EXPORT
#include "../inc/environment_possible.h"
//#undef PKAI_EXPORT

#include <ostream>

#include "../inc/environment_nonvolatile.h"
#include "../inc/signature.h"

#include <boost/static_assert.hpp>

BOOST_STATIC_ASSERT(sizeof(environment_possible) == (sizeof(uint64_t)*18));

environment_possible environment_possible::create(const environment_volatile& source, bool doHash)
{	
  environment_possible result = { { source, UINT64_MAX, fixedpoint::create<30>(1.0), 0 } };
  if (doHash) { result.generateHash(); }

  return result;
}





bool environment_possible::operator <(const environment_possible& other) const
{
  return data.probability < other.data.probability;
}





void environment_possible::generateHash()
{
#if defined(_USEFNVHASH)
    data.hash = hashes::hash_fnv(&getEnv(), sizeof(environment_volatile));
#elif defined(_USEMURMUR2)
    data.hash = hashes::hash_murmur2(&getEnv(), sizeof(environment_volatile));
#else
    data.hash = hashes::hash_murmur3(&getEnv(), sizeof(environment_volatile));
#endif
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
