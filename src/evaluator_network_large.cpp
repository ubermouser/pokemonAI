#include "pokemonai/evaluator_network_large.h"

#include <algorithm>

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
typedef BinaryEmbedding<5> TypeEmbedding;
typedef BinaryEmbedding<4> ToxicPoisonEmbedding;
typedef BinaryEmbedding<2> DamageTypeEmbedding;
typedef BinaryEmbedding<4> NonvolatileStatusEmbedding;
typedef BinaryEmbedding<3> ActivePokemonEmbedding;

struct FVec_NonVolatileStatus {
  NonvolatileStatusEmbedding nonvolatileStatus;


  void seed(const PokemonVolatile& pkmn) {
    nonvolatileStatus = NonvolatileStatusEmbedding{pkmn.getStatusAilment()};
  }
};

struct FVec_VolatileStatus {
  float isConfused;
  float isTrapped;
  float isLeechSeed;
  float hasSubstitute;
  ToxicPoisonEmbedding toxicPoison;

  void seed(const VolatileStatus& status) {
    isConfused = status.confused > 0 ? 1.0 : 0.0;
    isTrapped = status.trap > 0 ? 1.0 : 0.0;
    isLeechSeed = status.leechSeed > 0 ? 1.0 : 0.0;
    hasSubstitute = status.substitute > 0 ? 1.0 : 0.0;
    toxicPoison = ToxicPoisonEmbedding{status.toxicPoison_tier};
  }
};


struct FVec_MoveNonVolatile {
  DamageTypeEmbedding damageType;
  MoveEmbedding move;
  TypeEmbedding type;

  void seed(const MoveNonVolatile& mNV) {
    damageType = DamageTypeEmbedding{mNV.getBase().getDamageType()};
    move = MoveEmbedding{mNV.getBase().index_};
    type = TypeEmbedding{mNV.getBase().getType().index_};
  }
};


struct FVec_MoveVolatile {
  float pp;

  void seed(const ConstMoveVolatile& mV) {
    pp = (mV.hasPP() ? 0.1 : 0.0) + (mV.getPercentPP() * 0.9);
  }
};


struct FVec_PokemonNonVolatile {
  AbilityEmbedding ability;
  SpeciesEmbedding species;
  std::array<FVec_MoveNonVolatile, 4> moves;
  std::array<TypeEmbedding, 2> types;


  void seed(const PokemonNonVolatile& pkmn) {
    for (size_t iMove = 0; iMove != pkmn.getNumMoves(); ++iMove) {
      moves[iMove].seed(pkmn.getMove(iMove));
    }

    ability = AbilityEmbedding{pkmn.getAbility().index_};
    species = SpeciesEmbedding{pkmn.getBase().index_};
    types[0] = TypeEmbedding{pkmn.getBase().getType(0).index_};
    types[1] = TypeEmbedding{pkmn.getBase().getType(1).index_};
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
  ActivePokemonEmbedding activePokemon;

  void seed(const ConstTeamVolatile& teamV) {
    activePokemon = ActivePokemonEmbedding{teamV.getICPKV()};
    volatileStatus.seed(teamV.getVolatile());
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


template <class Base>
void EvaluatorNetworkLarge_impl<Base>::seed(
    FeatureVector::floatIterator_t inputBegin,
    const ConstEnvironmentVolatile& env,
    size_t _iTeam) const {
  assert(!precomputedNV_[_iTeam].empty());
  const size_t envNVSize = sizeof(FVec_EnvironmentNonVolatile) / sizeof(float);

  // 1. Copy precomputed NonVolatile portion
  std::copy(
      precomputedNV_[_iTeam].begin(), precomputedNV_[_iTeam].end(), inputBegin);

  // 2. Zero-initialize and seed Volatile portion directly into the output
  // buffer
  std::fill(inputBegin + envNVSize, inputBegin + numInputNeurons, 0.0f);
  FVec_EnvironmentVolatile* fVecV =
      reinterpret_cast<FVec_EnvironmentVolatile*>(&(*(inputBegin + envNVSize)));
  fVecV->seed(env, _iTeam);
}


template <class Base>
const size_t EvaluatorNetworkLarge_impl<Base>::numInputNeurons =
    sizeof(FeatureVectorLarge) / sizeof(float);

template <class Base>
const size_t EvaluatorNetworkLarge_impl<Base>::numOutputNeurons = 1U;


template <class Base>
EvaluatorNetworkLarge_impl<Base>::EvaluatorNetworkLarge_impl(const Config& cfg)
    : Base(cfg, inputSize(), outputSize()) {
  this->updateIdent();
}


template <class Base>
EvaluatorNetworkLarge_impl<Base>::EvaluatorNetworkLarge_impl(
    const EvaluatorNetworkLarge_impl& other)
    : Base(other), precomputedNV_(other.precomputedNV_) {}


template <class Base>
EvaluatorNetworkLarge_impl<Base>::EvaluatorNetworkLarge_impl(
    const neuralNet& cNet, const Config& cfg)
    : Base(cNet, cfg) {
  this->updateIdent();
}


template <class Base>
EvaluatorNetworkLarge_impl<Base>&
EvaluatorNetworkLarge_impl<Base>::setEnvironment(
    const std::shared_ptr<const EnvironmentNonvolatile>& env) {
  Base::setEnvironment(env);

  const size_t envNVSize = sizeof(FVec_EnvironmentNonVolatile) / sizeof(float);

  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    precomputedNV_[iTeam].resize(envNVSize);
    FVec_EnvironmentNonVolatile envNV{};
    envNV.seed(*env, iTeam);
    std::copy(
        reinterpret_cast<const float*>(&envNV),
        reinterpret_cast<const float*>(&envNV) + envNVSize,
        precomputedNV_[iTeam].begin());
  }

  return *this;
}

// Explicit instantiations
template class EvaluatorNetworkLarge_impl<EvaluatorNetwork>;
template class EvaluatorNetworkLarge_impl<TrainableEvaluatorNetwork>;
