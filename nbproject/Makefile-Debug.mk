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
CND_CONF=Debug
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


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-O0
CXXFLAGS=-O0

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L${CND_DISTDIR}/pkaiEngine/${CND_CONF}/${CND_PLATFORM}/ -lboost_thread -lboost_system -lboost_filesystem -lpthread -ldl pkaiEngine/dist/Debug/GNU-Linux/libpkaiengine.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: pkaiEngine/dist/Debug/GNU-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/src/agentMove.o: src/agentMove.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/agentMove.o src/agentMove.cpp

${OBJECTDIR}/src/backpropNet.o: src/backpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/backpropNet.o src/backpropNet.cpp

${OBJECTDIR}/src/evaluator.o: src/evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator.o src/evaluator.cpp

${OBJECTDIR}/src/evaluator_featureVector.o: src/evaluator_featureVector.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_featureVector.o src/evaluator_featureVector.cpp

${OBJECTDIR}/src/evaluator_network128.o: src/evaluator_network128.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network128.o src/evaluator_network128.cpp

${OBJECTDIR}/src/evaluator_network16.o: src/evaluator_network16.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network16.o src/evaluator_network16.cpp

${OBJECTDIR}/src/evaluator_network32.o: src/evaluator_network32.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network32.o src/evaluator_network32.cpp

${OBJECTDIR}/src/evaluator_network64.o: src/evaluator_network64.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_network64.o src/evaluator_network64.cpp

${OBJECTDIR}/src/evaluator_random.o: src/evaluator_random.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_random.o src/evaluator_random.cpp

${OBJECTDIR}/src/evaluator_simple.o: src/evaluator_simple.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/evaluator_simple.o src/evaluator_simple.cpp

${OBJECTDIR}/src/experienceNet.o: src/experienceNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/experienceNet.o src/experienceNet.cpp

${OBJECTDIR}/src/fixedpoint/fixed_func.o: src/fixedpoint/fixed_func.cpp
	${MKDIR} -p ${OBJECTDIR}/src/fixedpoint
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/fixedpoint/fixed_func.o src/fixedpoint/fixed_func.cpp

${OBJECTDIR}/src/game.o: src/game.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/game.o src/game.cpp

${OBJECTDIR}/src/init_toolbox.o: src/init_toolbox.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/init_toolbox.o src/init_toolbox.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/name.o: src/name.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/name.o src/name.cpp

${OBJECTDIR}/src/neuralNet.o: src/neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/neuralNet.o src/neuralNet.cpp

${OBJECTDIR}/src/orderHeuristic.o: src/orderHeuristic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/orderHeuristic.o src/orderHeuristic.cpp

${OBJECTDIR}/src/otherMove.o: src/otherMove.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/otherMove.o src/otherMove.cpp

${OBJECTDIR}/src/pkIO.o: src/pkIO.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pkIO.o src/pkIO.cpp

${OBJECTDIR}/src/planner.o: src/planner.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner.o src/planner.cpp

${OBJECTDIR}/src/planner_directed.o: src/planner_directed.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_directed.o src/planner_directed.cpp

${OBJECTDIR}/src/planner_human.o: src/planner_human.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_human.o src/planner_human.cpp

${OBJECTDIR}/src/planner_max.o: src/planner_max.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_max.o src/planner_max.cpp

${OBJECTDIR}/src/planner_minimax.o: src/planner_minimax.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax.o src/planner_minimax.cpp

${OBJECTDIR}/src/planner_minimax_thread.o: src/planner_minimax_thread.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_minimax_thread.o src/planner_minimax_thread.cpp

${OBJECTDIR}/src/planner_random.o: src/planner_random.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_random.o src/planner_random.cpp

${OBJECTDIR}/src/planner_stochastic.o: src/planner_stochastic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_stochastic.o src/planner_stochastic.cpp

${OBJECTDIR}/src/ply.o: src/ply.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ply.o src/ply.cpp

${OBJECTDIR}/src/pokemonAI.o: src/pokemonAI.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pokemonAI.o src/pokemonAI.cpp

${OBJECTDIR}/src/ranked.o: src/ranked.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked.o src/ranked.cpp

${OBJECTDIR}/src/ranked_evaluator.o: src/ranked_evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_evaluator.o src/ranked_evaluator.cpp

${OBJECTDIR}/src/ranked_neuralNet.o: src/ranked_neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_neuralNet.o src/ranked_neuralNet.cpp

${OBJECTDIR}/src/ranked_team.o: src/ranked_team.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_team.o src/ranked_team.cpp

${OBJECTDIR}/src/signature.o: src/signature.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/signature.o src/signature.cpp

${OBJECTDIR}/src/temporalpropNet.o: src/temporalpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/temporalpropNet.o src/temporalpropNet.cpp

${OBJECTDIR}/src/trainer.o: src/trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer.o src/trainer.cpp

${OBJECTDIR}/src/trainer_io.o: src/trainer_io.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer_io.o src/trainer_io.cpp

${OBJECTDIR}/src/transposition_table.o: src/transposition_table.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/transposition_table.o src/transposition_table.cpp

${OBJECTDIR}/src/trueSkill.o: src/trueSkill.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trueSkill.o src/trueSkill.cpp

${OBJECTDIR}/src/vertex.o: src/vertex.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/vertex.o src/vertex.cpp

# Subprojects
.build-subprojects:
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Debug
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Debug
	cd gen4_scripts && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Debug clean
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Debug clean
	cd gen4_scripts && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
