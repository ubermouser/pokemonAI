//#define PKAI_EXPORT
#include "pokemonai/name.h"

#include <algorithm>


bool Name::operator <(const Name& other) const
{
  std::string cName(name_);
  std::string oName(other.name_);
  {
    std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
    std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
  }
  return name_.compare(other.name_)<0?true:false;
};


bool Name::operator >(const Name& other) const
{
  std::string cName(name_);
  std::string oName(other.name_);
  {
    std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
    std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
  }
  return name_.compare(other.name_)>0?true:false;
};
