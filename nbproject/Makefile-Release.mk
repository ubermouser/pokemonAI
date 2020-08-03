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
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/agentMove.o \
	${OBJECTDIR}/src/backpropNet.o \
	${OBJECTDIR}/src/evaluator.o \
	${OBJECTDIR}/src/evaluator_featureVector.o \
	${OBJECTDIR}/src/evaluator_network128.o \
	${OBJECTDIR}/src/evaluator_network16.o \
	${OBJECTDIR}/src/evaluator_network32.o \
	${OBJECTDIR}/src/evaluator_network64.o \
	${OBJECTDIR}/src/evaluator_random.o \
	${OBJECTDIR}/src/evaluator_simple.o \
	${OBJECTDIR}/src/experienceNet.o \
	${OBJECTDIR}/src/fixedpoint/fixed_func.o \
	${OBJECTDIR}/src/game.o \
	${OBJECTDIR}/src/init_toolbox.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/name.o \
	${OBJECTDIR}/src/neuralNet.o \
	${OBJECTDIR}/src/orderHeuristic.o \
	${OBJECTDIR}/src/otherMove.o \
	${OBJECTDIR}/src/pkIO.o \
	${OBJECTDIR}/src/planner.o \
	${OBJECTDIR}/src/planner_directed.o \
	${OBJECTDIR}/src/planner_human.o \
	${OBJECTDIR}/src/planner_max.o \
	${OBJECTDIR}/src/planner_minimax.o \
	${OBJECTDIR}/src/planner_minimax_thread.o \
	${OBJECTDIR}/src/planner_random.o \
	${OBJECTDIR}/src/planner_stochastic.o \
	${OBJECTDIR}/src/ply.o \
	${OBJECTDIR}/src/pokemonAI.o \
	${OBJECTDIR}/src/ranked.o \
	${OBJECTDIR}/src/ranked_evaluator.o \
	${OBJECTDIR}/src/ranked_neuralNet.o \
	${OBJECTDIR}/src/ranked_team.o \
	${OBJECTDIR}/src/signature.o \
	${OBJECTDIR}/src/temporalpropNet.o \
	${OBJECTDIR}/src/trainer.o \
	${OBJECTDIR}/src/trainer_io.o \
	${OBJECTDIR}/src/transposition_table.o \
	${OBJECTDIR}/src/trueSkill.o \
	${OBJECTDIR}/src/vertex.o

# Test Directory
TESTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}/tests

# Test Files
TESTFILES= \
	${TESTDIR}/TestFiles/f2 \
	${TESTDIR}/TestFiles/f1

# Test Object Files
TESTOBJECTFILES= \
	${TESTDIR}/src/tests/test_engine.o \
	${TESTDIR}/src/tests/test_pokedex.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-mtune=native -march=native
CXXFLAGS=-mtune=native -march=native

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L${CND_DISTDIR}/pkaiEngine/${CND_CONF}/${CND_PLATFORM}/ -lboost_thread -lboost_system -lboost_filesystem -lpthread -ldl pkaiEngine/dist/Release/GNU-Linux/libpkaiengine.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: pkaiEngine/dist/Release/GNU-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/src/agentMove.o: src/agentMove.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/agentMove.o src/agentMove.cpp

${OBJECTDIR}/src/backpropNet.o: src/backpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/backpropNet.o src/backpropNet.cpp

${OBJECTDIR}/src/evaluator.o: src/evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator.o src/evaluator.cpp

${OBJECTDIR}/src/evaluator_featureVector.o: src/evaluator_featureVector.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_featureVector.o src/evaluator_featureVector.cpp

${OBJECTDIR}/src/evaluator_network128.o: src/evaluator_network128.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network128.o src/evaluator_network128.cpp

${OBJECTDIR}/src/evaluator_network16.o: src/evaluator_network16.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network16.o src/evaluator_network16.cpp

${OBJECTDIR}/src/evaluator_network32.o: src/evaluator_network32.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network32.o src/evaluator_network32.cpp

${OBJECTDIR}/src/evaluator_network64.o: src/evaluator_network64.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network64.o src/evaluator_network64.cpp

${OBJECTDIR}/src/evaluator_random.o: src/evaluator_random.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_random.o src/evaluator_random.cpp

${OBJECTDIR}/src/evaluator_simple.o: src/evaluator_simple.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_simple.o src/evaluator_simple.cpp

${OBJECTDIR}/src/experienceNet.o: src/experienceNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/experienceNet.o src/experienceNet.cpp

${OBJECTDIR}/src/fixedpoint/fixed_func.o: src/fixedpoint/fixed_func.cpp
	${MKDIR} -p ${OBJECTDIR}/src/fixedpoint
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fixedpoint/fixed_func.o src/fixedpoint/fixed_func.cpp

${OBJECTDIR}/src/game.o: src/game.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/game.o src/game.cpp

${OBJECTDIR}/src/init_toolbox.o: src/init_toolbox.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/init_toolbox.o src/init_toolbox.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/name.o: src/name.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/name.o src/name.cpp

${OBJECTDIR}/src/neuralNet.o: src/neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/neuralNet.o src/neuralNet.cpp

${OBJECTDIR}/src/orderHeuristic.o: src/orderHeuristic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/orderHeuristic.o src/orderHeuristic.cpp

${OBJECTDIR}/src/otherMove.o: src/otherMove.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/otherMove.o src/otherMove.cpp

${OBJECTDIR}/src/pkIO.o: src/pkIO.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pkIO.o src/pkIO.cpp

${OBJECTDIR}/src/planner.o: src/planner.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner.o src/planner.cpp

${OBJECTDIR}/src/planner_directed.o: src/planner_directed.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_directed.o src/planner_directed.cpp

${OBJECTDIR}/src/planner_human.o: src/planner_human.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_human.o src/planner_human.cpp

${OBJECTDIR}/src/planner_max.o: src/planner_max.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_max.o src/planner_max.cpp

${OBJECTDIR}/src/planner_minimax.o: src/planner_minimax.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax.o src/planner_minimax.cpp

${OBJECTDIR}/src/planner_minimax_thread.o: src/planner_minimax_thread.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax_thread.o src/planner_minimax_thread.cpp

${OBJECTDIR}/src/planner_random.o: src/planner_random.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_random.o src/planner_random.cpp

${OBJECTDIR}/src/planner_stochastic.o: src/planner_stochastic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_stochastic.o src/planner_stochastic.cpp

${OBJECTDIR}/src/ply.o: src/ply.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ply.o src/ply.cpp

${OBJECTDIR}/src/pokemonAI.o: src/pokemonAI.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pokemonAI.o src/pokemonAI.cpp

${OBJECTDIR}/src/ranked.o: src/ranked.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked.o src/ranked.cpp

${OBJECTDIR}/src/ranked_evaluator.o: src/ranked_evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_evaluator.o src/ranked_evaluator.cpp

${OBJECTDIR}/src/ranked_neuralNet.o: src/ranked_neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_neuralNet.o src/ranked_neuralNet.cpp

${OBJECTDIR}/src/ranked_team.o: src/ranked_team.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_team.o src/ranked_team.cpp

${OBJECTDIR}/src/signature.o: src/signature.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/signature.o src/signature.cpp

${OBJECTDIR}/src/temporalpropNet.o: src/temporalpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/temporalpropNet.o src/temporalpropNet.cpp

${OBJECTDIR}/src/trainer.o: src/trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer.o src/trainer.cpp

${OBJECTDIR}/src/trainer_io.o: src/trainer_io.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer_io.o src/trainer_io.cpp

${OBJECTDIR}/src/transposition_table.o: src/transposition_table.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/transposition_table.o src/transposition_table.cpp

${OBJECTDIR}/src/trueSkill.o: src/trueSkill.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trueSkill.o src/trueSkill.cpp

${OBJECTDIR}/src/vertex.o: src/vertex.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vertex.o src/vertex.cpp

# Subprojects
.build-subprojects:
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release
	cd gen4_scripts && ${MAKE}  -f Makefile CONF=Release

# Build Test Targets
.build-tests-conf: .build-tests-subprojects .build-conf ${TESTFILES}
.build-tests-subprojects:

${TESTDIR}/TestFiles/f2: ${TESTDIR}/src/tests/test_engine.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f2 $^ ${LDLIBSOPTIONS}   

${TESTDIR}/TestFiles/f1: ${TESTDIR}/src/tests/test_pokedex.o ${OBJECTFILES:%.o=%_nomain.o}
	${MKDIR} -p ${TESTDIR}/TestFiles
	${LINK.cc} -o ${TESTDIR}/TestFiles/f1 $^ ${LDLIBSOPTIONS}   


${TESTDIR}/src/tests/test_engine.o: src/tests/test_engine.cpp 
	${MKDIR} -p ${TESTDIR}/src/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/src/tests/test_engine.o src/tests/test_engine.cpp


${TESTDIR}/src/tests/test_pokedex.o: src/tests/test_pokedex.cpp 
	${MKDIR} -p ${TESTDIR}/src/tests
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -I. -MMD -MP -MF "$@.d" -o ${TESTDIR}/src/tests/test_pokedex.o src/tests/test_pokedex.cpp


${OBJECTDIR}/src/agentMove_nomain.o: ${OBJECTDIR}/src/agentMove.o src/agentMove.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/agentMove.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/agentMove_nomain.o src/agentMove.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/agentMove.o ${OBJECTDIR}/src/agentMove_nomain.o;\
	fi

${OBJECTDIR}/src/backpropNet_nomain.o: ${OBJECTDIR}/src/backpropNet.o src/backpropNet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/backpropNet.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/backpropNet_nomain.o src/backpropNet.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/backpropNet.o ${OBJECTDIR}/src/backpropNet_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_nomain.o: ${OBJECTDIR}/src/evaluator.o src/evaluator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_nomain.o src/evaluator.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator.o ${OBJECTDIR}/src/evaluator_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_featureVector_nomain.o: ${OBJECTDIR}/src/evaluator_featureVector.o src/evaluator_featureVector.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_featureVector.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_featureVector_nomain.o src/evaluator_featureVector.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_featureVector.o ${OBJECTDIR}/src/evaluator_featureVector_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_network128_nomain.o: ${OBJECTDIR}/src/evaluator_network128.o src/evaluator_network128.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_network128.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network128_nomain.o src/evaluator_network128.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_network128.o ${OBJECTDIR}/src/evaluator_network128_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_network16_nomain.o: ${OBJECTDIR}/src/evaluator_network16.o src/evaluator_network16.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_network16.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network16_nomain.o src/evaluator_network16.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_network16.o ${OBJECTDIR}/src/evaluator_network16_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_network32_nomain.o: ${OBJECTDIR}/src/evaluator_network32.o src/evaluator_network32.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_network32.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network32_nomain.o src/evaluator_network32.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_network32.o ${OBJECTDIR}/src/evaluator_network32_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_network64_nomain.o: ${OBJECTDIR}/src/evaluator_network64.o src/evaluator_network64.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_network64.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network64_nomain.o src/evaluator_network64.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_network64.o ${OBJECTDIR}/src/evaluator_network64_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_random_nomain.o: ${OBJECTDIR}/src/evaluator_random.o src/evaluator_random.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_random.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_random_nomain.o src/evaluator_random.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_random.o ${OBJECTDIR}/src/evaluator_random_nomain.o;\
	fi

${OBJECTDIR}/src/evaluator_simple_nomain.o: ${OBJECTDIR}/src/evaluator_simple.o src/evaluator_simple.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/evaluator_simple.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_simple_nomain.o src/evaluator_simple.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/evaluator_simple.o ${OBJECTDIR}/src/evaluator_simple_nomain.o;\
	fi

${OBJECTDIR}/src/experienceNet_nomain.o: ${OBJECTDIR}/src/experienceNet.o src/experienceNet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/experienceNet.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/experienceNet_nomain.o src/experienceNet.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/experienceNet.o ${OBJECTDIR}/src/experienceNet_nomain.o;\
	fi

${OBJECTDIR}/src/fixedpoint/fixed_func_nomain.o: ${OBJECTDIR}/src/fixedpoint/fixed_func.o src/fixedpoint/fixed_func.cpp 
	${MKDIR} -p ${OBJECTDIR}/src/fixedpoint
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/fixedpoint/fixed_func.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fixedpoint/fixed_func_nomain.o src/fixedpoint/fixed_func.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/fixedpoint/fixed_func.o ${OBJECTDIR}/src/fixedpoint/fixed_func_nomain.o;\
	fi

${OBJECTDIR}/src/game_nomain.o: ${OBJECTDIR}/src/game.o src/game.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/game.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/game_nomain.o src/game.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/game.o ${OBJECTDIR}/src/game_nomain.o;\
	fi

${OBJECTDIR}/src/init_toolbox_nomain.o: ${OBJECTDIR}/src/init_toolbox.o src/init_toolbox.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/init_toolbox.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/init_toolbox_nomain.o src/init_toolbox.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/init_toolbox.o ${OBJECTDIR}/src/init_toolbox_nomain.o;\
	fi

${OBJECTDIR}/src/main_nomain.o: ${OBJECTDIR}/src/main.o src/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/main.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main_nomain.o src/main.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/main.o ${OBJECTDIR}/src/main_nomain.o;\
	fi

${OBJECTDIR}/src/name_nomain.o: ${OBJECTDIR}/src/name.o src/name.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/name.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/name_nomain.o src/name.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/name.o ${OBJECTDIR}/src/name_nomain.o;\
	fi

${OBJECTDIR}/src/neuralNet_nomain.o: ${OBJECTDIR}/src/neuralNet.o src/neuralNet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/neuralNet.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/neuralNet_nomain.o src/neuralNet.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/neuralNet.o ${OBJECTDIR}/src/neuralNet_nomain.o;\
	fi

${OBJECTDIR}/src/orderHeuristic_nomain.o: ${OBJECTDIR}/src/orderHeuristic.o src/orderHeuristic.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/orderHeuristic.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/orderHeuristic_nomain.o src/orderHeuristic.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/orderHeuristic.o ${OBJECTDIR}/src/orderHeuristic_nomain.o;\
	fi

${OBJECTDIR}/src/otherMove_nomain.o: ${OBJECTDIR}/src/otherMove.o src/otherMove.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/otherMove.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/otherMove_nomain.o src/otherMove.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/otherMove.o ${OBJECTDIR}/src/otherMove_nomain.o;\
	fi

${OBJECTDIR}/src/pkIO_nomain.o: ${OBJECTDIR}/src/pkIO.o src/pkIO.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/pkIO.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pkIO_nomain.o src/pkIO.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/pkIO.o ${OBJECTDIR}/src/pkIO_nomain.o;\
	fi

${OBJECTDIR}/src/planner_nomain.o: ${OBJECTDIR}/src/planner.o src/planner.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_nomain.o src/planner.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner.o ${OBJECTDIR}/src/planner_nomain.o;\
	fi

${OBJECTDIR}/src/planner_directed_nomain.o: ${OBJECTDIR}/src/planner_directed.o src/planner_directed.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_directed.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_directed_nomain.o src/planner_directed.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_directed.o ${OBJECTDIR}/src/planner_directed_nomain.o;\
	fi

${OBJECTDIR}/src/planner_human_nomain.o: ${OBJECTDIR}/src/planner_human.o src/planner_human.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_human.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_human_nomain.o src/planner_human.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_human.o ${OBJECTDIR}/src/planner_human_nomain.o;\
	fi

${OBJECTDIR}/src/planner_max_nomain.o: ${OBJECTDIR}/src/planner_max.o src/planner_max.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_max.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_max_nomain.o src/planner_max.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_max.o ${OBJECTDIR}/src/planner_max_nomain.o;\
	fi

${OBJECTDIR}/src/planner_minimax_nomain.o: ${OBJECTDIR}/src/planner_minimax.o src/planner_minimax.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_minimax.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax_nomain.o src/planner_minimax.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_minimax.o ${OBJECTDIR}/src/planner_minimax_nomain.o;\
	fi

${OBJECTDIR}/src/planner_minimax_thread_nomain.o: ${OBJECTDIR}/src/planner_minimax_thread.o src/planner_minimax_thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_minimax_thread.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax_thread_nomain.o src/planner_minimax_thread.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_minimax_thread.o ${OBJECTDIR}/src/planner_minimax_thread_nomain.o;\
	fi

${OBJECTDIR}/src/planner_random_nomain.o: ${OBJECTDIR}/src/planner_random.o src/planner_random.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_random.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_random_nomain.o src/planner_random.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_random.o ${OBJECTDIR}/src/planner_random_nomain.o;\
	fi

${OBJECTDIR}/src/planner_stochastic_nomain.o: ${OBJECTDIR}/src/planner_stochastic.o src/planner_stochastic.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/planner_stochastic.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_stochastic_nomain.o src/planner_stochastic.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/planner_stochastic.o ${OBJECTDIR}/src/planner_stochastic_nomain.o;\
	fi

${OBJECTDIR}/src/ply_nomain.o: ${OBJECTDIR}/src/ply.o src/ply.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/ply.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ply_nomain.o src/ply.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/ply.o ${OBJECTDIR}/src/ply_nomain.o;\
	fi

${OBJECTDIR}/src/pokemonAI_nomain.o: ${OBJECTDIR}/src/pokemonAI.o src/pokemonAI.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/pokemonAI.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pokemonAI_nomain.o src/pokemonAI.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/pokemonAI.o ${OBJECTDIR}/src/pokemonAI_nomain.o;\
	fi

${OBJECTDIR}/src/ranked_nomain.o: ${OBJECTDIR}/src/ranked.o src/ranked.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/ranked.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_nomain.o src/ranked.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/ranked.o ${OBJECTDIR}/src/ranked_nomain.o;\
	fi

${OBJECTDIR}/src/ranked_evaluator_nomain.o: ${OBJECTDIR}/src/ranked_evaluator.o src/ranked_evaluator.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/ranked_evaluator.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_evaluator_nomain.o src/ranked_evaluator.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/ranked_evaluator.o ${OBJECTDIR}/src/ranked_evaluator_nomain.o;\
	fi

${OBJECTDIR}/src/ranked_neuralNet_nomain.o: ${OBJECTDIR}/src/ranked_neuralNet.o src/ranked_neuralNet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/ranked_neuralNet.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_neuralNet_nomain.o src/ranked_neuralNet.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/ranked_neuralNet.o ${OBJECTDIR}/src/ranked_neuralNet_nomain.o;\
	fi

${OBJECTDIR}/src/ranked_team_nomain.o: ${OBJECTDIR}/src/ranked_team.o src/ranked_team.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/ranked_team.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_team_nomain.o src/ranked_team.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/ranked_team.o ${OBJECTDIR}/src/ranked_team_nomain.o;\
	fi

${OBJECTDIR}/src/signature_nomain.o: ${OBJECTDIR}/src/signature.o src/signature.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/signature.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/signature_nomain.o src/signature.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/signature.o ${OBJECTDIR}/src/signature_nomain.o;\
	fi

${OBJECTDIR}/src/temporalpropNet_nomain.o: ${OBJECTDIR}/src/temporalpropNet.o src/temporalpropNet.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/temporalpropNet.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/temporalpropNet_nomain.o src/temporalpropNet.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/temporalpropNet.o ${OBJECTDIR}/src/temporalpropNet_nomain.o;\
	fi

${OBJECTDIR}/src/trainer_nomain.o: ${OBJECTDIR}/src/trainer.o src/trainer.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/trainer.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer_nomain.o src/trainer.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/trainer.o ${OBJECTDIR}/src/trainer_nomain.o;\
	fi

${OBJECTDIR}/src/trainer_io_nomain.o: ${OBJECTDIR}/src/trainer_io.o src/trainer_io.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/trainer_io.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer_io_nomain.o src/trainer_io.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/trainer_io.o ${OBJECTDIR}/src/trainer_io_nomain.o;\
	fi

${OBJECTDIR}/src/transposition_table_nomain.o: ${OBJECTDIR}/src/transposition_table.o src/transposition_table.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/transposition_table.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/transposition_table_nomain.o src/transposition_table.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/transposition_table.o ${OBJECTDIR}/src/transposition_table_nomain.o;\
	fi

${OBJECTDIR}/src/trueSkill_nomain.o: ${OBJECTDIR}/src/trueSkill.o src/trueSkill.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/trueSkill.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trueSkill_nomain.o src/trueSkill.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/trueSkill.o ${OBJECTDIR}/src/trueSkill_nomain.o;\
	fi

${OBJECTDIR}/src/vertex_nomain.o: ${OBJECTDIR}/src/vertex.o src/vertex.cpp 
	${MKDIR} -p ${OBJECTDIR}/src
	@NMOUTPUT=`${NM} ${OBJECTDIR}/src/vertex.o`; \
	if (echo "$$NMOUTPUT" | ${GREP} '|main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T main$$') || \
	   (echo "$$NMOUTPUT" | ${GREP} 'T _main$$'); \
	then  \
	    ${RM} "$@.d";\
	    $(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -Dmain=__nomain -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vertex_nomain.o src/vertex.cpp;\
	else  \
	    ${CP} ${OBJECTDIR}/src/vertex.o ${OBJECTDIR}/src/vertex_nomain.o;\
	fi

# Run Test Targets
.test-conf:
	@if [ "${TEST}" = "" ]; \
	then  \
	    ${TESTDIR}/TestFiles/f2 || true; \
	    ${TESTDIR}/TestFiles/f1 || true; \
	else  \
	    ./${TEST} || true; \
	fi

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release clean
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release clean
	cd gen4_scripts && ${MAKE}  -f Makefile CONF=Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
