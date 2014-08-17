//#define PKAI_IMPORT
#include "../inc/planner_human.h"

#include <string>
#include <sstream>
#include <iostream>

#include "../inc/pkCU.h"
#include "../inc/environment_possible.h"
#include "../inc/environment_nonvolatile.h"

const std::string planner_human::ident = "Human_Planner-NULLEVAL";

planner_human::planner_human()
	: cu(NULL),
	agentTeam(SIZE_MAX)
{
};

planner_human::planner_human(const planner_human& other)
	: cu(other.cu),
	agentTeam(other.agentTeam)
{
};

bool planner_human::isInitialized() const 
{ 
	if (agentTeam >= 2) { return false; }
	if (cu == NULL) { return false; }
	
	return true; 
}

void planner_human::setEnvironment(pkCU& _cu, size_t _agentTeam)
{
	cu = &_cu;
	agentTeam = _agentTeam;
};

uint32_t planner_human::generateSolution(const environment_possible& origin)
{
	std::cout << envP_print(cu->getNV(), origin, agentTeam);
	printActions(origin.getEnv());
	
	return actionSelect(origin.getEnv());
};

void planner_human::printActions(const environment_volatile& env)
{
	const environment_nonvolatile& envNV = cu->getNV();
	const team_nonvolatile& cTeam = envNV.getTeam(agentTeam);
	const team_volatile& currentTeam = env.getTeam(agentTeam);
	const pokemon_nonvolatile& cPokemon = cTeam.getPKNV(currentTeam);
	std::cout << "Active pokemon: \n";
	
	// if this is false, then the only move this pokemon may use is "thrash"
	for (unsigned int iAction = 0; iAction < cPokemon.getNumMoves(); iAction++)
	{
		const move_nonvolatile& cMove = cTeam.getPKNV(currentTeam).getMove(AT_MOVE_0 + iAction);
		
		const move_volatile& currentMove = currentTeam.getPKV().getMV(AT_MOVE_0 + iAction);

		if (cu->isValidAction(env, AT_MOVE_0 + iAction, agentTeam) == false) { continue; }
		
		std::cout << "\t" << (AT_MOVE_0 + iAction) << "-" << move_print(cMove, currentMove) << "\n";
	}
	
	if (cu->isValidAction(env, AT_MOVE_STRUGGLE, agentTeam)) // print information about struggle move
	{
		std::cout << "\t" << (AT_MOVE_STRUGGLE) << "-\"Struggle\" -/-\n"; 
	}
	
	if (cu->isValidAction(env, AT_MOVE_NOTHING, agentTeam))
	{
		std::cout << "\t" << (AT_MOVE_NOTHING) << "-\"Nothing\" -/-\n"; 
	}
	
	if (cTeam.getNumTeammates() > 1)
	{
		std::cout << "Or switch to a sidelined pokemon: \n";
		for (unsigned int iTeammate = 0; iTeammate < cTeam.getNumTeammates(); iTeammate++)
		{
			if (cu->isValidAction(env, AT_SWITCH_0 + iTeammate, agentTeam) == false) { continue; }
			
			std::cout << "\t" << (AT_SWITCH_0 + iTeammate) << "-" << pokemon_print(cTeam.teammate(iTeammate), currentTeam, currentTeam.teammate(iTeammate));
		}
	}
};

uint32_t planner_human::actionSelect(const environment_volatile& env)
{
	std::string input;
	uint32_t action;
	
	do
	{
		std::cout << "Please select the index of your desired action for Team " << (agentTeam==TEAM_A?"A":"B") << ":\n";
		getline(std::cin, input);
		std::stringstream inputResult(input);
		
		// determine if action is valid:
		if (!(inputResult >> action) || !cu->isValidAction(env, action, agentTeam)) 
		{
			std::cout << "Invalid action \"" << input << "\"!\n";
			
			continue; 
		}
		
		break;
	}while(true);
	
	return action;
}; // endOf actionSelect
