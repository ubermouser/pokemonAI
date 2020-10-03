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
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/511e4115/game_factory.o \
	${OBJECTDIR}/_ext/511e4115/league.o \
	${OBJECTDIR}/_ext/511e4115/ranked.o \
	${OBJECTDIR}/_ext/511e4115/ranked_battlegroup.o \
	${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o \
	${OBJECTDIR}/_ext/511e4115/ranked_planner.o \
	${OBJECTDIR}/_ext/511e4115/ranked_pokemon.o \
	${OBJECTDIR}/_ext/511e4115/ranked_team.o \
	${OBJECTDIR}/_ext/511e4115/ranker.o \
	${OBJECTDIR}/_ext/511e4115/ranker_main.o \
	${OBJECTDIR}/_ext/511e4115/team_factory.o \
	${OBJECTDIR}/_ext/511e4115/trainer.o \
	${OBJECTDIR}/_ext/511e4115/true_skill.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-O0 -fopenmp -std=c++17
CXXFLAGS=-O0 -fopenmp -std=c++17

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a -lboost_system -lboost_filesystem -lboost_program_options -fopenmp -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer: ../pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/trainer ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/511e4115/game_factory.o: ../src/game_factory.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/game_factory.o ../src/game_factory.cpp

${OBJECTDIR}/_ext/511e4115/league.o: ../src/league.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/league.o ../src/league.cpp

${OBJECTDIR}/_ext/511e4115/ranked.o: ../src/ranked.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked.o ../src/ranked.cpp

${OBJECTDIR}/_ext/511e4115/ranked_battlegroup.o: ../src/ranked_battlegroup.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_battlegroup.o ../src/ranked_battlegroup.cpp

${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o: ../src/ranked_evaluator.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_evaluator.o ../src/ranked_evaluator.cpp

${OBJECTDIR}/_ext/511e4115/ranked_planner.o: ../src/ranked_planner.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_planner.o ../src/ranked_planner.cpp

${OBJECTDIR}/_ext/511e4115/ranked_pokemon.o: ../src/ranked_pokemon.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_pokemon.o ../src/ranked_pokemon.cpp

${OBJECTDIR}/_ext/511e4115/ranked_team.o: ../src/ranked_team.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranked_team.o ../src/ranked_team.cpp

${OBJECTDIR}/_ext/511e4115/ranker.o: ../src/ranker.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranker.o ../src/ranker.cpp

${OBJECTDIR}/_ext/511e4115/ranker_main.o: ../src/ranker_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/ranker_main.o ../src/ranker_main.cpp

${OBJECTDIR}/_ext/511e4115/team_factory.o: ../src/team_factory.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/team_factory.o ../src/team_factory.cpp

${OBJECTDIR}/_ext/511e4115/trainer.o: ../src/trainer.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/trainer.o ../src/trainer.cpp

${OBJECTDIR}/_ext/511e4115/true_skill.o: ../src/true_skill.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -DDOUBLEPRECISION -DGEN4_STATIC -D_DEBUG -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/true_skill.o ../src/true_skill.cpp

# Subprojects
.build-subprojects:
	cd ../pkaiEngine && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
	cd ../pkaiEngine && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
