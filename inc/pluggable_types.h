#ifndef PLUGGABLE_TYPES_H
#define PLUGGABLE_TYPES_H

#include "../inc/pkai.h"

#include <stdint.h>

class plugin;
class MoveNonVolatile;
class PokemonNonVolatile;
class Type;
class PkCU;

class MoveVolatile;
class PokemonVolatile;
class TeamVolatile;

// script types:
#define PLUGIN_MAXSIZE 26
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
  PLUGIN_ON_UNINIT = 25
};

typedef int (*onSwitch_rawType)
  (
  PkCU&,
  PokemonVolatile);

typedef int (*onEvaluateMove_rawType)
  (
  PkCU&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile);

typedef int (*onModifyBracket_rawType)
  (PkCU&,
  MoveVolatile,
  PokemonVolatile,
  int32_t&);

typedef int (*onModifySpeed_rawType)
  (PkCU&,
  PokemonVolatile,
  uint32_t&);

typedef int (*onEndOfRound_rawType)
  (PkCU&,
  PokemonVolatile
  );

typedef int (*onBeginningOfTurn_rawType)
  (PkCU&,
  PokemonVolatile
  );

typedef int (*onSetPower_rawType)
  (PkCU&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  uint32_t&);

typedef int (*onModifyBasePower_rawType)
  (PkCU&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  uint32_t&);

typedef int (*onModifyPower_rawType)
  (PkCU&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  fpType&);

typedef int (*onModifyTypePower_rawType)
  (PkCU&,
  const Type&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  fpType&);

typedef int (*onModifyMoveType_rawType)
  (PkCU&,
  MoveVolatile,
  PokemonVolatile,
  PokemonVolatile,
  const Type*&);

typedef int (*onEndOfTurn_rawType)
  (PkCU&,
  PokemonVolatile
  );

typedef int (*onInitMove_rawType)
  (PokemonNonVolatile,
  MoveNonVolatile);

typedef void (*voidFunction_rawType)(void*);

//typedef bool (*regExtension_rawType)(const pokedex&, std::vector<plugin>&);

#endif /* PLUGGABLE_TYPES_H */
