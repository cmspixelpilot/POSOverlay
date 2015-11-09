from JMTTools import *

dir1, dir2 = sys.argv[1:3]
split_at = int(sys.argv[3])
out_dir = sys.argv[4]

assert os.path.isdir(dir1) and os.path.isdir(dir2)
assert split_at >= 0 and split_at < 79
assert not os.path.exists(out_dir)

fns1 = sorted(glob(os.path.join(dir1, 'ROC_Trims*dat')))
fns2 = sorted(glob(os.path.join(dir2, 'ROC_Trims*dat')))

assert [os.path.basename(fn) for fn in fns1] == [os.path.basename(fn) for fn in fns2]

print 'I take rows 0-%i inclusive from %s and %i-79 from %s' % (split_at, dir1, split_at+1, dir2)
raw_input('ok?')

os.system('mkdir -p %s' % out_dir)

for ifn, (fn1, fn2) in enumerate(izip(fns1, fns2)):
    bn = os.path.basename(fn1)
    out_fn = os.path.join(out_dir, bn)
    fout = open(out_fn, 'wt')
    def write(line):
        fout.write(line)
        fout.write('\n')

    lines1 = [x.strip() for x in open(fn1).read().split('\n') if x.strip()]
    lines2 = [x.strip() for x in open(fn2).read().split('\n') if x.strip()]

    assert len(lines1) == len(lines2)

    for line1, line2 in izip(lines1, lines2):
        if line1.startswith('ROC:'):
            assert line1 == line2
            write(line1)
        else:
            assert line1.startswith('col') and line2.startswith('col')
            col1, trims1 = line1.split()
            col2, trims2 = line2.split()
            assert col1 == col2
            line = col1 + '   ' + trims1[0:split_at+1] + trims2[split_at+1:]
            write(line)
    fout.close()
            
