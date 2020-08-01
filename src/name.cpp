//#define PKAI_EXPORT
#include "../inc/name.h"

#include <algorithm>

Name::Name()
: _name()
{
}





Name::Name(const std::string& source)
  : _name(source)
{
}





Name::Name(const Name& source)
  : _name(source._name)
{
}





Name& Name::operator=(const Name& source)
{
  // identity theorem - simply return what we have now if equal address
  if (this == &source) { return *this; }
  
  _name = source._name;
  
  return *this;
}





bool Name::operator <(const Name& other) const
{
  std::string cName(_name);
  std::string oName(other._name);
  {
    std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
    std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
  }
  return _name.compare(other._name)<0?true:false;
};





bool Name::operator >(const Name& other) const
{
  std::string cName(_name);
  std::string oName(other._name);
  {
    std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
    std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
  }
  return _name.compare(other._name)>0?true:false;
};
