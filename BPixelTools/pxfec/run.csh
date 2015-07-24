#!/bin/csh
source setup.csh
echo $LD_LIBRARY_PATH
python ../tools/python/porttest.py 2000
if $? == 0 then
 bin/pxfec -port 2000 -$INTERFACE -init $INITFILE
endif

