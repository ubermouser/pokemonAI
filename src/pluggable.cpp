
//#define PKAI_EXPORT
#include "pokemonai/pluggable.h"
//#undef PKAI_EXPORT

//#define PKAI_STATIC
#include "pokemonai/plugin.h"
//#undef PKAI_STATIC

plugin_t Pluggable::emptyPlugin = { NULL, 0, 0 };

bool Pluggable::registerPlugin_void(
  pluginType pType, 
  voidFunction_rawType _function, 
  int32_t _priority, 
  uint32_t _target,
  bool setIsImplemented)
{
  bool existed = (plugins[(size_t)pType].pFunction != NULL);
  {
    plugin_t result = { _function, _priority, _target };
    plugins[(size_t)pType] = result;
  }

  if (setIsImplemented) { implemented = true; }

  return existed;
};





bool Pluggable::registerPlugin(const plugin& cPlugin, bool setImp)
{
  return registerPlugin_void(cPlugin.getType(), cPlugin.getFunction(), cPlugin.getPriority(), cPlugin.getTarget(), setImp);
}





bool EnginePlugins::registerPlugin(const plugin& cPlugin, bool setImp)
{
  {
    plugin_t result = { cPlugin.getFunction(), cPlugin.getPriority(), cPlugin.getTarget() };
    plugins[(size_t)cPlugin.getType()].push_back(result);
  }

  return false;
}