//#define PKAI_IMPORT
#include "../inc/evaluator_simple.h"

#include <sstream>
#include <assert.h>

#include "../inc/fp_compare.h"

#include "../inc/environment_nonvolatile.h"
#include "../inc/team_nonvolatile.h"
#include "../inc/pokemon_nonvolatile.h"

#include "../inc/environment_volatile.h"
#include "../inc/team_volatile.h"
#include "../inc/pokemon_volatile.h"

evaluator_simple::evaluator_simple(fpType _bias)
  : ident(),
  bias(_bias),
  envNV(NULL)
{
  assert(mostlyGTE(bias, 0.0) && mostlyLTE(bias, 1.0));
  {
    std::ostringstream name;
    name << "Simple_Evaluator-" << bias;
    ident = name.str();
  }
};

evaluator_simple::evaluator_simple(const evaluator_simple& other)
  : ident(other.ident),
  bias(other.bias),
  envNV(other.envNV)
{
};

bool evaluator_simple::isInitialized() const
{
  if (envNV == NULL) { return false; }
  if (mostlyGT(bias, 1.0) || mostlyLT(bias, 0.0)) { return false; }
  return true;
}

fpType evaluator_simple::fitness_team(const team_volatile& tV, const team_nonvolatile& tNV)
{
  fpType accumulator = 0.0;
  
  for (size_t iTeammate = 0, _numTeammates = tNV.getNumTeammates(); iTeammate != _numTeammates; ++iTeammate)
  {
    const pokemon_volatile& cPKV = tV.teammate(iTeammate);

    accumulator += 900 * cPKV.getPercentHP(tNV.teammate(iTeammate)) + (cPKV.isAlive()?50:0);
  }

  if (accumulator>=50)
  {
    accumulator += 50; // if we haven't lost the game yet, add a bonus:
  }
  
  return accumulator;
};

fpType evaluator_simple::calculateFitness(const environment_nonvolatile& envNV, const environment_volatile& env, size_t iTeam, fpType bias)
{
  // calculate fitness
  fpType agentFitness = (bias)		*	fitness_team(env.getTeam(iTeam), envNV.getTeam(iTeam));
  fpType otherFitness = (1.0 - bias)	*	fitness_team(env.getOtherTeam(iTeam), envNV.getOtherTeam(iTeam));
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

evalResult_t evaluator_simple::calculateFitness(const environment_volatile& env, size_t iTeam)
{
  evalResult_t result = { calculateFitness(*envNV, env, iTeam, bias), -1, -1 };
  return result;
};





