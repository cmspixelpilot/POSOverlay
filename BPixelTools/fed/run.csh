#!/bin/csh
source setup.csh
echo "running setupPixelFED"
#bin/setupPixelFED 0x11000000 data/params_fed_0x11000000.dat
bin/setupPixelFED 0x10000000 data/params_fed_0x10000000.dat
echo "starting fed server"
#bin/fed -vmecaenusb -port 2003
bin/fed -$INTERFACE -port 2003
