/* 
 * File:   pokedex_static.h
 * Author: drendleman
 *
 * Created on August 1, 2020, 12:48 PM
 */

#ifndef POKEDEX_STATIC_H
#define POKEDEX_STATIC_H

#include "pkai.h"

#include <string>
#include <boost/program_options.hpp>

#include "pokedex.h"
#include "type.h"
#include "ability.h"
#include "nature.h"
#include "item.h"
#include "move.h"
#include "pokemon_base.h"
#include "plugin.h"
#include "orphan.h"

class PKAISHARED PokedexStatic: public Pokedex {
public:
  using OrphanSet = orphan::OrphanSet;

  struct Config {
    // location of the move library
    std::string movesPath_ = "data/gen4_moves.txt";
    // location of the pokemon library
    std::string pokemonPath_ = "data/gen4_pokemon.txt";
    // location of the natures library
    std::string naturesPath_ = "data/gen4_natures.txt";
    // location of the items library
    std::string itemsPath_ = "data/gen4_items.txt";
    // location of the abilities library
    std::string abilitiesPath_ = "data/gen4_abilities.txt";
    // location of the type library
    std::string typesPath_ = "data/gen4_types.txt";
    // location of pokemon movelists
    std::string movelistsPath_ = "data/gen4_movelist.txt";

    Config(){};

    static boost::program_options::options_description options(
        Config& cfg,
        const std::string& category="pokedex configuration",
        std::string prefix = "");
  };
  
  PokedexStatic(const Config& config=Config(), bool doInitialize=true);
  virtual ~PokedexStatic();
    
  virtual Moves& getMoves() { return moves_; };
  virtual const Moves& getMoves() const { return moves_; };
  
  virtual Types& getTypes() { return types_; };
  virtual const Types& getTypes() const { return types_; };
  
  virtual Pokemons& getPokemon() { return pokemon_; };
  virtual const Pokemons& getPokemon() const { return pokemon_; };
  
  virtual Abilities& getAbilities() { return abilities_; };
  virtual const Abilities& getAbilities() const { return abilities_; };
  
  virtual Natures& getNatures() { return natures_; };
  virtual const Natures& getNatures() const { return natures_; };
  
  virtual Items& getItems() { return items_; };
  virtual const Items& getItems() const { return items_; };
  
  virtual EnginePlugins& getExtensions() { return engineExtensions_; };
  virtual const EnginePlugins& getExtensions() const { return engineExtensions_; };
  
  virtual bool initialize();
  virtual bool inputPlugins(); // input scripts for registered moves

protected:
  bool registerPlugin(
    regExtension_type registerExtensions,
    size_t* numExtensions = NULL,
    size_t* numOverwritten = NULL,
    OrphanSet* mismatchedItems = NULL,
    OrphanSet* mismatchedAbilities = NULL,
    OrphanSet* mismatchedMoves = NULL,
    OrphanSet* mismatchedCategories = NULL);
  void registerPlugin_orphanCount(
      const std::string& source,
      const size_t& numExtensions,
      const size_t& numOverwritten,
      const size_t& numPluginsLoaded,
      const size_t& numPluginsTotal,
      const OrphanSet& mismatchedItems,
      const OrphanSet& mismatchedAbilities,
      const OrphanSet& mismatchedMoves,
      const OrphanSet& mismatchedCategories) const;
  
  Config config_;

  Moves moves_; // list of all acceptable moves
  Types types_; // list of all acceptable types
  Pokemons pokemon_; // list of all acceptable pokemon
  Abilities abilities_; // list of all acceptable abilities
  Natures natures_; // list of all acceptable natures
  Items items_; // list of all acceptable items
  EnginePlugins engineExtensions_; // list of engine extensions
};

#endif /* POKEDEX_STATIC_H */
