#include "../inc/evaluator_network32.h"

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

#define NEURONSPERTEAMMATE 2
#define NEURONSPERSTATUS 4
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)
#define EVAL_INPUTNEURONS (NEURONSPERTEAM*2)
#define EVAL_OUTPUTNEURONS 1U

const size_t evaluator_network32::numInputNeurons = (NEURONSPERTEAM*2);
const size_t evaluator_network32::numOutputNeurons = 1U;

evaluator_network32::evaluator_network32()
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

evaluator_network32::evaluator_network32(const evaluator_network32& other)
  : ident(other.ident),
  envNV(other.envNV),
  network(other.network!=NULL?new neuralNet(*other.network):NULL),
  iBestMoves(other.iBestMoves),
  dBestMoves(other.dBestMoves),
  orders(other.orders)
{
};

evaluator_network32::evaluator_network32(const neuralNet& _cNet)
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

evaluator_network32::~evaluator_network32()
{
  if (network != NULL) { delete network; network = NULL; }
}

bool evaluator_network32::isInitialized() const
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

void evaluator_network32::resetNetwork(const neuralNet& cNet)
{
  if (network != NULL) { delete network; network = NULL; }
  network = new neuralNet(cNet);
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
}

void evaluator_network32::resetEvaluator(const EnvironmentNonvolatile& _envNV)
{
  envNV = &_envNV;
  // reset memoized data:
  generateBestMoves();
  generateOrders();
  // zero input layer:
  if (network != NULL) { network->clearInput(); }
};

EvalResult evaluator_network32::calculateFitness(const EnvironmentVolatile& env, size_t iTeam)
{
  return calculateFitness(*network, env, iTeam);
};

EvalResult evaluator_network32::calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam)
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

  EvalResult result= { fitness , -1/*agentMove*/ , -1/*otherMove*/ };
  return result;
};

void evaluator_network32::seed(float* inputBegin, const EnvironmentVolatile& env, size_t _iTeam) const
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

    // order of inputs:
    const std::array<uint8_t, 6>& iTeammates = orders[iTeam][cTV.getICPKV()];
    const std::array<uint8_t, 6>& iOTeammates = orders[iOTeam][tTV.getICPKV()];

    const PokemonVolatile& tPKV = tTV.teammate(iOTeammates[0]);

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

      if (!tPKV.isAlive()) { cInput[1] = 0.0f; }
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
          dType = cPKNV.getMove_base(iMove + AT_MOVE_0).getDamageType() % ATK_FIXED; // atk_fixed becomes atk_no damage
          break;
        }
        // if we weren't able to find a best move:
        if (dType == ATK_NODMG) { cInput[1] = 0.0f; }
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
          cInput[1] = std::max(0.0f, std::min(1.0f, bestDamage * modifier));
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
      const PokemonVolatile& cPKV = cTV.getPKV();

      // 4 neurons per status:
      float statusAilment = cPKV.getStatusAilment();
      cInput[0] = scale(statusAilment, (float)AIL_NV_POISON_TOXIC, (float)AIL_NV_NONE);
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





const float* evaluator_network32::getInput() const
{
  if (network == NULL) { return NULL; }
  return &*network->inputBegin();
};





void evaluator_network32::generateBestMoves()
{
  featureVector_impl::generateBestMoves(*envNV, iBestMoves, dBestMoves);
}; // endOf generateBestMoves





void evaluator_network32::generateOrders()
{
  featureVector_impl::generateOrders(dBestMoves, orders);
}; // endOf generateOrders





void evaluator_network32::outputNames(std::ostream& oS) const
{
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      oS << "percentHP-" << iTeam << iTeammate << ", ";
      oS << "bestDamage-" << iTeam << iTeammate << "0, ";
    }

    oS << "nonvolatileStatus-" << iTeam << ", ";
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
