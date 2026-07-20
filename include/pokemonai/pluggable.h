#ifndef PLUGGABLE_H
#define PLUGGABLE_H

#include "pkai.h"

#include <string>
#include <vector>
#include <array>
#include <boost/function.hpp>

class plugin;
class Actor;
class EnvironmentVolatile;
class Action;
class Move;
class IPluginEvaluationContext;
typedef void (*voidFunction_rawType)(void*);

enum class PluginSourceKind {
  GLOBAL,
  MOVE,
  ABILITY,
  ITEM
};

#define PLUGIN_MAXSIZE 32

enum pluginTarget {
  current_team = 0,
  other_team = 1,
  all_teams = 2
};

enum pluginType {
  PLUGIN_ON_INIT = 0,
  PLUGIN_ON_RESET = 1,
  PLUGIN_ON_SETSPEEDBRACKET = 2,
  PLUGIN_ON_MODIFYSPEED = 3,
  PLUGIN_ON_BEGINNINGOFTURN = 4,
  PLUGIN_ON_EVALUATEMOVE = 5,
  PLUGIN_ON_SETBASEPOWER = 6,
  PLUGIN_ON_MODIFYBASEPOWER = 7,
  PLUGIN_ON_MODIFYATTACKPOWER = 8,
  PLUGIN_ON_MODIFYCRITICALPOWER = 9,
  PLUGIN_ON_MODIFYRAWDAMAGE = 10,
  PLUGIN_ON_SETMOVETYPE = 11,
  PLUGIN_ON_MODIFYSTAB = 12,
  PLUGIN_ON_SETDEFENSETYPE = 13,
  PLUGIN_ON_MODIFYITEMPOWER = 14,
  PLUGIN_ON_MODIFYHITPROBABILITY = 15,
  PLUGIN_ON_MODIFYCRITPROBABILITY = 16,
  PLUGIN_ON_CALCULATEDAMAGE = 17,
  PLUGIN_ON_ENDOFMOVE = 18,
  PLUGIN_ON_MODIFYSECONDARYPROBABILITY = 19,
  PLUGIN_ON_SECONDARYEFFECT = 20,
  PLUGIN_ON_ENDOFTURN = 21,
  PLUGIN_ON_ENDOFROUND = 22,
  PLUGIN_ON_SWITCHOUT = 23,
  PLUGIN_ON_SWITCHIN = 24,
  PLUGIN_ON_TESTMOVE = 25,
  PLUGIN_ON_TESTSWITCH = 26,
  PLUGIN_ON_MODIFYACTION = 27,
  PLUGIN_ON_EXECUTESWITCH = 28,
  PLUGIN_ON_UNINIT = 29,
  PLUGIN_ON_POSTROUND = 30,
  PLUGIN_ON_BEGINNINGOFGAME = 31
};

enum pluginCategory {
  move,
  item,
  ability,
  engine
};

PKAISHARED const char* pluginTargetToString(pluginTarget target);
PKAISHARED const char* pluginTypeToString(pluginType type);
PKAISHARED const char* pluginCategoryToString(pluginCategory category);

class PKAISHARED plugin
{
private:
  pluginCategory pCategory;
  std::string pName;
  pluginType pType;
  void* pFunction;
  int32_t priority;
  pluginTarget target;
  const class Pluggable* source;
  PluginSourceKind sourceKind;

public:
  plugin()
    : pCategory(engine),
    pName(""),
    pType(PLUGIN_ON_INIT),
    pFunction(nullptr),
    priority(0),
    target(current_team),
    source(nullptr),
    sourceKind(PluginSourceKind::GLOBAL)
  {
  };

  plugin(const plugin& other)
    : pCategory(other.pCategory),
    pName(other.pName),
    pType(other.pType),
    pFunction(other.pFunction),
    priority(other.priority),
    target(other.target),
    source(other.source),
    sourceKind(other.sourceKind)
  {
  };

  template <class unknown_rawType>
  plugin(
      pluginCategory _category,
      const std::string& _name,
      pluginType _pType,
      unknown_rawType _function,
      int32_t _priority = 0,
      pluginTarget _target = current_team)
      : pCategory(_category),
        pName(_name),
        pType(_pType),
        pFunction((void*)_function),
        priority(_priority),
        target(_target),
        source(nullptr),
        sourceKind(PluginSourceKind::GLOBAL){};

  ~plugin() { };

  pluginType getType() const { return pType; };

  void* getFunction() const { return pFunction; };

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

  const class Pluggable* getSource() const { return source; }
  PluginSourceKind getSourceKind() const { return sourceKind; }
  void setSource(const class Pluggable* _source);

  bool isActive(const IPluginEvaluationContext& ctx) const;
  bool isActiveAtBeginningOfGame(
      const EnvironmentVolatile& env, const Actor& actor) const;

  bool operator== (const plugin& other) const
  {
    return pFunction == other.pFunction && source == other.source;
  };

  bool operator< (const plugin& other) const
  {
    if (priority != other.priority) {
      return priority < other.priority;
    } else if (pFunction != other.pFunction) {
      return pFunction < other.pFunction;
    } else {
      return source < other.source;
    }
  };
};

class PKAISHARED IPluginEvaluationContext {
 public:
  virtual ~IPluginEvaluationContext() = default;
  virtual EnvironmentVolatile getEnv() const = 0;
  virtual const Actor& getCActor() const = 0;
  virtual const Actor& getTarget() const = 0;
  virtual const Action& getCAction() const = 0;
  virtual const Move& getMV() const = 0;
};

class PKAISHARED PluggableInterface
{
 public:
  virtual bool registerPlugin(const plugin& cPlugin, bool setImp = true) = 0;
};

class PKAISHARED Pluggable : public PluggableInterface
{
private:
  static plugin emptyPlugin;
  std::array<plugin, PLUGIN_MAXSIZE> plugins;
  bool implemented;

public:
  virtual PluginSourceKind getSourceKind() const { return PluginSourceKind::GLOBAL; }

  bool registerPlugin_void(
    pluginType pType, 
    voidFunction_rawType _function, 
    int32_t _priority = 0, 
    pluginTarget _target = current_team,
    bool setIsImplemented = true);
protected:
public:
  Pluggable()
    : plugins(),
    implemented(false)
  {
    plugins.fill(emptyPlugin);
  };

  bool registerPlugin(const plugin& cPlugin, bool setImp);

  void setHasNoPlugins()
  {
    plugins.fill(emptyPlugin);
    implemented = true;
  };

  void removePlugin(size_t pType)
  {
    plugins[pType] = emptyPlugin;
  };

  virtual bool isImplemented() const
  {
    return implemented;
  };

  pluginTarget getTarget(size_t pType) const
  {
    return plugins[pType].getTarget();
  };

  int32_t getPriority(size_t pType) const
  {
    return plugins[pType].getPriority();
  };

  const plugin& getPlugin(size_t pType) const
  {
    return plugins[pType];
  };

  void* getFunction(size_t pType) const {
    return plugins[pType].getFunction();
  };
};

class PKAISHARED EnginePlugins : public PluggableInterface
{
private:
  std::array<std::vector<plugin>, PLUGIN_MAXSIZE> plugins;

protected:
public:
  EnginePlugins()
    : plugins()
  {
  };

  bool registerPlugin(const plugin& cPlugin, bool setImp);

  size_t getNumPlugins() const {
    size_t count = 0;
    for (const auto& pluginset: plugins) {
      count += pluginset.size();
    }

    return count;
  }

  size_t getNumPlugins(size_t pType) const
  {
    return plugins[pType].size();
  }

  uint32_t getPriority(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin].getPriority();
  };

  pluginTarget getTarget(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin].getTarget();
  };

  const plugin& getPlugin(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin];
  }

  void* getFunction(size_t pType, size_t iPlugin) const {
    return plugins[pType][iPlugin].getFunction();
  };
};

#endif /* PLUGGABLE_H */