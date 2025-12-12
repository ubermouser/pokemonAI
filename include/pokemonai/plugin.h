#ifndef PLUGIN_H
#define PLUGIN_H

#include "pkai.h"

#include <stdint.h>
#include <string>
#include <vector>
#include <boost/function.hpp>

#include "pluggable.h"

class Pokedex;
class plugin;

typedef boost::function<bool (const Pokedex&, std::vector<plugin>&)> regExtension_type;

enum pluginCategory {
  move,
  item,
  ability,
  engine
};

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

// TODO(@drendleman) - why do both plugin and plugin_t exist? One is redundant with the other?
class PKAISHARED plugin
{
private:
  pluginCategory pCategory;
  std::string pName;
  pluginType pType;
  voidFunction_rawType pFunction;
  int32_t priority;
  pluginTarget target;

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
  plugin(pluginCategory _category, const std::string& _name, pluginType _pType, unknown_rawType _function, int32_t _priority = 0, pluginTarget _target = current_team)
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

  pluginCategory getCategory() const
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

  pluginTarget getTarget() const
  {
    return target;
  };
};

#endif /* PLUGIN_H */
