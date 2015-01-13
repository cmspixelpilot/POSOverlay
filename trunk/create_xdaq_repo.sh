#/bin/bash

#
# create xdaq.repo file according to machine architecture and SLC version
#

XDAQVERSION=11
SLCVERSION=slc`cat /etc/redhat-release | awk 'match($0,"release"){print substr($0,RSTART+8,1)}'`x
ARCHITECTURE=`uname -m`

while getopts x:s:a: option
do
        case "${option}"
        in
                a) ARCHITECTURE=${OPTARG};;
                s) SLCVERSION=${OPTARG};;
                v) XDAQVERSION=${OPTARG};;
        esac
done

echo "Using XDAQ version $XDAQVERSION - use option '-x XDAQVERSION' to change"
echo "Using SLC version $SLCVERSION - use option '-s SLCVERSION' to change"
echo "Using architecture $ARCHITECTURE - use option '-a ARCHITECTURE' to change"
echo

cat > xdaq.repo << EOF
[xdaq]
name=XDAQ Software Base
baseurl=http://xdaq.web.cern.ch/xdaq/repo/$XDAQVERSION/$SLCVERSION/$ARCHITECTURE/base/RPMS/
enabled=1
gpgcheck=0
[xdaq-sources]
name=XDAQ Software Base Sources
baseurl=http://xdaq.web.cern.ch/xdaq/repo/$XDAQVERSION/$SLCVERSION/$ARCHITECTURE/base/SRPMS/
enabled=0
gpgcheck=0
[xdaq-updates]
name=XDAQ Software Updates
baseurl=http://xdaq.web.cern.ch/xdaq/repo/$XDAQVERSION/$SLCVERSION/$ARCHITECTURE/updates/RPMS/
enabled=1
gpgcheck=0
[xdaq-updates-sources]
name=XDAQ Software Updates Sources
baseurl=http://xdaq.web.cern.ch/xdaq/repo/$XDAQVERSION/$SLCVERSION/$ARCHITECTURE/updates/SRPMS/
enabled=1
gpgcheck=0
[xdaq-extras]
name=XDAQ Software Extras
baseurl=http://xdaq.web.cern.ch/xdaq/repo/$XDAQVERSION/$SLCVERSION/$ARCHITECTURE/extras/RPMS/
enabled=1
gpgcheck=0
EOF

echo "--------- Content of file ---------"
cat xdaq.repo
echo "----------- End of file -----------"

echo
echo "xdaq.repo file created. Now do as root 'cp xdaq.repo /etc/yum.repos.d/xdaq.repo'"
