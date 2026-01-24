#include "pokemonai/evaluator_network16.h"
#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/environment_volatile.h"
#include "pokemonai/team_volatile.h"
#include "pokemonai/pokemon_volatile.h"
#include <sstream>
#include <assert.h>
#include <iostream>

#define NEURONSPERTEAMMATE 1
#define NEURONSPERSTATUS 2
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)

template <class Base>
const size_t evaluator_network16_impl<Base>::numInputNeurons =
    (NEURONSPERTEAM * 2);

template <class Base>
const size_t evaluator_network16_impl<Base>::numOutputNeurons = 1U;

template <class Base>
evaluator_network16_impl<Base>::evaluator_network16_impl(
    const typename Base::Config& cfg)
    : Base(cfg, numInputNeurons, numOutputNeurons) {
  this->updateIdent();
}

template <class Base>
evaluator_network16_impl<Base>::evaluator_network16_impl(
    const evaluator_network16_impl& other)
    : Base(other) {}

template <class Base>
evaluator_network16_impl<Base>::evaluator_network16_impl(
    const neuralNet& _cNet, const typename Base::Config& cfg)
    : Base(_cNet, cfg) {
  this->updateIdent();
}

template <class Base>
void evaluator_network16_impl<Base>::seed(
    FeatureVector::floatIterator_t inputBegin,
    const ConstEnvironmentVolatile& env,
    size_t _iTeam) const {
  FeatureVector::floatIterator_t cInput;
  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam) {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    const ConstTeamVolatile& cTV = env.getTeam(iTeam);
    assert(this->nv_ != nullptr);
    const TeamNonVolatile& cTNV = this->nv_->getTeam(iTeam);
    const std::array<uint8_t, 6>& iTeammates =
        this->orders_[iTeam][cTV.getICPKV()];
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
      *cInput++ = (float)cPKV.getPercentHP();
    }
    std::fill(
        cInput, cInput + NEURONSPERTEAMMATE * (6 - numTeammatesAlive), 0.0f);
    cInput += NEURONSPERTEAMMATE * (6 - numTeammatesAlive);
    const ConstPokemonVolatile& cPKV = cTV.getPKV();
    cInput[0] = scale((float)cPKV.getStatusAilment(), (float)AIL_NV_POISON_TOXIC, (float)AIL_NV_NONE);
    cInput[1] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
    cInput += NEURONSPERSTATUS;
  }
}

template <class Base>
void evaluator_network16_impl<Base>::generateOrders() {
  std::array< uint8_t , 6> preOrder;
  for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) preOrder[iTeammate] = (uint8_t)iTeammate;
  for (size_t iTeam = 0; iTeam < 2; ++iTeam) {
      for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) {
        auto it = this->orders_[iTeam][iTeammate].begin();
        *it++ = (uint8_t)iTeammate;
        for (size_t iNTeammate = 0; iNTeammate != 6; ++iNTeammate) {
          if (iTeammate == preOrder[iNTeammate]) continue;
          *it++ = preOrder[iNTeammate];
          }
      }
  }
}

// Explicit instantiations
template class evaluator_network16_impl<EvaluatorNetwork>;
template class evaluator_network16_impl<TrainableEvaluatorNetwork>;
