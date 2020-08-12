#ifndef PKAI_H
#define	PKAI_H

// we use C limits instead of C++ limits, so define our usage of them
#define __STDC_LIMIT_MACROS
#define BOOST_FILESYSTEM_VERSION 3

// standard C libraries:
#include <stddef.h>
#include <stdint.h>

#include "../src/fixedpoint/fixed_class.h"

// C++ libraries:


// debugging switches:

// pokemonAI macros:
// determine whether arguments are acceptable, and if so, create datastructures from them:

// errors:
#define checkRangeE(value, min, max) \
if (!(value >= min && value <= max)) \
{ std::cerr << "ERR " << __FILE__ << "." << __LINE__ << ": " << #value << "=" << value << "; Range " << min << ".." << max << " \n"; return false; }

// Floating point precision of the project
#ifdef DOUBLEPRECISION
#define PRECISION 64
typedef double fpType;
#define FP_EPSILON 0.0000001
#else
#define PRECISION 32
typedef float fpType;
#define FP_EPSILON 0.00001
#endif
typedef fixedpoint::fixed_point<30> fixType;
#define FIX_EPSILON 108 // this value is "equivalent" to the FP_EPSILON for doubles above


// Global constants:
#define SRANDOMSEED 5

#define MAXPLUGINS 200
#define MINSTACKSIZE 16
#define MAXMOVES 400
#define MAXABILITIES 150
#define MAXTYPES 20
#define MAXNATURES 30
#define MAXPOKEMON 600
#define MAXMOVELIST 65534
#define MAXITEMS 100
#define MAXEFFORTVALUE 510
#define MAXTHREADS 32
#define MAXSEARCHDEPTH 31
#define MAXBESTOF 10001
#define MAXPLIES 1560
#define MAXTRIES 50
#define MAXLEAGUEPOPULATION 50000
#define MAXNETWORKPOPULATION 10000
#define MAXGENERATIONS 9999

#define MAXNETWORKLAYERS 9U
#define MAXNETWORKWIDTH 1024U

#define WEIGHTMULTIPLIER 10
#define FPMULTIPLIER 100

#define RANDOMENVIRONMENTS 1

//enum types:
// transposition table node types:
#define NODET_FULLEVAL 0 // node is a leaf and fully evaluated (highest priority)
#define NODET_CUTOFF 1 // node is on the horizon - its evaluation is estimated (medium priority)
#define NODET_CFULLEVAL 2 // all nodes evaluated by this vertex are leaf nodes. Evaluation to greater depth not possible. (highest priority)
#define NODET_CCUTOFF 3 // one or more nodes evaluated by this vertex are cutoff nodes. Can be evaluated to a greater depth. (medium priority)
#define NODET_NOTEVAL 4 // this vertex has not been evaluated yet (lowest priority)

// weather:
#define WEATHER_NORMAL 0
#define WEATHER_SUNNY 1
#define WEATHER_RAIN 2
#define WEATHER_SAND 3
#define WEATHER_HAIL 4
#define WEATHER_FOG 5
#define WEATHER_SHADOWSKY 6

// targets:
#define TARGET_TEAM_A0 0
#define TARGET_TEAM_A1 1
#define TARGET_TEAM_A2 2
#define TARGET_TEAM_A3 3
#define TARGET_TEAM_A4 4
#define TARGET_TEAM_A5 5
#define TARGET_TEAM_B0 6
#define TARGET_TEAM_B1 7
#define TARGET_TEAM_B2 8
#define TARGET_TEAM_B3 9
#define TARGET_TEAM_B4 10
#define TARGET_TEAM_B5 11
#define TARGET_SELF 12
#define TARGET_OPPONENT 13

// Genders:
#define SEX_MALE 0
#define SEX_FEMALE 1
#define SEX_NEUTER 2

// FV indecies:
#define FV_ATTACK 0U
#define FV_SPATTACK 1U
#define FV_DEFENSE 2U
#define FV_SPDEFENSE 3U
#define FV_SPEED 4U
#define FV_HITPOINTS 5U
#define FV_EVASION 6U
#define FV_ACCURACY 7U
#define FV_CRITICALHIT 8U

// attack types:
#define ATK_NODMG 0
#define ATK_PHYSICAL 1
#define ATK_SPECIAL 2
#define ATK_FIXED 3

// ailments (volatile):
#define AIL_V_NONE 0
#define AIL_V_CONFUSED_0T 1
#define AIL_V_CONFUSED_1T 2
#define AIL_V_CONFUSED_2T 3
#define AIL_V_CONFUSED_3T 4
#define AIL_V_CONFUSED_4T 5
#define AIL_V_CONFUSED_5T 6
#define AIL_V_CONFUSED 7 // & this with ailments and test if > 0 to determine if sleeping. MUST BE 7
#define AIL_V_INFATUATED 8
#define AIL_V_FLINCHED 16
#define AIL_V_FLINCH 32

// ailments (non-volatile):
#define AIL_NV_NONE 0
#define AIL_NV_SLEEP_0T 0
#define AIL_NV_SLEEP_1T 1
#define AIL_NV_SLEEP_2T 2
#define AIL_NV_SLEEP_3T 3
#define AIL_NV_SLEEP_4T 4
#define AIL_NV_SLEEP 4
#define AIL_NV_REST_0T 4
#define AIL_NV_REST_1T 5
#define AIL_NV_REST_2T 6
#define AIL_NV_REST_3T 7
#define AIL_NV_REST 7
#define AIL_NV_FREEZE 8
#define AIL_NV_PARALYSIS 9
#define AIL_NV_BURN 10
#define AIL_NV_POISON 11
#define AIL_NV_POISON_TOXIC 12 // has 15 possible stages from 1/16 hp to 15/16 hp. This is stage 1/16
#define AIL_NV_MAX 12

// teams:
#define TEAM_A 0
#define TEAM_B 1

// win conditions:
enum MatchState
{
  MATCH_MIDGAME = -1,
  MATCH_TEAM_A_WINS = TEAM_A,
  MATCH_TEAM_B_WINS = TEAM_B,
  MATCH_TIE = 2
};
#define MATCH_DRAW MATCH_MIDGAME

// action types:
#define AT_MOVE_0 0 // beginning of move range
#define AT_MOVE_1 1
#define AT_MOVE_2 2
#define AT_MOVE_3 3 // end of default move range
#define AT_MOVE_STRUGGLE 4
#define AT_MOVE_NOTHING 5 // end of extended move range
#define AT_SWITCH_0 6 // beginning of switch range
#define AT_SWITCH_1 7
#define AT_SWITCH_2 8
#define AT_SWITCH_3 9
#define AT_SWITCH_4 10
#define AT_SWITCH_5 11 // end of switch range
#define AT_ITEM_USE 12 // last user chosen item
#define AT_MOVE_CONFUSED 13 

// multithreading vars:
#define THREADS_MINDEPTH 2
#define THREADS_GENERATE 0 // generate a solution
#define THREADS_WAIT 1 // busy wait
#define THREADS_HALT 2 // halt, wait for invocation
#define THREADS_KILL 4 // kill ALL the threads

// game type:
enum GameType_t
{
  GT_DIAG_HUVSHU = 0 , // diagnostic-human , human (known) vs human (known)
  GT_DIAG_HUVSHU_UNCERTAIN = 1, // diagnostic-uncertainty , human (known) vs human (unknown)
  GT_DIAG_HUVSCPU = 2, // diagnostic , human (known) vs computer (known)
  GT_NORM_HUVSCPU = 3, // normal , human (unknown) vs computer (known)
  GT_NORM_CPUVSCPU = 4, // ai battle, computer (known) vs computer (known)
  GT_DIAG_BENCHMARK = 5, // plays the first team loaded against its self for secondsPerMove seconds, then quits
  GT_NORM_CPUVSHU = 6, // normal (swapped), computer (known) vs human (unknown)
  GT_OTHER_CONSOLE = 10, // do not start a game, drop directly to the console
  GT_OTHER_EVOTEAMS = 11, // do not start a game, instead attempt to evolve new pokemon teams with genetic algorithm
  GT_OTHER_EVONETS = 12, // do not start a game, instead attempt to evolve new evaluation networks
  GT_OTHER_EVOBOTH = 13, // do not start a game, instead attempt to evolve both new teams and networks
  GT_OTHER_GAUNTLET_TEAM = 14, // do not start a game, instead rank a provided pokemon team
  GT_OTHER_GAUNTLET_NET = 15, // do not start a game, instead rank a provided evaluation network
  GT_OTHER_GAUNTLET_BOTH = 16 // do not start a game, instead rank a provided team and network
};

// command-line and text output variables
extern int verbose;
// 0 = errors
// 1 = warnings
// 2 = run-time general statistics
// 3 = run-time specific statistics
// 4 = run-time per step statistics (normally disabled by default)
// 5 = run-time detailed stiatistics (used for per step debugging)
extern int warning;
// 0 - normal operation.
// 1 - warning
// 2 - error

class Pokedex;
extern const Pokedex* pkdex;

#if defined(WIN32)
#pragma warning( disable: 4251 )
#endif

#endif	/* PKAI_H */

// used to define symbols
#undef PKAISHARED
#if defined(PKAI_STATIC)
#define PKAISHARED
#elif defined(PKAI_EXPORT) && (defined(WIN32) || defined(_CYGWIN))
#define PKAISHARED __declspec( dllexport )
#elif defined(PKAI_IMPORT) && (defined(WIN32) || defined(_CYGWIN))
#define PKAISHARED __declspec( dllimport )
#else
#define PKAISHARED
#endif
