#ifndef NAME_H
#define NAME_H

#include "../inc/pkai.h"

#include <string>

class PKAISHARED hasName
{
public:
  virtual ~hasName() {}

  virtual const std::string& getName() const = 0;

  const char* getCName() const { return getName().c_str(); };
};


class PKAISHARED name : public hasName
{

private:
  std::string _name;

public:
  void setName(const std::string& source)
  {
    _name = source;
  };

  const std::string& getName() const
  {
    return _name;
  };

  bool operator <(const name& other) const;

  name();
  name(const std::string& source);
  name(const name& orig);
  ~name() {};

  name& operator=(const name& source);

  bool operator >(const name& other) const;
  
  const char* getCName() const
  {
    return _name.c_str();
  };
  
  void setName(const char* source)
  {
    _name = std::string(source);
  };

private:
  void init(const name& source);

};

#endif	/* NAME_H */

