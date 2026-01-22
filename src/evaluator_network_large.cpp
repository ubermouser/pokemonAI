#include "pokemonai/evaluator_network_large.h"

#include "pokemonai/binary_embedding.h"
#include "pokemonai/evaluator_network.h"
#include "pokemonai/move_nonvolatile.h"
#include "pokemonai/pkai.h"
#include "pokemonai/positional_embedding.h"


typedef PositionalEmbedding<5, 1, 1024, 1> FinalValueEmbedding;
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
    damageType = DamageTypeEmbedding{mNV.getBase().getDamageType()};
  }
};


struct FVec_MoveVolatile {
  float pp;

  void seed(const ConstMoveVolatile& mV) {
    pp = mV.hasPP() ? 0.1 : 0.0 + (mV.getPercentPP() * 0.9);
  }
};


struct FVec_PokemonNonVolatile {
  AbilityEmbedding ability;
  SpeciesEmbedding species;
  std::array<FVec_MoveNonVolatile, 4> moves;


  void seed(const PokemonNonVolatile& pkmn) {
    for (size_t iMove = 0; iMove != pkmn.getNumMoves(); ++iMove) {
      moves[iMove].seed(pkmn.getMove(iMove));
    }

    // TODO - find the index of the ability, species, move, etc
    //size_t iAbility = pkdex->getAbilities().find(pkmn.getAbility().getName()) - pkdex->getAbilities().cbegin();
    //ability = AbilityEmbedding{iAbility};
  }
};


struct FVec_PokemonVolatile {
  std::array<FVec_MoveVolatile, 4> moves;
  std::array<FinalValueEmbedding, 6> finalValues;
  FinalValueEmbedding currentHitpoints;
  ItemEmbedding item;
  FVec_NonVolatileStatus nonVolatileStatus;

  void seed(const ConstPokemonVolatile& pkmn) {
    for (size_t iMove = 0; iMove != pkmn.nv().getNumMoves(); ++iMove) {
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
    for (size_t iTeammate = 0; iTeammate != teamNV.getNumTeammates();
         ++iTeammate) {
      teammates[iTeammate].seed(teamNV.teammate(iTeammate));
    }
  }
};


struct FVec_TeamVolatile {
  std::array<FVec_PokemonVolatile, 6> teammates;
  FVec_VolatileStatus volatileStatus;

  void seed(const ConstTeamVolatile& teamV) {
    for (size_t iTeammate = 0; iTeammate != teamV.nv().getNumTeammates();
         ++iTeammate) {
      teammates[iTeammate].seed(teamV.teammate(iTeammate));
    }
  }
};


struct FVec_EnvironmentNonVolatile {
  std::array<FVec_TeamNonVolatile, 2> teams;

  void seed(const EnvironmentNonvolatile& envNV, size_t cTeam) {
    teams[0].seed(envNV.getTeam(cTeam));
    teams[1].seed(envNV.getOtherTeam(cTeam));
  }
};


struct FVec_EnvironmentVolatile {
  std::array<FVec_TeamVolatile, 2> teams;

  void seed(const ConstEnvironmentVolatile& envV, size_t cTeam) {
    teams[0].seed(envV.getTeam(cTeam));
    teams[1].seed(envV.getOtherTeam(cTeam));
  }
};


struct FeatureVectorLarge {
  FVec_EnvironmentNonVolatile envNV;
  FVec_EnvironmentVolatile envV;
};


union FeatureVectorLargeUnion {
  FeatureVectorLarge fVec;
  std::array<float, sizeof(FeatureVectorLarge) / sizeof(float)> fVecAsFloat;
};


void EvaluatorNetworkLarge::seed(
    FeatureVector::floatIterator_t inputBegin,
    const ConstEnvironmentVolatile& env,
    size_t _iTeam) const {
  FeatureVectorLargeUnion fVec = {};
  // TODO - precompute the NonVolatile portion and just copy
  fVec.fVec.envNV.seed(env.nv(), _iTeam);
  fVec.fVec.envV.seed(env, _iTeam);

  std::copy(fVec.fVecAsFloat.begin(), fVec.fVecAsFloat.end(), inputBegin);
}


const size_t EvaluatorNetworkLarge::numInputNeurons =
    sizeof(FeatureVectorLarge) / sizeof(float);
const size_t EvaluatorNetworkLarge::numOutputNeurons = 1U;


EvaluatorNetworkLarge::EvaluatorNetworkLarge(const Config& cfg)
    : EvaluatorNetwork(cfg, inputSize(), outputSize()) {}


EvaluatorNetworkLarge::EvaluatorNetworkLarge(const EvaluatorNetworkLarge& other)
    : EvaluatorNetwork(other) {}


EvaluatorNetworkLarge::EvaluatorNetworkLarge(
    const neuralNet& cNet, const Config& cfg)
    : EvaluatorNetwork(cNet, cfg) {}


EvaluatorNetworkLarge* EvaluatorNetworkLarge::clone() const {
  return new EvaluatorNetworkLarge(*this);
}
