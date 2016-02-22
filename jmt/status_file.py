import re
import sys
from collections import defaultdict
from itertools import izip
from pprint import pprint

def parse(fn):
    line_re = re.compile(r'chnl: (\d+) Num. of (.*): (\d+)')

    d = defaultdict(lambda: defaultdict(list))

    for line in open(fn):
        line = line.strip()
        mo = line_re.search(line)
        if mo:
            channel, kind, num = mo.groups()
            channel, num = int(channel), int(num)
            #print channel, kind, num
            d[kind][channel].append(num)

    return d

def analyze(fn):
    d = parse(fn)

    max_by = {
        'Timeouts': 2**18,
        'NOR Errs': 2**18,
        'OOS': 2**14,
        }

    e = defaultdict(lambda: defaultdict(int))

    for kind in d.keys():
        for channel, l in d[kind].iteritems():
            num_over = 0
            for a,b in izip(l, l[1:]):
                if b < a:
                    num_over += 1
            if num_over:
                if not max_by.has_key(kind):
                    print l
                    print kind, channel
            e[kind][channel] = l[-1] + num_over * max_by[kind]
        #print kind, dict(e[kind])
    return e

fns = sys.argv[1:]
for fn in fns:
    e = analyze(fn)
    
runs = {
    0: (2229, 107e6),
#    1: (2230, 2.6e6),
    2: (2231, 101e6),
    3: (2232, 102e6),
    4: (2233, 101e6),
    5: (2235, 102e6),
    6: (2236, 38e6),
#    7: (2239, 440e3),
    8: (2240, 102e6),
}

channels = [3,4,7,8,9,10,25,26,27,28,31,32,33,34]

min_by_channel = defaultdict(list)

for step, (run, ntrigs) in runs.iteritems():
    print 'step', step
    fn = '/pixelscratch/pixelscratch/pilot/Runs/Run_2000/Run_%i/StatusFED_40_%i.txt' % (run, run)
    e = analyze(fn)
    ee = e['Timeouts']
    for ch in channels:
        r = ee[ch] / ntrigs
        min_by_channel[ch].append((r, step))
        min_by_channel[ch].sort()
pprint(dict(min_by_channel))
