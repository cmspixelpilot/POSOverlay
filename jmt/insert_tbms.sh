#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo usage: $0 oldtbmkey newtbmkey
    exit 1
fi

oldkey=$1
newkey=$2

oldtbm=${PIXELCONFIGURATIONBASE}/tbm/${oldkey}
newtbm=${PIXELCONFIGURATIONBASE}/tbm/${newkey}

if [ ! -d "$oldtbm" ]; then
    echo $oldtbm does not exist
    exit 1
fi

if [ -d "$newtbm" ]; then
    echo $newtbm already exists
    exit 1
fi

matches=$(shopt -s nullglob ; echo TBM*dat)

if [ "$matches" == "" ]; then
    echo no matches for TBM*dat
    exit 1
fi

echo will copy matches:
echo $matches
echo into $newtbm
echo merging in old from
echo $oldtbm

mkdir $newtbm
cp $matches ${newtbm}/
cp -n ${oldtbm}/* ${newtbm}/
echo
echo the diffs:
diff -r $oldtbm $newtbm
echo
echo if happy, next run:
echo ${BUILD_HOME}/pixel/bin/PixelConfigDBCmd.exe --insertVersionAlias tbm $newkey Default


