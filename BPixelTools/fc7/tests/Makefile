
# Project = fc7_software
# Package = fc7/tests

Packager = Mark Pesaresi, Paschalis Vichoudis

PACKAGE_VER_MAJOR = 1
PACKAGE_VER_MINOR = 0
PACKAGE_VER_PATCH = 0
PACKAGE_RELEASE = 0

LIBRARY = lib/libcactus_fc7_tests.so
LIBRARY_SOURCES = $(wildcard src/common/*.cpp)
LIBRARY_OBJECT_FILES = $(patsubst src/common/%.cpp,obj/%.o,${LIBRARY_SOURCES})

EXECUTABLE_SOURCES = $(wildcard src/common/*.cxx)
EXECUTABLE_OBJECT_FILES = $(patsubst src/common/%.cxx,obj/%.o,${EXECUTABLE_SOURCES})
EXECUTABLES = $(patsubst src/common/%.cxx,bin/%.exe,${EXECUTABLE_SOURCES})

# When compiling standalone
FC7_FC7_PREFIX=../fc7/
FC7_FC7_INCLUDE_PREFIX = ${FC7_FC7_PREFIX}include
FC7_FC7_LIB_PREFIX = ${FC7_FC7_PREFIX}lib

LIBRARY_PATH = 	\
		-Llib \
		-L${FC7_FC7_LIB_PREFIX} \
		-L${XDAQ_ROOT}/lib \
		-L${CACTUS_ROOT}/lib

INCLUDE_PATH = 	-Iinclude  \
		-I${FC7_FC7_INCLUDE_PREFIX} \
		-I${CACTUS_ROOT}/include

LIBRARIES = 	\
		-lpthread \
		\
		-lboost_thread \
		-lboost_filesystem \
		-lboost_regex \
		-lboost_system \
		-lboost_thread \
		-lboost_program_options \
		\
		-lcactus_extern_pugixml \
		-lcactus_uhal_log \
		-lcactus_uhal_grammars \
		-lcactus_uhal_uhal 

EXECUTABLE_LIBRARIES = ${LIBRARIES} -lcactus_fc7_fc7 -lcactus_fc7_tests



CPP_FLAGS = -g -Wall -O0 -rdynamic -MMD -MP -fPIC ${INCLUDE_PATH} -DBIG_ENDIAN_HACK

LINK_LIBRARY_FLAGS = -shared -fPIC -Wall -g -O0 -rdynamic ${LIBRARY_PATH} ${LIBRARIES}

LINK_EXECUTABLE_FLAGS = -Wall -g -O0 -rdynamic ${LIBRARY_PATH} ${EXECUTABLE_LIBRARIES} 

RPMBUILD_DIR = ${PackagePath}/RPMBUILD

.PHONY: all _all clean _cleanall build _buildall install _installall rpm _rpmall test _testall spec_update

default: build

clean: _cleanall
_cleanall:
	rm -rf obj
	rm -rf bin
	rm -rf lib/*

all: _all
build: _all
buildall: _all
_all: ${LIBRARY} ${EXECUTABLES}


${EXECUTABLES}: bin/%.exe: obj/%.o ${EXECUTABLE_OBJECT_FILES}
	g++ ${LINK_EXECUTABLE_FLAGS} $< -o $@

${EXECUTABLE_OBJECT_FILES}: obj/%.o : src/common/%.cxx
	mkdir -p {bin,obj,lib}
	g++ -c ${CPP_FLAGS}  $< -o $@

-include $(EXECUTABLE_OBJECT_FILES:.o=.d)

${LIBRARY}: ${LIBRARY_OBJECT_FILES}
	g++ ${LINK_LIBRARY_FLAGS} ${LIBRARY_OBJECT_FILES} -o $@

${LIBRARY_OBJECT_FILES}: obj/%.o : src/common/%.cpp 
	mkdir -p {bin,obj,lib}
	g++ -c ${CPP_FLAGS} $< -o $@

-include $(LIBRARY_OBJECT_FILES:.o=.d)

