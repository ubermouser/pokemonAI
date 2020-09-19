//#define PKAI_IMPORT
#include "../inc/evaluator_network16.h"

#include <sstream>
#include <assert.h>
#include <iostream>
#include <boost/foreach.hpp>

#include "../inc/fp_compare.h"

#include "../inc/evaluator_featureVector.h"

#include "../inc/environment_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/environment_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_volatile.h"

#define NEURONSPERTEAMMATE 1
#define NEURONSPERSTATUS 2
#define NEURONSPERTEAM (NEURONSPERTEAMMATE * 6 + NEURONSPERSTATUS)
#define EVAL_INPUTNEURONS (NEURONSPERTEAM*2)
#define EVAL_OUTPUTNEURONS 1U

const size_t evaluator_network16::numInputNeurons = (NEURONSPERTEAM*2);
const size_t evaluator_network16::numOutputNeurons = 1U;

evaluator_network16::evaluator_network16()
  : ident(),
  envNV(NULL),
  network(NULL),
  orders()
{
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-NULLNETWORK";
    ident = name.str();
  }
};

evaluator_network16::evaluator_network16(const evaluator_network16& other)
  : ident(other.ident),
  envNV(other.envNV),
  network(other.network!=NULL?new neuralNet(*other.network):NULL),
  orders(other.orders)
{
};

evaluator_network16::evaluator_network16(const neuralNet& _cNet)
  : envNV(NULL),
  network(new neuralNet(_cNet)),
  orders()
{
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
};

evaluator_network16::~evaluator_network16()
{
  if (network != NULL) { delete network; network = NULL; }
}

bool evaluator_network16::isInitialized() const
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

void evaluator_network16::resetNetwork(const neuralNet& cNet)
{
  if (network != NULL) { delete network; network = NULL; }
  network = new neuralNet(cNet);
  {
    std::ostringstream name;
    name << "neural_Evaluator(" << numInputNeurons << "." << numOutputNeurons << ")-" << network->getName();
    ident = name.str();
  }
}

void evaluator_network16::resetEvaluator(const EnvironmentNonvolatile& _envNV)
{
  envNV = &_envNV;
  // reset memoized data:
  generateOrders();
  // zero input layer:
  if (network != NULL) { network->clearInput(); }
};

EvalResult evaluator_network16::calculateFitness(const EnvironmentVolatile& env, size_t iTeam)
{
  return calculateFitness(*network, env, iTeam);
};

EvalResult evaluator_network16::calculateFitness(neuralNet& cNet, const EnvironmentVolatile& env, size_t iTeam)
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





void evaluator_network16::seed(float* inputBegin, const EnvironmentVolatile& env, size_t _iTeam) const
{
  float* cInput;

  for (size_t iNTeam = 0; iNTeam < 2; ++iNTeam)
  {
    size_t iTeam = (_iTeam + iNTeam) & 1;
    const TeamVolatile& cTV = env.getTeam(iTeam);
    const TeamNonVolatile& cTNV = envNV->getTeam(iTeam);

    // order of inputs:
    const std::array<uint8_t, 6>& iTeammates = orders[iTeam][cTV.getICPKV()];

    cInput = inputBegin + (NEURONSPERTEAM * iNTeam);

    // 1 neuron per pokemon:
    size_t numTeammatesAlive = cTNV.getNumTeammates();
    for (size_t iNTeammate = 0; iNTeammate != cTNV.getNumTeammates(); ++iNTeammate)
    {
      size_t iTeammate = iTeammates[iNTeammate];
      const PokemonVolatile& cPKV = cTV.teammate(iTeammate);
      if (!cPKV.isAlive()) 
      {
        // special case when this is the first teammate
        if (iNTeammate == 0) { memset(cInput, 0, sizeof(float)*NEURONSPERTEAMMATE); cInput+=NEURONSPERTEAMMATE; }
        else { numTeammatesAlive--; }
        continue; 
      }
      const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);

      // percent hitpoints of pokemon: (guaranteed to be nonzero)
      float percentHP = (float)cPKV.getPercentHP(cPKNV);
      *cInput++ = percentHP; //scale(percentHP + 0.1f, 1.1f, 0.0f);
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

      // 2 neurons per status:
      // if pokemon has been statused:
      float statusAilment = cPKV.getStatusAilment();
      cInput[0] = scale(statusAilment, (float)AIL_NV_POISON_TOXIC, (float)AIL_NV_NONE);
      // confused or infatuated:
      cInput[1] = (float)((cTV.getVolatile().confused | cTV.getVolatile().infatuate) > AIL_V_NONE);

      cInput += NEURONSPERSTATUS;
    } // endOf volatile status
    assert((cInput - inputBegin) == (NEURONSPERTEAM*(iNTeam+1)));
  } // endOf foreach team
}; // endOf seed volatile





const float* evaluator_network16::getInput() const
{
  if (network == NULL) { return NULL; }
  return &*network->inputBegin();
};





void evaluator_network16::generateOrders()
{
  std::array< uint8_t , 6> preOrder;
  // seed the unmodified order into preOrders:
  for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate) { preOrder[iTeammate] = iTeammate; }

  // place modified order into orders:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    std::array< std::array< uint8_t , 6> , 6>& cOrders = orders[iTeam];
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      std::array<uint8_t, 6>::iterator cTeammate = cOrders[iTeammate].begin();
      *cTeammate++ = iTeammate;
      for (size_t iNTeammate = 0; iNTeammate != 6; ++iNTeammate)
      {
        if (iTeammate == preOrder[iNTeammate]) { continue; }
        *cTeammate++ = preOrder[iNTeammate];
      }
    }
  }
}; // endOf generateOrders





void evaluator_network16::outputNames(std::ostream& oS) const
{
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      oS << "percentHP-" << iTeam << iTeammate << ", ";
    }

    oS << "nonvolatileStatus-" << iTeam << ", ";
    oS << "volatileStatus-" << iTeam << ", ";
  }
  oS << "fitness-0";
  for (size_t iOutput = 1; iOutput < outputSize(); ++iOutput)
  {
    oS << ", fitness-" << iOutput;
  }
  oS << "\n";
};
