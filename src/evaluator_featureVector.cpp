#include "../inc/evaluator_featureVector.h"

#include <typeinfo>
#include <map>

#include <assert.h>

#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
//#include <boost/ptr_container/ptr_map.hpp>

#include "../inc/fp_compare.h"

#include "../inc/type.h"
#include "../inc/move.h"
#include "../inc/pokemon_base.h"
#include "../inc/environment_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/pokemon_nonvolatile.h"
#include "../inc/environment_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_volatile.h"

#include "../inc/evaluator_network16.h"
#include "../inc/evaluator_network32.h"
#include "../inc/evaluator_network64.h"
#include "../inc/evaluator_network128.h"

static size_t evalIndex(size_t numInput, size_t numOutput) { return numInput * MAXNETWORKWIDTH + numOutput; }

evaluatorMap_t evaluator_featureVector::evaluators = evaluatorMap_t(); /*boost::assign::map_list_of 
    (evalIndex(evaluator_network16::numInputNeurons, evaluator_network16::numOutputNeurons), (evaluator_featureVector*)new evaluator_network16())
    (evalIndex(evaluator_network128::numInputNeurons, evaluator_network128::numOutputNeurons), (evaluator_featureVector*)new evaluator_network128());*/

void evaluator_featureVector::initStatic()
{
  if (!evaluators.empty()) { return; }
  evaluators
    [
    evalIndex(evaluator_network16::numInputNeurons, evaluator_network16::numOutputNeurons)
    ] = new evaluator_network16();
  evaluators
    [
    evalIndex(evaluator_network32::numInputNeurons, evaluator_network32::numOutputNeurons)
    ] = new evaluator_network32();
  evaluators
    [
    evalIndex(evaluator_network64::numInputNeurons, evaluator_network64::numOutputNeurons)
    ] = new evaluator_network64();
  evaluators
    [
    evalIndex(evaluator_network128::numInputNeurons, evaluator_network128::numOutputNeurons)
    ] = new evaluator_network128();
};

void evaluator_featureVector::uninitStatic()
{
  BOOST_FOREACH(evaluatorMap_t::value_type& iEval, evaluators)
  {
    delete iEval.second;
  }
  evaluators.clear();
}





const evaluator_featureVector* evaluator_featureVector::getEvaluator(size_t numInputNeurons, size_t numOutputNeurons)
{
  evaluatorMap_t::const_iterator iEval = evaluators.find(evalIndex(numInputNeurons, numOutputNeurons));
  if (iEval == evaluators.end()) { return NULL; }
  return iEval->second;
};

evaluator_featureVector* evaluator_featureVector::getEvaluator(const neuralNet& cNet)
{
  const evaluator_featureVector* _cEval = getEvaluator(cNet.numInputs(), cNet.numOutputs());
  if (_cEval == NULL) // no evaluator of this type exists!
  { 
    return NULL; 
  }
  else
  {
    evaluator_featureVector* cEval = dynamic_cast<evaluator_featureVector*>(_cEval->clone());
    if (cEval == NULL) { return NULL; } // allocation or dynamic cast failed?
    cEval->resetNetwork(cNet);
    return cEval;
  }
};

bool evaluator_featureVector::hasEvaluator(const neuralNet& cNet)
{
  return getEvaluator(cNet.numInputs(), cNet.numOutputs()) != NULL;
};





void featureVector::outputNames(std::ostream& oS) const
{
  for (size_t iInput = 0, iSize = inputSize(); iInput != iSize; ++iInput)
  {
    oS << typeid(*this).name() << "-" << iInput << ", ";
  }
  oS << "fitness-0";
  for (size_t iOutput = 1; iOutput < outputSize(); ++iOutput)
  {
    oS << ", fitness-" << iOutput;
  }
  oS << "\n";
};





void featureVector_impl::generateBestMoves(const EnvironmentNonvolatile& envNV, bestMoveOrders_t& iBestMoves, bestMoveDamages_t& dBestMoves)
{
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    const TeamNonVolatile& cTNV = envNV.getTeam(iTeam);
    const TeamNonVolatile& tTNV = envNV.getOtherTeam(iTeam);

    for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
    {
      // zero the array:
      for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate)
      {
        std::array<uint8_t, 4>& cIBestMoves = iBestMoves[iTeam][iTeammate][iOTeammate];
        std::array<float, 4>& cBestMoves = dBestMoves[iTeam][iTeammate][iOTeammate];

        cBestMoves.fill(0.0f);
        cIBestMoves.fill(UINT8_MAX);
      }

      if (iTeammate >= cTNV.getNumTeammates())  { continue; }

      const PokemonNonVolatile& cPKNV = cTNV.teammate(iTeammate);
      const PokemonBase& cPKB = cPKNV.getBase();

      // level modifier, RNG modifier
      float levelModifier = (((((float)cPKNV.getLevel() * 2.0f) / 5.0f) + 2.0f) / 50.0f) * 0.85f;

      for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate)
      {
        std::array<uint8_t, 4>& cIBestMoves = iBestMoves[iTeam][iTeammate][iOTeammate];
        std::array<float, 4>& cBestMoves = dBestMoves[iTeam][iTeammate][iOTeammate];
        std::array<bool, 4> valid;
        valid.fill(true);

        cBestMoves.fill(0.0f);
        cIBestMoves.fill(UINT8_MAX);

        // zero element if pokemon does not exist:
        if (iOTeammate >= tTNV.getNumTeammates()) { continue; }

        const PokemonNonVolatile& tPKNV = tTNV.teammate(iOTeammate);
        const PokemonBase& tPKB = tPKNV.getBase();

        float physicalDamage = levelModifier * ((float)cPKNV.getFV_base(FV_ATTACK)) / ((float)tPKNV.getFV_base(FV_DEFENSE));
        float specialDamage = levelModifier * ((float)cPKNV.getFV_base(FV_SPATTACK)) / ((float)tPKNV.getFV_base(FV_SPDEFENSE));
        assert(boost::math::isnormal(physicalDamage));
        assert(boost::math::isnormal(specialDamage));

        for (size_t iNMove = 0; iNMove != cPKNV.getNumMoves(); ++iNMove)
        {
          // determine best possible damage move:
          float bestDamage = -std::numeric_limits<float>::infinity();
          uint8_t iBestDamage = UINT8_MAX;

          for (size_t iMove = 0; iMove !=  cPKNV.getNumMoves(); ++iMove)
          {
            if (!valid[iMove]) { continue; }

            const Move& cMove = cPKNV.getMove_base(iMove + AT_MOVE_0);
            const Type& cType = cMove.getType();

            bool hasStab = ((&cPKB.getType(0) == &cType) || (&cPKB.getType(1) == &cType));

            float typeStabBonus = (float)cType.getModifier(tPKB.getType(0)) *
                      (float)cType.getModifier(tPKB.getType(1)) *
                      (hasStab?1.5f:1.0f);
            float damageTypeBonus = (cMove.getDamageType()==ATK_PHYSICAL)?physicalDamage:(cMove.getDamageType()==ATK_SPECIAL)?specialDamage:0.0f;
            float simpleDamage = 
              (float)cMove.getPower() * 
              damageTypeBonus * // contains levelModifier, rngModifier
              typeStabBonus;
            if (mostlyGT(simpleDamage, 0.0f)) { simpleDamage += 0.125f; } // can we put this here? (boosts calculated later)

            if (simpleDamage > bestDamage) { bestDamage = simpleDamage; iBestDamage = (uint8_t)iMove; }
          } // endOf foreach move

          // scale by percentage of target pokemon's HP:
          bestDamage = scale(bestDamage, (float)tPKNV.getFV_base(FV_HITPOINTS), 0.0f);

          valid[iBestDamage] = false;
          cIBestMoves[iNMove] = iBestDamage;
          cBestMoves[iNMove] = bestDamage;
        }
      } // endOf foreach other teammate
    } // endOf foreach current teammate
  } // endOf foreach team
}; // endOf generateBestMoves





void featureVector_impl::generateOrders(const bestMoveDamages_t& dBestMoves, orders_t& orders)
{
  std::array< std::array< uint8_t , 6> , 2> preOrders;
  // seed the unmodified order into preOrders:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    std::array<uint8_t, 6>& cOrder = preOrders[iTeam];

    //size_t iOTeam = (iTeam + 1) % 2;
    std::array<bool, 6> valid; valid.fill(true);
    for (size_t iNTeammate = 0; iNTeammate != 6; ++iNTeammate)
    {
      float bestCoverage = -std::numeric_limits<float>::infinity();
      size_t iBestCoverage = SIZE_MAX;
      for (size_t iTeammate = 0; iTeammate != 6; ++iTeammate)
      {
        if (!valid[iTeammate]) { continue; }

        float currentCoverage = 0.0;
        for (size_t iOTeammate = 0; iOTeammate != 6; ++iOTeammate)
        {
          currentCoverage += dBestMoves[iTeam][iTeammate][iOTeammate][0];
        }
        if (currentCoverage > bestCoverage) { bestCoverage = currentCoverage; iBestCoverage = iTeammate; }
      }

      valid[iBestCoverage] = false;
      cOrder[iNTeammate] = iBestCoverage;
    }
  }

  // place modified order into orders:
  for (size_t iTeam = 0; iTeam < 2; ++iTeam)
  {
    std::array< std::array< uint8_t , 6> , 6>& cOrders = orders[iTeam];
    std::array< uint8_t , 6>& preOrder = preOrders[iTeam];
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
