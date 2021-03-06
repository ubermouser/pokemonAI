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
	${OBJECTDIR}/_ext/511e4115/gen4_scripts.o


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
LDLIBSOPTIONS=-L../dist/Release/GNU-Linux-x86/

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../plugins/libgen4_scripts.so

../plugins/libgen4_scripts.so: ${OBJECTFILES}
	${MKDIR} -p ../plugins
	${LINK.cc} -o ../plugins/libgen4_scripts.so ${OBJECTFILES} ${LDLIBSOPTIONS} -shared -fPIC

${OBJECTDIR}/_ext/511e4115/gen4_scripts.o: ../src/gen4_scripts.cpp
	${MKDIR} -p ${OBJECTDIR}/_ext/511e4115
	${RM} "$@.d"
	$(COMPILE.cc) -O3 -DDOUBLEPRECISION -DGEN4_SCRIPTS_EXPORTS -DNDEBUG -DTOLUA_RELEASE -D_LINUX -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/511e4115/gen4_scripts.o ../src/gen4_scripts.cpp

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
