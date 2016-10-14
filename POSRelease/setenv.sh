# site specifics in local.sh, not in git
# it should export
#   ROOTSYS, where root is installed,
#   BUILD_HOME, this TriDAS dir,
#   POS_OUTPUT_DIRS, where the run outputs will go,
#   POS_LOG_DIR, where the log files will be kept,
#   PIXELCONFIGURATIONBASE, where the config files (aliases,configurations.txt, etc.) are
source local.sh

export CACTUS_ROOT=/opt/cactus
export XDAQ_ROOT=/opt/xdaq
export XDAQ_BASE=/opt/xdaq
export XDAQ_OS=linux
export XDAQ_PLATFORM=x86_64_slc6
export XDAQ_DOCUMENT_ROOT=$XDAQ_ROOT/htdocs
export XDAQ_SETUP_ROOT=$XDAQ_ROOT/share
export ENV_CMS_TK_DIAG_ROOT=$BUILD_HOME/DiagSystem
export ENV_CMS_TK_ONLINE_ROOT=$BUILD_HOME/FecSoftwareV3_0
export ENV_CMS_TK_FEC_ROOT=$BUILD_HOME/FecSoftwareV3_0
export ENV_CMS_TK_HAL_ROOT=$XDAQ_ROOT
export ENV_CMS_TK_CAEN_ROOT=$XDAQ_ROOT
export XERCESCROOT=$XDAQ_ROOT
export POS_LIB_DIRS=:$BUILD_HOME/pixel/lib

if [ -z "$LD_LIBRARY_PATH" ]; then
    export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib
else
    export LD_LIBRARY_PATH=${XDAQ_ROOT}/lib:${LD_LIBRARY_PATH}
fi
export LD_LIBRARY_PATH=${ROOTSYS}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${CACTUS_ROOT}/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${BUILD_HOME}/FecSoftwareV3_0/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${ENV_CMS_TK_DIAG_ROOT}/tools/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${BUILD_HOME}/pixel/lib:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/usr/lib64:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=/usr/lib64/root:${LD_LIBRARY_PATH}

cd $ROOTSYS
source bin/thisroot.sh
cd - > /dev/null
