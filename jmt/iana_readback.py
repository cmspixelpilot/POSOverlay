import sys

lines = '''
Selected ROC: Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Will set Vana = 136
iread: 0
Selected ROC:Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Iana: 0.44
Readback: vd: ReadDigFEDOSDFifo: RocHi: 2211 RocLo: 2211
Readback: va: ReadDigFEDOSDFifo: RocHi: 2426 RocLo: 2426
Readback: vana: ReadDigFEDOSDFifo: RocHi: 2719 RocLo: 2719
Readback: vbg: ReadDigFEDOSDFifo: RocHi: 2975 RocLo: 2975
Readback: iana: ReadDigFEDOSDFifo: RocHi: 3239 RocLo: 3239
iread: 1
Selected ROC:Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Iana: 0.44
Readback: vd: ReadDigFEDOSDFifo: RocHi: 2210 RocLo: 2210
Readback: va: ReadDigFEDOSDFifo: RocHi: 2338 RocLo: 2338
Readback: vana: ReadDigFEDOSDFifo: RocHi: 2716 RocLo: 2716
Readback: vbg: ReadDigFEDOSDFifo: RocHi: 2972 RocLo: 2972
Readback: iana: ReadDigFEDOSDFifo: RocHi: 3228 RocLo: 3228
Selected ROC: Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Will set Vana = 34
iread: 0
Selected ROC:Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Iana: 0.44
Readback: vd: ReadDigFEDOSDFifo: RocHi: 2213 RocLo: 2213
Readback: va: ReadDigFEDOSDFifo: RocHi: 2428 RocLo: 2428
Readback: vana: ReadDigFEDOSDFifo: RocHi: 2940 RocLo: 2940
Readback: vbg: ReadDigFEDOSDFifo: RocHi: 2968 RocLo: 2968
Readback: iana: ReadDigFEDOSDFifo: RocHi: 3124 RocLo: 3124
iread: 1
Selected ROC:Pilt_BmO_D3_BLD10_PNL1_PLQ1_ROC0
Iana: 0.44
Readback: vd: ReadDigFEDOSDFifo: RocHi: 2100 RocLo: 2100
Readback: va: ReadDigFEDOSDFifo: RocHi: 2419 RocLo: 2419
Readback: vana: ReadDigFEDOSDFifo: RocHi: 2703 RocLo: 2703
Readback: vbg: ReadDigFEDOSDFifo: RocHi: 2959 RocLo: 2959
Readback: iana: ReadDigFEDOSDFifo: RocHi: 3124 RocLo: 3124
'''.split('\n')

Vana = None
roc = None
iread = None
Iana = []
RB_vd = []
RB_va = []
RB_vana = []
RB_vbg = []
RB_iana = []

last = ''
def chomp(line, s):
    global last
    if line.startswith(s):
        last = line.replace(s, '').strip()
        return True
    else:
        return False

print 'roc,Vana,Iana0,Iana1,RB_vd0,RB_vd1,RB_va0,RB_va1,RB_vana0,RB_vana1,RB_vbg0,RB_vbg1,RB_iana0,RB_iana1'
for line in open(sys.argv[1]):
    line = line.strip()
    if chomp(line, 'Will set Vana = '):
        if Iana:
            print '%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s' % (roc, Vana, Iana[0], Iana[1], RB_vd[0], RB_vd[1], RB_va[0], RB_va[1], RB_vana[0], RB_vana[1], RB_vbg[0], RB_vbg[1], RB_iana[0], RB_iana[1])
        Iana = []
        RB_vd = []
        RB_va = []
        RB_vana = []
        RB_vbg = []
        RB_iana = []
        Vana = last
    elif not line.startswith('Selected ROC: ') and chomp(line, 'Selected ROC:'):
        roc = last
    elif chomp(line, 'iread: '):
        iread = int(last)
    elif chomp(line, 'Iana: '):
        Iana.append(last)
    else:
        for RB in 'vd va vana vbg iana'.split():
            if chomp(line, 'Readback: %s: ReadDigFEDOSDFifo: RocHi: ' % RB):
                hi, lo = last.split(' RocLo: ')
                assert hi == lo
                eval('RB_%s' % RB).append(lo)
                break

        

    
