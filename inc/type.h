/* 
 * File:   type.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:58 PM
 */

#ifndef TYPE_H
#define	TYPE_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>

#include "name.h"

class Pokedex;

class PKAISHARED Type: public Name 
{
public:
  static const Type* no_type;

  std::vector<uint8_t> modTable_; // type's values
  
  Type() = default;
  Type(const Type& source) = default;
  virtual ~Type() override = default;

  fpType getModifier(const Type& other) const;
private:

};


class PKAISHARED Types: public std::vector<Type>
{
public:
  bool initialize(const std::string& path);

protected:
  bool loadFromFile(const std::string& path);
  bool loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* TYPE_H */

