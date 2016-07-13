#!/usr/bin/env python

import sys
from itertools import combinations

fns = sys.argv[1:]

ok = True

for a,b in combinations(fns, 2):
    z = []
    for x in a,b:
        y = open(x).read()
        for c in '\n\r\t ':
            y = y.replace(c, '')
        z.append(y)
    aa,bb = z
    if aa != bb:
        print a,b
        ok = False

if ok: print 'all OK'

