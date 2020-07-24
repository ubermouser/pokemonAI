#include "../inc/planner_directed.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <boost/foreach.hpp>

#include <math.h>
#include <boost/math/special_functions/fpclassify.hpp>

#include "../inc/evaluator.h"
#include "../inc/pkCU.h"
#include "../inc/fp_compare.h"

#include "../inc/environment_possible.h"
#include "../inc/environment_nonvolatile.h"

#include "../inc/experienceNet.h"
#include "../inc/evaluator_featureVector.h"

planner_directed::planner_directed(const experienceNet& _exp, size_t _engineAccuracy, fpType _bias, bool _updateExperience)
  : ident(),
  results(),
  cu(NULL),
  eval(NULL),
  exp(_exp),
  agentTeam(SIZE_MAX),
  engineAccuracy(_engineAccuracy),
  bias(_bias),
  updateExperience(_updateExperience)
{
  {
    std::ostringstream name;
    name << "directed_planner("
      << bias << ")-NULLEVAL";
    ident = name.str();
  }
};

planner_directed::planner_directed(
  const evaluator_featureVector& _eval, 
  const experienceNet& _exp, 
  size_t _engineAccuracy, 
  fpType _bias,
  bool _updateExperience)
  : ident(),
  results(),
  cu(NULL),
  eval(dynamic_cast<evaluator_featureVector*>(_eval.clone())),
  exp(_exp),
  agentTeam(SIZE_MAX),
  engineAccuracy(_engineAccuracy),
  bias(_bias),
  updateExperience(_updateExperience)
{
  {
    std::ostringstream name;
    name << "directed_planner("
      << bias << ")-"
      << eval->getName();
    ident = name.str();
  }
};

planner_directed::planner_directed(const planner_directed& other)
  : ident(other.ident),
  results(),
  cu(other.cu!=NULL?new pkCU(*other.cu):NULL),
  eval(other.eval!=NULL?dynamic_cast<evaluator_featureVector*>(other.eval->clone()):NULL),
  exp(other.exp),
  agentTeam(other.agentTeam),
  engineAccuracy(other.engineAccuracy),
  bias(other.bias),
  updateExperience(other.updateExperience)
{
};

planner_directed::~planner_directed()
{
  if (eval != NULL) { delete eval; eval = NULL; }
  if (cu != NULL) { delete cu; cu = NULL; }
};





void planner_directed::setEvaluator(const evaluator& evalType)
{
  if (eval != NULL) { delete eval; }
  eval = dynamic_cast<evaluator_featureVector*>(evalType.clone());
  if (eval != NULL)
  {
    if (cu != NULL) { eval->resetEvaluator(cu->getNV()); };
    exp.resize(eval->inputSize());
    {
      std::ostringstream name;
      name << "directed_planner("
        << bias << ")-"
        << eval->getName();
      ident = name.str();
    }
  }
  else // not the correct type of evaluator:
  {
    {
      std::ostringstream name;
      name << "directed_planner("
        << bias << ")-NULLEVAL";
      ident = name.str();
    }
  }
};

const evaluator* planner_directed::getEvaluator() const { return eval; }





void planner_directed::setEnvironment(pkCU& _cu, size_t _agentTeam)
{
  agentTeam = _agentTeam;
  if (cu == NULL) { cu = new pkCU(_cu); cu->setAccuracy(engineAccuracy); }
  else { cu->setEnvironment(_cu.getNV()); }
  if (eval != NULL) { eval->resetEvaluator(_cu.getNV()); }
  //exp.clear();
};





void planner_directed::setExperience(const experienceNet& _exp)
{
  if (eval == NULL) { exp = _exp; }
  else if (eval->inputSize() != _exp.inputSize())
  {
    if (verbose >= 5)
    {
      std::cerr << "WAR " << __FILE__ << "." << __LINE__ << 
        ": Attempted to set planner experienceNet dimensions to a size other evaluator size " << eval->inputSize() << 
        " ( " << _exp.inputSize() << 
        " )!\n";
    }
    exp.clear();
  }
  else
  {
    exp = _exp;
  }
};





bool planner_directed::isInitialized() const 
{
  if (!boost::math::isnormal(bias) || bias > 1.0) { return false; }
  if (agentTeam >= 2) { return false; }
  if (cu == NULL) { return false; }
  if (eval == NULL) { return false; }
  if (!eval->isInitialized()) { return false; }
  
  return true;
};





uint32_t planner_directed::generateSolution(const environment_possible& origin)
{
  // update experience:
  fpType maxExperienceRecip = 1.0;
  if (updateExperience) 
  { 
    std::vector<float> fVec(eval->inputSize(), 0.0f);
    eval->seed(fVec.begin(), origin.getEnv(), agentTeam);
    exp.addExperience(fVec.begin()); 
  }
  if (exp.getSettings().eType == EXPERIENCENET_HISTOGRAM)
  {
    maxExperienceRecip = 1.0 / std::max((double)exp.maximum(), 1.0);
  }

  // a count of the number of nodes evaluated:
  size_t nodesEvaluated = 0;

  // generate array of all possible actions:
  std::vector<environment_possible> rEnvP;

  // determine the best action based upon the evaluator's prediction:
  fpType bestModFitness = -std::numeric_limits<double>::infinity();
  fpType bestExperience, bestFitness;
  assert(eval->getInput() != NULL);
  size_t iBestAction = SIZE_MAX;

  for (size_t iAction = 0; iAction != AT_ITEM_USE; ++iAction)
  {
    if (!cu->isValidAction(origin.getEnv(), iAction, agentTeam)) { continue; }

    // produce the resulting state of iAction:
    rEnvP.clear();
    cu->updateState(origin.getEnv(), rEnvP, agentTeam==TEAM_A?iAction:AT_MOVE_NOTHING, agentTeam==TEAM_B?iAction:AT_MOVE_NOTHING);

    fpType lbModFitness = 0.0;
    fpType lbFitness = 0.0;
    fpType lbExperience = 0.0;
    fpType uncertainty = 1.0;
    BOOST_FOREACH(const environment_possible& cEnvP, rEnvP)
    {
      if (cEnvP.isPruned()) { continue; }

      fpType cProbability = cEnvP.getProbability().to_double();

      // determine fitness of state generated by iAction:
      evalResult_t evalResult = eval->calculateFitness(rEnvP.front().getEnv(), agentTeam);
      // determine experience of generated state (and scale by max if using histogram instead of recency):
      fpType cExperience = exp.getExperience(eval->getInput()) * maxExperienceRecip;
      // fitness modified by experience:
      fpType cModFitness = 
        (1.0 - bias) * evalResult.fitness 
        + 
        bias * (1.0 - cExperience);

      lbModFitness += cModFitness * cProbability;
      lbFitness += evalResult.fitness * cProbability;
      lbExperience += cExperience * cProbability;

      uncertainty -= cProbability;
      ++nodesEvaluated;

      // if there's no possibility this action is the best, do not continue evaluating
      if (mostlyLTE(lbModFitness + uncertainty, bestModFitness)) { break; }
    }
    // is the returned fitness better than the current best fitness:
    assert(mostlyGTE(lbModFitness, 0.0) && mostlyLTE(lbModFitness, 1.0));
    if (mostlyGT(lbModFitness, bestModFitness)) 
    {
      assert(mostlyEQ(uncertainty, 0.0));
      bestModFitness = lbModFitness;

      bestExperience = lbExperience;
      bestFitness = lbFitness;
      iBestAction = iAction;
    }
  }
  
  results.clear(); 
  results.push_back(plannerResult(1, iBestAction, -1, bestModFitness, bestModFitness));

  if (verbose >= 4)
  {
    if (iBestAction != UINT32_MAX)
    {
      std::clog << "----T" << (agentTeam==TEAM_A?"A":"B") <<
        ": ply=" << std::setw(2) << 1 << 
        " act=" << std::setw(2) << iBestAction <<
        " oact=" << std::setw(2) << -1 <<
        " lbFit=" << std::setw(9) << bestFitness <<
        " lbExp=" << std::setw(9) << bestExperience <<
        " nnod=" << std::dec << nodesEvaluated << 
        "\n"; 
    }
    else
    {
      std::clog << "~~~~T" << (agentTeam==TEAM_A?"A":"B") <<
        ": NO SOLUTIONS FOUND FOR ANY DEPTH!\n";
    }
  }

  // return best action:
  return (uint32_t)iBestAction;
};