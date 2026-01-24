#include "pokemonai/evaluator_network64.h"
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

#define NEURONSPERTEAMMATE 5
#define NEURONSPERSTATUS 2
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)

template <class Base>
const size_t evaluator_network64_impl<Base>::numInputNeurons =
    (NEURONSPERTEAM * 2);

template <class Base>
const size_t evaluator_network64_impl<Base>::numOutputNeurons = 1U;

template <class Base>
evaluator_network64_impl<Base>::evaluator_network64_impl(
    const typename Base::Config& cfg)
    : Base(cfg, numInputNeurons, numOutputNeurons) {
  this->updateIdent();
}

template <class Base>
evaluator_network64_impl<Base>::evaluator_network64_impl(
    const evaluator_network64_impl& other)
    : Base(other) {}

template <class Base>
evaluator_network64_impl<Base>::evaluator_network64_impl(
    const neuralNet& _cNet, const typename Base::Config& cfg)
    : Base(_cNet, cfg) {
  this->updateIdent();
}

template <class Base>
void evaluator_network64_impl<Base>::seed(
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

    const std::array<uint8_t, 6>& iTeammates =
        this->orders_[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates =
        this->orders_[iOTeam][tTV.getICPKV()];
    const ConstPokemonVolatile& tPKV = tTV.teammate(iOTeammates[0]);

    std::array<float, 4> modifiers = {{
      statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)],
      statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)] * statMultipliers[cTV.cGetBoost(FV_ATTACK)+6],
      statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)],
      statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)] * statMultipliers[cTV.cGetBoost(FV_SPATTACK)+6]
    }};

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
      if (!tPKV.isAlive()) { cInput[4] = 0.0f; }
      else {
        float bestDamage = 0.0f; uint8_t dType = ATK_NODMG;
        const std::array<uint8_t, 4>& cBestMoves =
            this->iBestMoves_[iTeam][iTeammate][iOTeammates[0]];
        for (size_t iNMove = 0, iMoveSize = cPKNV.getNumMoves(); iNMove != iMoveSize; ++iNMove) {
          size_t iMove = cBestMoves[iNMove];
          if (!cPKV.getMV(iMove).hasPP()) continue;
          bestDamage = this->dBestMoves_[iTeam][iTeammate][0][iNMove];
          dType = cPKNV.getMove_base(iMove).getDamageType() % ATK_FIXED;
          break;
        }
        if (dType == ATK_NODMG) { cInput[4] = 0.0f; }
        else {
          assert((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL));
          size_t iModifier = (dType-1)*2 + (iNTeammate == 0 ? 1 : 0);
          cInput[4] = std::max(0.0f, std::min(1.0f, bestDamage * modifiers[iModifier]));
        }
      }
      cInput += NEURONSPERTEAMMATE;
    }
    std::fill(
        cInput, cInput + NEURONSPERTEAMMATE * (6 - numTeammatesAlive), 0.0f);
    cInput += NEURONSPERTEAMMATE * (6 - numTeammatesAlive);
    cInput[0] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
    cInput[1] = scale((float)((cTV.getNonVolatile().spikes>0) + (cTV.getNonVolatile().stealthRock>0)), 2.0f, 0.0f);
    cInput += NEURONSPERSTATUS;
  }
}

// Explicit instantiations
template class evaluator_network64_impl<EvaluatorNetwork>;
template class evaluator_network64_impl<TrainableEvaluatorNetwork>;
