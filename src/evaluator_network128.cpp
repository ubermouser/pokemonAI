//#define PKAI_IMPORT
#include "../inc/evaluator_network128.h"

#include <sstream>
#include <assert.h>
#include <algorithm>
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/math/special_functions/fpclassify.hpp>

#include "../inc/fp_compare.h"

#include "../inc/evaluator_featureVector.h"

#include "../inc/type.h"
#include "../inc/move.h"
#include "../inc/pokemon_base.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/environment_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_volatile.h"

#define NEURONSPERTEAMMATE 10
#define NEURONSPERSTATUS 4
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)
#define EVAL_INPUTNEURONS (NEURONSPERTEAM*2)
#define EVAL_OUTPUTNEURONS 1U

const size_t evaluator_network128::numInputNeurons = (NEURONSPERTEAM*2);
const size_t evaluator_network128::numOutputNeurons = 1U;

evaluator_network128::evaluator_network128()
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

evaluator_network128::evaluator_network128(const evaluator_network128& other)
  : ident(other.ident),
  envNV(other.envNV),
  network(other.network!=NULL?new neuralNet(*other.network):NULL),
  iBestMoves(other.iBestMoves),
  dBestMoves(other.dBestMoves),
  orders(other.orders)
{
};

evaluator_network128::evaluator_network128(const neuralNet& _cNet)
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

evaluator_network128::~evaluator_network128()
{
  if (network != NULL) { delete network; network = NULL; }
}

bool evaluator_network128::isInitialized() const
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

void evaluator_network128::resetNetwork(const neuralNet& cNet)
{
  if (network != NULL) { delete network; network = NULL; }
  network = new neuralNet(cNet);
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
}

void evaluator_network128::resetEvaluator(const EnvironmentNonvolatile& _envNV)
{
  envNV = &_envNV;
  // reset memoized data:
  generateBestMoves();
  generateOrders();
  // zero input layer:
  if (network != NULL) { network->clearInput(); }
};

EvalResult_t evaluator_network128::calculateFitness(const EnvironmentVolatile& env, size_t iTeam)
{
  return calculateFitness(*network, env, iTeam);
};

EvalResult_t evaluator_network128::calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam)
{
  // seed network with values:
  seed(&*cNet.inputBegin(), env, iTeam);
  // perform feed-forward evaluation:
  cNet.feedForward();
  // parse output:
  constFloatIterator_t output = cNet.outputBegin();
  // values returned less than 0.15 have 0 fitness, greater than 0.85 have 1 fitness
  fpType fitness = *output;
  fitness = std::max(0.0, std::min(1.0, scale(fitness, 0.85, 0.15)));

  EvalResult_t result= { fitness , -1/*agentMove*/ , -1/*otherMove*/ };
  return result;
};

void evaluator_network128::seed(float* inputBegin, const EnvironmentVolatile& env, size_t _iTeam) const
{
  float* cInput;

  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam)
  {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    size_t iOTeam = (iTeam + 1) & 1;
    //size_t iTeam = iNTeam;
    const TeamVolatile& cTV = env.getTeam(iTeam);
    const TeamNonVolatile& cTNV = envNV->getTeam(iTeam);

    const TeamVolatile& tTV = env.getTeam(iOTeam);
    const TeamNonVolatile& tTNV = envNV->getTeam(iOTeam);

    // order of inputs:
    const std::array<uint8_t, 6>& iTeammates = orders[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates = orders[iOTeam][tTV.getICPKV()];

    // affect of boosts:
    static const std::array<float, 13> statMultipliers = 
    {{ 0.25f, 2.0f/7.0f, 2.0f/6.0f, 0.4f, 0.5f, 2.0f/3.0f, 1.0f, 1.5f, 2.0f, 2.5f, 3.0f, 3.5f, 4.0f }};
    std::array<float, 8> modifiers =
    {{
      1.0f, // physical:
      statMultipliers[cTV.cGetBoost(FV_ATTACK)+6],
      statMultipliers[6-tTV.cGetBoost(FV_DEFENSE)],
      modifiers[1]*modifiers[2],
      1.0f, // special:
      statMultipliers[cTV.cGetBoost(FV_SPATTACK)+6],
      statMultipliers[6-tTV.cGetBoost(FV_SPDEFENSE)],
      modifiers[5]*modifiers[6]
    }};

    cInput = inputBegin + (NEURONSPERTEAM * iNTeam);

    // 10 neurons per other pokemon:
    size_t numTeammatesAlive = cTNV.getNumTeammates();
    for (size_t iNTeammate = 0; iNTeammate != cTNV.getNumTeammates(); ++iNTeammate)
    {
      size_t iTeammate = iTeammates[iNTeammate];
      const PokemonVolatile& cPKV = cTV.teammate(iTeammate);
      if (!cPKV.isAlive()) 
      {
        // special case when this is the first teammate
        if (iNTeammate == 0) { memset(&*cInput, 0, sizeof(float)*NEURONSPERTEAMMATE); cInput+=NEURONSPERTEAMMATE; }
        else { numTeammatesAlive--; }
        continue; 
      }
      const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);

      // percent hitpoints of pokemon: (guaranteed to be nonzero)
      float percentHP = (float)cPKV.getPercentHP(cPKNV);
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
      
      cInput += 4;

      size_t numOTeammatesAlive = tTNV.getNumTeammates();
      for (size_t iNOTeammate = 0; iNOTeammate != tTNV.getNumTeammates(); ++iNOTeammate)
      {
        size_t iOTeammate = iOTeammates[iNOTeammate];
        const PokemonVolatile& tPKV = tTV.teammate(iOTeammate);
        if (!tPKV.isAlive()) 
        {
          // special case when this is the first oTeammate
          if (iNOTeammate == 0) { *(cInput++) = 0.0f; }
          else { numOTeammatesAlive--; }
          continue; 
        }
        //const pokemon_nonvolatile& tPKNV = tTNV.teammate(iTeammate);

        float bestDamage = 0.0f; uint8_t dType = ATK_NODMG;
        const std::array<uint8_t, 4>& cBestMoves = iBestMoves[iTeam][iTeammate][iOTeammate];
        for (size_t iNMove = 0, iMoveSize = cPKNV.getNumMoves(); iNMove != iMoveSize; ++iNMove)
        {
          size_t iMove = cBestMoves[iNMove];
          assert((iMove <= 3) && (iMove >= 0));

          if (!cPKV.getMV(iMove).hasPP()) { continue; }
          bestDamage = dBestMoves[iTeam][iTeammate][iOTeammate][iNMove];
          dType = cPKNV.getMove_base(iMove + AT_MOVE_0).getDamageType() % ATK_FIXED; // atk_fixed becomes atk_no damage
          break;
        }
        // if we weren't able to find a best move:
        if (dType == ATK_NODMG) { *(cInput++) = 0.0f; continue; }

        float modifier;
        // if this pokemon has any valid physical or special attacks:
        assert((dType == ATK_PHYSICAL) || (dType == ATK_SPECIAL));
        {
          size_t iModifier = 0;
          iModifier += (dType-1)*4; // physical or special attack
          iModifier += (iNTeammate == 0) * 1; // if current pokemon is active, add attack bonus
          iModifier += (iNOTeammate == 0) * 2; //  if other current is active, add defense bonus
          modifier = modifiers[iModifier];
        }

        // damage of this pokemon's move:
        *(cInput++) = std::max(0.0f, std::min(1.0f, bestDamage * modifier));
      } // endof other team teammates
      {
        // other pokemon past index or not alive:
        memset(&*cInput, 0, sizeof(float) * (6 - numOTeammatesAlive));
        cInput += (6 - numOTeammatesAlive);
      }
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
      //cInput = cNet.inputBegin() + (NEURONSPERVTEAM * iNTeam) + (NEURONSPERVTEAMMATE * 5) + (NEURONSPERMOVE * 4);

      // 4 neurons per status:
      // pokemon's speed:
      cInput[0] = (float)(cTV.cGetFV_boosted(cTNV, FV_SPEED) > tTV.cGetFV_boosted(tTNV, FV_SPEED));
      // accuracy / evasion:
      float accuracy = (float)(cTV.cGetAccuracy_boosted(FV_ACCURACY) * tTV.cGetAccuracy_boosted(FV_EVASION));
      cInput[1] = std::max(0.0f, std::min(1.0f, accuracy));
      // confused or infatuated:
      cInput[2] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);
      // entry hazard:
      float entryHazard = (float)((cTV.getNonVolatile().spikes>0) + (cTV.getNonVolatile().stealthRock>0));
      cInput[3] = scale(entryHazard, 2.0f, 0.0f);

      cInput += NEURONSPERSTATUS;
    } // endOf volatile status
    assert((cInput - inputBegin) == (NEURONSPERTEAM*(iNTeam+1)));
  } // endOf foreach team
}; // endOf seed volatile





const float* evaluator_network128::getInput() const
{
  if (network == NULL) { return NULL; }
  return &*network->inputBegin();
};





void evaluator_network128::generateBestMoves()
{
  featureVector_impl::generateBestMoves(*envNV, iBestMoves, dBestMoves);
}; // endOf generateBestMoves





void evaluator_network128::generateOrders()
{
  featureVector_impl::generateOrders(dBestMoves, orders);
}; // endOf generateOrders





void evaluator_network128::outputNames(std::ostream& oS) const
{
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      oS << "percentHP-" << iTeam << iTeammate << ", ";
      oS << "damagingStatus-" << iTeam << iTeammate << ", ";
      oS << "sleepStatus-" << iTeam << iTeammate << ", ";
      oS << "debilitatingStatus-" << iTeam << iTeammate << ", ";
      for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate)
      {
        oS << "bestDamage-" << iTeam << iTeammate << iOTeammate << ", ";
      }
    }

    oS << "speed-" << iTeam << ", ";
    oS << "accuracy-" << iTeam << ", ";
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

