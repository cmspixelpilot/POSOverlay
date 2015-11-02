
cfg = '''key %(key)i
detconfig   6
lowvoltagemap   0
globaldelay25   %(gd)i
nametranslation   0
fedconfig   0
fecconfig   0
fedcard   9
dac   %(dac)i
mask   1
trim   11
tbm   1
portcard   0
portcardmap   0
ttcciconfig   7
ltcconfig   0
tkfecconfig   0
'''

ali = 'Physics_%(ad_s)s     %(key)i   detconfig  Golden2  lowvoltagemap  Default  nametranslation  Default  fedconfig  Default  fedcard  Physics  fecconfig  Default  dac  %(dac_s)s  mask  OldHotPixels  trim  Default  tbm  Default  portcard  Default  portcardmap  Default  ttcciconfig  iCIGlobal  ltcconfig  Default  tkfecconfig  Default  globaldelay25  %(gd_s)s  ;'

xs = -14

for ali1 in (0,1):
    for x in range(xs, 48):
        key = x - xs + 139
        if x < 0:
            gd = 25 - abs(x)
            ad_s = 'm%ins' % abs(x)
            dac_s = 'WBC169'
        else:
            ad_s = 'p%ins' % x
            gd = x % 25
            if x / 25 == 0:
                dac_s = 'WBC168'
            else:
                dac_s = 'WBC167'
        gd_s = 'p%ins' % gd
        dac = 28
        if dac_s == 'WBC168':
            dac = 37
        elif dac_s == 'WBC169':
            dac = 38

        if ali1:
            print ali % locals()
        else:
            print cfg % locals()

