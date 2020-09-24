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
	${OBJECTDIR}/_ext/511e4115/game_factory.o \
	${OBJECTDIR}/_ext/511e4115/ranked.o \
	${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o \
	${OBJECTDIR}/_ext/511e4115/ranked_team.o \
	${OBJECTDIR}/_ext/511e4115/ranker.o \
	${OBJECTDIR}/_ext/511e4115/ranker_main.o \
	${OBJECTDIR}/_ext/511e4115/trainer_io.o \
	${OBJECTDIR}/_ext/511e4115/true_skill.o


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

${OBJECTDIR}/_ext/511e4115/game_factory.o: ../src/game_factory.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/game_factory.o ../src/game_factory.cpp

${OBJECTDIR}/_ext/511e4115/ranked.o: ../src/ranked.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked.o ../src/ranked.cpp

${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o: ../src/ranked_evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o ../src/ranked_evaluator.cpp

${OBJECTDIR}/_ext/511e4115/ranked_team.o: ../src/ranked_team.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_team.o ../src/ranked_team.cpp

${OBJECTDIR}/_ext/511e4115/ranker.o: ../src/ranker.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranker.o ../src/ranker.cpp

${OBJECTDIR}/_ext/511e4115/ranker_main.o: ../src/ranker_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranker_main.o ../src/ranker_main.cpp

${OBJECTDIR}/_ext/511e4115/trainer_io.o: ../src/trainer_io.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trainer_io.o ../src/trainer_io.cpp

${OBJECTDIR}/_ext/511e4115/true_skill.o: ../src/true_skill.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/true_skill.o ../src/true_skill.cpp

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
