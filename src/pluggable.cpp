#include "pokemonai/pluggable.h"

#include "pokemonai/plugin.h"


plugin Pluggable::emptyPlugin = plugin();


bool Pluggable::registerPlugin_void(
    pluginType pType,
    voidFunction_rawType _function,
    int32_t _priority,
    pluginTarget _target,
    bool setIsImplemented) {
  bool existed = (plugins[(size_t)pType].getFunction() != NULL);
  {
    plugin result = plugin(engine, "", pType, _function, _priority, _target);
    plugins[(size_t)pType] = result;
  }

  if (setIsImplemented) { implemented = true; }

  return existed;
};


bool Pluggable::registerPlugin(const plugin& cPlugin, bool setImp) {
  bool existed = (plugins[(size_t)cPlugin.getType()].getFunction() != NULL);
  plugins[(size_t)cPlugin.getType()] = cPlugin;
  plugins[(size_t)cPlugin.getType()].setSource(this);

  if (setImp) { implemented = true; }
  return existed;
}


bool EnginePlugins::registerPlugin(const plugin& cPlugin, bool setImp) {
  auto& pluginSet = plugins[(size_t)cPlugin.getType()];
  pluginSet.push_back(cPlugin);
  pluginSet.back().setSource(nullptr);
  return false;
}


const char* pluginTargetToString(pluginTarget target) {
  switch (target) {
  case current_team:
    return "current_team";
  case other_team:
    return "other_team";
  case all_teams:
    return "all_teams";
  default:
    return "unknown_target";
  }
}


const char* pluginCategoryToString(pluginCategory category) {
  switch (category) {
  case move:
    return "move";
  case ability:
    return "ability";
  case item:
    return "item";
  case engine:
    return "engine";
  default:
    return "unknown_category";
  }
}


const char* pluginTypeToString(pluginType type) {
  switch (type) {
  case PLUGIN_ON_INIT:
    return "PLUGIN_ON_INIT";
  case PLUGIN_ON_RESET:
    return "PLUGIN_ON_RESET";
  case PLUGIN_ON_SETSPEEDBRACKET:
    return "PLUGIN_ON_SETSPEEDBRACKET";
  case PLUGIN_ON_MODIFYSPEED:
    return "PLUGIN_ON_MODIFYSPEED";
  case PLUGIN_ON_BEGINNINGOFTURN:
    return "PLUGIN_ON_BEGINNINGOFTURN";
  case PLUGIN_ON_EVALUATEMOVE:
    return "PLUGIN_ON_EVALUATEMOVE";
  case PLUGIN_ON_SETBASEPOWER:
    return "PLUGIN_ON_SETBASEPOWER";
  case PLUGIN_ON_MODIFYBASEPOWER:
    return "PLUGIN_ON_MODIFYBASEPOWER";
  case PLUGIN_ON_MODIFYATTACKPOWER:
    return "PLUGIN_ON_MODIFYATTACKPOWER";
  case PLUGIN_ON_MODIFYCRITICALPOWER:
    return "PLUGIN_ON_MODIFYCRITICALPOWER";
  case PLUGIN_ON_MODIFYRAWDAMAGE:
    return "PLUGIN_ON_MODIFYRAWDAMAGE";
  case PLUGIN_ON_SETMOVETYPE:
    return "PLUGIN_ON_SETMOVETYPE";
  case PLUGIN_ON_MODIFYSTAB:
    return "PLUGIN_ON_MODIFYSTAB";
  case PLUGIN_ON_SETDEFENSETYPE:
    return "PLUGIN_ON_SETDEFENSETYPE";
  case PLUGIN_ON_MODIFYITEMPOWER:
    return "PLUGIN_ON_MODIFYITEMPOWER";
  case PLUGIN_ON_MODIFYHITPROBABILITY:
    return "PLUGIN_ON_MODIFYHITPROBABILITY";
  case PLUGIN_ON_MODIFYCRITPROBABILITY:
    return "PLUGIN_ON_MODIFYCRITPROBABILITY";
  case PLUGIN_ON_CALCULATEDAMAGE:
    return "PLUGIN_ON_CALCULATEDAMAGE";
  case PLUGIN_ON_ENDOFMOVE:
    return "PLUGIN_ON_ENDOFMOVE";
  case PLUGIN_ON_MODIFYSECONDARYPROBABILITY:
    return "PLUGIN_ON_MODIFYSECONDARYPROBABILITY";
  case PLUGIN_ON_SECONDARYEFFECT:
    return "PLUGIN_ON_SECONDARYEFFECT";
  case PLUGIN_ON_ENDOFTURN:
    return "PLUGIN_ON_ENDOFTURN";
  case PLUGIN_ON_ENDOFROUND:
    return "PLUGIN_ON_ENDOFROUND";
  case PLUGIN_ON_SWITCHOUT:
    return "PLUGIN_ON_SWITCHOUT";
  case PLUGIN_ON_SWITCHIN:
    return "PLUGIN_ON_SWITCHIN";
  case PLUGIN_ON_TESTMOVE:
    return "PLUGIN_ON_TESTMOVE";
  case PLUGIN_ON_TESTSWITCH:
    return "PLUGIN_ON_TESTSWITCH";
  case PLUGIN_ON_MODIFYACTION:
    return "PLUGIN_ON_MODIFYACTION";
  case PLUGIN_ON_EXECUTESWITCH:
    return "PLUGIN_ON_EXECUTESWITCH";
  case PLUGIN_ON_UNINIT:
    return "PLUGIN_ON_UNINIT";
  case PLUGIN_ON_POSTROUND:
    return "PLUGIN_ON_POSTROUND";
  default:
    return "PLUGIN_UNKNOWN";
  }
}