#!/bin/csh
source setup.csh
python ../tools/python/porttest.py 2002
if $? == 0 then 
bin/ccu -$INTERFACE -port 2002
endif
