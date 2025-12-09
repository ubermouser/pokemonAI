#include "pokemonai/evaluator_network64.h"

#include <sstream>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include "pokemonai/fp_compare.h"

#include "pokemonai/evaluator_featureVector.h"

#include "pokemonai/type.h"
#include "pokemonai/move.h"
#include "pokemonai/pokemon_base.h"
#include "pokemonai/environment_nonvolatile.h"
#include "pokemonai/team_nonvolatile.h"
#include "pokemonai/pokemon_nonvolatile.h"
#include "pokemonai/environment_volatile.h"
#include "pokemonai/team_volatile.h"
#include "pokemonai/pokemon_volatile.h"

#define NEURONSPERTEAMMATE 5
#define NEURONSPERSTATUS 2
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)
#define EVAL_INPUTNEURONS (NEURONSPERTEAM*2)
#define EVAL_OUTPUTNEURONS 1U

const size_t evaluator_network64::numInputNeurons = (NEURONSPERTEAM*2);
const size_t evaluator_network64::numOutputNeurons = 1U;

evaluator_network64::evaluator_network64()
  : ident(),
  envNV(NULL),
  network(NULL),
  iBestMoves(),
  dBestMoves(),
  orders()
{
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-NULLNETWORK";
    ident = name.str();
  }
};

evaluator_network64::evaluator_network64(const evaluator_network64& other)
  : ident(other.ident),
  envNV(other.envNV),
  network(other.network!=NULL?new neuralNet(*other.network):NULL),
  iBestMoves(other.iBestMoves),
  dBestMoves(other.dBestMoves),
  orders(other.orders)
{
};

evaluator_network64::evaluator_network64(const neuralNet& _cNet)
  : envNV(NULL),
  network(new neuralNet(_cNet)),
  iBestMoves(),
  dBestMoves(),
  orders()
{
  assert(network->numOutputs() == numOutputNeurons && network->numInputs() == numInputNeurons);
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
};

evaluator_network64::~evaluator_network64()
{
  if (network != NULL) { delete network; network = NULL; }
}

bool evaluator_network64::isInitialized() const
{
  if (envNV == NULL) { return false; }
  if (network == NULL) { return false; }
  if (!network->isInitialized()) { return false; }
  // check neural network size:
  if ((network->numInputs() != numInputNeurons) || (network->numOutputs() != numOutputNeurons))
  {
    std::cerr << "ERR " << __FILE__ << "." << __LINE__ << 
      ": neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")" <<
      " requires input-" << numInputNeurons <<
      " (has " << network->numInputs() <<
      "), output-" << numOutputNeurons <<
      " (has " << network->numOutputs() << 
      ")!\n";
    return false; 
  }
  return true;
}

void evaluator_network64::resetNetwork(const neuralNet& cNet)
{
  if (network != NULL) { delete network; network = NULL; }
  network = new neuralNet(cNet);
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
}

void evaluator_network64::resetEvaluator(const EnvironmentNonvolatile& _envNV)
{
  envNV = &_envNV;
  // reset memoized data:
  generateBestMoves();
  generateOrders();
  // zero input layer:
  if (network != NULL) { network->clearInput(); }
};

EvalResult evaluator_network64::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam) const
{
  return calculateFitness(*network, env, iTeam);
};

EvalResult evaluator_network64::calculateFitness(neuralNet& cNet, const ConstEnvironmentVolatile& env, size_t iTeam) const
{
  // seed network with values:
  seed(&*cNet.inputBegin(), env, iTeam);
  // perform feed-forward evaluation:
  cNet.feedForward();
  // parse output:
  constFloatIterator_t output = cNet.outputBegin();
  // values returned less than 0.15 have 0 fitness, greater than 0.85 have 1 fitness
  fpType fitness = *output;
  fitness = std::max((fpType)0.0, std::min((fpType)1.0, scale(fitness, (fpType)0.85, (fpType)0.15)));

  EvalResult result{ Fitness{fitness}};
  return result;
};

void evaluator_network64::seed(float* inputBegin, const ConstEnvironmentVolatile& env, size_t _iTeam) const
{
  float* cInput;

  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam)
  {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    size_t iOTeam = (iTeam + 1) & 1;
    //size_t iTeam = iNTeam;
    const ConstTeamVolatile& cTV = env.getTeam(iTeam);
    const TeamNonVolatile& cTNV = envNV->getTeam(iTeam);

    const ConstTeamVolatile& tTV = env.getTeam(iOTeam);

    // order of inputs:
    const std::array<uint8_t, 6>& iTeammates = orders[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates = orders[iOTeam][tTV.getICPKV()];

    const ConstPokemonVolatile& tPKV = tTV.teammate(iOTeammates[0]);

    // affect of boosts:
    static const std::array<float, 13> statMultipliers = 
    {{ 0.25f, 2.0f/7.0f, 2.0f/6.0f, 0.4f, 0.5f, 2.0f/3.0f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f }};
    std::array<float, 4> modifiers =
    {{
      statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)], // physical:
      modifiers[0] * statMultipliers[cTV.cGetBoost(FV_ATTACK)+6],
      statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)], // special:
      modifiers[2] * statMultipliers[cTV.cGetBoost(FV_SPATTACK)+6]
    }};

    cInput = inputBegin + (NEURONSPERTEAM * iNTeam);

    // 2 neurons per pokemon:
    size_t numTeammatesAlive = cTNV.getNumTeammates();
    for (size_t iNTeammate = 0; iNTeammate != cTNV.getNumTeammates(); ++iNTeammate)
    {
      size_t iTeammate = iTeammates[iNTeammate];
      const ConstPokemonVolatile& cPKV = cTV.teammate(iTeammate);
      if (!cPKV.isAlive()) 
      {
        // special case when this is the first teammate
        if (iNTeammate == 0) { memset(&*cInput, 0, sizeof(float)*NEURONSPERTEAMMATE); cInput+=NEURONSPERTEAMMATE; }
        else { numTeammatesAlive--; }
        continue; 
      }
      const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);

      // percent hitpoints of pokemon: (guaranteed to be nonzero)
      float percentHP = (float)cPKV.getPercentHP();
      cInput[0] = percentHP; //scale(percentHP + 0.1f, 1.1f, 0.0f);
      // if pokemon is burned, poisoned, or badly poisoned:
      uint32_t statusAilment = cPKV.getStatusAilment();
      static const std::array<float, 16> damageStatusVal = 
      {{ 0.0625f, 0.125f, 0.1875f, 0.25f, 0.3125f, 0.375f, 0.4375f, 0.5f, 0.5625f, 0.625f, 0.6875f, 0.75f, 0.8125f, 0.875f, 0.9375f, 1.0f }};
      float statusDamage = 
        (statusAilment==AIL_NV_BURN||statusAilment==AIL_NV_POISON)?damageStatusVal[1]:
        (statusAilment==AIL_NV_POISON_TOXIC)?damageStatusVal[cTV.getVolatile().toxicPoison_tier]:0.0f;
      cInput[1] = std::min(1.0f, statusDamage / percentHP);
      // if pokemon is asleep or resting (and if so, what chance to break out):
      static const std::array<float, 8> sleepStatusVal = {{ 0.0f, 0.5f, 2.0f/3.0f, 0.75f, 1.0f, 1.0f, 1.0f, 1.0f }};
      cInput[2] = (float) (statusAilment<=AIL_NV_REST)?sleepStatusVal[statusAilment]:0.0f;
      // if pokemon is frozen (1.0f) or paralyzed (0.5f):
      cInput[3] = (float) (statusAilment==AIL_NV_FREEZE)?1.0f:(statusAilment==AIL_NV_PARALYSIS)?0.5f:0.0f;

      if (!tPKV.isAlive()) { cInput[4] = 0.0f; }
      else
      {
        float bestDamage = 0.0f; uint8_t dType = ATK_NODMG;
        const std::array<uint8_t, 4>& cBestMoves = iBestMoves[iTeam][iTeammate][iOTeammates[0]];
        for (size_t iNMove = 0, iMoveSize = cPKNV.getNumMoves(); iNMove != iMoveSize; ++iNMove)
        {
          size_t iMove = cBestMoves[iNMove];
          assert((iMove <= 3) && (iMove >= 0));

          if (!cPKV.getMV(iMove).hasPP()) { continue; }
          bestDamage = dBestMoves[iTeam][iTeammate][0][iNMove];
          dType = cPKNV.getMove_base(iMove).getDamageType() % ATK_FIXED; // atk_fixed becomes atk_no damage
          break;
        }
        // if we weren't able to find a best move:
        if (dType == ATK_NODMG) { cInput[4] = 0.0f; }
        else
        {
          float modifier;
          // if this pokemon has any valid physical or special attacks:
          assert((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL));
          {
            size_t iModifier = 0;
            iModifier += (dType-1)*2; // physical or special attack
            iModifier += (iNTeammate == 0) * 1; // if current pokemon is active, add attack bonus
            modifier = modifiers[iModifier];
          }

          // damage of this pokemon's move:
          cInput[4] = std::max(0.0f, std::min(1.0f, bestDamage * modifier));
        }
      }
      
      cInput += NEURONSPERTEAMMATE;

    } // endof current team teammates
    {
      // pokemon is past index or not alive:
      memset(&*cInput, 0, sizeof(float)*NEURONSPERTEAMMATE * (6 - numTeammatesAlive)); 
      cInput += NEURONSPERTEAMMATE * (6 - numTeammatesAlive);
    }

    assert((cInput - inputBegin) == ((NEURONSPERTEAM * iNTeam) + (NEURONSPERTEAMMATE * 6)));
    
    // volatile status:
    {
      assert((cInput - inputBegin) == ((NEURONSPERTEAM * iNTeam) + (NEURONSPERTEAMMATE * 6)));

      // confused or infatuated:
      cInput[0] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
      // entry hazard:
      float entryHazard = (float)((cTV.getNonVolatile().spikes>0) + (cTV.getNonVolatile().stealthRock>0));
      cInput[1] = scale(entryHazard, 2.0f, 0.0f);

      cInput += NEURONSPERSTATUS;
    } // endOf volatile status
    assert((cInput - inputBegin) == (NEURONSPERTEAM*(iNTeam+1)));
  } // endOf foreach team
}; // endOf seed volatile





const float* evaluator_network64::getInput() const
{
  if (network == NULL) { return NULL; }
  return &*network->inputBegin();
};





void evaluator_network64::generateBestMoves()
{
  featureVector_impl::generateBestMoves(*envNV, iBestMoves, dBestMoves);
}; // endOf generateBestMoves





void evaluator_network64::generateOrders()
{
  featureVector_impl::generateOrders(dBestMoves, orders);
}; // endOf generateOrders





void evaluator_network64::outputNames(std::ostream& oS) const
{
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      oS << "percentHP-" << iTeam << iTeammate << ", ";
      oS << "damagingStatus-" << iTeam << iTeammate << ", ";
      oS << "sleepStatus-" << iTeam << iTeammate << ", ";
      oS << "debilitatingStatus-" << iTeam << iTeammate << ", ";
      oS << "bestDamage-" << iTeam << iTeammate << "0, ";
    }

    oS << "volatileStatus-" << iTeam << ", ";
    oS << "entryHazard-" << iTeam << ", ";
  }
  oS << "fitness-0";
  for (size_t iOutput = 1; iOutput < outputSize(); ++iOutput)
  {
    oS << ", fitness-" << iOutput;
  }
  oS << "\n";
};
