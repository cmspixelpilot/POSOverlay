#!/bin/csh
source setup.csh
#setenv caensem 0x3005c105
setenv caensem 0x3005129f

set lock=`ipcs -s | grep $caensem | wc -l`
if($lock == 1) then
    ipcrm -S $caensem
endif

bin/caen -port 2001


