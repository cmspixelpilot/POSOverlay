#!/bin/sh
PACKAGE_VER_MAJOR=`awk 'BEGIN{IGNORECASE=1} /PACKAGE_VER_MAJOR/ {print $2;}' $BUILD_HOME/pixel/VERSION`
PACKAGE_VER_MINOR=`awk 'BEGIN{IGNORECASE=1} /PACKAGE_VER_MINOR/ {print $2;}' $BUILD_HOME/pixel/VERSION`
PACKAGE_VER_PATCH=`awk 'BEGIN{IGNORECASE=1} /PACKAGE_VER_PATCH/ {print $2;}' $BUILD_HOME/pixel/VERSION`
PACKAGE_RELEASE=`awk 'BEGIN{IGNORECASE=1} /PACKAGE_RELEASE/ {print $2;}' $BUILD_HOME/pixel/VERSION`
echo PACKAGE_VER_MAJOR: $PACKAGE_VER_MAJOR
echo PACKAGE_VER_MINOR: $PACKAGE_VER_MINOR
echo PACKAGE_VER_PATCH: $PACKAGE_VER_PATCH
echo PACKAGE_RELEASE  : $PACKAGE_RELEASE

rm -rf $BUILD_HOME/DiagSystem/rpm
mkdir $BUILD_HOME/DiagSystem/rpm
mkdir $BUILD_HOME/DiagSystem/rpm/RPMBUILD
mkdir $BUILD_HOME/DiagSystem/rpm/RPMBUILD/BUILD
mkdir $BUILD_HOME/DiagSystem/rpm/RPMBUILD/RPMS
mkdir $BUILD_HOME/DiagSystem/rpm/RPMBUILD/RPMS/i386
cp $BUILD_HOME/pixel/PixelUtilities/DiagSystem.spec $BUILD_HOME/DiagSystem/rpm/.
perl -p -i -e 's#__version__#'$PACKAGE_VER_MAJOR'.'$PACKAGE_VER_MINOR'.'$PACKAGE_VER_PATCH'#' $BUILD_HOME/DiagSystem/rpm/DiagSystem.spec
perl -p -i -e 's#__build__#'$PACKAGE_RELEASE'.1.slc4#' $BUILD_HOME/DiagSystem/rpm/DiagSystem.spec
cd $BUILD_HOME/DiagSystem/rpm
rpmbuild  -bb --define  "_topdir $BUILD_HOME/DiagSystem/rpm/RPMBUILD"  DiagSystem.spec

rm -rf $BUILD_HOME/FecSoftwareV3_0/rpm
mkdir $BUILD_HOME/FecSoftwareV3_0/rpm
mkdir $BUILD_HOME/FecSoftwareV3_0/rpm/RPMBUILD
mkdir $BUILD_HOME/FecSoftwareV3_0/rpm/RPMBUILD/BUILD
mkdir $BUILD_HOME/FecSoftwareV3_0/rpm/RPMBUILD/RPMS
mkdir $BUILD_HOME/FecSoftwareV3_0/rpm/RPMBUILD/RPMS/i386
cp $BUILD_HOME/pixel/PixelUtilities/FecSoftwareV3_0.spec $BUILD_HOME/FecSoftwareV3_0/rpm/.
perl -p -i -e 's#__version__#'$PACKAGE_VER_MAJOR'.'$PACKAGE_VER_MINOR'.'$PACKAGE_VER_PATCH'#' $BUILD_HOME/FecSoftwareV3_0/rpm/FecSoftwareV3_0.spec
perl -p -i -e 's#__build__#'$PACKAGE_RELEASE'.1.slc4#' $BUILD_HOME/FecSoftwareV3_0/rpm/FecSoftwareV3_0.spec
cd $BUILD_HOME/FecSoftwareV3_0/rpm
rpmbuild  -bb --define  "_topdir $BUILD_HOME/FecSoftwareV3_0/rpm/RPMBUILD"  FecSoftwareV3_0.spec

rm -rf $BUILD_HOME/TTCSoftware/rpm
mkdir $BUILD_HOME/TTCSoftware/rpm
mkdir $BUILD_HOME/TTCSoftware/rpm/RPMBUILD
mkdir $BUILD_HOME/TTCSoftware/rpm/RPMBUILD/BUILD
mkdir $BUILD_HOME/TTCSoftware/rpm/RPMBUILD/RPMS
mkdir $BUILD_HOME/TTCSoftware/rpm/RPMBUILD/RPMS/i386
cp $BUILD_HOME/pixel/PixelUtilities/TTCSoftware.spec $BUILD_HOME/TTCSoftware/rpm/.
perl -p -i -e 's#__version__#'$PACKAGE_VER_MAJOR'.'$PACKAGE_VER_MINOR'.'$PACKAGE_VER_PATCH'#' $BUILD_HOME/TTCSoftware/rpm/TTCSoftware.spec
perl -p -i -e 's#__build__#'$PACKAGE_RELEASE'.1.slc4#' $BUILD_HOME/TTCSoftware/rpm/TTCSoftware.spec
cd $BUILD_HOME/TTCSoftware/rpm
rpmbuild  -bb --define  "_topdir $BUILD_HOME/TTCSoftware/rpm/RPMBUILD"  TTCSoftware.spec

rm -rf $BUILD_HOME/RPM_$PACKAGE_VER_MAJOR.$PACKAGE_VER_MINOR.$PACKAGE_VER_PATCH-$PACKAGE_RELEASE
mkdir $BUILD_HOME/RPM_$PACKAGE_VER_MAJOR.$PACKAGE_VER_MINOR.$PACKAGE_VER_PATCH-$PACKAGE_RELEASE
cp `find $BUILD_HOME/. | grep $PACKAGE_VER_MAJOR.$PACKAGE_VER_MINOR.$PACKAGE_VER_PATCH | grep -v dev | grep -v src | grep -v tgz | grep -v RPM_` $BUILD_HOME/RPM_$PACKAGE_VER_MAJOR.$PACKAGE_VER_MINOR.$PACKAGE_VER_PATCH-$PACKAGE_RELEASE/.
