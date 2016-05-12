#!/usr/bin/env python

id2name = {
1  : 'M-H-2-15',
2  : 'M-G-2-40',
3  : 'M-H-1-13',
4  : 'M-H-1-23',
5  : 'M-G-1-34',
6  : 'M-H-2-14',
7  : 'M-H-2-42',
8  : 'M-G-1-36',
9  : 'M-G-1-46',
10 : 'M-K-1-03',
11 : 'M-H-2-37',
12 : 'M-H-2-48',
13 : 'M-H-2-35',
14 : 'M-H-1-48',
}

dacdir = 'dac/8'
tbmdir = 'tbm/3'
trimdir = 'trim/4'

WBC = 92
Readback = 12

#######

import os

base = os.environ.get('PIXELCONFIGURATIONBASE', '')
dacdir = os.path.join(base, dacdir)
tbmdir = os.path.join(base, tbmdir)
trimdir = os.path.join(base, trimdir)

if not os.path.isdir(base) or any(not os.path.isdir(os.path.join(base, x)) for x in ('dac', 'tbm', 'trim')):
    raise IOError('PIXELCONFIGURATIONBASE="%s" does not seem to be a pixel config directory' % base)
for x in (dacdir, tbmdir, trimdir):
    if os.path.isdir(x):
        raise IOError('%s already exists' % x)

for x in (dacdir, tbmdir, trimdir):
    os.mkdir(x)

for modid in xrange(1,15):
    oldname2 = id2name[modid]
    oldname = 'FPIX_%s' % oldname2
    olddir = 'TestArea/%s' % oldname2

    newname = 'Pilt_BmI_D3_BLD%(modid)i_PNL1' % locals()
    newname2 = 'Pilt_BmI_D3_BLD%(modid)i_PNL1_PLQ1' % locals()

    print modid
    print oldname
    print oldname2
    print olddir
    print newname
    print newname2
    print

    olddacfn = '%s/ROC_DAC_module_%s.dat' % (olddir, oldname)
    newdacfn = '%s/ROC_DAC_module_%s.dat' % (dacdir, newname)
    f = open(newdacfn, 'wt')
    for line in open(olddacfn):
        if line.startswith('ROC:'):
            roc = line.split('ROC')[-1]
            line = 'ROC: ' + newname2 + '_ROC' + roc
        elif 'WBC' in line:
            line = 'WBC: %s\n' % WBC
        elif 'Readback' in line:
            line = 'Readback: %s\n' % Readback
        f.write(line)
    f.close()

    oldtbmfn = '%s/TBM_module_%s.dat' % (olddir, oldname)
    newtbmfn = '%s/TBM_module_%s.dat' % (tbmdir, newname)
    f = open(newtbmfn, 'wt')
    first = True
    for line in open(oldtbmfn):
        if first:
            line = newname2 + '_ROC0\n'
            first = False
        elif 'DisablePKAM' in line:
            line = line.replace('0', '1')
        f.write(line)
    f.close()
    
    oldtrimfn = '%s/ROC_Trims_module_%s.dat' % (olddir, oldname)
    newtrimfn = '%s/ROC_Trims_module_%s.dat' % (trimdir, newname)
    f = open(newtrimfn, 'wt')
    for line in open(oldtrimfn):
        if line.startswith('ROC:'):
            roc = line.split('ROC')[-1]
            line = 'ROC: ' + newname2 + '_ROC' + roc
        f.write(line)
    f.close()
