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
CC=clang-9
CCC=clang++-9
CXX=clang++-9
FC=gfortran
AS=as

# Macros
CND_PLATFORM=CLANG-Linux
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
	${OBJECTDIR}/src/backpropNet.o \
	${OBJECTDIR}/src/evaluator_featureVector.o \
	${OBJECTDIR}/src/evaluator_network128.o \
	${OBJECTDIR}/src/evaluator_network16.o \
	${OBJECTDIR}/src/evaluator_network32.o \
	${OBJECTDIR}/src/evaluator_network64.o \
	${OBJECTDIR}/src/experienceNet.o \
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/neuralNet.o \
	${OBJECTDIR}/src/old_trainer.o \
	${OBJECTDIR}/src/pkIO.o \
	${OBJECTDIR}/src/planner_directed.o \
	${OBJECTDIR}/src/planner_stochastic.o \
	${OBJECTDIR}/src/pokemonAI.o \
	${OBJECTDIR}/src/ranked_neuralNet.o \
	${OBJECTDIR}/src/temporalpropNet.o \
	${OBJECTDIR}/src/trainer_neural.o


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
LDLIBSOPTIONS=-L${CND_DISTDIR}/pkaiEngine/${CND_CONF}/${CND_PLATFORM}/ pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a -lboost_thread -lboost_system -lboost_filesystem -lpthread -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/src/backpropNet.o: src/backpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/backpropNet.o src/backpropNet.cpp

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

${OBJECTDIR}/src/experienceNet.o: src/experienceNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/experienceNet.o src/experienceNet.cpp

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/neuralNet.o: src/neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/neuralNet.o src/neuralNet.cpp

${OBJECTDIR}/src/old_trainer.o: src/old_trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/old_trainer.o src/old_trainer.cpp

${OBJECTDIR}/src/pkIO.o: src/pkIO.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pkIO.o src/pkIO.cpp

${OBJECTDIR}/src/planner_directed.o: src/planner_directed.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_directed.o src/planner_directed.cpp

${OBJECTDIR}/src/planner_stochastic.o: src/planner_stochastic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_stochastic.o src/planner_stochastic.cpp

${OBJECTDIR}/src/pokemonAI.o: src/pokemonAI.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pokemonAI.o src/pokemonAI.cpp

${OBJECTDIR}/src/ranked_neuralNet.o: src/ranked_neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/ranked_neuralNet.o src/ranked_neuralNet.cpp

${OBJECTDIR}/src/temporalpropNet.o: src/temporalpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/temporalpropNet.o src/temporalpropNet.cpp

${OBJECTDIR}/src/trainer_neural.o: src/trainer_neural.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/trainer_neural.o src/trainer_neural.cpp

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
