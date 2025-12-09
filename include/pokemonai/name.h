#ifndef NAME_H
#define NAME_H

#include "pokemonai/pkai.h"

#include <string>

class PKAISHARED HasName
{
public:
  virtual ~HasName() {}

  virtual const std::string& getName() const = 0;
};


class PKAISHARED Name : public HasName
{

protected:
  std::string name_;

public:
  void setName(const std::string& source)
  {
    name_ = source;
  };

  virtual const std::string& getName() const override
  {
    return name_;
  };

  bool operator <(const Name& other) const;

  Name(const std::string& source) : name_(source) { };
  Name() = default;
  Name(const Name& orig) = default;
  Name& operator=(const Name& source) = default;
  virtual ~Name() override = default;

  bool operator >(const Name& other) const;
};

#endif	/* NAME_H */

