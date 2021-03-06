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
	${OBJECTDIR}/_ext/511e4115/battler_main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-O0 -std=c++17
CXXFLAGS=-O0 -std=c++17

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a -lboost_system -lboost_filesystem -lboost_program_options -fopenmp -ldl

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/battler

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/battler: ../pkaiEngine/dist/Debug/CLANG-Linux/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/battler: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/battler ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/511e4115/battler_main.o: ../src/battler_main.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -g -Wall -DDOUBLEPRECISION -DGEN4_STATIC -DPKAI_EXPORT -D_DEBUG -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -I.. -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/battler_main.o ../src/battler_main.cpp

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
