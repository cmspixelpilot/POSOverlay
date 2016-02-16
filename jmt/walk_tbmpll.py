import sys, os

add400 = int(sys.argv[1])
add160 = int(sys.argv[2])

for fn in sys.argv[3:]:
    print fn
    lines = open(fn).readlines()
    assert lines[-3].startswith('TBMPLLDelay:')
    old = int(lines[-3].split()[-1])
    old400 = (old & 28) >> 2
    old160 = (old & 224) >> 5
    new400 = (old400 + add400) % 8
    new160 = (old160 + add160) % 8
    new = (new160 << 5) | (new400 << 2)
    print lines[-3],
    lines[-3] = 'TBMPLLDelay: %i\n' % new
    print '->', lines[-3]
    open(fn, 'wt').write(''.join(lines))
            
