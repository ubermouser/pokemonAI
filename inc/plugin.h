#ifndef PLUGIN_H
#define PLUGIN_H

#include "../inc/pkai.h"
#include <stdint.h>
#include <string>
#include <vector>
#include <boost/function.hpp>

#include "../inc/pluggable_types.h"

class Pokedex;
class plugin;
class MoveNonVolatile;
class PokemonNonVolatile;
union TeamVolatile;
union PokemonVolatile;

typedef boost::function<bool (const Pokedex&, std::vector<plugin>&)> regExtension_type;

static const std::string MOVE_PLUGIN = "move";
static const std::string ITEM_PLUGIN = "item";
static const std::string ABILITY_PLUGIN = "ability";
static const std::string ENGINE_PLUGIN = "engine";

#if defined(GEN4_SCRIPTS_EXPORTS) || defined(GEN4_SCRIPTS_STATIC)
#if (defined(WIN32) || defined(_CYGWIN))
#define GEN4SHARED __declspec( dllexport )
#else
#define GEN4SHARED
#endif
extern "C"
{
  /* register all functions from this dll's extension library */
  extern GEN4SHARED bool registerExtensions(const Pokedex& pkAI, std::vector<plugin>& extensions);
};
#endif

class PKAISHARED plugin
{
private:
  std::string pCategory;
  std::string pName;
  pluginType pType;
  voidFunction_rawType pFunction;
  uint32_t priority;
  int32_t target;

public:
  plugin(const plugin& other)
    : pCategory(other.pCategory),
    pName(other.pName),
    pType(other.pType),
    pFunction(other.pFunction),
    priority(other.priority),
    target(other.target)
  {
  };

  template<class unknown_rawType>
  plugin(const std::string& _category, const std::string& _name, pluginType _pType, unknown_rawType _function, int32_t _priority = 0, uint32_t _target = 0)
    : pCategory(_category),
    pName(_name),
    pType(_pType),
    pFunction((voidFunction_rawType)_function),
    priority(_priority),
    target(_target)
  {
  };

  ~plugin() { };

  pluginType getType() const { return pType; };

  voidFunction_rawType getFunction() const { return pFunction; };

  const std::string& getCategory() const
  {
    return pCategory;
  };

  const std::string& getName() const
  {
    return pName;
  };

  int32_t getPriority() const
  {
    return priority;
  };

  uint32_t getTarget() const
  {
    return target;
  };
};

#endif /* PLUGIN_H */
