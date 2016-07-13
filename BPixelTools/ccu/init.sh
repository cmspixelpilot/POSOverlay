#!/bin/bash

cat | (bin/ccu -utca connections.xml -fechardwareid board)  <<EOF
scanccu
i2cr 10 40
pia get data
EOF
