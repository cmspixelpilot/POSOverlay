
id2name = {
5 :'M-G-1-34',
8 :'M-G-1-36',
9 :'M-G-1-46',
2 :'M-G-2-40',
3 :'M-H-1-13',
6 :'M-H-2-14',
1 :'M-H-2-15',
11:'M-H-2-37',
}

dacdir = 'dac/5'
tbmdir = 'tbm/1'
trimdir = 'trim/2'

lastlines = '''WBC: 92
ChipContReg: 0
Readback: 12
'''


for modid in xrange(1,15):
    if id2name.has_key(modid):
        oldname2 = id2name[modid]
    else:
        oldname2 = 'M-H-2-37'
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
            line = 'WBC: 92\n'
        elif 'Readback' in line:
            line = 'Readback: 12\n'
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

