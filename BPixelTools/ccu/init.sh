#!/bin/bash

channel=11

cat | (bin/ccu -utca connections.xml -fechardwareid board)  <<EOF
scanccu
ccu 0x7b
i2cr $channel 40
piaChannel 31
pia get ddr
pia get data
piaChannel 32
pia get ddr
pia get data
piaChannel 33
pia get ddr
pia get data
ccu 0x7c
i2cr $channel 40
piaChannel 31
pia get ddr
pia get data
piaChannel 32
pia get ddr
pia get data
piaChannel 33
pia get ddr
pia get data
ccu 0x7e
i2cr $channel 40
piaChannel 31
pia get ddr
pia get data
piaChannel 32
pia get ddr
pia get data
piaChannel 33
pia get ddr
pia get data
ccu 0x7d
i2cr $channel 40
piaChannel 31
pia get ddr
pia get data
piaChannel 32
pia get ddr
pia get data
piaChannel 33
pia get ddr
pia get data
EOF
