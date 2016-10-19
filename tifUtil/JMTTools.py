import sys, os, cPickle
from array import array
from glob import glob
from collections import defaultdict
from pprint import pprint
from itertools import izip

BUILD_HOME = os.environ['BUILD_HOME']
POS_OUTPUT_DIRS = os.environ['POS_OUTPUT_DIRS']
PIXELCONFIGURATIONBASE = os.environ['PIXELCONFIGURATIONBASE']

def mkdir_p(d):
    try:
        os.mkdir(d)
    except OSError:
        pass

def countem(l):
    d = defaultdict(int)
    for x in l:
        d[x] += 1
    return dict(d)

def config_fn(x):
    return os.path.join(PIXELCONFIGURATIONBASE, x)

def config_key_fn(x, t=None):
    if os.path.isfile(x):
        return x
    cfg_fn = config_fn(x)
    if os.path.isfile(cfg_fn):
        return cfg_fn
    
def new_config_key(x, min_key=0):
    new_key = min_key
    d = None
    while 1:
        d = config_fn(os.path.join(x, str(new_key)))
        if os.path.isdir(d):
            new_key += 1
        else:
            break
    return new_key, d

def run_from_argv():
    run = None
    for x in sys.argv:
        try:
            run = int(x)
            break
        except ValueError:
            pass
    if run is None:
        cwd = os.getcwd()
        cwd = os.path.basename(cwd)
        if cwd.startswith('Run_'):
            run = int(cwd.split('Run_')[1])
        else:
            raise ValueError('no number in argv and cannot grok %s' % os.getcwd())
    return run

def run_dir(run):
    run_thousand = run / 1000 * 1000
    return os.path.join(POS_OUTPUT_DIRS, 'Run_%i' % run_thousand, 'Run_%i' % run)

def run_dir_from_argv():
    return run_dir(run_from_argv())

def run_fn(run, fn):
    return os.path.join(run_dir(run), fn)

def run_fn_from_argv(fn):
    return os.path.join(run_dir_from_argv(), fn)

class calib_dat:
    def __init__(self, run_or_fn):
        if type(run_or_fn) == int:
            run = run_or_fn
            fn = os.path.join(run_dir(run), 'calib.dat')
        else:
            run = None
            fn = run_or_fn

        if not os.path.isfile(fn):
            raise IOError('no file found at %s' % fn)

        tokens = open(fn).read().strip().split()
        assert tokens[0] == 'Mode:'
        self.mode = tokens[1]

        self.parameters = {}
        if 'Parameters:' in tokens:
            partoks = tokens[tokens.index('Parameters:') + 1 : tokens.index('Rows:')]
            assert len(partoks) % 2 == 0
            self.parameters = dict((partoks[i],partoks[i+1]) for i in xrange(0, len(partoks), 2))

        def group(seq, sep, conv=None):
            g = []
            sep_isseq = type(sep) in (tuple,list)
            for el in seq:
                if (not sep_isseq and el == sep) or (sep_isseq and el in sep):
                    yield g
                    g = []
                else:
                    g.append(el if conv is None else conv(el))
            yield g

        def intable(x):
            try:
                int(x)
            except ValueError:
                return False
            return True

        rind = tokens.index('Rows:')
        cind = tokens.index('Cols:')
        for ind, token in enumerate(tokens[cind+1:]):
            if token != '|' and not intable(token):
                cindlast = cind + ind
                break
        self.row_groups = list(group(tokens[rind+1:cind],       '|', int))
        self.col_groups = list(group(tokens[cind+1:cindlast+1], '|', int))

        self.pixels = [(r,c) for rg in self.row_groups for r in rg for cg in self.col_groups for c in cg]

        nind = tokens.index('Repeat:')
        self.ntrigs = int(tokens[nind + 1])

        self.vcalhigh = True
        if 'VcalLow' in tokens:
            self.vcalhigh = False

        def lfind(l, i):
            try:
                return l.index(i)
            except ValueError:
                return 9999999

        self.scans = []
        sind = min(lfind(tokens, 'Scan:'), lfind(tokens, 'ScanValues:'))
        if sind != 9999999:
            self.scans = list(group(tokens[sind+1:nind], ('Scan:', 'ScanValues:')))

        self.rocs = tokens[tokens.index('ToCalibrate:')+1:]

class detconfig_dat:
    def __init__(self, run_or_fn):
        if type(run_or_fn) == int:
            run = run_or_fn
            fn = os.path.join(run_dir(run), 'detectconfig.dat')
        else:
            run = None
            fn = run_or_fn

        if not os.path.isfile(fn):
            raise IOError('no file found at %s' % fn)

        lines = [x.strip() for x in open(fn).read().split('\n') if x.strip()]
        assert lines[0] == 'Rocs:'

        self.rocs = {
            'all': [],
            'noqual': [],
            'qual': [],
            }

        for line in lines[1:]:
            line = line.split()
            self.rocs['all'].append(line[0])
            if len(line) > 1:
                self.rocs['qual'].append((line[0], tuple(line[1:])))
            else:
                self.rocs['noqual'].append(line[0])

class TrimResult:
    def __init__(self, s):
        s = s.split()
        assert len(s) == 9
        dummy, self.roc, r, c, noise, thresh, istat, chi2, prob = s
        self.row = int(r)
        self.col = int(c)
        self.noise = float(noise)
        self.thresh = float(thresh)
        self.istat = int(istat)
        self.chi2 = float(chi2)
        self.prob = float(prob)

def dec(dcol, pxl):
    dcol = dcol & 0x1f
    pxl = pxl & 0xff
    col = dcol*2 + pxl%2
    row = 80 - pxl/2
    return col, row

class configurations_txt:
    def __init__(self):
        fn = os.path.join(PIXELCONFIGURATIONBASE, 'configurations.txt')
        self.d = d = {}
        e = None
        key = None
        for line in open(fn):
            line = line.strip()
            if line:
                if line.startswith('key'):
                    if e:
                        d[key] = e
                    e = {}
                    key = int(line.split()[-1])
                else:
                    line = line.split()
                    assert len(line) == 2
                    e[line[0]] = line[1]
        if e:
            d[key] = e

    def __getitem__(self, key):
        return self.d[key]

    def compare(self, k0, k1):
        #d0 = set(self.d[k0].items())
        #d1 = set(self.d[k1].items())
        #in_k0_not_k1 = d0 - d1
        #in_k1_not_k0 = d1 - d0
        #if in_k0_not_k1: print 'in', k0, 'not', k1, in_k0_not_k1
        #if in_k1_not_k0: print 'in', k1, 'not', k0, in_k1_not_k0
        #return not in_k0_not_k1 and not in_k1_not_k0

        d0 = self.d[k0]
        d1 = self.d[k1]
        if d0 == d1:
            return True
        sk0 = set(d0.keys())
        sk1 = set(d1.keys())
        print '%-15s%15s%15s' % ('diffs:', k0, k1)
        for sk in sorted(sk0 | sk1):
            v0 = d0.get(sk, '')
            v1 = d1.get(sk, '')
            if v0 != v1:
                print '%15s%15s%15s' % (sk, v0, v1)
        return False
        
def aliases_txt():
    cd, vd = [], []

    fn = os.path.join(PIXELCONFIGURATIONBASE, 'aliases.txt')
    lines = open(fn).readlines()
    assert lines[0].strip() == 'ConfigurationAliases'
    lines.pop(0)

    d = cd
    for line in lines:
        line = line.strip()
        if line == 'VersionAliases':
            d = vd
            continue
        line = line.split()
        assert line[-1] == ';'
        d.append((line[0], int(line[1]), ' '.join(line[2:-1])))

    return cd, vd

class dac_dat:
    DACS = ['Vdd', 'Vana', 'Vsh', 'Vcomp', 'VwllPr', 'VwllSh', 'VHldDel', 'Vtrim', 'VcThr', 'VIbias_bus', 'PHOffset', 'Vcomp_ADC', 'PHScale', 'VIColOr', 'Vcal', 'CalDel', 'TempRange', 'WBC', 'ChipContReg', 'Readback']

    def __init__(self, fn):
        self.fn = fn
        self.basefn  = os.path.basename(fn)
        self.dacs_by_roc = {}
        this_roc = None
        these_dacs = {}
        def add():
            global these_dacs, this_roc
        for line in open(fn):
            if line.startswith('ROC:'):
                if these_dacs:
                    assert this_roc is not None
                    assert set(these_dacs.keys()) == set(self.DACS)
                    self.dacs_by_roc[this_roc] = these_dacs
                this_roc = line.split()[-1]
                assert '_ROC' in this_roc
                these_dacs = {}
            else:
                dacname, dacval = line.split()
                assert dacname.endswith(':')
                dacname = dacname.replace(':', '')
                dacval = int(dacval)
                these_dacs[dacname] = dacval
        assert this_roc is not None
        assert set(these_dacs.keys()) == set(self.DACS)
        self.dacs_by_roc[this_roc] = these_dacs
        assert len(self.dacs_by_roc) == 16

    def write(self, f):
        if type(f) == str:
            fn = f
            f = open(fn, 'wt')
        elif type(f) == int:
            fn = os.path.join(PIXELCONFIGURATIONBASE, 'dac/' + str(f) + '/' + self.basefn)
            f = open(fn, 'wt')
        elif not hasattr(f, 'write'):
            raise TypeError("can't handle f %r" % f)

        rocs = self.dacs_by_roc.keys()
        rocs.sort(key=lambda x: int(x.split('_ROC')[1]))
        for roc in rocs:
            f.write('ROC:           %s\n' % roc)
            dacs = self.dacs_by_roc[roc]
            for dac in self.DACS:
                f.write((dac + ':').ljust(15))
                f.write('%i\n' % dacs[dac])

class portcardmap_dat:
    def __init__(self, fn):
        self.fn = fn
        if os.path.isdir(fn):
            fn = os.path.join(fn, 'portcardmap.dat')
        f = open(fn)
        self.l = []
        self.m = defaultdict(list)
        self.p = {}
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            line = line.split()
            if len(line) == 3:
                pc, module, ch = line
            else:
                pc, module, aorb, ch = line
            ch = int(ch)
            self.l.append((pc, module, ch))
            self.m[pc].append((module, ch))
            self.p[module] = (pc, ch)
        self.l.sort(key=lambda x: (x[0], x[2]))

class mask_dat:
    def __init__(self, fn):
        self.m = {}
        self.fn = fn
        if os.path.isdir(fn):
            fn = os.path.join(fn, 'portcardmap.dat')
        f = open(fn)
        lines = [l.strip() for l in f.readlines() if l.strip()]
        assert len(lines) == 16*(52 + 1)
        self.module = None
        for roc in xrange(16):
            rocline = lines[0]
            masklines = lines[1:53]
            lines = lines[53:]

            assert rocline.startswith('ROC:')
            roc = rocline.split()[-1]
            module, roc = roc.split('_ROC')
            roc = int(roc)
            if self.module is not None:
                assert module == self.module
            else:
                self.module = module
            self.m[roc] = [None]*52

            for col, maskline in enumerate(masklines):
                assert maskline.startswith('col%02i:' % col)
                maskbits = maskline.split()[-1]
                assert len(maskbits) == 80
                self.m[roc][col] = list(maskbits)

    def set(self, val, roc, **args):
        assert str(val) in '10'

        a = set(args.keys())
        if a == set(('col', 'row')):
            col, row = args['col'], args['row']
        elif a == set(('dc', 'pxl')):
            col, row = dec(args['dc'], args['pxl'])
        else:
            raise ValueError('args have to be col&row or dc&pxl')

        assert 0 <= col <= 51 and 0 <= row <= 79
        self.m[roc][col][row] = str(val)
        
    def write(self, fn):
        f = open(fn, 'wt')
        for roc in xrange(16):
            f.write('ROC:     ' + self.module + '_ROC' + str(roc) + '\n')
            for col in xrange(52):
                f.write('col%02i:   ' % col + ''.join(self.m[roc][col]) + '\n')
        f.close()

class translation_dat:
    class entry:
        def __init__(self, line):
            line = line.split()
            assert len(line) == 11
            self.roc_name = line[0]
            self.module, self.roc = self.roc_name.split('_ROC')
            assert int(self.roc) % 8 == int(line[-1])
            self.roc = int(self.roc)
            self.aorb = line[1]
            self.fec, self.mfec, self.mfecch, self.hubid, self.port, self.rocid, self.fedid, self.fedch = [int(x) for x in line[2:-1]]

    def __init__(self, fn):
        self.l = []
        for line in open(fn):
            line = line.strip()
            if line.startswith('#') or not line:
                continue
            self.l.append(translation_dat.entry(line))

    def module_name_from_fed_ch(self, fedid, fedch):
        for x in self.l:
            if x.fedid == fedid and x.fedch == fedch:
                return x.module

class trim_dat:
    class entry:
        def __init__(self, sg, th, istat, chi2, prob):
            self.sg = sg
            self.th = th
            self.istat = istat
            self.chi2 = chi2
            self.prob = prob

    def __init__(self, fn):
        self.ls = defaultdict(lambda: [0]*4160)
        self.seens = defaultdict(set)
        for iline, line in enumerate(open(fn)):
            line = line.strip()
            if not line:
                continue

            line = line.split()
            assert len(line) == 9
            #assert line[0] == '[PixelSCurveHistoManager::fit()]RocName='
            sanity = line[1].startswith('FPix_') or line[1].startswith('Pilt_')
            assert sanity
            roc = line[1]
            seen = self.seens[roc]
            l = self.ls[roc]
            r, c = int(line[2]), int(line[3])
            assert 0 <= c <= 51
            assert 0 <= r <= 79
            assert (c,r) not in seen
            seen.add((c,r))
            sg, th = float(line[4]), float(line[5])
            #assert 0 <= th
            assert 0 <= sg
            istat, chi2, prob = float(line[6]), float(line[7]), float(line[8])
            assert istat in [0,1,2,3]
            assert 0 <= chi2
            assert 0 <= prob
            l[c*80 + r] = trim_dat.entry(sg, th, istat, chi2, prob)

    def update(self, other):
        for roc, l in other.ls.iteritems():
            for i, x in enumerate(l):
                if x != 0:
                    self.ls[roc][i] = x

    def write(self, fn):
        f = open(fn, 'wt')
        for roc in sorted(self.ls.keys()):
            for i in xrange(4160):
                col = i / 80
                row = i % 80
                e = self.ls[roc][i]
                if e != 0:
                    f.write('X %(roc)s %(row)s %(col)s ' % locals())
                    f.write('%.6f %.6f %i %.2f %.2f\n' % (e.sg, e.th, e.istat, e.chi2, e.prob))

def merge_trim_dats(fns, new_fn=None):
    # assemble a trim_dat from the fns, letting later fns override
    # results for earlier fns
    print fns[0]
    td_merge = trim_dat(fns[0])
    for fn in fns[1:]:
        print fn
        td_merge.update(trim_dat(fn))
    if new_fn:
        td_merge.write(new_fn)
        
    return td_merge

if __name__ == '__main__' and len(sys.argv) > 1:
    mode = sys.argv[1]

    if mode == 'merge_trim_dats':
        if len(sys.argv) < 4:
            print 'usage: python JMTTools.py %s txt_fn out_fn' % mode
        else:
            txt_fn = sys.argv[2]
            out_fn = sys.argv[3]

            fns = [line.strip() for line in open(txt_fn) if line.strip()]
            td = merge_trim_dats(fns, out_fn)

    elif mode == 'configs':
        c = configurations_txt()
        cd, vd = aliases_txt()
        print 'configurations.txt in c and aliases.txt in cd, vd'
