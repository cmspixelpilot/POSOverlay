#====================================================================================================
#
# File: Makefile
# Authors: D. Menasce, L. Uplegger
# 
# This is the Makefile that builds all the tools pertaining to the PSI chip 
# test stand project
#
#====================================================================================================
PROJECT_NAME = Sibilla
OBJDIR = obj
INCDIR = inc
SRCDIR = src
LIBDIR = lib

ifdef DEBUG
	OPTIMIZER_FLAGS = -g -Wall -fbounds-check -fPIC
else
	OPTIMIZER_FLAGS = -Wall  
endif

CC = g++

INCFLAGS = -I$(INCDIR)                       \
	   -I$(QTDIR)/include                \
	   -DLINUX                           \
	   `root-config --cflags`         

LIBFLAGS = -Wl,-rpath,$(PWD)/$(LIBDIR) -L$(LIBDIR) -l$(PROJECT_NAME)      \
	   -Wl,-rpath,$(QTDIR)/lib -L$(QTDIR)/lib -lqt-mt                 \
	   -Wl,-rpath,$(shell root-config --libdir) -L$(shell root-config --libdir) `root-config --libs`         

CCFLAGS  = $(INCFLAGS) $(OPTIMIZER_FLAGS)

SRCLIST = $(wildcard src/*.cpp)
OBJLIST = $(patsubst src/%.cpp,obj/%.o,$(SRCLIST))

DYNLIBSMANAGER  = $(LIBDIR)/lib$(PROJECT_NAME).so

HERE   := $(shell pwd)

# Warning: order in the FORCED tag below is important!!!!!!
FORCED  = dynlib baseTempCorr 

.PHONY : all dep CHECK $(FORCED) EOJ clean cleanall dynlib cleancore

#---------------------------------------#
all : CHECK dep $(FORCED) cleancore EOJ     # Main sequence ----------------------#
#---------------------------------------#

#*********************************************************************************************************
#--------------------------------------------------------------------------------------------------------#
CHECK : 
	@echo '[1;33m[1m>========== [7m' 'Building xml components in $(HERE) [0m' '[1;33m[1m================[0m'
	@echo ' '
	
#*********************************************************************************************************

#--------------------------------------------------------------------------------------------------------#
dep :   
	@echo '            [1;32m[1m>>********** Making dependencies ***********<<[0m'
	@if [ ! -e dependencies ] ; then touch dependencies ;fi
        ifdef CPPVERBOSE
	  g++ -MM $(SRCDIR)/*.cpp $(CCFLAGS) | sed 's/.*\.o:/$(OBJDIR)\/&/' >  dependencies
        else
	 @g++ -MM $(SRCDIR)/*.cpp $(CCFLAGS) | sed 's/.*\.o:/$(OBJDIR)\/&/' >  dependencies
        endif
	@echo ' '
	@echo '            [1;32m[1m>>********** Dependencies done ***********<<[0m'
	@echo ' '
	@echo '            [1;32m[1m>>********** Compiling components ***********<<[0m'
	@echo ' '
#--------------------------------------------------------------------------------------------------------#

#--------------------------------------------------------------------------------------------------------#
dynlib :  $(DYNLIBSMANAGER)

#--------------------------------------------------------------------------------------------------------#
obj/%.o : src/%.cpp
	@echo '            [0;31m[1m[7mCompiling[0m [0;36m[1m[7m$< [0m'
        ifdef CPPVERBOSE
	  $(CC) $(CCFLAGS) -c $< -o $@
        else
	 @$(CC) $(CCFLAGS) -c $< -o $@
        endif

#--------------------------------------------------------------------------------------------------------#
$(DYNLIBSMANAGER) : $(OBJLIST) 
	@echo ' '
	@echo '[1;32m[1m----------- Making xml dynamic libraries ------------------------------------------[0m'
        ifdef CPPVERBOSE
	  $(CC) -shared -o $(DYNLIBSMANAGER) $(OBJLIST) 
        else
	 @$(CC) -shared -o $(DYNLIBSMANAGER) $(OBJLIST) 
        endif
	@echo ' '
	@echo '            [0;35m[1m[7mxml main sequence built[0m';
	@echo ' '

#--------------------------------------------------------------------------------------------------------#
baseTempCorr : baseTempCorr.cpp 
	      @echo ' '
	      @echo '[1;32m[1m----------- Making $(PROJECT_NAME) baseTempCorr ------------------------------------------[0m'
              ifdef CPPVERBOSE
	  	$(CC) -o $@ $< $(CCFLAGS) $(LIBFLAGS)
              else
	  	@$(CC) -o $@ $< $(CCFLAGS) $(LIBFLAGS)
              endif	
		@echo ' '
		@echo '            [0;35m[1m[7mxml main sequence built[0m';
		@echo ' '

#--------------------------------------------------------------------------------------------------------#
clean : cleanall
	@echo ' '
	@echo '[1;32m[1m*************************************************************************************[0m'
	@echo '[1;32m[1m----------- xml main sequence clean up --------------------------------------------------[0m'
        ifdef CPPVERBOSE
	  rm -rf $(OBJDIR)/*.o *~
        else
	 @rm -rf $(OBJDIR)/*.o *~
        endif
	@echo ' '
	@echo ' '


#--------------------------------------------------------------------------------------------------------#
cleanall : 
	@echo ' '
	@echo '[1;32m[1m*************************************************************************************[0m'
	@echo '[1;32m[1m----------- xml complete clean up -------------------------------------------------------[0m'
        ifdef CPPVERBOSE
	  -rm -rf $(OBJDIR)/*.o
	  -rm -rf $(DYNLIBSMANAGER) $(LIBFORMATTER)
        else
	 @-rm -rf $(OBJDIR)/*.o
	 @-rm -rf $(DYNLIBSMANAGER) $(LIBFORMATTER)
        endif
	
	@echo ' '
	@echo ' '

#--------------------------------------------------------------------------------------------------------#
cleancore :
	@rm -rf core*
#--------------------------------------------------------------------------------------------------------#
#--------------------------------------------------------------------------------------------------------#
#--------------------------------------------------------------------------------------------------------#
#--------------------------------------------------------------------------------------------------------#
EOJ : 
	@echo ' '
	@echo '            [0;35m[1m[7mxml done!![0m';
	@echo ' '

#--------------------------------------------------------------------------------------------------------#
-include dependencies

help:
	@clear
	@echo ' '
	@echo '[1;31m[1mUsage: [0m'
	@echo ' '
	@echo "       [1;32m[1mmake cleanall[0m 	(recursively cleans the whole project)"
	@echo "       [1;32m[1mmake clean[0m 	(cleans only the main sequence of the project)"
	@echo "       [1;32m[1mmake[0m 		(rebuilds the whole project's components)"
	@echo ' '
	@echo 'To enable  debug traceback of make execution: [7msetenv CPPVERBOSE yes[0m'
	@echo 'To disable debug traceback of make execution: [7munsetenv CPPVERBOSE  [0m'
	@echo ' '

