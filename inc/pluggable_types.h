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

//typedef bool (*regExtension_rawType)(const pokedex&, std::vector<plugin>&);

#endif /* PLUGGABLE_TYPES_H */
