#include "pokemonai/evaluator_network32.h"
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

#define NEURONSPERTEAMMATE 2
#define NEURONSPERSTATUS 4
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)

template <class Base>
const size_t evaluator_network32_impl<Base>::numInputNeurons =
    (NEURONSPERTEAM * 2);

template <class Base>
const size_t evaluator_network32_impl<Base>::numOutputNeurons = 1U;

template <class Base>
evaluator_network32_impl<Base>::evaluator_network32_impl(
    const typename Base::Config& cfg)
    : Base(cfg, numInputNeurons, numOutputNeurons) {
  this->updateIdent();
}

template <class Base>
evaluator_network32_impl<Base>::evaluator_network32_impl(
    const evaluator_network32_impl& other)
    : Base(other) {}

template <class Base>
evaluator_network32_impl<Base>::evaluator_network32_impl(
    const neuralNet& _cNet, const typename Base::Config& cfg)
    : Base(_cNet, cfg) {
  this->updateIdent();
}

template <class Base>
void evaluator_network32_impl<Base>::seed(
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

    // clang-format off
    std::array<float, 4> modifiers = {{
      statMultipliers[6-tTV.getPKV().getBoost(FV_DEFENSE)],
      statMultipliers[6-tTV.getPKV().getBoost(FV_DEFENSE)] * statMultipliers[cTV.getPKV().getBoost(FV_ATTACK)+6],
      statMultipliers[6-tTV.getPKV().getBoost(FV_SPDEFENSE)],
      statMultipliers[6-tTV.getPKV().getBoost(FV_SPDEFENSE)] * statMultipliers[cTV.getPKV().getBoost(FV_SPATTACK)+6]
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
      cInput[0] = (float)cPKV.getPercentHP();
      if (!tPKV.isAlive()) { cInput[1] = 0.0f; }
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
        if (dType == ATK_NODMG) { cInput[1] = 0.0f; }
        else {
          assert((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL));
          size_t iModifier = (dType-1)*2 + (iNTeammate == 0 ? 1 : 0);
          cInput[1] = std::max(0.0f, std::min(1.0f, bestDamage * modifiers[iModifier]));
        }
      }
      cInput += NEURONSPERTEAMMATE;
    }
    std::fill(
        cInput, cInput + NEURONSPERTEAMMATE * (6 - numTeammatesAlive), 0.0f);
    cInput += NEURONSPERTEAMMATE * (6 - numTeammatesAlive);
    const ConstPokemonVolatile& cPKV = cTV.getPKV();
    // clang-format off
    cInput[0] = scale((float)cPKV.getStatusAilment(), (float)AIL_NV_POISON_TOXIC, (float)AIL_NV_NONE);
    cInput[1] = std::max(0.0f, std::min(1.0f, (float)(cPKV.getAccuracy_boosted(FV_ACCURACY) * tPKV.getAccuracy_boosted(FV_EVASION))));
    cInput[2] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
    cInput[3] = scale((float)((cTV.status().spikes>0) + (cTV.status().stealthRock>0)), 2.0f, 0.0f);
    // clang-format on
    cInput += NEURONSPERSTATUS;
  }
}

// Explicit instantiations
template class evaluator_network32_impl<EvaluatorNetwork>;
template class evaluator_network32_impl<TrainableEvaluatorNetwork>;
