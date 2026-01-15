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

const size_t evaluator_network128::numInputNeurons = (NEURONSPERTEAM*2);
const size_t evaluator_network128::numOutputNeurons = 1U;

evaluator_network128::evaluator_network128(const Config& cfg)
    : EvaluatorNetwork(cfg, numInputNeurons, numOutputNeurons) {
  updateIdent();
}

evaluator_network128::evaluator_network128(const evaluator_network128& other) : EvaluatorNetwork(other) {
}

evaluator_network128::evaluator_network128(const neuralNet& _cNet, const Config& cfg) : EvaluatorNetwork(_cNet, cfg) {
  updateIdent();
}

void evaluator_network128::seed(
    neuralNet::floatIterator_t inputBegin,
    const ConstEnvironmentVolatile& env,
    size_t _iTeam) const {
  neuralNet::floatIterator_t cInput;
  static const std::array<float, 13> statMultipliers = {{ 0.25f, 2.0f/7.0f, 2.0f/6.0f, 0.4f, 0.5f, 2.0f/3.0f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f }};

  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam) {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    size_t iOTeam = (iTeam + 1) & 1;
    const ConstTeamVolatile& cTV = env.getTeam(iTeam);
    assert(nv_ != nullptr);
    const TeamNonVolatile& cTNV = nv_->getTeam(iTeam);
    const ConstTeamVolatile& tTV = env.getTeam(iOTeam);
    const TeamNonVolatile& tTNV = nv_->getOtherTeam(iTeam);

    const std::array<uint8_t, 6>& iTeammates = orders_[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates = orders_[iOTeam][tTV.getICPKV()];

    std::array<float, 8> modifiers = {{ 1.0f, statMultipliers[cTV.cGetBoost(FV_ATTACK)+6], statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)], statMultipliers[cTV.cGetBoost(FV_ATTACK)+6]*statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)], 1.0f, statMultipliers[cTV.cGetBoost(FV_SPATTACK)+6], statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)], statMultipliers[cTV.cGetBoost(FV_SPATTACK)+6]*statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)] }};

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
        const std::array<uint8_t, 4>& cBestMoves = iBestMoves_[iTeam][iTeammate][iOTeammate];
        for (size_t iNMove = 0, iMoveSize = cPKNV.getNumMoves(); iNMove != iMoveSize; ++iNMove) {
          size_t iMove = cBestMoves[iNMove];
          if (!cPKV.getMV(iMove).hasPP()) continue;
          bestDamage = dBestMoves_[iTeam][iTeammate][iOTeammate][iNMove];
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
    cInput[0] = (float)(cTV.cGetFV_boosted(FV_SPEED) > tTV.cGetFV_boosted(FV_SPEED));
    cInput[1] = std::max(0.0f, std::min(1.0f, (float)(cTV.cGetAccuracy_boosted(FV_ACCURACY) * tTV.cGetAccuracy_boosted(FV_EVASION))));
    cInput[2] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
    cInput[3] = scale((float)((cTV.getNonVolatile().spikes>0) + (cTV.getNonVolatile().stealthRock>0)), 2.0f, 0.0f);
    cInput += NEURONSPERSTATUS;
  }
}

void evaluator_network128::outputNames(std::ostream& oS) const {
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
        oS << "percentHP-" << iTeam << iTeammate << ", damagingStatus-" << iTeam << iTeammate << ", sleepStatus-" << iTeam << iTeammate << ", debilitatingStatus-" << iTeam << iTeammate << ", ";
        for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate) oS << "bestDamage-" << iTeam << iTeammate << iOTeammate << ", ";
    }
    oS << "speed-" << iTeam << ", accuracy-" << iTeam << ", volatileStatus-" << iTeam << ", entryHazard-" << iTeam << ", ";
  }
  oS << "fitness-0";
  for (size_t iOutput = 1; iOutput < outputSize(); ++iOutput) oS << ", fitness-" << iOutput;
  oS << "\n";
}
