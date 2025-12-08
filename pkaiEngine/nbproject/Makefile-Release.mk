#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=clang
CCC=clang++
CXX=clang++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLANG-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/511e4115/ability.o \
	${OBJECTDIR}/_ext/511e4115/action.o \
	${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o \
	${OBJECTDIR}/_ext/511e4115/environment_possible.o \
	${OBJECTDIR}/_ext/511e4115/environment_volatile.o \
	${OBJECTDIR}/_ext/511e4115/evaluator.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_random.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_simple.o \
	${OBJECTDIR}/_ext/511e4115/evaluators.o \
	${OBJECTDIR}/_ext/511e4115/fitness.o \
	${OBJECTDIR}/_ext/e96877d6/fixed_func.o \
	${OBJECTDIR}/_ext/511e4115/game.o \
	${OBJECTDIR}/_ext/511e4115/gen4_scripts.o \
	${OBJECTDIR}/_ext/511e4115/init_toolbox.o \
	${OBJECTDIR}/_ext/511e4115/item.o \
	${OBJECTDIR}/_ext/511e4115/move.o \
	${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o \
	${OBJECTDIR}/_ext/511e4115/move_volatile.o \
	${OBJECTDIR}/_ext/511e4115/name.o \
	${OBJECTDIR}/_ext/511e4115/nature.o \
	${OBJECTDIR}/_ext/511e4115/order_heuristic.o \
	${OBJECTDIR}/_ext/511e4115/orphan.o \
	${OBJECTDIR}/_ext/511e4115/pkCU.o \
	${OBJECTDIR}/_ext/511e4115/planner.o \
	${OBJECTDIR}/_ext/511e4115/planner_human.o \
	${OBJECTDIR}/_ext/511e4115/planner_max.o \
	${OBJECTDIR}/_ext/511e4115/planner_maximin.o \
	${OBJECTDIR}/_ext/511e4115/planner_minimax.o \
	${OBJECTDIR}/_ext/511e4115/planner_random.o \
	${OBJECTDIR}/_ext/511e4115/planners.o \
	${OBJECTDIR}/_ext/511e4115/pluggable.o \
	${OBJECTDIR}/_ext/511e4115/pokedex.o \
	${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o \
	${OBJECTDIR}/_ext/511e4115/pokemon_base.o \
	${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o \
	${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o \
	${OBJECTDIR}/_ext/511e4115/serializable.o \
	${OBJECTDIR}/_ext/511e4115/signature.o \
	${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o \
	${OBJECTDIR}/_ext/511e4115/team_volatile.o \
	${OBJECTDIR}/_ext/511e4115/transposition_table.o \
	${OBJECTDIR}/_ext/511e4115/type.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f1 \
	${TESTDIR}/TestFiles/f5 \
	${TESTDIR}/TestFiles/f3 \
	${TESTDIR}/TestFiles/f4 \
	${TESTDIR}/TestFiles/f2

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/_ext/a55270a7/test_engine.o \
	${TESTDIR}/_ext/a55270a7/test_evaluator.o \
	${TESTDIR}/_ext/a55270a7/test_game.o \
	${TESTDIR}/_ext/a55270a7/test_planner.o \
	${TESTDIR}/_ext/a55270a7/test_pokedex.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-g -mtune=native -march=native -fopenmp -std=c++17
CXXFLAGS=-g -mtune=native -march=native -fopenmp -std=c++17

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a

${OBJECTDIR}/_ext/511e4115/ability.o: ../src/ability.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ability.o ../src/ability.cpp

${OBJECTDIR}/_ext/511e4115/action.o: ../src/action.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/action.o ../src/action.cpp

${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o: ../src/environment_nonvolatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o ../src/environment_nonvolatile.cpp

${OBJECTDIR}/_ext/511e4115/environment_possible.o: ../src/environment_possible.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_possible.o ../src/environment_possible.cpp

${OBJECTDIR}/_ext/511e4115/environment_volatile.o: ../src/environment_volatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_volatile.o ../src/environment_volatile.cpp

${OBJECTDIR}/_ext/511e4115/evaluator.o: ../src/evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator.o ../src/evaluator.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o: ../src/evaluator_montecarlo.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o ../src/evaluator_montecarlo.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_random.o: ../src/evaluator_random.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_random.o ../src/evaluator_random.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_simple.o: ../src/evaluator_simple.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_simple.o ../src/evaluator_simple.cpp

${OBJECTDIR}/_ext/511e4115/evaluators.o: ../src/evaluators.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluators.o ../src/evaluators.cpp

${OBJECTDIR}/_ext/511e4115/fitness.o: ../src/fitness.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/fitness.o ../src/fitness.cpp

${OBJECTDIR}/_ext/e96877d6/fixed_func.o: ../src/fixedpoint/fixed_func.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/e96877d6
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e96877d6/fixed_func.o ../src/fixedpoint/fixed_func.cpp

${OBJECTDIR}/_ext/511e4115/game.o: ../src/game.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/game.o ../src/game.cpp

${OBJECTDIR}/_ext/511e4115/gen4_scripts.o: ../src/gen4_scripts.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/gen4_scripts.o ../src/gen4_scripts.cpp

${OBJECTDIR}/_ext/511e4115/init_toolbox.o: ../src/init_toolbox.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/init_toolbox.o ../src/init_toolbox.cpp

${OBJECTDIR}/_ext/511e4115/item.o: ../src/item.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/item.o ../src/item.cpp

${OBJECTDIR}/_ext/511e4115/move.o: ../src/move.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move.o ../src/move.cpp

${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o: ../src/move_nonvolatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o ../src/move_nonvolatile.cpp

${OBJECTDIR}/_ext/511e4115/move_volatile.o: ../src/move_volatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move_volatile.o ../src/move_volatile.cpp

${OBJECTDIR}/_ext/511e4115/name.o: ../src/name.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/name.o ../src/name.cpp

${OBJECTDIR}/_ext/511e4115/nature.o: ../src/nature.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/nature.o ../src/nature.cpp

${OBJECTDIR}/_ext/511e4115/order_heuristic.o: ../src/order_heuristic.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/order_heuristic.o ../src/order_heuristic.cpp

${OBJECTDIR}/_ext/511e4115/orphan.o: ../src/orphan.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/orphan.o ../src/orphan.cpp

${OBJECTDIR}/_ext/511e4115/pkCU.o: ../src/pkCU.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pkCU.o ../src/pkCU.cpp

${OBJECTDIR}/_ext/511e4115/planner.o: ../src/planner.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner.o ../src/planner.cpp

${OBJECTDIR}/_ext/511e4115/planner_human.o: ../src/planner_human.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_human.o ../src/planner_human.cpp

${OBJECTDIR}/_ext/511e4115/planner_max.o: ../src/planner_max.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_max.o ../src/planner_max.cpp

${OBJECTDIR}/_ext/511e4115/planner_maximin.o: ../src/planner_maximin.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_maximin.o ../src/planner_maximin.cpp

${OBJECTDIR}/_ext/511e4115/planner_minimax.o: ../src/planner_minimax.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_minimax.o ../src/planner_minimax.cpp

${OBJECTDIR}/_ext/511e4115/planner_random.o: ../src/planner_random.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_random.o ../src/planner_random.cpp

${OBJECTDIR}/_ext/511e4115/planners.o: ../src/planners.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planners.o ../src/planners.cpp

${OBJECTDIR}/_ext/511e4115/pluggable.o: ../src/pluggable.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pluggable.o ../src/pluggable.cpp

${OBJECTDIR}/_ext/511e4115/pokedex.o: ../src/pokedex.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokedex.o ../src/pokedex.cpp

${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o: ../src/pokedex_dynamic.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o ../src/pokedex_dynamic.cpp

${OBJECTDIR}/_ext/511e4115/pokemon_base.o: ../src/pokemon_base.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_base.o ../src/pokemon_base.cpp

${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o: ../src/pokemon_nonvolatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o ../src/pokemon_nonvolatile.cpp

${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o: ../src/pokemon_volatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o ../src/pokemon_volatile.cpp

${OBJECTDIR}/_ext/511e4115/serializable.o: ../src/serializable.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/serializable.o ../src/serializable.cpp

${OBJECTDIR}/_ext/511e4115/signature.o: ../src/signature.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/signature.o ../src/signature.cpp

${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o: ../src/team_nonvolatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o ../src/team_nonvolatile.cpp

${OBJECTDIR}/_ext/511e4115/team_volatile.o: ../src/team_volatile.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/team_volatile.o ../src/team_volatile.cpp

${OBJECTDIR}/_ext/511e4115/transposition_table.o: ../src/transposition_table.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/transposition_table.o ../src/transposition_table.cpp

${OBJECTDIR}/_ext/511e4115/type.o: ../src/type.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/type.o ../src/type.cpp

# Subprojects
.build-subprojects:

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f1: ${TESTDIR}/_ext/a55270a7/test_engine.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS}   

${TESTDIR}/TestFiles/f5: ${TESTDIR}/_ext/a55270a7/test_evaluator.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f5 $^ ${LDLIBSOPTIONS}   

${TESTDIR}/TestFiles/f3: ${TESTDIR}/_ext/a55270a7/test_game.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f3 $^ ${LDLIBSOPTIONS}   

${TESTDIR}/TestFiles/f4: ${TESTDIR}/_ext/a55270a7/test_planner.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f4 $^ ${LDLIBSOPTIONS}   

${TESTDIR}/TestFiles/f2: ${TESTDIR}/_ext/a55270a7/test_pokedex.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS}   


${TESTDIR}/_ext/a55270a7/test_engine.o: ../src/tests/test_engine.cpp 
	${MKDIR} -p ${TESTDIR}/_ext/a55270a7
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${TESTDIR}/_ext/a55270a7/test_engine.o ../src/tests/test_engine.cpp


${TESTDIR}/_ext/a55270a7/test_evaluator.o: ../src/tests/test_evaluator.cpp 
	${MKDIR} -p ${TESTDIR}/_ext/a55270a7
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/_ext/a55270a7/test_evaluator.o ../src/tests/test_evaluator.cpp


${TESTDIR}/_ext/a55270a7/test_game.o: ../src/tests/test_game.cpp 
	${MKDIR} -p ${TESTDIR}/_ext/a55270a7
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/_ext/a55270a7/test_game.o ../src/tests/test_game.cpp


${TESTDIR}/_ext/a55270a7/test_planner.o: ../src/tests/test_planner.cpp 
	${MKDIR} -p ${TESTDIR}/_ext/a55270a7
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/_ext/a55270a7/test_planner.o ../src/tests/test_planner.cpp


${TESTDIR}/_ext/a55270a7/test_pokedex.o: ../src/tests/test_pokedex.cpp 
	${MKDIR} -p ${TESTDIR}/_ext/a55270a7
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${TESTDIR}/_ext/a55270a7/test_pokedex.o ../src/tests/test_pokedex.cpp


${OBJECTDIR}/_ext/511e4115/ability_nomain.o: ${OBJECTDIR}/_ext/511e4115/ability.o ../src/ability.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/ability.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ability_nomain.o ../src/ability.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/ability.o ${OBJECTDIR}/_ext/511e4115/ability_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/action_nomain.o: ${OBJECTDIR}/_ext/511e4115/action.o ../src/action.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/action.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/action_nomain.o ../src/action.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/action.o ${OBJECTDIR}/_ext/511e4115/action_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/environment_nonvolatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o ../src/environment_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile_nomain.o ../src/environment_nonvolatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile.o ${OBJECTDIR}/_ext/511e4115/environment_nonvolatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/environment_possible_nomain.o: ${OBJECTDIR}/_ext/511e4115/environment_possible.o ../src/environment_possible.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/environment_possible.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_possible_nomain.o ../src/environment_possible.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/environment_possible.o ${OBJECTDIR}/_ext/511e4115/environment_possible_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/environment_volatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/environment_volatile.o ../src/environment_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/environment_volatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/environment_volatile_nomain.o ../src/environment_volatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/environment_volatile.o ${OBJECTDIR}/_ext/511e4115/environment_volatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/evaluator_nomain.o: ${OBJECTDIR}/_ext/511e4115/evaluator.o ../src/evaluator.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/evaluator.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_nomain.o ../src/evaluator.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/evaluator.o ${OBJECTDIR}/_ext/511e4115/evaluator_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo_nomain.o: ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o ../src/evaluator_montecarlo.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo_nomain.o ../src/evaluator_montecarlo.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo.o ${OBJECTDIR}/_ext/511e4115/evaluator_montecarlo_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/evaluator_random_nomain.o: ${OBJECTDIR}/_ext/511e4115/evaluator_random.o ../src/evaluator_random.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/evaluator_random.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_random_nomain.o ../src/evaluator_random.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/evaluator_random.o ${OBJECTDIR}/_ext/511e4115/evaluator_random_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/evaluator_simple_nomain.o: ${OBJECTDIR}/_ext/511e4115/evaluator_simple.o ../src/evaluator_simple.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/evaluator_simple.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_simple_nomain.o ../src/evaluator_simple.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/evaluator_simple.o ${OBJECTDIR}/_ext/511e4115/evaluator_simple_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/evaluators_nomain.o: ${OBJECTDIR}/_ext/511e4115/evaluators.o ../src/evaluators.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/evaluators.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluators_nomain.o ../src/evaluators.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/evaluators.o ${OBJECTDIR}/_ext/511e4115/evaluators_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/fitness_nomain.o: ${OBJECTDIR}/_ext/511e4115/fitness.o ../src/fitness.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/fitness.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/fitness_nomain.o ../src/fitness.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/fitness.o ${OBJECTDIR}/_ext/511e4115/fitness_nomain.o;\
	fi

${OBJECTDIR}/_ext/e96877d6/fixed_func_nomain.o: ${OBJECTDIR}/_ext/e96877d6/fixed_func.o ../src/fixedpoint/fixed_func.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/e96877d6
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/e96877d6/fixed_func.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/e96877d6/fixed_func_nomain.o ../src/fixedpoint/fixed_func.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/e96877d6/fixed_func.o ${OBJECTDIR}/_ext/e96877d6/fixed_func_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/game_nomain.o: ${OBJECTDIR}/_ext/511e4115/game.o ../src/game.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/game.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/game_nomain.o ../src/game.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/game.o ${OBJECTDIR}/_ext/511e4115/game_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/gen4_scripts_nomain.o: ${OBJECTDIR}/_ext/511e4115/gen4_scripts.o ../src/gen4_scripts.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/gen4_scripts.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/gen4_scripts_nomain.o ../src/gen4_scripts.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/gen4_scripts.o ${OBJECTDIR}/_ext/511e4115/gen4_scripts_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/init_toolbox_nomain.o: ${OBJECTDIR}/_ext/511e4115/init_toolbox.o ../src/init_toolbox.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/init_toolbox.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/init_toolbox_nomain.o ../src/init_toolbox.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/init_toolbox.o ${OBJECTDIR}/_ext/511e4115/init_toolbox_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/item_nomain.o: ${OBJECTDIR}/_ext/511e4115/item.o ../src/item.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/item.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/item_nomain.o ../src/item.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/item.o ${OBJECTDIR}/_ext/511e4115/item_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/move_nomain.o: ${OBJECTDIR}/_ext/511e4115/move.o ../src/move.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/move.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move_nomain.o ../src/move.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/move.o ${OBJECTDIR}/_ext/511e4115/move_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/move_nonvolatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o ../src/move_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move_nonvolatile_nomain.o ../src/move_nonvolatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/move_nonvolatile.o ${OBJECTDIR}/_ext/511e4115/move_nonvolatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/move_volatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/move_volatile.o ../src/move_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/move_volatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/move_volatile_nomain.o ../src/move_volatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/move_volatile.o ${OBJECTDIR}/_ext/511e4115/move_volatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/name_nomain.o: ${OBJECTDIR}/_ext/511e4115/name.o ../src/name.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/name.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/name_nomain.o ../src/name.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/name.o ${OBJECTDIR}/_ext/511e4115/name_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/nature_nomain.o: ${OBJECTDIR}/_ext/511e4115/nature.o ../src/nature.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/nature.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/nature_nomain.o ../src/nature.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/nature.o ${OBJECTDIR}/_ext/511e4115/nature_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/order_heuristic_nomain.o: ${OBJECTDIR}/_ext/511e4115/order_heuristic.o ../src/order_heuristic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/order_heuristic.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/order_heuristic_nomain.o ../src/order_heuristic.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/order_heuristic.o ${OBJECTDIR}/_ext/511e4115/order_heuristic_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/orphan_nomain.o: ${OBJECTDIR}/_ext/511e4115/orphan.o ../src/orphan.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/orphan.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/orphan_nomain.o ../src/orphan.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/orphan.o ${OBJECTDIR}/_ext/511e4115/orphan_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pkCU_nomain.o: ${OBJECTDIR}/_ext/511e4115/pkCU.o ../src/pkCU.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pkCU.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pkCU_nomain.o ../src/pkCU.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pkCU.o ${OBJECTDIR}/_ext/511e4115/pkCU_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner.o ../src/planner.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_nomain.o ../src/planner.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner.o ${OBJECTDIR}/_ext/511e4115/planner_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_human_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner_human.o ../src/planner_human.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner_human.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_human_nomain.o ../src/planner_human.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner_human.o ${OBJECTDIR}/_ext/511e4115/planner_human_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_max_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner_max.o ../src/planner_max.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner_max.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_max_nomain.o ../src/planner_max.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner_max.o ${OBJECTDIR}/_ext/511e4115/planner_max_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_maximin_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner_maximin.o ../src/planner_maximin.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner_maximin.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_maximin_nomain.o ../src/planner_maximin.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner_maximin.o ${OBJECTDIR}/_ext/511e4115/planner_maximin_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_minimax_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner_minimax.o ../src/planner_minimax.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner_minimax.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_minimax_nomain.o ../src/planner_minimax.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner_minimax.o ${OBJECTDIR}/_ext/511e4115/planner_minimax_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planner_random_nomain.o: ${OBJECTDIR}/_ext/511e4115/planner_random.o ../src/planner_random.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planner_random.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planner_random_nomain.o ../src/planner_random.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planner_random.o ${OBJECTDIR}/_ext/511e4115/planner_random_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/planners_nomain.o: ${OBJECTDIR}/_ext/511e4115/planners.o ../src/planners.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/planners.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/planners_nomain.o ../src/planners.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/planners.o ${OBJECTDIR}/_ext/511e4115/planners_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pluggable_nomain.o: ${OBJECTDIR}/_ext/511e4115/pluggable.o ../src/pluggable.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pluggable.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pluggable_nomain.o ../src/pluggable.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pluggable.o ${OBJECTDIR}/_ext/511e4115/pluggable_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pokedex_nomain.o: ${OBJECTDIR}/_ext/511e4115/pokedex.o ../src/pokedex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pokedex.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokedex_nomain.o ../src/pokedex.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pokedex.o ${OBJECTDIR}/_ext/511e4115/pokedex_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pokedex_dynamic_nomain.o: ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o ../src/pokedex_dynamic.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic_nomain.o ../src/pokedex_dynamic.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic.o ${OBJECTDIR}/_ext/511e4115/pokedex_dynamic_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pokemon_base_nomain.o: ${OBJECTDIR}/_ext/511e4115/pokemon_base.o ../src/pokemon_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pokemon_base.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_base_nomain.o ../src/pokemon_base.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pokemon_base.o ${OBJECTDIR}/_ext/511e4115/pokemon_base_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o ../src/pokemon_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile_nomain.o ../src/pokemon_nonvolatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile.o ${OBJECTDIR}/_ext/511e4115/pokemon_nonvolatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/pokemon_volatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o ../src/pokemon_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/pokemon_volatile_nomain.o ../src/pokemon_volatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/pokemon_volatile.o ${OBJECTDIR}/_ext/511e4115/pokemon_volatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/serializable_nomain.o: ${OBJECTDIR}/_ext/511e4115/serializable.o ../src/serializable.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/serializable.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/serializable_nomain.o ../src/serializable.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/serializable.o ${OBJECTDIR}/_ext/511e4115/serializable_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/signature_nomain.o: ${OBJECTDIR}/_ext/511e4115/signature.o ../src/signature.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/signature.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/signature_nomain.o ../src/signature.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/signature.o ${OBJECTDIR}/_ext/511e4115/signature_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/team_nonvolatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o ../src/team_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/team_nonvolatile_nomain.o ../src/team_nonvolatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/team_nonvolatile.o ${OBJECTDIR}/_ext/511e4115/team_nonvolatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/team_volatile_nomain.o: ${OBJECTDIR}/_ext/511e4115/team_volatile.o ../src/team_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/team_volatile.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/team_volatile_nomain.o ../src/team_volatile.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/team_volatile.o ${OBJECTDIR}/_ext/511e4115/team_volatile_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/transposition_table_nomain.o: ${OBJECTDIR}/_ext/511e4115/transposition_table.o ../src/transposition_table.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/transposition_table.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/transposition_table_nomain.o ../src/transposition_table.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/transposition_table.o ${OBJECTDIR}/_ext/511e4115/transposition_table_nomain.o;\
	fi

${OBJECTDIR}/_ext/511e4115/type_nomain.o: ${OBJECTDIR}/_ext/511e4115/type.o ../src/type.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	@NMOUTPUT=`${NM} ${OBJECTDIR}/_ext/511e4115/type.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/type_nomain.o ../src/type.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/_ext/511e4115/type.o ${OBJECTDIR}/_ext/511e4115/type_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f1 || true; \
	    ${TESTDIR}/TestFiles/f5 || true; \
	    ${TESTDIR}/TestFiles/f3 || true; \
	    ${TESTDIR}/TestFiles/f4 || true; \
	    ${TESTDIR}/TestFiles/f2 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
