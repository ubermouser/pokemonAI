
//#define PKAI_EXPORT
#include "../inc/pokedex_static.h"

#include <stdexcept>

#include <boost/function.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../inc/init_toolbox.h"
#include "../inc/orphan.h"
#include "../inc/pluggable.h"

using namespace INI;
using namespace orphan;

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

  //POKEMON library (requires sorted input of abilities, moves, and types!)
  if (!pokemon_.initialize(
      config_.pokemonPath_,
      config_.movelistsPath_,
      types_,
      abilities_,
      moves_)) {
    return false;
  }

#ifndef _DISABLEPLUGINS
  // initialize scripts:
  if (config_.pluginsPath_.empty()) {
    if (verbose >= 6) {
      std::cerr << "INF " << __FILE__ << "." << __LINE__ <<
        ": A plugins root directory has not been defined. No plugins will be loaded.\n";
    }
  } else {
    if (verbose >= 1) std::cout << " Loading Plugins...\n";
    if (!inputPlugins()) {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ <<
          ": inputPlugins failed to initialize an acceptable set of plugins!\n";
      return false;
    }
  }
#endif /* _DISABLESCRIPTS */

  pkdex = this;
  return true;
}


bool PokedexStatic::inputPlugins() {
#ifndef _DISABLEPLUGINS
  std::vector<std::string> mismatchedItems;
  std::vector<std::string> mismatchedAbilities;
  std::vector<std::string> mismatchedMoves;
  //std::vector<std::string> mismatchedGears; // engine components
  std::vector<std::string> mismatchedCategories;
  size_t numOverwritten = 0;
  size_t numExtensions = 0;
  size_t numPluginsLoaded = 0;
  size_t numPluginsTotal = 0;

  /*bool success = registerPlugin(
    registerExtensions, 
    &numExtensions, 
    &numOverwritten, 
    &mismatchedItems, 
    &mismatchedAbilities, 
    &mismatchedMoves, 
    &mismatchedCategories);*/

  registerPlugin_orphanCount(
      config_.pluginsPath_,
      numExtensions,
      numOverwritten,
      numPluginsLoaded,
      numPluginsTotal,
      mismatchedItems,
      mismatchedAbilities,
      mismatchedMoves,
      mismatchedCategories);

#endif /* _DISABLEPLUGINS */
  return true;
} // endOf inputScript


bool PokedexStatic::registerPlugin(
  regExtension_type registerExtensions,
  size_t* _numExtensions,
  size_t* _numOverwritten,
  std::vector<std::string>* _mismatchedItems,
  std::vector<std::string>* _mismatchedAbilities,
  std::vector<std::string>* _mismatchedMoves,
  std::vector<std::string>* _mismatchedCategories)
{
  assert(registerExtensions != NULL);

  std::vector<std::string> mismatchedItems;
  std::vector<std::string> mismatchedAbilities;
  std::vector<std::string> mismatchedMoves;
  std::vector<std::string> mismatchedCategories;
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
    if (cCPlugin.getCategory().compare(MOVE_PLUGIN) == 0)
    {
      size_t iMove = orphanCheck(getMoves(), &mismatchedMoves, cCPlugin.getName());
      if (iMove == SIZE_MAX) { continue; } // orphan!
      element = &getMoves()[iMove];
    }
    else if (cCPlugin.getCategory().compare(ABILITY_PLUGIN) == 0)
    {
      size_t iAbility = orphanCheck(getAbilities(), &mismatchedAbilities, cCPlugin.getName());
      if (iAbility == SIZE_MAX) { continue; } // orphan!
      element = &getAbilities()[iAbility];
    }
    else if (cCPlugin.getCategory().compare(ITEM_PLUGIN) == 0)
    {
      size_t iItem = orphanCheck(getItems(), &mismatchedItems, cCPlugin.getName());
      if (iItem == SIZE_MAX) { continue; } // orphan!
      element = &getItems()[iItem];
    }
    else if (cCPlugin.getCategory().compare(ENGINE_PLUGIN) == 0)
    {
      element = &getExtensions();
    }
    else // unknown category:
    {
      mismatchedCategories.push_back(cCPlugin.getCategory());
      continue;
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
          ": plugin for [" << cCPlugin.getCategory() <<
          "][" << cCPlugin.getName() << "] -- overwriting previously defined plugin!\n";
      }
      numOverwritten++;
    }
    numExtensions++;
  } // endOf foreach plugin element

  // add mismatched elements, if the user wants them:
  if (_mismatchedItems != NULL) { _mismatchedItems->insert(_mismatchedItems->end(), mismatchedItems.begin(), mismatchedItems.end()); }
  if (_mismatchedAbilities != NULL) { _mismatchedAbilities->insert(_mismatchedAbilities->end(), mismatchedAbilities.begin(), mismatchedAbilities.end()); }
  if (_mismatchedMoves != NULL) { _mismatchedMoves->insert(_mismatchedMoves->end(), mismatchedMoves.begin(), mismatchedMoves.end()); }
  if (_mismatchedCategories != NULL) { _mismatchedCategories->insert(_mismatchedCategories->end(), mismatchedCategories.begin(), mismatchedCategories.end()); }

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
    const std::vector<std::string>& mismatchedItems,
    const std::vector<std::string>& mismatchedAbilities,
    const std::vector<std::string>& mismatchedMoves,
    const std::vector<std::string>& mismatchedCategories) const {
  // print orphans:
  // print mismatched items
  if (mismatchedItems.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << source <<
      "\" - " << mismatchedItems.size() << " Orphaned plugin-items!\n";
    if (verbose >= 4)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedItems.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned item \"" << mismatchedItems.at(iOrphan) << "\"\n";
      }
    }
  }

  // print mismatched abilities
  if (mismatchedAbilities.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << source <<
      "\" - " << mismatchedAbilities.size() << " Orphaned plugin-abilities!\n";
    if (verbose >= 5)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedAbilities.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned ability \"" << mismatchedAbilities.at(iOrphan) << "\"\n";
      }
    }
  }

  // print mismatched moves
  if (mismatchedMoves.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << source <<
      "\" - " << mismatchedMoves.size() << " Orphaned plugin-moves!\n";
    if (verbose >= 5)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedMoves.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned move \"" << mismatchedMoves.at(iOrphan) << "\"\n";
      }
    }
  }

  // print mismatched categories
  if (mismatchedCategories.size() != 0)
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << source <<
      "\" - " << mismatchedCategories.size() << " Orphaned plugin-categories!\n";
    if (verbose >= 5)
    {
      for (size_t iOrphan = 0; iOrphan < mismatchedMoves.size(); iOrphan++)
      {
        std::cerr << "\tOrphaned category \"" << mismatchedCategories.at(iOrphan) << "\"\n";
      }
    }
  }

  if (verbose >= 6)
  {
    std::cerr << "INF " << __FILE__ << "." << __LINE__ << 
      " Successfully loaded  " << numPluginsLoaded << 
      " of " << numPluginsTotal <<
      " plugins, loaded " << numExtensions << 
      " ( " << numOverwritten << " overwritten ) extensions!\n";
  }
}
