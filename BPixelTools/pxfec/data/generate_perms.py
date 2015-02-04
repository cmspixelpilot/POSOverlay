#!/usr/bin/python

import os

header = '''echo this is d.ini
sys
fec  11
mfec 11 8 -6P

cn -6PL12
#cn -6PL3
cn hello
'''

template = '''module %(module)i
echo configuring rocs on module %(module)i
roc 0:15
Vdig         8
Vana        85
Vsh         30
Vcomp       12
VwllPr     150
VwllSh     150
VhldDel    250
Vtrim        0 
VthrComp   110
VIBias_Bus  30
PHOffset   200
Vcomp_ADC   50
PHScale    255
VIColOr    100
Vcal       200
CalDel      66
CtrlReg      0
WBC         92
mask
arm      4 4

echo tbm reset
tbm
reset tbm
fullspeed
mode cal
tbmadelay 219 
tbmbdelay 219

roc 0:15
ReadBack 12
'''

footer = '''
module 6,7,14,15
roc 0:15

echo done with d.ini
'''

def do(modules):
    name = 'd.ini.BmI.separate.%i_%i_%i_%i' % modules
    f = open(name, 'wt')
    f.write(header)
    for module in modules:
        f.write(template % locals())
    f.write(footer)
    f.close()
    cmd = 'rm d.ini ; cp %s d.ini ; grep module %s' % (name, name)
    raw_input(cmd + ' ? ')
    os.system(cmd)

#do((6,7,14,15))
#do((7,6,14,15))
#do((14,7,6,15))
#do((15,7,14,6))
#do((6,14,7,15))
#do((6,15,14,7))
#do((14,7,6,15))
do((6,14,7,15))
do((6,7,15,14))
do((14,15,6,7))
do((15,14,6,7))
do((15,14,7,6))
