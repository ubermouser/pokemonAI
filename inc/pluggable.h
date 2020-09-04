#ifndef PLUGGABLE_H
#define PLUGGABLE_H

#include "pkai.h"

#include <vector>
#include <array>
#include <boost/function.hpp>

class plugin;
typedef void (*voidFunction_rawType)(void*);

#define PLUGIN_MAXSIZE 28
enum pluginType
{
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
  PLUGIN_ON_UNINIT = 27
};


struct plugin_t
{
  /* raw pointer to actual function */
  voidFunction_rawType pFunction;

  /* a lower priority function is executed first, and has the ability to stop higher priority functions fro executing */
  int32_t priority;

  /* may be one of TEAM_A(CTEAM), TEAM_B(OTEAM), or  2 (both) to register to all teammates in contact with this plugin */
  uint32_t target;

  bool operator== (const plugin_t& other) const
  {
    return pFunction == other.pFunction;
  };

  bool operator< (const plugin_t& other) const
  {
    return priority < other.priority;
  };
};

class PKAISHARED PluggableInterface
{
public:
  virtual bool registerPlugin(const plugin& cPlugin, bool setImp = true) = 0;
};

class PKAISHARED Pluggable : public PluggableInterface
{
private:
  static plugin_t emptyPlugin;
  std::array<plugin_t, PLUGIN_MAXSIZE> plugins;
  bool implemented;

  bool registerPlugin_void(
    pluginType pType, 
    voidFunction_rawType _function, 
    int32_t _priority = 0, 
    uint32_t _target = TEAM_A, 
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

  uint32_t getTarget(size_t pType) const
  {
    return plugins[pType].target;
  };

  int32_t getPriority(size_t pType) const
  {
    return plugins[pType].priority;
  };

  const plugin_t& getPlugin(size_t pType) const
  {
    return plugins[pType];
  };

  voidFunction_rawType getFunction(size_t pType) const
  {
    return plugins[pType].pFunction;
  };
};

class PKAISHARED EnginePlugins : public PluggableInterface
{
private:
  std::array<std::vector<plugin_t>, PLUGIN_MAXSIZE> plugins;

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
    return plugins[pType][iPlugin].priority;
  };

  int32_t getTarget(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin].target;
  };

  const plugin_t& getPlugin(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin];
  }

  voidFunction_rawType getFunction(size_t pType, size_t iPlugin) const
  {
    return plugins[pType][iPlugin].pFunction;
  };
};

#endif /* PLUGGABLE_H */