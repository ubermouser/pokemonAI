//#define PKAI_IMPORT
#include "../inc/planner_human.h"

#include <string>
#include <sstream>
#include <iostream>

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"
#include "../inc/environment_nonvolatile.h"

const std::string PlannerHuman::ident = "Human_Planner-NULLEVAL";


bool PlannerHuman::isInitialized() const {
  if (agentTeam_ >= 2) { return false; }
  if (nv_ == NULL) { return false; }
  if (cu_ == NULL) { return false; }
  
  return true; 
}


uint32_t PlannerHuman::generateSolution(const ConstEnvironmentPossible& origin)
{
  std::cout << origin;
  printActions(origin.getEnv());
  
  return actionSelect(origin.getEnv());
};


void PlannerHuman::printActions(const ConstEnvironmentVolatile& env)
{
  const ConstTeamVolatile cTeam = env.getTeam(agentTeam_);
  const ConstPokemonVolatile cPokemon = cTeam.getPKV();
  std::cout << "Active pokemon: \n";
  
  // if this is false, then the only move this pokemon may use is "thrash"
  for (unsigned int iAction = 0; iAction < cPokemon.nv().getNumMoves(); iAction++) {
    const ConstMoveVolatile cMove = cPokemon.getMV(AT_MOVE_0 + iAction);

    if (cu_->isValidAction(env, AT_MOVE_0 + iAction, agentTeam_) == false) { continue; }
    
    std::cout << "\t" << (AT_MOVE_0 + iAction) << "-" << cMove << "\n";
  }
  
  if (cu_->isValidAction(env, AT_MOVE_STRUGGLE, agentTeam_)) { // print information about struggle move
    std::cout << "\t" << (AT_MOVE_STRUGGLE) << "-\"Struggle\" -/-\n"; 
  }
  
  if (cu_->isValidAction(env, AT_MOVE_NOTHING, agentTeam_)) {
    std::cout << "\t" << (AT_MOVE_NOTHING) << "-\"Nothing\" -/-\n"; 
  }
  
  if (cTeam.nv().getNumTeammates() > 1)
  {
    std::cout << "Or switch to a sidelined pokemon: \n";
    for (unsigned int iTeammate = 0; iTeammate < cTeam.nv().getNumTeammates(); iTeammate++) {
      if (cu_->isValidAction(env, AT_SWITCH_0 + iTeammate, agentTeam_) == false) { continue; }
      
      std::cout << "\t" << (AT_SWITCH_0 + iTeammate) << "-" << cTeam.teammate(iTeammate);
    }
  }
};


uint32_t PlannerHuman::actionSelect(const ConstEnvironmentVolatile& env) {
  std::string input;
  uint32_t action;
  
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
