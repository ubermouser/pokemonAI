//#define PKAI_IMPORT
#include "../inc/planner_human.h"

#include <string>
#include <sstream>
#include <iostream>

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"
#include "../inc/environment_nonvolatile.h"

const std::string PlannerHuman::ident = "HumanPlanner-NULLEVAL";


PlyResult PlannerHuman::generateSolutionAtDepth(
    const ConstEnvironmentVolatile& origin, size_t maxPly) const {
  std::cout << origin;
  printActions(origin);

  PlyResult result;
  result.agentAction = actionSelect(origin);
  return result;
};


void PlannerHuman::printActions(const ConstEnvironmentVolatile& env) const {
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);
  const ConstPokemonVolatile cPokemon = cTeam.getPKV();
  std::cout << "Active pokemon: \n";
  
  // if this is false, then the only move this pokemon may use is "thrash"
  for (const auto& action : cu_->getValidMoveActions(env, agentTeam_)) {
    if (action == AT_MOVE_STRUGGLE) {
      std::cout << "\t" << (AT_MOVE_STRUGGLE) << "-\"Struggle\" -/-\n";
    } else if (action == AT_MOVE_NOTHING) {
      std::cout << "\t" << (AT_MOVE_NOTHING) << "-\"Nothing\" -/-\n";
    } else {
      const ConstMoveVolatile cMove = cPokemon.getMV(action);
      std::cout << "\t" << (action) << "-" << cMove << "\n";
    }
  }
  if (cTeam.nv().getNumTeammates() > 1) {
    std::cout << "Or switch to a sidelined pokemon: \n";
    for (const auto& action: cu_->getValidSwapActions(env, agentTeam_)) {
      std::cout << "\t" << (action) << "-" << cTeam.teammate(action - AT_SWITCH_0);
    }
  }
};


int32_t PlannerHuman::actionSelect(const ConstEnvironmentVolatile& env) const {
  std::string input;
  int32_t action;
  
  do {
    std::cout << "Please select the index of your desired action for Team " << (agentTeam_==TEAM_A?"A":"B") << ":\n";
    getline(std::cin, input);
    std::stringstream inputResult(input);
    
    // determine if action is valid:
    if (!(inputResult >> action) || !cu_->isValidAction(env, action, agentTeam_)) {
      std::cout << "Invalid action \"" << input << "\"!\n";
      
      continue; 
    }
    
    break;
  } while(true);
  
  return action;
}; // endOf actionSelect
