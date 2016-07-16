import sys, os
from array import array
from glob import glob
from collections import defaultdict
from pprint import pprint
from itertools import izip

BUILD_HOME = os.environ['BUILD_HOME']
POS_OUTPUT_DIRS = os.environ['POS_OUTPUT_DIRS']
PIXELCONFIGURATIONBASE = os.environ['PIXELCONFIGURATIONBASE']

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

def configurations_txt():
    fn = os.path.join(PIXELCONFIGURATIONBASE, 'configurations.txt')
    d = {}
    e = None
    key = None
    for line in open(fn):
        line = line.strip()
        if line:
            if line.startswith('key'):
                if e:
                    d[key] = e
                e = {}
                key = line.split()[-1]
            else:
                line = line.split()
                assert len(line) == 2
                e[line[0]] = line[1]
    return d

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
        f = open(fn)
        self.l = []
        self.m = defaultdict(list)
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            line = line.split()
            assert len(line) == 3
            pc, module, ch = line
            ch = int(ch)
            self.l.append((pc, module, ch))
            self.m[pc].append((module, ch))
        self.l.sort(key=lambda x: (x[0], x[2]))

#def translation_dat(key):
#    fn = os.path.join(PIXELCONFIGURATIONBASE, 'nametranslation/%s/translation.dat')
#    by_module = 
#    for line in open(fn):
#        line = line.strip():
#        if line:
#            line = line.split()
#def tbm(key):
    
if __name__ == '__main__':
    dacs = dac_dat('/home/fnaltest/TriDAS/Config/dac/8/ROC_DAC_module_FPix_BmI_D3_BLD1_PNL1_RNG1.dat')
    
