import sys
import re
from JMTTools import *

in_fn = sys.argv[1]
roc_re = sys.argv[2]
dac = sys.argv[3]
delta = int(sys.argv[4])

if roc_re.startswith('*'):
    roc_re = '.*' + roc_re[1:]

dac_d = dac_dat(in_fn)
dacs = dac_d.dacs_by_roc
for roc in dacs.keys():
    if re.search(roc_re, roc):
        roc_dacs = dacs[roc]
        val = roc_dacs[dac]
        newval = val + delta
        if newval < 0:
            sys.stderr.write('clamping at 0 for ' +  roc + '\n')
            newval = 0
        elif newval > 255:
            sys.stderr.write('clamping at 255 for ' +  roc + '\n')
            newval = 255
        roc_dacs[dac] = newval

dac_d.write(sys.stdout)

            
