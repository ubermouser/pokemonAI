
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

const char* pluginCategoryToString(pluginCategory category) {
  switch(category) {
    case move: return "move";
    case item: return "item";
    case ability: return "ability";
    case engine: return "engine";
    default: return "unknown";
  }
}

using namespace INI;
using namespace orphan;
namespace po = boost::program_options;

int verbose = 0;
int warning = 0;
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
  if (!types_.initialize(config_.typesPath_)) {
    return false;
  }

  //NATURE library
  if (!natures_.initialize(config_.naturesPath_)) {
    return false;
  }

  //MOVE library
  if (!moves_.initialize(config_.movesPath_, types_)) {
    return false;
  }

  //ABILITY library
  if (!abilities_.initialize(config_.abilitiesPath_)) {
    return false;
  }

  //ITEM library
  if (!items_.initialize(config_.itemsPath_, types_)) {
    return false;
  }

  //POKEMON library (requires abilities, moves, and types!)
  if (!pokemon_.initialize(
      config_.pokemonPath_,
      config_.movelistsPath_,
      types_,
      abilities_,
      moves_)) {
    return false;
  }

  // initialize scripts:
  if (verbose >= 1) std::cout << " Loading Plugins...\n";
  if (!this->inputPlugins()) {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
        ": inputPlugins failed to initialize an acceptable set of plugins!\n";
    return false;
  }

  pkdex = this;
  return true;
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

#ifdef GEN4_STATIC
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
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": engine plugin was not able to generate a list of valid plugins!\n";
    return false;
  }

  // load in all functions from this module:
  for (size_t iCPlugin = 0; iCPlugin != collectedPlugins.size(); ++iCPlugin)
  {
    PluggableInterface* element = NULL;

    // find which element this plugin refers to
    plugin& cCPlugin = collectedPlugins[iCPlugin];
    if (cCPlugin.getCategory() == ::move)
    {
      element = orphanCheck(getMoves(), cCPlugin.getName(), &orphanMoves);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == ::ability)
    {
      element = orphanCheck(getAbilities(), cCPlugin.getName(), &orphanAbilities);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == ::item)
    {
      element = orphanCheck(getItems(), cCPlugin.getName(), &orphanItems);
      if (element == NULL) { continue; } // orphan!
    }
    else if (cCPlugin.getCategory() == ::engine)
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
      overwritten = element->registerPlugin(cCPlugin);
    }

    // if plugin overwrote a plugin that was previously installed:
    if (overwritten)
    {
      if (verbose >= 5)
      {
        std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
          ": plugin for [" << pluginCategoryToString(cCPlugin.getCategory()) <<
          "][" << cCPlugin.getName() << "] -- overwriting previously defined plugin!\n";
      }
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

  if (verbose >= 6) {
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      " Successfully loaded  " << numPluginsLoaded << 
      " of " << numPluginsTotal <<
      " plugins, loaded " << numExtensions << 
      " ( " << numOverwritten << " overwritten ) extensions!\n";
  }
}


po::options_description PokedexStatic::Config::options(
    const std::string& category,
    std::string prefix) {
  Config defaults{};
  po::options_description desc{category};

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
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
      "location of pokemon movelists");

  return desc;
}