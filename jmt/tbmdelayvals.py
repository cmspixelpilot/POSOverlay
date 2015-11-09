y = []
print 'ScanValues: TBMADelay',
for x in xrange(4):
    for d in xrange(64):
        p0 = d & 7
        p1 = (d >> 3) & 7
        if abs(p0-p1) <= 2:
            z = (x << 6) | d
            y.append(z)
            print z,
print -1
print len(y)
