#include "../inc/pokedex_dynamic.h"

#include <iostream>

#include <boost/dll/shared_library.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

using namespace boost::dll;

bool PokedexDynamic::inputPlugins()
{
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

  boost::filesystem::path pluginLocation(config_.pluginsPath_);
  
  // determine if folder exists:
  if (!boost::filesystem::exists(pluginLocation) || !boost::filesystem::is_directory(pluginLocation))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": A script folder was not found at location \"" << config_.pluginsPath_ << "\"!\n";
    return false;
  }

  // determine if folder is a directory:
  if (!boost::filesystem::is_directory(pluginLocation))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": \"" << config_.pluginsPath_ << "\" is not a directory!\n";
    return false;
  }

  // iterate through all files in moves directory:
  for ( boost::filesystem::directory_iterator iPlugin(pluginLocation), endPlugin; iPlugin != endPlugin; ++iPlugin)
  {
    // ignore directories
    if (boost::filesystem::is_directory(*iPlugin)) { continue; }

    // make sure extension is that of a plugin:
#if defined(WIN32) || defined(_CYGWIN)
    if (iPlugin->path().extension().compare(".dll") != 0) { continue; }
#else // probably linux
    if (iPlugin->path().extension().compare(".so") != 0) { continue; }
#endif

    if (verbose >= 6)
    {
      std::cout << "Loading plugin at " << *iPlugin << "...\n";
    }

    numPluginsTotal++;
    std::unique_ptr<shared_library> cPlugin = std::make_unique<shared_library>(iPlugin->path());

    // attempt to load plugin:
    if (!cPlugin)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": plugin \"" << *iPlugin <<
        "\" could not be loaded:!\n";
      continue;
    }

    // attempt to find function which enumerates scripts within this plugin:
    //regExtension_type registerExtensions = NULL;
    regExtension_type registerExtensions(cPlugin->get<bool(const Pokedex&, std::vector<plugin>&)>("registerExtensions"));
    if (!registerExtensions)
    {
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": could not find registerExtensions method in plugin \"" << *iPlugin << 
        "\"!\n";
      // close faulty module:
      cPlugin->unload();
      continue;
    }

    bool success = registerPlugin(
      registerExtensions, 
      &numExtensions, 
      &numOverwritten, 
      &mismatchedItems, 
      &mismatchedAbilities, 
      &mismatchedMoves, 
      &mismatchedCategories);

    if (!success)
    {
      cPlugin->unload();
      std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
        ": FATAL - register plugin call failed \"" << *iPlugin << 
        "\"!\n";
      return false;
    }

    // push back successful load of plugin
    plugins_.push_back(std::move(cPlugin));
    numPluginsLoaded++;
  }// endof foreach plugin

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
