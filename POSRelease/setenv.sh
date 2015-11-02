# TriDAS setenv file
export XDAQ_ROOT=/opt/xdaq
export XDAQ_BASE=/opt/xdaq
export XDAQ_OS=linux
export XDAQ_PLATFORM=x86_64_slc6
export ROOTSYS=~/root
export BUILD_HOME=~/build/TriDAS

# the part below should usually not be touched
export XDAQ_DOCUMENT_ROOT=$XDAQ_ROOT/htdocs
export XDAQ_SETUP_ROOT=$XDAQ_ROOT/share
export ENV_CMS_TK_DIAG_ROOT=$BUILD_HOME/DiagSystem
export ENV_CMS_TK_ONLINE_ROOT=$BUILD_HOME/FecSoftwareV3_0
export ENV_CMS_TK_FEC_ROOT=$BUILD_HOME/FecSoftwareV3_0
export ENV_CMS_TK_HAL_ROOT=$XDAQ_ROOT
export ENV_CMS_TK_CAEN_ROOT=$XDAQ_ROOT
export XERCESCROOT=$XDAQ_ROOT
export POS_LIB_DIRS=:$BUILD_HOME/pixel/lib
export POS_OUTPUT_DIRS=$BUILD_HOME/pixel/PixelRun/Runs

export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib/:${ROOTSYS}/lib
export LD_LIBRARY_PATH=${BUILD_HOME}/FecSoftwareV3_0/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${ENV_CMS_TK_DIAG_ROOT}/tools/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${BUILD_HOME}/pixel/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/usr/lib64:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/usr/lib64/root:${LD_LIBRARY_PATH}
export PIXELCONFIGURATIONBASE=$BUILD_HOME/pixel/PixelConfigDataExamples/

cd $ROOTSYS
source bin/thisroot.sh
cd -
