//#define PKAI_EXPORT
#include "../inc/name.h"

#include <algorithm>

name::name()
: _name()
{
}





name::name(const std::string& source)
	: _name(source)
{
}





name::name(const name& source)
	: _name(source._name)
{
}





name& name::operator=(const name& source)
{
	// identity theorem - simply return what we have now if equal address
	if (this == &source) { return *this; }
	
	_name = source._name;
	
	return *this;
}





bool name::operator <(const name& other) const
{
	std::string cName(_name);
	std::string oName(other._name);
	{
		std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
		std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
	}
	return _name.compare(other._name)<0?true:false;
};





bool name::operator >(const name& other) const
{
	std::string cName(_name);
	std::string oName(other._name);
	{
		std::transform(cName.begin(), cName.end(), cName.begin(), ::tolower);
		std::transform(oName.begin(), oName.end(), oName.begin(), ::tolower);
	}
	return _name.compare(other._name)>0?true:false;
};
