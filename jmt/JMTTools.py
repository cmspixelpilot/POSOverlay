import sys, os
from array import array
from glob import glob
from collections import defaultdict
from pprint import pprint
from itertools import izip

POS_OUTPUT_DIRS = os.environ['POS_OUTPUT_DIRS']

def run_from_argv():
    run = None
    for x in sys.argv:
        try:
            run = int(x)
            break
        except ValueError:
            pass
    if run is None:
        raise ValueError('no number in argv')
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

        
if __name__ == '__main__':
    c = calib_dat(1440)
    d = detconfig_dat(1440)

