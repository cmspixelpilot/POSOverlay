#Include xdaq make structure
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

include $(ENV_CMS_TK_ONLINE_ROOT)/FecHeader.linux
include $(ENV_CMS_TK_ONLINE_ROOT)/config/FecRules.linux

FECSOFTWARE_ROOT:=${ENV_CMS_TK_ONLINE_ROOT}

Project=pixel
Package=PixelRun


CC        = c++
CCFLAGS   = -g -Wall
INCFLAGS  = -I${BUILD_HOME}/pixel/PixelUtilities/PixelFEDDataTools/include
LDFLAGS   = -L ${BUILD_HOME}/pixel/PixelUtilities/PixelFEDDataTools/lib/linux/x86 -l PixelFEDDataTools

ROOTLibs =  -L$(shell root-confgi --libdir) \
         -lAfterImage -lASImageGui -lASImage \
         -lCore -lCint -lNew -lGed -lGeomPainter -lGeom -lGpad \
         -lGui -lGuiBld -lGX11 -lGX11TTF -lfreetype -lGraf -lGraf3d \
	 -lHist -lHistPainter \
         -lMatrix -lMinuit -lPostscript -lTree -lTreePlayer -lTreeViewer \
         -lRint -lRGL -lX3d -lEG


#all:
#	$(CC) $(CCFLAGS) -o SLink2Human $(INCFLAGS) $(LDFLAGS) $(ROOTLibs) $(SOURCES) SLink2Human.c

clean:
	rm -rf SLink2Human

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
