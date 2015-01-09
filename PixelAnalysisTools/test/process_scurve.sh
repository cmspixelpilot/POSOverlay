#!/bin/bash

./bin/linux/x86/PixelAnalysis.exe SCurve $1 | grep RocName | awk '{ print $2,$3,$4,$5,$6 }' > $POS_OUTPUT_DIRS/Run_$1/$2

