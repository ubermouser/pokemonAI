#ifndef PLUGGABLE_TYPES_H
#define PLUGGABLE_TYPES_H

#include "../inc/pkai.h"

#include <stdint.h>

class plugin;
class move_nonvolatile;
class pokemon_nonvolatile;
class type;
class pkCU;

union pokemon_volatile;
union team_volatile;

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
	pkCU&,
	const pokemon_nonvolatile&, 
	team_volatile&,
	pokemon_volatile&);

typedef int (*onEvaluateMove_rawType)
	(
	pkCU&,
	const move_nonvolatile&, 
	const pokemon_nonvolatile&, 
	const pokemon_nonvolatile&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&);

typedef int (*onModifyBracket_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const team_volatile&,
	const pokemon_volatile&,
	int32_t&);

typedef int (*onModifySpeed_rawType)
	(pkCU&,
	const pokemon_nonvolatile&,
	const team_volatile&,
	const pokemon_volatile&,
	uint32_t&);

typedef int (*onEndOfRound_rawType)
	(pkCU&,
	const pokemon_nonvolatile&,
	team_volatile&,
	pokemon_volatile&
	);

typedef int (*onBeginningOfTurn_rawType)
	(pkCU&,
	const pokemon_nonvolatile&,
	team_volatile&,
	pokemon_volatile&
	);

typedef int (*onSetPower_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const pokemon_nonvolatile&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&,
	uint32_t&);

typedef int (*onModifyBasePower_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const pokemon_nonvolatile&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&,
	uint32_t&);

typedef int (*onModifyPower_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const pokemon_nonvolatile&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&,
	fpType&);

typedef int (*onModifyTypePower_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const pokemon_nonvolatile&,
	const type&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&,
	fpType&);

typedef int (*onModifyMoveType_rawType)
	(pkCU&,
	const move_nonvolatile&,
	const pokemon_nonvolatile&,
	const pokemon_nonvolatile&,
	team_volatile&,
	team_volatile&,
	pokemon_volatile&,
	pokemon_volatile&,
	const type*&);

typedef int (*onEndOfTurn_rawType)
	(pkCU&,
	const pokemon_nonvolatile&,
	team_volatile&,
	pokemon_volatile&
	);

typedef int (*onInitMove_rawType)
	(pokemon_nonvolatile&,
	move_nonvolatile&);

typedef void (*voidFunction_rawType)(void*);

//typedef bool (*regExtension_rawType)(const pokedex&, std::vector<plugin>&);

#endif /* PLUGGABLE_TYPES_H */
