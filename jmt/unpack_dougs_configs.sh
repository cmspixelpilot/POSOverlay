#!/bin/sh

if [ "$1" == "" ] || [ ! -d "$1" ]; then
    echo usage: $0 dir_with_cfgs
    exit 1
fi

if [ -d dac ] || [ -d trim ] || [ -d mask ] || [ -d tbm ]; then
    echo one of "  dac trim mask tbm  " already exists!
    exit 1
fi

mkdir dac trim mask tbm
find "$1" -name TBM_module_\*.dat -exec cp -p {} tbm/ \;
find "$1" -name ROC_DAC_module_\*.dat -exec cp -p {} dac/ \;
find "$1" -name ROC_Trims_module_\*.dat -exec cp -p {} trim/ \;
find "$1" -name ROC_Masks_module_\*.dat -exec cp -p {} mask/ \;

ntbm=$(ls -1 tbm | wc -l)
n=$ntbm
ndac=$(ls -1 dac | wc -l)
ntrim=$(ls -1 trim | wc -l)
nmask=$(ls -1 mask | wc -l)

if [ $n -ne $ndac ] || [ $n -ne $ntrim ] || [ $n -ne $nmask ]; then
    echo number of configs different! ntbm $ntbm ndac $ndac ntrim $ntrim nmask $nmask
    exit 1
fi

echo got configs for $n modules

cd tbm
sed -i 's/DisablePKAMCounter: 0/DisablePKAMCounter: 1/' *
cd ../dac
sed -i 's/Vdd:           6/Vdd:           8/' *
sed -i 's/Readback:.*/Readback:      12/' *
#sed -i 's/WBC:.*/WBC:           92/' *
cd ..
