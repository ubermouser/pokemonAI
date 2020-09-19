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
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/511e4115/backpropNet.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_featureVector.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_network128.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_network16.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_network32.o \
	${OBJECTDIR}/_ext/511e4115/evaluator_network64.o \
	${OBJECTDIR}/_ext/511e4115/experienceNet.o \
	${OBJECTDIR}/_ext/511e4115/neuralNet.o \
	${OBJECTDIR}/_ext/511e4115/old_trainer.o \
	${OBJECTDIR}/_ext/511e4115/ranked.o \
	${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o \
	${OBJECTDIR}/_ext/511e4115/ranked_neuralNet.o \
	${OBJECTDIR}/_ext/511e4115/ranked_team.o \
	${OBJECTDIR}/_ext/511e4115/ranker_main.o \
	${OBJECTDIR}/_ext/511e4115/temporalpropNet.o \
	${OBJECTDIR}/_ext/511e4115/trainer.o \
	${OBJECTDIR}/_ext/511e4115/trainer_io.o \
	${OBJECTDIR}/_ext/511e4115/trainer_neural.o \
	${OBJECTDIR}/_ext/511e4115/trueSkill.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/511e4115/backpropNet.o: ../src/backpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/backpropNet.o ../src/backpropNet.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_featureVector.o: ../src/evaluator_featureVector.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_featureVector.o ../src/evaluator_featureVector.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_network128.o: ../src/evaluator_network128.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_network128.o ../src/evaluator_network128.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_network16.o: ../src/evaluator_network16.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_network16.o ../src/evaluator_network16.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_network32.o: ../src/evaluator_network32.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_network32.o ../src/evaluator_network32.cpp

${OBJECTDIR}/_ext/511e4115/evaluator_network64.o: ../src/evaluator_network64.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/evaluator_network64.o ../src/evaluator_network64.cpp

${OBJECTDIR}/_ext/511e4115/experienceNet.o: ../src/experienceNet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/experienceNet.o ../src/experienceNet.cpp

${OBJECTDIR}/_ext/511e4115/neuralNet.o: ../src/neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/neuralNet.o ../src/neuralNet.cpp

${OBJECTDIR}/_ext/511e4115/old_trainer.o: ../src/old_trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/old_trainer.o ../src/old_trainer.cpp

${OBJECTDIR}/_ext/511e4115/ranked.o: ../src/ranked.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked.o ../src/ranked.cpp

${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o: ../src/ranked_evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o ../src/ranked_evaluator.cpp

${OBJECTDIR}/_ext/511e4115/ranked_neuralNet.o: ../src/ranked_neuralNet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_neuralNet.o ../src/ranked_neuralNet.cpp

${OBJECTDIR}/_ext/511e4115/ranked_team.o: ../src/ranked_team.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_team.o ../src/ranked_team.cpp

${OBJECTDIR}/_ext/511e4115/ranker_main.o: ../src/ranker_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranker_main.o ../src/ranker_main.cpp

${OBJECTDIR}/_ext/511e4115/temporalpropNet.o: ../src/temporalpropNet.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/temporalpropNet.o ../src/temporalpropNet.cpp

${OBJECTDIR}/_ext/511e4115/trainer.o: ../src/trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trainer.o ../src/trainer.cpp

${OBJECTDIR}/_ext/511e4115/trainer_io.o: ../src/trainer_io.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trainer_io.o ../src/trainer_io.cpp

${OBJECTDIR}/_ext/511e4115/trainer_neural.o: ../src/trainer_neural.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trainer_neural.o ../src/trainer_neural.cpp

${OBJECTDIR}/_ext/511e4115/trueSkill.o: ../src/trueSkill.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trueSkill.o ../src/trueSkill.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
