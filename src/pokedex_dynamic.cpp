#include "pokemonai/pokedex_dynamic.h"

#include <iostream>

#include <boost/dll/shared_library.hpp>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include "pokemonai/orphan.h"

using namespace boost::dll;
using namespace orphan;
namespace po = boost::program_options;


PokedexDynamic::PokedexDynamic(const Config& config, bool doInitialize)
 : PokedexStatic(config, false), config_(config) {
  if (doInitialize && !initialize()) {
    throw std::runtime_error("Failed to Initialize Pokedex!");
  }
}


bool PokedexDynamic::inputPlugins()
{
  OrphanSet mismatchedItems;
  OrphanSet mismatchedAbilities;
  OrphanSet mismatchedMoves;
  //std::vector<std::string> mismatchedGears; // engine components
  OrphanSet mismatchedCategories;
  size_t numOverwritten = 0;
  size_t numExtensions = 0;
  size_t numPluginsLoaded = 0;
  size_t numPluginsTotal = 0;

  boost::filesystem::path pluginLocation(config_.pluginsPath_);
  
  // determine if folder exists:
  if (!boost::filesystem::exists(pluginLocation) || !boost::filesystem::is_directory(pluginLocation))
  {
    SPDLOG_CRITICAL("A script folder was not found at location \"{}\"!", config_.pluginsPath_);
    return false;
  }

  // determine if folder is a directory:
  if (!boost::filesystem::is_directory(pluginLocation))
  {
    SPDLOG_CRITICAL("\"{}\" is not a directory!", config_.pluginsPath_);
    return false;
  }

  // iterate through all files in moves directory:
  for ( boost::filesystem::directory_iterator iPlugin(pluginLocation), endPlugin; iPlugin != endPlugin; ++iPlugin)
  {
    // ignore subdirectories
    if (boost::filesystem::is_directory(*iPlugin)) { continue; }

    // make sure extension is that of a plugin:
#if defined(WIN32) || defined(_CYGWIN)
    if (iPlugin->path().extension().compare(".dll") != 0) { continue; }
#else // probably linux
    if (iPlugin->path().extension().compare(".so") != 0) { continue; }
#endif

    SPDLOG_INFO("Loading plugin at {}...", iPlugin->path().string());

    numPluginsTotal++;
    std::unique_ptr<shared_library> cPlugin = std::make_unique<shared_library>(iPlugin->path());

    // attempt to load plugin:
    if (!cPlugin)
    {
      SPDLOG_ERROR("plugin \"{}\" could not be loaded!", iPlugin->path().string());
      continue;
    }

    // attempt to find function which enumerates scripts within this plugin:
    //regExtension_type registerExtensions = NULL;
    regExtension_type registerExtensions(cPlugin->get<bool(const Pokedex&, std::vector<plugin>&)>("registerExtensions"));
    if (!registerExtensions)
    {
      SPDLOG_ERROR("could not find registerExtensions method in plugin \"{}\"!", iPlugin->path().string());
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
      SPDLOG_CRITICAL("Register plugin call failed \"{}\"!", iPlugin->path().string());
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

  return true;
} // endOf inputScript


po::options_description PokedexDynamic::Config::options(
    const std::string& category,
    std::string prefix) {
  Config defaults{};
  po::options_description desc = PokedexStatic::Config::options(category, prefix);

  if (prefix.size() > 0) { prefix.append("-"); }
  desc.add_options()
      ((prefix + "plugins").c_str(),
      po::value<std::string>(&pluginsPath_)->default_value(defaults.pluginsPath_),
      "location of the plugin library root directory");

  return desc;
}
