/* 
 * File:   pokedex_dynamic.h
 * Author: drendleman
 *
 * Created on July 31, 2020, 11:25 PM
 */

#ifndef POKEDEX_DYNAMIC_H
#define POKEDEX_DYNAMIC_H

#include "pkai.h"

#include <memory>

#include <boost/dll/shared_library.hpp>
#include <boost/program_options.hpp>

#include "pokedex_static.h"

class PKAISHARED PokedexDynamic : public PokedexStatic {
public:
  struct Config: public PokedexStatic::Config {
    // location of the plugin library root directory
    std::string pluginsPath_ = "plugins";
    
    Config() : PokedexStatic::Config() {};

    virtual boost::program_options::options_description options(
        const std::string& category="pokedex configuration",
        std::string prefix = "") override;
  };
  
  PokedexDynamic(const Config& config=Config(), bool doInitialize=true);
  virtual ~PokedexDynamic() override {};
    
  virtual bool inputPlugins() override; // input scripts for registered moves
  
protected:

  Config config_;
  // growable list of plugins (need to be closed upon exiting program)
  std::vector<std::unique_ptr<boost::dll::shared_library> > plugins_; 
};


#endif /* POKEDEX_DYNAMIC_H */

