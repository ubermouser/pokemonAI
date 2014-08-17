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
CND_PLATFORM=GNU-Linux-x86
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
	${OBJECTDIR}/_ext/1360937237/ability.o \
	${OBJECTDIR}/_ext/1360937237/pluggable.o \
	${OBJECTDIR}/_ext/1360937237/nature.o \
	${OBJECTDIR}/_ext/1360937237/move_volatile.o \
	${OBJECTDIR}/_ext/1360937237/pokemon_base.o \
	${OBJECTDIR}/_ext/1360937237/item.o \
	${OBJECTDIR}/_ext/1360937237/pokemon_volatile.o \
	${OBJECTDIR}/_ext/1360937237/environment_volatile.o \
	${OBJECTDIR}/_ext/379029546/fixed_func.o \
	${OBJECTDIR}/_ext/1360937237/init_toolbox.o \
	${OBJECTDIR}/_ext/1360937237/team_nonvolatile.o \
	${OBJECTDIR}/_ext/1360937237/type.o \
	${OBJECTDIR}/_ext/1360937237/move.o \
	${OBJECTDIR}/_ext/1360937237/signature.o \
	${OBJECTDIR}/_ext/1360937237/team_volatile.o \
	${OBJECTDIR}/_ext/1360937237/environment_nonvolatile.o \
	${OBJECTDIR}/_ext/1360937237/pokedex.o \
	${OBJECTDIR}/_ext/1360937237/move_nonvolatile.o \
	${OBJECTDIR}/_ext/1360937237/pkCU.o \
	${OBJECTDIR}/_ext/1360937237/pokemon_nonvolatile.o \
	${OBJECTDIR}/_ext/1360937237/name.o \
	${OBJECTDIR}/_ext/1360937237/environment_possible.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a
	${AR} -rv ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a ${OBJECTFILES} 
	$(RANLIB) ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a

${OBJECTDIR}/_ext/1360937237/ability.o: ../src/ability.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/ability.o ../src/ability.cpp

${OBJECTDIR}/_ext/1360937237/pluggable.o: ../src/pluggable.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pluggable.o ../src/pluggable.cpp

${OBJECTDIR}/_ext/1360937237/nature.o: ../src/nature.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/nature.o ../src/nature.cpp

${OBJECTDIR}/_ext/1360937237/move_volatile.o: ../src/move_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/move_volatile.o ../src/move_volatile.cpp

${OBJECTDIR}/_ext/1360937237/pokemon_base.o: ../src/pokemon_base.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pokemon_base.o ../src/pokemon_base.cpp

${OBJECTDIR}/_ext/1360937237/item.o: ../src/item.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/item.o ../src/item.cpp

${OBJECTDIR}/_ext/1360937237/pokemon_volatile.o: ../src/pokemon_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pokemon_volatile.o ../src/pokemon_volatile.cpp

${OBJECTDIR}/_ext/1360937237/environment_volatile.o: ../src/environment_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/environment_volatile.o ../src/environment_volatile.cpp

${OBJECTDIR}/_ext/379029546/fixed_func.o: ../src/fixedpoint/fixed_func.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/379029546
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/379029546/fixed_func.o ../src/fixedpoint/fixed_func.cpp

${OBJECTDIR}/_ext/1360937237/init_toolbox.o: ../src/init_toolbox.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/init_toolbox.o ../src/init_toolbox.cpp

${OBJECTDIR}/_ext/1360937237/team_nonvolatile.o: ../src/team_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/team_nonvolatile.o ../src/team_nonvolatile.cpp

${OBJECTDIR}/_ext/1360937237/type.o: ../src/type.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/type.o ../src/type.cpp

${OBJECTDIR}/_ext/1360937237/move.o: ../src/move.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/move.o ../src/move.cpp

${OBJECTDIR}/_ext/1360937237/signature.o: ../src/signature.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/signature.o ../src/signature.cpp

${OBJECTDIR}/_ext/1360937237/team_volatile.o: ../src/team_volatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/team_volatile.o ../src/team_volatile.cpp

${OBJECTDIR}/_ext/1360937237/environment_nonvolatile.o: ../src/environment_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/environment_nonvolatile.o ../src/environment_nonvolatile.cpp

${OBJECTDIR}/_ext/1360937237/pokedex.o: ../src/pokedex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pokedex.o ../src/pokedex.cpp

${OBJECTDIR}/_ext/1360937237/move_nonvolatile.o: ../src/move_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/move_nonvolatile.o ../src/move_nonvolatile.cpp

${OBJECTDIR}/_ext/1360937237/pkCU.o: ../src/pkCU.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pkCU.o ../src/pkCU.cpp

${OBJECTDIR}/_ext/1360937237/pokemon_nonvolatile.o: ../src/pokemon_nonvolatile.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/pokemon_nonvolatile.o ../src/pokemon_nonvolatile.cpp

${OBJECTDIR}/_ext/1360937237/name.o: ../src/name.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/name.o ../src/name.cpp

${OBJECTDIR}/_ext/1360937237/environment_possible.o: ../src/environment_possible.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/1360937237
	${RM} $@.d
	$(COMPILE.cc) -O3 -Wall -DDOUBLEPRECISION -DNDEBUG -DPKAI_EXPORT -D_DISABLEFINEGRAINEDLOCKING -D_HTCOLLECTSTATISTICS -D_LINUX -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/1360937237/environment_possible.o ../src/environment_possible.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libpkaiengine.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
