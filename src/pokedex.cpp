
//#define PKAI_EXPORT
#include "pokemonai/pokedex_static.h"

#include <stdexcept>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/program_options.hpp>

#include "pokemonai/init_toolbox.h"
#include "pokemonai/orphan.h"
#include "pokemonai/pluggable.h"

#ifdef GEN4_STATIC
#include "pokemonai/gen4_scripts.h"
#endif

using namespace INI;
using namespace orphan;
namespace po = boost::program_options;

const Pokedex* pkdex = NULL;


PokedexStatic::PokedexStatic(const Config& config, bool doInitialize)
 : Pokedex(), config_(config) {
  if (doInitialize && !initialize()) {
    throw std::runtime_error("Failed to Initialize Pokedex!");
  }
}


PokedexStatic::~PokedexStatic() {
  if (pkdex == this) {
    pkdex = NULL;
  }
}


bool PokedexStatic::initialize() {
  // verify global pointer for pokedex is set:
  // TODO(@drendleman) - remove the need for a global pointer
  if (pkdex != NULL) {
    throw std::runtime_error("Cannot initialize multiple pokedexes at a time!");
  }

  //load data from disk:
  //TYPE library
  auto path_prefix = boost::filesystem::path(config_.prefixPath_);
  if (!types_.initialize((path_prefix / config_.typesPath_).string())) {
    return false;
  }

  //NATURE library
  if (!natures_.initialize((path_prefix / config_.naturesPath_).string())) {
    return false;
  }

  //MOVE library
  if (!moves_.initialize((path_prefix / config_.movesPath_).string(), types_)) {
    return false;
  }

  //ABILITY library
  if (!abilities_.initialize((path_prefix / config_.abilitiesPath_).string())) {
    return false;
  }

  //ITEM library
  if (!items_.initialize((path_prefix / config_.itemsPath_).string(), types_)) {
    return false;
  }

  //POKEMON library (requires abilities, moves, and types!)
  if (!pokemon_.initialize(
          (path_prefix / config_.pokemonPath_).string(),
          (path_prefix / config_.movelistsPath_).string(),
          types_,
          abilities_,
          moves_)) {
    return false;
  }

  // initialize scripts:
  if (!this->inputPlugins()) {
    SPDLOG_CRITICAL(
        "inputPlugins failed to initialize an acceptable set of plugins!");
    return false;
  }

  pkdex = this;
  return true;
}

const Move& PokedexStatic::move(const std::string& name) const {
  try {
    return moves_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Move not found: " + name);
  }
}

const Type& PokedexStatic::type(const std::string& name) const {
  try {
    return types_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Type not found: " + name);
  }
}

const PokemonBase& PokedexStatic::pokemon(const std::string& name) const {
  try {
    return pokemon_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Pokemon not found: " + name);
  }
}

const Ability& PokedexStatic::ability(const std::string& name) const {
  try {
    return abilities_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Ability not found: " + name);
  }
}

const Nature& PokedexStatic::nature(const std::string& name) const {
  try {
    return natures_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Nature not found: " + name);
  }
}

const Item& PokedexStatic::item(const std::string& name) const {
  try {
    return items_.at(name);
  } catch (const std::out_of_range& e) {
    throw std::out_of_range("Item not found: " + name);
  }
}

bool PokedexStatic::inputPlugins() {
  OrphanSet mismatchedItems;
  OrphanSet mismatchedAbilities;
  OrphanSet mismatchedMoves;
  //std::vector<std::string> mismatchedGears; // engine components
  OrphanSet mismatchedCategories;
  size_t numOverwritten = 0;
  size_t numExtensions = 0;
  size_t numPluginsLoaded = 0;
  size_t numPluginsTotal = 0;

#if defined(GEN4_STATIC) || defined(GEN1_STATIC)
  SPDLOG_WARN("Loading Plugins from <STATIC>...");
  bool success = registerPlugin(
      registerExtensions,
      &numExtensions,
      &numOverwritten,
      &mismatchedItems,
      &mismatchedAbilities,
      &mismatchedMoves,
      &mismatchedCategories);
#endif

  registerPlugin_orphanCount(
      "STATIC",
      numExtensions,
      numOverwritten,
      numPluginsLoaded,
      numPluginsTotal,
      mismatchedItems,
      mismatchedAbilities,
      mismatchedMoves,
      mismatchedCategories);

  return true;
} // endOf inputScript



bool PokedexStatic::registerPlugin(
  regExtension_type registerExtensions,
  size_t* _numExtensions,
  size_t* _numOverwritten,
  OrphanSet* _orphanItems,
  OrphanSet* _orphanAbilities,
  OrphanSet* _orphanMoves,
  OrphanSet* _orphanCategories)
{
  assert(registerExtensions != NULL);

  OrphanSet orphanItems;
  OrphanSet orphanAbilities;
  OrphanSet orphanMoves;
  OrphanSet orphanCategories;
  size_t numOverwritten = 0;
  size_t numExtensions = 0;

  // register function handles:
  std::vector<plugin> collectedPlugins;
  if (!registerExtensions(*this, collectedPlugins))
  {
    SPDLOG_CRITICAL(
        "engine plugin was not able to generate a list of valid plugins!");
    return false;
  }

  // load in all functions from this module:
  for (size_t iCPlugin = 0; iCPlugin != collectedPlugins.size(); ++iCPlugin)
  {
    PluggableInterface* element = NULL;

    // find which element this plugin refers to
    plugin& cCPlugin = collectedPlugins[iCPlugin];
    if (cCPlugin.getCategory() == pluginCategory::move)
    {
      element = orphanCheck(getMoves(), cCPlugin.getName(), &orphanMoves);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == pluginCategory::ability)
    {
      element = orphanCheck(getAbilities(), cCPlugin.getName(), &orphanAbilities);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == pluginCategory::item)
    {
      element = orphanCheck(getItems(), cCPlugin.getName(), &orphanItems);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == pluginCategory::engine)
    {
      element = &getExtensions();
    }
    else // unknown category:
    {
      // orphanCategories.insert(cCPlugin.getCategory());
      throw std::runtime_error("Unknown plugin category");
    }

    bool overwritten = false;
    // register plugin to its move/ability/item/engine:
    if (element != NULL)
    {
      const Pluggable* pluggable = dynamic_cast<const Pluggable*>(element);
      if (pluggable) { cCPlugin.setSource(pluggable); }
      overwritten = element->registerPlugin(cCPlugin);
    }

    // if plugin overwrote a plugin that was previously installed:
    if (overwritten)
    {
      SPDLOG_WARN(
          "plugin for {}:{} Name={} -- overwriting previously defined plugin!",
          pluginCategoryToString(cCPlugin.getCategory()),
          pluginTypeToString(cCPlugin.getType()),
          cCPlugin.getName());
      numOverwritten++;
    }
    numExtensions++;
  } // endOf foreach plugin element

  // add mismatched elements, if the user wants them:
  if (_orphanItems != NULL) { _orphanItems->insert(orphanItems.begin(), orphanItems.end()); }
  if (_orphanAbilities != NULL) { _orphanAbilities->insert(orphanAbilities.begin(), orphanAbilities.end()); }
  if (_orphanMoves != NULL) { _orphanMoves->insert(orphanMoves.begin(), orphanMoves.end()); }
  if (_orphanCategories != NULL) { _orphanCategories->insert(orphanCategories.begin(), orphanCategories.end()); }

  if (_numOverwritten != NULL) { *_numOverwritten += numOverwritten; }
  if (_numExtensions != NULL) { *_numExtensions += numExtensions; }

  return true;
}; // endOf registerPlugin


void PokedexStatic::registerPlugin_orphanCount(
    const std::string& source,
    const size_t& numExtensions,
    const size_t& numOverwritten,
    const size_t& numPluginsLoaded,
    const size_t& numPluginsTotal,
    const OrphanSet& mismatchedItems,
    const OrphanSet& mismatchedAbilities,
    const OrphanSet& mismatchedMoves,
    const OrphanSet& mismatchedCategories) const {
  // print orphans:
  // print mismatched items
  printOrphans(mismatchedItems, source, "plugin-items", "item");

  // print mismatched abilities
  printOrphans(mismatchedAbilities, source, "plugin-abilities", "ability");

  // print mismatched moves
  printOrphans(mismatchedMoves, source, "plugin-moves", "move");

  // print mismatched categories
  printOrphans(mismatchedCategories, source, "plugin-categories", "category");

  SPDLOG_INFO(
      "{}loaded {} of {} plugins, loaded {} ( {} overwritten ) "
      "extensions!",
      numExtensions > 0 ? "Successfully " : "",
      numPluginsLoaded,
      numPluginsTotal,
      numExtensions,
      numOverwritten);
}


po::options_description PokedexStatic::Config::options(
    const std::string& category,
    std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  // clang-format off
  desc.add_options()
      ((prefix + "prefix-path").c_str(),
      po::value<std::string>(&prefixPath_)->default_value(defaults.prefixPath_),
      "prefix path of all library files")
      ((prefix + "moves").c_str(),
      po::value<std::string>(&movesPath_)->default_value(defaults.movesPath_),
      "location of the move library")
      ((prefix + "pokemon").c_str(),
      po::value<std::string>(&pokemonPath_)->default_value(defaults.pokemonPath_),
      "location of the pokemon library")
      ((prefix + "natures").c_str(),
      po::value<std::string>(&naturesPath_)->default_value(defaults.naturesPath_),
      "location of the natures library")
      ((prefix + "items").c_str(),
      po::value<std::string>(&itemsPath_)->default_value(defaults.itemsPath_),
      "location of the items library")
      ((prefix + "abilities").c_str(),
      po::value<std::string>(&abilitiesPath_)->default_value(defaults.abilitiesPath_),
      "location of the abilities library")
      ((prefix + "types").c_str(),
      po::value<std::string>(&typesPath_)->default_value(defaults.typesPath_),
      "location of the types library")
      ((prefix + "movelist").c_str(),
      po::value<std::string>(&movelistsPath_)->default_value(defaults.movelistsPath_),
      "location of pokemon movelists")
      ((prefix + "allow-invalid-pokemon").c_str(),
      po::value<bool>(&allowInvalidPokemon_)->default_value(defaults.allowInvalidPokemon_),
      "allow pokemon to have moves/abilities they can't normally learn")
      ((prefix + "allow-invalid-teams").c_str(),
      po::value<bool>(&allowInvalidTeams_)->default_value(defaults.allowInvalidTeams_),
      "allow teams to have multiple of the same species");
  // clang-format on
  return desc;
}