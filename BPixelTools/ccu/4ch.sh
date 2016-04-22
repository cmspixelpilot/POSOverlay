#!/bin/bash

cat | (bin/ccu -utca connections.xml -fechardwareid board)  <<EOF
ccu 7b
i2c 10 30 40
i2c 10 31 40
i2c 10 32 40
i2c 10 33 40
i2c 10 34 40
i2c 10 35 f0
EOF

