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
    std::string prefixPath_ = "data/gen4/";
    // location of the move library
    std::string movesPath_ = "moves.csv";
    // location of the pokemon library
    std::string pokemonPath_ = "pokemon.csv";
    // location of the natures library
    std::string naturesPath_ = "natures.csv";
    // location of the items library
    std::string itemsPath_ = "items.csv";
    // location of the abilities library
    std::string abilitiesPath_ = "abilities.csv";
    // location of the type library
    std::string typesPath_ = "types.csv";
    // location of pokemon movelists
    std::string movelistsPath_ = "movelist.csv";

    bool allowInvalidPokemon_ = false;
    bool allowInvalidTeams_ = false;

    Config(){};

    virtual boost::program_options::options_description options(
        const std::string& category="pokedex configuration",
        std::string prefix = "");
  };

  PokedexStatic(const Config& config=Config(), bool doInitialize=true);
  virtual ~PokedexStatic();

  virtual Moves& getMoves() override { return moves_; };
  virtual const Moves& getMoves() const override { return moves_; };

  virtual Types& getTypes() override { return types_; };
  virtual const Types& getTypes() const override { return types_; };

  virtual Pokemons& getPokemon() override { return pokemon_; };
  virtual const Pokemons& getPokemon() const override { return pokemon_; };

  virtual Abilities& getAbilities() override { return abilities_; };
  virtual const Abilities& getAbilities() const override { return abilities_; };

  virtual Natures& getNatures() override { return natures_; };
  virtual const Natures& getNatures() const override { return natures_; };

  virtual Items& getItems() override { return items_; };
  virtual const Items& getItems() const override { return items_; };

  virtual EnginePlugins& getExtensions() override { return engineExtensions_; };
  virtual const EnginePlugins& getExtensions() const override {
    return engineExtensions_;
  };

  virtual const Move& move(const std::string& name) const override;
  virtual const Type& type(const std::string& name) const override;
  virtual const PokemonBase& pokemon(const std::string& name) const override;
  virtual const Ability& ability(const std::string& name) const override;
  virtual const Nature& nature(const std::string& name) const override;
  virtual const Item& item(const std::string& name) const override;

  virtual bool allowInvalidPokemon() const override {
    return config_.allowInvalidPokemon_;
  }
  virtual bool allowInvalidTeams() const override { return config_.allowInvalidTeams_; }

  virtual void setAllowInvalidPokemon(bool allow) override {
    config_.allowInvalidPokemon_ = allow;
  }
  virtual void setAllowInvalidTeams(bool allow) override {
    config_.allowInvalidTeams_ = allow;
  }

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
