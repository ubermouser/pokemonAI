#include "pokemonai/evaluator_networkLarge.h"

#include "pokemonai/move_nonvolatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/evaluator_network.h"
#include "pokemonai/binary_embedding.h"
#include "pokemonai/positional_embedding.h"

evaluator_networkLarge::evaluator_networkLarge(const Config& cfg) : EvaluatorNetwork(cfg) {}
evaluator_networkLarge::evaluator_networkLarge(const evaluator_networkLarge& other) : EvaluatorNetwork(other) {}
evaluator_networkLarge::evaluator_networkLarge(const neuralNet& cNet, const Config& cfg) : EvaluatorNetwork(cNet, cfg) {}
evaluator_networkLarge* evaluator_networkLarge::clone() const { return new evaluator_networkLarge(*this); }

typedef PositionalEmbedding<5, 0, 1024, 1> FinalValueEmbedding;
typedef BinaryEmbedding<7> AbilityEmbedding;
typedef BinaryEmbedding<9> SpeciesEmbedding;
typedef BinaryEmbedding<9> MoveEmbedding;
typedef BinaryEmbedding<9> ItemEmbedding;
typedef BinaryEmbedding<4> ToxicPoisonEmbedding;
typedef BinaryEmbedding<5> TypeEmbedding;
typedef BinaryEmbedding<2> DamageTypeEmbedding;

struct FVec_NonVolatileStatus {
  float isSleeping;
  float isParalyzed;
  float isBurned;
  float isFrozen;
  ToxicPoisonEmbedding toxicPoison;

  void seed(const PokemonVolatile& pkmn) {
    isFrozen = pkmn.getStatusAilment() == AIL_NV_FREEZE ? 1.0 : 0.0;
    isBurned = pkmn.getStatusAilment() == AIL_NV_BURN ? 1.0 : 0.0;
    isParalyzed = pkmn.getStatusAilment() == AIL_NV_PARALYSIS ? 1.0 : 0.0;
  }
};

struct FVec_VolatileStatus {
  float isConfused;
  float isTrapped;
  float isLeechSeed;
  float hasSubstitute;

  void seed(const VolatileStatus& status) {
    isConfused = status.confused > 0 ? 1.0 : 0.0;
    isTrapped = status.trap > 0 ? 1.0 : 0.0;
    isLeechSeed = status.leechSeed > 0 ? 1.0 : 0.0;
    hasSubstitute = status.substitute > 0 ? 1.0 : 0.0;
  }
};


struct FVec_MoveNonVolatile {
  DamageTypeEmbedding damageType;
  MoveEmbedding move;
  TypeEmbedding type;

  void seed(const MoveNonVolatile& mNV) {

  }
};


struct FVec_MoveVolatile {
  float pp;

  void seed (const MoveVolatile& mV) {
    pp = mV.hasPP() ? 0.1 : 0.0 + (mV.getPercentPP() * 0.9);
  }
};


struct FVec_PokemonNonVolatile {
  AbilityEmbedding ability;
  SpeciesEmbedding species;
  std::array<FVec_MoveNonVolatile, 4> moves;


  void seed(const PokemonNonVolatile& pkmn) {
    for (size_t iMove = 0; iMove != 4; ++iMove) {
      moves[iMove].seed(pkmn.getMove(iMove));
    }

    size_t iAbility = pkdex->getAbilities().find(pkmn.getAbility().getName()) - pkdex->getAbilities().begin();
    ability.index_ = iAbility;
  }
};


struct FVec_PokemonVolatile {
  std::array<FVec_MoveVolatile, 4> moves;
  std::array<FinalValueEmbedding, 6> finalValues;
  FinalValueEmbedding currentHitpoints;
  ItemEmbedding item;
  FVec_NonVolatileStatus nonVolatileStatus;

  void seed(const PokemonVolatile& pkmn) {
    for (size_t iMove = 0; iMove != 4; ++iMove) {
      moves[iMove].seed(pkmn.getMV(iMove));
    }

    // Final Values (atk, spa, def, spd, spe, max-HP), including current boost stage:
    for (size_t iFV = 0; iFV != finalValues.size(); ++iFV) {
      finalValues[iFV] = FinalValueEmbedding{pkmn.getFV_boosted(iFV)};
    }

    // Current HP:
    currentHitpoints = FinalValueEmbedding{pkmn.getHP()};

    // held item:
    item = ItemEmbedding{pkmn.data().iHeldItem};
  }
};


struct FVec_TeamNonVolatile {
  std::array<FVec_PokemonNonVolatile, 6> teammates;

  void seed(const TeamNonVolatile& teamNV) {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
      teammates[iTeammate].seed(teamNV.teammate(iTeammate));
    }
  }
};


struct FVec_TeamVolatile {
  std::array<FVec_PokemonVolatile, 6> teammates;
  FVec_VolatileStatus volatileStatus;

  void seed(const TeamVolatile& teamV) {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
      teammates[iTeammate].seed(teamV.teammate(iTeammate));
    }
  }
};


struct FVec_EnvironmentNonVolatile {
  std::array<FVec_TeamNonVolatile, 2> teams;

  void seed(const EnvironmentNonvolatile& envNV) {
    for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
      teams[iTeam].seed(envNV.getTeam(iTeam));
    }
  }
};


struct FVec_EnvironmentVolatile {
  std::array<FVec_TeamVolatile, 2> teams;

  void seed(const EnvironmentVolatile& envV) {
    for (size_t iTeam = 0; iTeam != 2; ++iTeam) {
      teams[iTeam].seed(envV.getTeam(iTeam));
    }
  }
};


struct FeatureVectorLarge {
  FVec_EnvironmentNonVolatile envNV;
  FVec_EnvironmentVolatile envV;
};
