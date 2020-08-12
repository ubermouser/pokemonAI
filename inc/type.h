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
#include <unordered_map>

#include "../inc/name.h"
#include "../inc/collection.h"

class Pokedex;

class PKAISHARED Type: public Name 
{
public:
  static const Type* no_type;

  std::unordered_map<const Type*, uint8_t> modTable_; // type's values

  size_t index_;
  
  Type() = default;
  Type(const Type& source) = default;
  virtual ~Type() override = default;

  fpType getModifier(const Type& other) const;
private:

};


class PKAISHARED Types: public Collection<Type>
{
public:
  bool initialize(const std::string& path);

  const Type* atByIndex(size_t index) const { return byIndex_.at(index); }

protected:
  bool loadFromFile(const std::string& path);
  bool loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine);

  std::unordered_map<size_t, const Type*> byIndex_;
};

#endif	/* TYPE_H */

