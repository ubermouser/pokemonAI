#ifndef PLUGGABLE_TYPES_H
#define PLUGGABLE_TYPES_H

#include "pkai.h"

#include <stdint.h>

#include "engine.h"
#include "pkCU.h"

// script types:

typedef int (*onSwitch_rawType)
  (
  PkCUEngine&,
  PokemonVolatile);

typedef int (*onEvaluateMove_rawType)
  (
  PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile);

typedef int (*onModifyBracket_rawType)
  (PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  int32_t&);

typedef int (*onModifySpeed_rawType)
  (PkCUEngine&,
  PokemonVolatile,
  uint32_t&);

typedef int (*onEndOfRound_rawType)
  (PkCUEngine&,
  PokemonVolatile
  );

typedef int (*onBeginningOfTurn_rawType)
  (PkCUEngine&,
  PokemonVolatile
  );

typedef int (*onSetPower_rawType)
  (PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  uint32_t&);

typedef int (*onModifyBasePower_rawType)
  (PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  uint32_t&);

typedef int (*onModifyPower_rawType)
  (PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  fpType&);

typedef int (*onModifyTypePower_rawType)
  (PkCUEngine&,
  const Type&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  fpType&);

typedef int (*onModifyProbability_rawType)(
    PkCUEngine&, MoveVolatile, PokemonVolatile, PokemonVolatile, FixType&);

typedef int (*onModifyMoveType_rawType)
  (PkCUEngine&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  const Type*&);

typedef int (*onEndOfTurn_rawType)
  (PkCUEngine&,
  PokemonVolatile
  );

typedef int (*onInitMove_rawType)
  (PokemonNonVolatile&,
  MoveNonVolatile&);

typedef int (*onTestMove_rawType)
  (ConstTeamVolatile,
  ConstPokemonVolatile,
  ConstMoveVolatile,
  const Action&,
  ValidMoveSet&);

typedef int (*onTestSwitch_rawType)
  (ConstPokemonVolatile,
  ConstPokemonVolatile,
  const Action&,
  ValidSwapSet&);

typedef int (*onModifyAction_rawType)(PkCUEngine&, Action&);
typedef int (*onReset_rawType)(PkCUEngine&, void*);

// Plugin Signature Mapping
template <pluginType P>
struct PluginSignature;

template <> struct PluginSignature<PLUGIN_ON_INIT> { using type = onInitMove_rawType; };
template <> struct PluginSignature<PLUGIN_ON_RESET> { using type = onReset_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SETSPEEDBRACKET> { using type = onModifyBracket_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYSPEED> { using type = onModifySpeed_rawType; };
template <> struct PluginSignature<PLUGIN_ON_BEGINNINGOFTURN> { using type = onBeginningOfTurn_rawType; };
template <> struct PluginSignature<PLUGIN_ON_EVALUATEMOVE> { using type = onEvaluateMove_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SETBASEPOWER> { using type = onSetPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYBASEPOWER> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYATTACKPOWER> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYCRITICALPOWER> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYRAWDAMAGE> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SETMOVETYPE> { using type = onModifyMoveType_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYSTAB> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SETDEFENSETYPE> { using type = onModifyTypePower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYITEMPOWER> { using type = onModifyPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYHITPROBABILITY> { using type = onModifyProbability_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYCRITPROBABILITY> { using type = onModifyProbability_rawType; };
template <> struct PluginSignature<PLUGIN_ON_CALCULATEDAMAGE> { using type = onSetPower_rawType; };
template <> struct PluginSignature<PLUGIN_ON_ENDOFMOVE> { using type = onEvaluateMove_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYSECONDARYPROBABILITY> { using type = onModifyProbability_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SECONDARYEFFECT> { using type = onEvaluateMove_rawType; };
template <> struct PluginSignature<PLUGIN_ON_ENDOFTURN> { using type = onEndOfTurn_rawType; };
template <> struct PluginSignature<PLUGIN_ON_ENDOFROUND> { using type = onEndOfRound_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SWITCHOUT> { using type = onSwitch_rawType; };
template <> struct PluginSignature<PLUGIN_ON_SWITCHIN> { using type = onSwitch_rawType; };
template <> struct PluginSignature<PLUGIN_ON_TESTMOVE> { using type = onTestMove_rawType; };
template <> struct PluginSignature<PLUGIN_ON_TESTSWITCH> { using type = onTestSwitch_rawType; };
template <> struct PluginSignature<PLUGIN_ON_MODIFYACTION> { using type = onModifyAction_rawType; };
template <> struct PluginSignature<PLUGIN_ON_UNINIT> { using type = onInitMove_rawType; };

// Convenience functions
#define PLUGIN_FACTORY(name, pType, pRawType) \
inline plugin plugin##name(pluginCategory category, \
                           const std::string& pluginName, \
                           pRawType function, \
                           int32_t priority = 0, \
                           pluginTarget target = current_team) { \
    return plugin(category, pluginName, pType, (void*)function, priority, target); \
}

PLUGIN_FACTORY(OnInit, PLUGIN_ON_INIT, onInitMove_rawType)
PLUGIN_FACTORY(OnReset, PLUGIN_ON_RESET, onReset_rawType)
PLUGIN_FACTORY(OnSetSpeedBracket, PLUGIN_ON_SETSPEEDBRACKET, onModifyBracket_rawType)
PLUGIN_FACTORY(OnModifySpeed, PLUGIN_ON_MODIFYSPEED, onModifySpeed_rawType)
PLUGIN_FACTORY(OnBeginningOfTurn, PLUGIN_ON_BEGINNINGOFTURN, onBeginningOfTurn_rawType)
PLUGIN_FACTORY(OnEvaluateMove, PLUGIN_ON_EVALUATEMOVE, onEvaluateMove_rawType)
PLUGIN_FACTORY(OnSetBasePower, PLUGIN_ON_SETBASEPOWER, onSetPower_rawType)
PLUGIN_FACTORY(OnModifyBasePower, PLUGIN_ON_MODIFYBASEPOWER, onModifyPower_rawType)
PLUGIN_FACTORY(OnModifyAttackPower, PLUGIN_ON_MODIFYATTACKPOWER, onModifyPower_rawType)
PLUGIN_FACTORY(OnModifyCriticalPower, PLUGIN_ON_MODIFYCRITICALPOWER, onModifyPower_rawType)
PLUGIN_FACTORY(OnModifyRawDamage, PLUGIN_ON_MODIFYRAWDAMAGE, onModifyPower_rawType)
PLUGIN_FACTORY(OnSetMoveType, PLUGIN_ON_SETMOVETYPE, onModifyMoveType_rawType)
PLUGIN_FACTORY(OnModifyStab, PLUGIN_ON_MODIFYSTAB, onModifyPower_rawType)
PLUGIN_FACTORY(OnSetDefenseType, PLUGIN_ON_SETDEFENSETYPE, onModifyTypePower_rawType)
PLUGIN_FACTORY(OnModifyItemPower, PLUGIN_ON_MODIFYITEMPOWER, onModifyPower_rawType)
PLUGIN_FACTORY(OnModifyHitProbability, PLUGIN_ON_MODIFYHITPROBABILITY, onModifyProbability_rawType)
PLUGIN_FACTORY(OnModifyCritProbability, PLUGIN_ON_MODIFYCRITPROBABILITY, onModifyProbability_rawType)
PLUGIN_FACTORY(OnCalculateDamage, PLUGIN_ON_CALCULATEDAMAGE, onSetPower_rawType)
PLUGIN_FACTORY(OnEndOfMove, PLUGIN_ON_ENDOFMOVE, onEvaluateMove_rawType)
PLUGIN_FACTORY(OnModifySecondaryProbability, PLUGIN_ON_MODIFYSECONDARYPROBABILITY, onModifyProbability_rawType)
PLUGIN_FACTORY(OnSecondaryEffect, PLUGIN_ON_SECONDARYEFFECT, onEvaluateMove_rawType)
PLUGIN_FACTORY(OnEndOfTurn, PLUGIN_ON_ENDOFTURN, onEndOfTurn_rawType)
PLUGIN_FACTORY(OnEndOfRound, PLUGIN_ON_ENDOFROUND, onEndOfRound_rawType)
PLUGIN_FACTORY(OnSwitchOut, PLUGIN_ON_SWITCHOUT, onSwitch_rawType)
PLUGIN_FACTORY(OnSwitchIn, PLUGIN_ON_SWITCHIN, onSwitch_rawType)
PLUGIN_FACTORY(OnTestMove, PLUGIN_ON_TESTMOVE, onTestMove_rawType)
PLUGIN_FACTORY(OnTestSwitch, PLUGIN_ON_TESTSWITCH, onTestSwitch_rawType)
PLUGIN_FACTORY(OnModifyAction, PLUGIN_ON_MODIFYACTION, onModifyAction_rawType)
PLUGIN_FACTORY(OnUninit, PLUGIN_ON_UNINIT, onInitMove_rawType)

#undef PLUGIN_FACTORY

#endif /* PLUGGABLE_TYPES_H */
