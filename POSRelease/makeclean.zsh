#!/usr/bin/env zsh
#
cd ..
for dir in $(ls | grep Pixel | grep -v '.cc')
    do
        cd $dir
        [[ -a Makefile ]] && make clean
        cd ..
    done
cd PixelUtilities
for dir in $(ls | grep Pixel | grep -v '.cc')
    do
        cd $dir
        [[ -a Makefile ]] && make clean
        cd ..
    done
