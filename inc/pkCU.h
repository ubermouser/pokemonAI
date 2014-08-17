#ifndef PKAI_CU_H
#define	PKAI_CU_H

#include "../inc/pkai.h"

#include <stdint.h>
#include <vector>
#include <boost/array.hpp>
#include <assert.h>

#include "../inc/environment_possible.h"
#include "../inc/pluggable.h"

class environment_nonvolatile;
class pokemon_nonvolatile;
class team_nonvolatile;

union environment_possible;
union environment_volatile;
union team_volatile;
union pokemon_volatile;

// seed and priority evaluation:
#define STAGE_DNE 0
#define STAGE_SEEDED 1
#define STAGE_PRETURN 2
// switch evaluation:
#define STAGE_PRESWITCH 3
#define STAGE_POSTSWITCH 4
// pre move evaluation:
#define STAGE_STATUS 5
// move damage evaluation:
#define STAGE_MOVEBASE 6
#define STAGE_SETBASEPOWER 7
#define STAGE_SETMOVETYPE 8
#define STAGE_MODIFYBASEPOWER 9
#define STAGE_MODIFYATTACKPOWER 10
#define STAGE_MODIFYCRITICALPOWER 11
#define STAGE_MODIFYRAWDAMAGE 12
#define STAGE_MODIFYSTAB 13
#define STAGE_MODIFYTYPERESISTANCE 14
#define STAGE_MODIFYITEMPOWER 15
#define STAGE_MODIFYHITCHANCE 16
#define STAGE_EVALUATEHITCHANCE 17
#define STAGE_MODIFYCRITCHANCE 18
#define STAGE_PREDAMAGE 19
#define STAGE_POSTDAMAGE 20
// post move evaluation:
#define STAGE_POSTMOVE 21
#define STAGE_PRESECONDARY 22
#define STAGE_MODIFYSECONDARYHITCHANCE 23
#define STAGE_SECONDARY 24
#define STAGE_POSTSECONDARY 25
/// post turn status
#define STAGE_POSTTURN 26
// post round status
#define STAGE_POSTROUND 27
#define STAGE_FINAL 28
#define STAGE_HASH 29

struct damageComponents_t
{
	uint32_t damage;
	uint32_t damageCrit;
	const type* mType;
	fpType cProbability;
	fpType tProbability;
};

struct _moveBracket
{
	int actionBracket;
	unsigned int speed;
};

class PKAISHARED pkCU
{
private:
	/* the current nonvolatile environment; pkCU loads plugins only for these two teams to fight */
	const environment_nonvolatile* nv;

	/* array containing all possible matchups and the plugins they may call upon */
	boost::array< boost::array< boost::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>, 6>, 12> pluginSets;

	/* the current matchup, based on which two pokemon are active */
	boost::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>* cPluginSet;

	/* stack of environment_possible objects */
	std::vector<environment_possible>* _stack;

	/* the stage of computation each element on the stack is in */
	std::vector<uint32_t> stackStage;
	
	/* memoized array of all current pokemon volatile object pointers */
	std::vector<pokemon_volatile*> PKVOffsets;

	/* when executing the default damage pathway, these components are used for persistent data */
	std::vector<damageComponents_t> damageComponents;

	/* an array of which team is going when. 0 is current team, 1 is other team */
	boost::array<size_t, 2> iTeams;

	/* an array of each team's action. 0 is current team, 1 is other team */
	boost::array<size_t, 2> iActions;

	/* iterator - the environment that the current operation is creating values from */
	size_t iBase;

	/* used for detecting reallocations of the vector object */
	size_t prevStackCapacity;

	/* number of random environments to create per hit/crit 1-16 */
	size_t numRandomEnvironments;

	/* if iTeam = SIZE_MAX, insert for both teams. if iCTeammate =  SIZE_MAX, insert for all teammates. True if non-duplicate */
	size_t insertPluginHandler(plugin_t& cPlugin, size_t pluginType, size_t iNTeammate = SIZE_MAX);

	void updateState_move();
	
	/*
	 * Determine the priority of a given action by a given team's 
	 * currently active pokemon
	 * 
	 * action:
	 * AT_MOVE_0-3: pokemon's move
	 * AT_MOVE_STRUGGLE  : struggle
	 * AT_MOVE_NOTHING  : do nothing
	 * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
	 * AT_ITEM_USE: pokemon uses an item (not implemented)
	 * 
	 * team:
	 * 0 - team A
	 * 1 - team B
	 * 
	 * Source: http://www.smogon.com/dp/articles/move_priority
	 */
	int32_t movePriority_Bracket();

	uint32_t movePriority_Speed();

	void evaluateMove_switch();

	void evaluateMove_preMove();

	void evaluateMove_postMove();
	
	/*
	 * 
	 * evaluates all possible outcomes for a given move, and calculates 
	 * their probability of occurrence.
	 * 
	 * @param action:
	 * AT_MOVE_0-3: pokemon's move
	 * 
	 * @param team:
	 * 0 - team A
	 * 1 - team B
	 * 
	 * @return TRUE if success, FALSE if failure
	 * 
	 * Source: http://www.smogon.com/dp/articles/damage_formula
	 */
	void evaluateMove_damage();

	void evaluateMove_script();

	/* do things that occur after a turn but before the end of every round */
	void evaluateMove_postTurn();
	
	/* do things that occur at the end of every round */
	void evaluateRound_end();
	
	/* returns the number of unique environments in the result array, applies the isPruned tag to pruned environments */
	size_t combineSimilarEnvironments();

	void seedCurrentState(const environment_volatile& cEnv);

	void swapTeamIndexes() 
	{ 
		size_t iCTeam = iTeams[0];
		size_t iCAction = iActions[0];

		iTeams[0] = iTeams[1];
		iTeams[1] = iCTeam;
		iActions[0] = iActions[1];
		iActions[1] = iCAction;

		setCPluginSet();
		// reset the ENTIRE array, as we swapped the teams. Every PKV is invalid
		resetPKVArray();
	};

	boost::array<std::vector<plugin_t>, PLUGIN_MAXSIZE>& getCPluginSet();

	void setCPluginSet();

	/* fully seeds the pluginset vector */
	bool initialize();

	pkCU();
public:

	pkCU(const environment_nonvolatile& _nv, size_t engineAccuracy = SIZE_MAX);
	pkCU(const pkCU& other);

	/* pkCU stores a reference to the environment at cEnvironment. This reference must not be destroyed */
	void setEnvironment(const environment_nonvolatile& _cEnvironment);

	/* sets accuracy of pkCU */
	void setAccuracy(size_t engineAccuracy);

	~pkCU();
	
	/* 
	 * Determine who goes first, given two pokemon and their move
	 * 
	 * @param action:
	 * AT_MOVE_0-3: pokemon's move
	 * AT_MOVE_STRUGGLE  : struggle
	 * AT_MOVE_NOTHING  : do nothing
	 * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
	 * AT_ITEM_USE: pokemon uses an item (not implemented)
	 * 
	 * @return 0 if teamA wins, 1 if teamB wins, 2 if tie
	 */
	uint32_t movePriority();
	
	/*
	 * 
	 * evaluates damage and to-hit components of a standard move, calculates
	 * their probability of occurance
	 * 
	 * @param action:
	 * AT_MOVE_0-3: pokemon's move
	 * AT_MOVE_STRUGGLE  : struggle
	 * AT_MOVE_NOTHING  : do nothing
	 * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
	 * AT_ITEM_USE: pokemon uses an item (not implemented)
	 * 
	 * @param team:
	 * 0 - team A
	 * 1 - team B
	 * 
	 * @return TRUE if success, FALSE if failure
	 * 
	 * Source: http://www.smogon.com/dp/articles/damage_formula
	 * Source: http://www.smogon.com/dp/articles/status
	 */
	void evaluateMove();
	
	/*
	 * calculates stochastic component of standard damage dealt
	 */
	void calculateDamage(bool hasCrit);
	
	/*
	 * given actions A and B and currentEnvironment, generate
	 * resultEnvironment vector - what would likely happen if this
	 * sequence of actions took place
	 * 
	 * Source: http://www.smogon.com/dp/articles/damage_formula
	 * 
	 * action:
	 * AT_MOVE_0-3: pokemon's move
	 * AT_MOVE_STRUGGLE  : struggle
	 * AT_MOVE_NOTHING  : do nothing
	 * AT_SWITCH_0-5: pokemon switches out for pokemon n-6
	 * AT_ITEM_USE: pokemon uses an item (not implemented)
	 * 
	 * returns: number of unique environments in vector
	 */
	size_t updateState(const environment_volatile& currentEnvironment, std::vector<environment_possible>& resultEnvironments, size_t actionA, size_t actionB);

	/* determines whether a given action is a selectable one, given the current state */
	bool isValidAction(const environment_volatile& envV, size_t action, size_t iTeam);

	/* determines whether a game has ended, given the current state. Returns an enum of the game's current status */
	matchState isGameOver(const environment_volatile& envV);

	std::vector<environment_possible>& getStack() { return *_stack; };

	const std::vector<environment_possible>& getStack() const { assert(_stack != NULL); return *_stack; };

	damageComponents_t& getDamageComponent(size_t iStack) { return damageComponents[iStack]; };

	damageComponents_t& getDamageComponent() { return damageComponents[iBase]; };

	const damageComponents_t& getDamageComponent() const { return damageComponents[iBase]; };

	environment_possible& getBase() { return getStack()[iBase]; };

	const environment_possible& getBase() const { return getStack()[iBase]; };

	size_t getICTeam() const { return iTeams[0]; };

	size_t getIOTeam() const { return iTeams[1]; };

	size_t getICAction() const { return iActions[0]; };

	size_t getIOAction() const { return iActions[1]; };

	size_t getIBase() const { return iBase; };

	static bool isMoveAction(size_t iAction) 
	{ 
		return (iAction >= AT_MOVE_0 && iAction <= AT_MOVE_3) || (iAction == AT_MOVE_STRUGGLE);
	};

	static bool isSwitchAction(size_t iAction)
	{
		return (iAction >= AT_SWITCH_0 && iAction <= AT_SWITCH_5);
	};

	void duplicateState(boost::array<size_t, 2>& result, fpType probability, size_t iState = SIZE_MAX);

	void triplicateState(boost::array<size_t, 3>& result, fpType probabilityA, fpType probabilityB, size_t iState = SIZE_MAX);

	uint32_t getStackStage() const { return stackStage[iBase]; };

	void advanceStackStage() { ++stackStage[iBase]; };

	const environment_nonvolatile& getNV() const { return *nv; };

	const pokemon_nonvolatile& getPKNV();
	const pokemon_nonvolatile& getTPKNV();

	void setPKV(size_t iState);

	void setPKV();

	void pushPKV();

	void resetPKVArray();

	team_volatile& getTMV();
	team_volatile& getTTMV();
	team_volatile& getTMV(size_t iState);
	team_volatile& getTTMV(size_t iState);
	pokemon_volatile& getPKV();
	pokemon_volatile& getTPKV();
	pokemon_volatile& getPKV(size_t iState);
	pokemon_volatile& getTPKV(size_t iState);

	template<size_t numEnvironments>
	void nPlicateState(boost::array<size_t, numEnvironments>& result, size_t iState = SIZE_MAX)
	{
		if (iState == SIZE_MAX) { iState = iBase; }
		std::vector<environment_possible>& stack = getStack();

		//assert((stack.size() + numEnvironments) <= MAXSTACKSIZE);

		result[0] = iState;
		uint32_t baseStage = stackStage[iState];
		const damageComponents_t& baseComponent = damageComponents[iState];
		for (size_t iEnvironment = 1; iEnvironment < numEnvironments; ++iEnvironment)
		{
			size_t cSize = stack.size();

			result[iEnvironment] = cSize;
			stackStage.push_back(baseStage);
			damageComponents.push_back(baseComponent);

			stack.push_back(stack[iState]);

			pushPKV();
		}
		// re-memoize arrays of memoized pointer data:
		if (stack.capacity() != prevStackCapacity)
		{
			resetPKVArray();
			prevStackCapacity = stack.capacity();
		}
	};
};

#endif	/* PKAI_CU_H */

