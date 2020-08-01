#ifndef NAME_H
#define NAME_H

#include "../inc/pkai.h"

#include <string>

class PKAISHARED HasName
{
public:
  virtual ~HasName() {}

  virtual const std::string& getName() const = 0;

  const char* getCName() const { return getName().c_str(); };
};


class PKAISHARED Name : public HasName
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

  bool operator <(const Name& other) const;

  Name();
  Name(const std::string& source);
  Name(const Name& orig);
  ~Name() {};

  Name& operator=(const Name& source);

  bool operator >(const Name& other) const;
  
  const char* getCName() const
  {
    return _name.c_str();
  };
  
  void setName(const char* source)
  {
    _name = std::string(source);
  };

private:
  void init(const Name& source);

};

#endif	/* NAME_H */

