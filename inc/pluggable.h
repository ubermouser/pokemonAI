#ifndef PLUGGABLE_H
#define PLUGGABLE_H

#include "../inc/pkai.h"
#include "../inc/pluggable_types.h"

#include <vector>
#include <boost/array.hpp>
#include <boost/function.hpp>

class plugin;

struct plugin_t
{
  /* raw pointer to actual function */
  voidFunction_rawType pFunction;

  /* a lower priority function is executed first, and has the ability to stop higher priority functions fro executing */
  uint32_t priority;

  /* may be one of TEAM_A(CTEAM), TEAM_B(OTEAM), or  2 (both) to register to all teammates in contact with this plugin */
  int32_t target;

  bool operator== (const plugin_t& other) const
  {
    return pFunction == other.pFunction;
  };

  bool operator< (const plugin_t& other) const
  {
    return priority < other.priority;
  };
};

class PKAISHARED pluggableInterface
{
public:
  virtual bool registerPlugin(const plugin& cPlugin, bool setImp = true) = 0;
};

class PKAISHARED pluggable : public pluggableInterface
{
private:
  static plugin_t emptyPlugin;
  boost::array<plugin_t, PLUGIN_MAXSIZE> plugins;
  bool implemented;

  bool registerPlugin_void(
    pluginType pType, 
    voidFunction_rawType _function, 
    int32_t _priority = 0, 
    uint32_t _target = TEAM_A, 
    bool setIsImplemented = true);
protected:
public:
  pluggable()
    : plugins(),
    implemented(false)
  {
    plugins.assign(emptyPlugin);
  };

  bool registerPlugin(const plugin& cPlugin, bool setImp);

  void setHasNoPlugins()
  {
    plugins.assign(emptyPlugin);
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

class PKAISHARED enginePlugins : public pluggableInterface
{
private:
  boost::array<std::vector<plugin_t>, PLUGIN_MAXSIZE> plugins;

protected:
public:
  enginePlugins()
    : plugins()
  {
  };

  bool registerPlugin(const plugin& cPlugin, bool setImp);

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