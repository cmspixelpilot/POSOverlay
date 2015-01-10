
include $(XDAQ_ROOT)/config/mfAutoconf.rules
include $(XDAQ_ROOT)/config/mfDefs.$(XDAQ_OS)

Project=pixel
Package=PixelUtilities/PixelFEDDataTools
PackageName=PixelFEDDataTools

Sources=$(wildcard src/common/*.cc)

DynamicLibrary=PixelFEDDataTools 
StaticLibrary=

IncludeDirs = \
	$(BUILD_HOME)/$(Project) \
	$(ENV_CMS_TK_DIAG_ROOT) \
  $(shell root-config --incdir)

#$(ROOTSYS)/include 
 


LibraryDirs = 


DependentLibraryDirs = $(BUILD_HOME)/pixel/CalibFormats/SiPixelObjects/lib/$(XDAQ_OS)/$(XDAQ_PLATFORM)
DependentLibraries = SiPixelObjects


ROOTLibs = -L$(shell root-config --libdir) -lCore -lCint -lHist -lGraf -lGraf3d -lGpad -lTree -lRint -lPostscript -lMatrix -lPhysics -lGui -pthread -lm -ldl -rdynamic -lX3d

#UserDynamicLinkFlags = $(ROOTLibs)
UserDynamicLinkFlags =



#ROOTLibs =  -L$(ROOTSYS)/lib/ \
#	 -lAfterImage -lASImageGui -lASImage \
#	 -lCore -lCint -lNew -lGed -lGeomPainter -lGeom -lGpad \
#	 -lGui -lGuiBld -lGX11 -lGX11TTF -lfreetype -lGraf -lGraf3d -lHist -lHistPainter \
#	 -lMatrix -lMinuit -lPostscript -lTree -lTreePlayer -lTreeViewer \
#	 -lRint -lRGL -lX3d -lEG

include $(XDAQ_ROOT)/config/Makefile.rules
include $(BUILD_HOME)/$(Project)/RPM.version
include $(XDAQ_ROOT)/config/mfRPM.rules
