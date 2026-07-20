/* 
 * File:   ability.h
 * Author: Ubermouser
 *
 * Created on June 8, 2011, 3:45 PM
 */

#ifndef ABILITY_H
#define ABILITY_H

#include "pokemonai/pkai.h"

#include <string>

#include "pokemonai/name.h"
#include "pokemonai/collection.h"
#include "pokemonai/pluggable.h"

class PKAISHARED Ability: public Name, public Pluggable
{
 public:
  static const Ability* no_ability;

  Ability() = default;
  Ability(const Ability& source) = default;
  Ability& operator=(const Ability& source) = default;
  virtual ~Ability() override = default;

  PluginSourceKind getSourceKind() const override {
    return PluginSourceKind::ABILITY;
  }

  std::string script_;

  /*
   * index of this ability in the Abilities collection
   */
  size_t index_ = 0;
};


class PKAISHARED Abilities: public Collection<Ability>
{
public:
  bool initialize(const std::string& path);

protected:
  bool loadFromFile(const std::string& path);
  bool loadFromFile_lines(const std::vector<std::string>& lines, size_t& iLine);
};

#endif	/* ABILITY_H */

