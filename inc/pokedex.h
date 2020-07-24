#ifndef POKEDEX_H
#define POKEDEX_H

#include "../inc/pkai.h"

#include <vector>
#include <string>

#include "../inc/type.h"
#include "../inc/ability.h"
#include "../inc/nature.h"
#include "../inc/item.h"
#include "../inc/move.h"
#include "../inc/pokemon_base.h"
#include "../inc/pluggable_types.h"
#include "../inc/plugin.h"

namespace boost { namespace dll { class shared_library; } }

class PKAISHARED pokedex
{
private:
  virtual std::vector<move>& getMoves() = 0;

  virtual std::vector<type>& getTypes() = 0;

  virtual std::vector<pokemon_base>& getPokemon() = 0;

  virtual std::vector<ability>& getAbilities() = 0;

  virtual std::vector<nature>& getNatures() = 0;

  virtual std::vector<item>& getItems() = 0;

  virtual enginePlugins& getExtensions() = 0;

  virtual std::vector<boost::dll::shared_library*>& getPlugins() = 0;

public:
  virtual const std::vector<move>& getMoves() const = 0;

  virtual const std::vector<type>& getTypes() const = 0;

  virtual const std::vector<pokemon_base>& getPokemon() const = 0;

  virtual const std::vector<ability>& getAbilities() const = 0;

  virtual const std::vector<nature>& getNatures() const = 0;

  virtual const std::vector<item>& getItems() const = 0;

  virtual const enginePlugins& getExtensions() const = 0;

  bool inputPlugins(const std::string& input_pluginFolder);
  bool registerPlugin(
    regExtension_type registerExtensions,
    size_t* numExtensions = NULL,
    size_t* numOverwritten = NULL,
    std::vector<std::string>* mismatchedItems = NULL,
    std::vector<std::string>* mismatchedAbilities = NULL,
    std::vector<std::string>* mismatchedMoves = NULL,
    std::vector<std::string>* mismatchedCategories = NULL);

  bool inputMoves(const std::vector<std::string>& lines, size_t& iLine);
  bool inputItems(const std::vector<std::string>& lines, size_t& iLine);
  bool inputNatures(const std::vector<std::string>& lines, size_t& iLine);
  bool inputTypes(const std::vector<std::string>& lines, size_t& iLine);
  bool inputAbilities(const std::vector<std::string>& lines, size_t& iLine);
  bool inputPokemon(const std::vector<std::string>& lines, size_t& iLine);
  bool inputMovelist(const std::vector<std::string>& lines, size_t& iLine);
};

#endif /* POKDEX_H */
