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
	${OBJECTDIR}/src/main.o \
	${OBJECTDIR}/src/pkIO.o \
	${OBJECTDIR}/src/planner_directed.o \
	${OBJECTDIR}/src/planner_stochastic.o \
	${OBJECTDIR}/src/pokemonAI.o


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
LDLIBSOPTIONS=-L${CND_DISTDIR}/pkaiEngine/${CND_CONF}/${CND_PLATFORM}/ pkaiEngine/dist/Release/GNU-Linux/libpkaiengine.a -lboost_thread -lboost_system -lboost_filesystem -lpthread -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: pkaiEngine/dist/Release/GNU-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/pokemonai ${OBJECTFILES} ${LDLIBSOPTIONS} -Wl,--export-dynamic

${OBJECTDIR}/src/main.o: src/main.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/main.o src/main.cpp

${OBJECTDIR}/src/pkIO.o: src/pkIO.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pkIO.o src/pkIO.cpp

${OBJECTDIR}/src/planner_directed.o: src/planner_directed.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_directed.o src/planner_directed.cpp

${OBJECTDIR}/src/planner_stochastic.o: src/planner_stochastic.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/planner_stochastic.o src/planner_stochastic.cpp

${OBJECTDIR}/src/pokemonAI.o: src/pokemonAI.cpp
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -D_DISABLEFINEGRAINEDLOCKING -D_DISABLETEMPORALTRACE -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/pokemonAI.o src/pokemonAI.cpp

# Subprojects
.build-subprojects:
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release
	cd pkaiEngine && ${MAKE}  -f Makefile CONF=Release
	cd gen4_scripts && ${MAKE}  -f Makefile CONF=Release

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
