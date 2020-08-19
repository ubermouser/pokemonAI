//#define PKAI_IMPORT
#include "../inc/evaluator_simple.h"

#include <sstream>
#include <assert.h>

#include "../inc/fp_compare.h"

#include "../inc/engine.h"


evaluator_simple::evaluator_simple(fpType _bias)
    : Evaluator(),
    bias(_bias) {
  assert(mostlyGTE(bias, 0.0) && mostlyLTE(bias, 1.0));
  {
    std::ostringstream name;
    name << "Simple_Evaluator-" << bias;
    setName(name.str());
  }
};


bool evaluator_simple::isInitialized() const
{
  if (nv_ == NULL) { return false; }
  if (mostlyGT(bias, 1.0) || mostlyLT(bias, 0.0)) { return false; }
  return true;
}


fpType evaluator_simple::fitness_team(const ConstTeamVolatile& tV) {
  fpType accumulator = 0.0;
  
  for (size_t iTeammate = 0, count = tV.nv().getNumTeammates(); iTeammate != count; ++iTeammate) {
    ConstPokemonVolatile cPKV = tV.teammate(iTeammate);

    accumulator += 900 * cPKV.getPercentHP() + (cPKV.isAlive()?50:0);
  }

  if (accumulator>=50)
  {
    accumulator += 50; // if we haven't lost the game yet, add a bonus:
  }
  
  return accumulator;
};


fpType evaluator_simple::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam, fpType bias) {
  // calculate fitness
  fpType agentFitness = (bias)       *fitness_team(env.getTeam(iTeam));
  fpType otherFitness = (1.0 - bias) * fitness_team(env.getOtherTeam(iTeam));
  fpType maxFitness = std::max(agentFitness, otherFitness);
  // if maxFitness is about 0, we've tied the game. Ties do not favor either team
  if (mostlyEQ(maxFitness, 0.0)) { return 0.5; }

  fpType fitness =
    (agentFitness - otherFitness) /
    maxFitness;

  // normalize fitness from 0..1 instead of -1..1
  fitness = (fitness + 1.0) / 2.0;

  return fitness;
};


EvalResult_t evaluator_simple::calculateFitness(const ConstEnvironmentVolatile& env, size_t iTeam)
{
  EvalResult_t result = { calculateFitness(env, iTeam, bias), -1, -1 };
  return result;
};
