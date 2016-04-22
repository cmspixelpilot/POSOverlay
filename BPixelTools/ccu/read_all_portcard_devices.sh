#!/bin/bash

cat | (bin/ccu -utca connections.xml -fechardwareid board)  <<EOF
i2cr 10 40
i2cr 10 41
i2cr 10 42
i2cr 10 43
i2cr 10 44
i2cr 10 45
i2cr 10 30
i2cr 10 31
i2cr 10 32
i2cr 10 33
i2cr 10 34
i2cr 10 35
i2cr 10 36
i2cr 10 37
i2cr 10 38
i2cr 10 39
i2cr 10 3a
i2cr 10 3b
i2cr 10 20
i2cr 10 21
i2cr 10 22
i2cr 10 23
i2cr 10 70
i2cr 10 71
i2cr 10 72
i2cr 10 73
EOF


