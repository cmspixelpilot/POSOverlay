#/bin/bash

#
# create setenv.sh file according to machine architecture and SLC version
#

OUTFILE=setenv.sh
XDAQLOCATION=/opt/xdaq
TRIDASDIR=$(dirname `dirname $PWD`)

while getopts a:o:r:s:t:x option
do
        case "${option}"
        in
                a) ARCHITECTURE=${OPTARG};;
                o) OSTYP=${OPTARG};;
                r) ROOTLOCATION=${OPTARG};;                
                s) SLCVERSION=${OPTARG};;
                t) TRIDASDIR=${OPTARG};;
                x) XDAQLOCATION=${OPTARG};;
        esac
done

if [ -z "$ARCHITECTURE" ]; then
  ARCHITECTURE=`uname -m`
  if [[ $? -ne 0 ]]; then
    echo "Error determining architecture - use option '-a ARCHITECTURE' to override"
    exit 1
  fi
fi

if [ -z "$OSTYP" ]; then
  OSTYP=`uname | awk '{print tolower($0)}'`
  if [[ $? -ne 0 ]]; then
    echo "Error determining OS type - use option '-o OSTYP' to override"
    exit 1
  fi
fi

if [ -z "$ROOTLOCATION" ]; then
  echo "Please provide location of ROOTSYS via -r ROOTSYS"
  exit 1
else
echo "Using ROOTSYS $ROOTLOCATION - use option '-r ROOTSYS' to change"
fi

if [ -z "$SLCVERSION" ]; then
  SLCVERSION=slc`cat /etc/redhat-release | awk 'match($0,"release"){print substr($0,RSTART+8,1)}'`
  if [ ! -f /etc/redhat-release ]; then
    echo "Error determining SLC version - use option '-s SLCVERSION' to override"
    exit 1
  fi
fi




echo "Using SLC version $SLCVERSION - use option '-s SLCVERSION' to change"
echo "Using architecture $ARCHITECTURE - use option '-a ARCHITECTURE' to change"
echo "Using OS type $OSTYP - use option '-o OSTYP' to change"
echo "Using XDAQ location $XDAQLOCATION - use option '-x XDAQLOCATION' to change"
echo "Using as TriDAS directory $TRIDASDIR - use option '-t TRIDASDIR' to change"
echo



cat > $OUTFILE << EOF
# TriDAS setenv file
export XDAQ_ROOT=${XDAQLOCATION}
export XDAQ_BASE=${XDAQLOCATION}
export XDAQ_OS=${OSTYP}
export XDAQ_PLATFORM=${ARCHITECTURE}_${SLCVERSION}
export ROOTSYS=${ROOTLOCATION}
export BUILD_HOME=${TRIDASDIR}

EOF

cat >> $OUTFILE << "EOF"
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
#export PIXELCONFIGURATIONBASE=$BUILD_HOME/pixel/PixelConfigDataExamples/
export PIXELCONFIGURATIONBASE=/pixelscratch/pixelscratch/config/Pix
export PATH=${HOME}/bin:/sbin:${ROOTSYS}/bin:${PATH}:${BUILD_HOME}/pixel/bin

cd $ROOTSYS
source bin/thisroot.sh
cd -
EOF

echo "--------- Content of file ---------"
cat $OUTFILE
echo "----------- End of file -----------"

echo
echo "$OUTFILE file created. Now do 'source setenv.sh'"
