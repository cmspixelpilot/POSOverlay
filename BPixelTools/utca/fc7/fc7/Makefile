
# Project = fc7_software
# Package = fc7/fc7

Packager = Mark Pesaresi, Paschalis Vichoudis

PACKAGE_VER_MAJOR = 1
PACKAGE_VER_MINOR = 0
PACKAGE_VER_PATCH = 0
PACKAGE_RELEASE = 0

ifndef BUILD_VERSION
BUILD_VERSION=1
endif

LIBRARY = lib/libcactus_fc7_fc7.so

SOURCES = \
	Firmware.cpp \
	MmcPipeInterface.cpp

SOURCE_FILES = $(addprefix src/common/,${SOURCES})
OBJECT_FILES = $(patsubst src/common/%.cpp,obj/%.o,${SOURCE_FILES})


INCLUDE_PATH = 	-Iinclude  \
		-I$(CACTUS_ROOT)/include 

LIBRARY_PATH = \
	-L$(CACTUS_ROOT)/lib 

LIBRARIES = \
	-lboost_system \
	-lboost_date_time \
	-lcactus_uhal_grammars \
	-lcactus_uhal_log \
	-lcactus_uhal_uhal	


CPP_FLAGS = -g -O0 -rdynamic -Wall -MMD -MP -fPIC ${INCLUDE_PATH} 

LINK_FLAGS = -g -shared -fPIC -Wall -O0 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

RPMBUILD_DIR = ${PackagePath}/RPMBUILD

.PHONY: all _all clean _cleanall build _buildall install _installall rpm _rpmall test _testall

default: build
clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf lib/*

all: _all
build: _all
buildall: _all
_all: ${LIBRARY}


${LIBRARY}: ${OBJECT_FILES}
	g++ ${LINK_FLAGS} ${OBJECT_FILES} -o $@

${OBJECT_FILES}: obj/%.o : src/common/%.cpp 
	mkdir -p {lib,obj}
	g++ ${CPP_FLAGS} -c $< -o $@

-include $(OBJECT_FILES:.o=.d)

