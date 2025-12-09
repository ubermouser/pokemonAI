#ifndef POKEDEX_H
#define POKEDEX_H

#include "pokemonai/pkai.h"

class Moves;
class Types;
class Pokemons;
class Abilities;
class Natures;
class Items;
class EnginePlugins;

class PKAISHARED Pokedex {
public:
  virtual Moves& getMoves() = 0;
  virtual const Moves& getMoves() const = 0;
  
  virtual Types& getTypes() = 0;
  virtual const Types& getTypes() const = 0;
  
  virtual Pokemons& getPokemon() = 0;
  virtual const Pokemons& getPokemon() const = 0;
  
  virtual Abilities& getAbilities() = 0;
  virtual const Abilities& getAbilities() const = 0;
  
  virtual Natures& getNatures() = 0;
  virtual const Natures& getNatures() const = 0;
  
  virtual Items& getItems() = 0;
  virtual const Items& getItems() const = 0;
  
  virtual EnginePlugins& getExtensions() = 0;
  virtual const EnginePlugins& getExtensions() const = 0;

  virtual ~Pokedex() = default;
};

#endif /* POKDEX_H */
