#include "pokemonai/evaluator_network128.h"
#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/environment_volatile.h"
#include "pokemonai/team_volatile.h"
#include "pokemonai/pokemon_volatile.h"
#include "pokemonai/type.h"
#include "pokemonai/move.h"
#include "pokemonai/pokemon_base.h"
#include "pokemonai/fp_compare.h"
#include <sstream>
#include <assert.h>
#include <iostream>

#define NEURONSPERTEAMMATE 10
#define NEURONSPERSTATUS 4
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)

template <class Base>
const size_t evaluator_network128_impl<Base>::numInputNeurons =
    (NEURONSPERTEAM * 2);

template <class Base>
const size_t evaluator_network128_impl<Base>::numOutputNeurons = 1U;

template <class Base>
evaluator_network128_impl<Base>::evaluator_network128_impl(
    const typename Base::Config& cfg)
    : Base(cfg, numInputNeurons, numOutputNeurons) {
  this->updateIdent();
}

template <class Base>
evaluator_network128_impl<Base>::evaluator_network128_impl(
    const evaluator_network128_impl& other)
    : Base(other) {}

template <class Base>
evaluator_network128_impl<Base>::evaluator_network128_impl(
    const neuralNet& _cNet, const typename Base::Config& cfg)
    : Base(_cNet, cfg) {
  this->updateIdent();
}

template <class Base>
void evaluator_network128_impl<Base>::seed(
    FeatureVector::floatIterator_t inputBegin,
    const ConstEnvironmentVolatile& env,
    size_t _iTeam) const {
  FeatureVector::floatIterator_t cInput;
  static const std::array<float, 13> statMultipliers = {{ 0.25f, 2.0f/7.0f, 2.0f/6.0f, 0.4f, 0.5f, 2.0f/3.0f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f }};

  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam) {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    size_t iOTeam = (iTeam + 1) & 1;
    const ConstTeamVolatile& cTV = env.getTeam(iTeam);
    assert(this->nv_ != nullptr);
    const TeamNonVolatile& cTNV = this->nv_->getTeam(iTeam);
    const ConstTeamVolatile& tTV = env.getTeam(iOTeam);
    const TeamNonVolatile& tTNV = this->nv_->getOtherTeam(iTeam);

    const std::array<uint8_t, 6>& iTeammates =
        this->orders_[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates =
        this->orders_[iOTeam][tTV.getICPKV()];

    // clang-format off
    std::array<float, 8> modifiers = {{ 
      1.0f, 
      statMultipliers[cTV.getPKV().getBoost(FV_ATTACK)+6], 
      statMultipliers[6-tTV.getPKV().getBoost(FV_DEFENSE)], 
      statMultipliers[cTV.getPKV().getBoost(FV_ATTACK)+6] * statMultipliers[6-tTV.getPKV().getBoost(FV_DEFENSE)], 
      1.0f, 
      statMultipliers[cTV.getPKV().getBoost(FV_SPATTACK)+6], 
      statMultipliers[6-tTV.getPKV().getBoost(FV_SPDEFENSE)], 
      statMultipliers[cTV.getPKV().getBoost(FV_SPATTACK)+6] * statMultipliers[6-tTV.getPKV().getBoost(FV_SPDEFENSE)] 
    }};
    // clang-format on

    cInput = inputBegin + (NEURONSPERTEAM * iNTeam);
    size_t numTeammatesAlive = cTNV.getNumTeammates();
    for (size_t iNTeammate = 0; iNTeammate != cTNV.getNumTeammates(); ++iNTeammate) {
      size_t iTeammate = iTeammates[iNTeammate];
      const ConstPokemonVolatile& cPKV = cTV.teammate(iTeammate);
      if (!cPKV.isAlive()) {
        if (iNTeammate == 0) {
          std::fill(cInput, cInput + NEURONSPERTEAMMATE, 0.0f);
          cInput += NEURONSPERTEAMMATE;
        } else {
          numTeammatesAlive--;
        }
        continue;
      }
      const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);
      float percentHP = (float)cPKV.getPercentHP();
      cInput[0] = percentHP;
      uint32_t statusAilment = cPKV.getStatusAilment();
      static const std::array<float, 16> damageStatusVal = {{ 0.0625f, 0.125f, 0.1875f, 0.25f, 0.3125f, 0.375f, 0.4375f, 0.5f, 0.5625f, 0.625f, 0.6875f, 0.75f, 0.8125f, 0.875f, 0.9375f, 1.0f }};
      float statusDamage = (statusAilment==AIL_NV_BURN||statusAilment==AIL_NV_POISON)?damageStatusVal[1]:(statusAilment==AIL_NV_POISON_TOXIC)?damageStatusVal[cTV.getVolatile().toxicPoison_tier]:0.0f;
      cInput[1] = std::min(1.0f, statusDamage / percentHP);
      static const std::array<float, 8> sleepStatusVal = {{ 0.0f, 0.5f, 2.0f/3.0f, 0.75f, 1.0f, 1.0f, 1.0f, 1.0f }};
      cInput[2] = (float) (statusAilment<=AIL_NV_REST)?sleepStatusVal[statusAilment]:0.0f;
      cInput[3] = (float) (statusAilment==AIL_NV_FREEZE)?1.0f:(statusAilment==AIL_NV_PARALYSIS)?0.5f:0.0f;
      cInput += 4;
      size_t numOTeammatesAlive = tTNV.getNumTeammates();
      for (size_t iNOTeammate = 0; iNOTeammate != tTNV.getNumTeammates(); ++iNOTeammate) {
        size_t iOTeammate = iOTeammates[iNOTeammate];
        const ConstPokemonVolatile& tPKV = tTV.teammate(iOTeammate);
        if (!tPKV.isAlive()) { if (iNOTeammate == 0) { *(cInput++) = 0.0f; } else { numOTeammatesAlive--; } continue; }
        float bestDamage = 0.0f; uint8_t dType = ATK_NODMG;
        const std::array<uint8_t, 4>& cBestMoves =
            this->iBestMoves_[iTeam][iTeammate][iOTeammate];
        for (size_t iNMove = 0, iMoveSize = cPKNV.getNumMoves(); iNMove != iMoveSize; ++iNMove) {
          size_t iMove = cBestMoves[iNMove];
          if (!cPKV.getMV(iMove).hasPP()) continue;
          bestDamage = this->dBestMoves_[iTeam][iTeammate][iOTeammate][iNMove];
          dType = cPKNV.getMove_base(iMove).getDamageType() % ATK_FIXED;
          break;
        }
        if (dType == ATK_NODMG) { *(cInput++) = 0.0f; continue; }
        assert((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL));
        size_t iModifier = (dType-1)*4 + (iNTeammate == 0 ? 1 : 0) + (iNOTeammate == 0 ? 2 : 0);
        *(cInput++) = std::max(0.0f, std::min(1.0f, bestDamage * modifiers[iModifier]));
      }
      std::fill(cInput, cInput + (6 - numOTeammatesAlive), 0.0f);
      cInput += (6 - numOTeammatesAlive);
    }
    std::fill(
        cInput, cInput + NEURONSPERTEAMMATE * (6 - numTeammatesAlive), 0.0f);
    cInput += NEURONSPERTEAMMATE * (6 - numTeammatesAlive);
    // clang-format off
    cInput[0] = (float)(cTV.getPKV().getFV_boosted(FV_SPEED) > tTV.getPKV().getFV_boosted(FV_SPEED));
    cInput[1] = std::max(0.0f, std::min(1.0f, (float)(cTV.getPKV().getAccuracy_boosted(FV_ACCURACY) * tTV.getPKV().getAccuracy_boosted(FV_EVASION))));
    cInput[2] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
    cInput[3] = scale((float)((cTV.status().spikes>0) + (cTV.status().stealthRock>0)), 2.0f, 0.0f);
    // clang-format on
    cInput += NEURONSPERSTATUS;
  }
}

// Explicit instantiations
template class evaluator_network128_impl<EvaluatorNetwork>;
template class evaluator_network128_impl<TrainableEvaluatorNetwork>;
