template = '''ROC: Pilt_BmI_D3_BLD%(bld)i_PNL%(pnl)i_PLQ1_ROC%(roc)i
Vdd:          8
Vana:        85
Vsh:         30
Vcomp:       12
VwllPr:     150
VwllSh:     150
VHldDel:    250
Vtrim:        0 
VcThr:      110
VIbias_bus:  30
PHOffset:   200
Vcomp_ADC:   50
PHScale:    255
VIColOr:    100
Vcal:       200
CalDel:      66
WBC:         92
ChipContReg:  0
Readback:    12
'''

for bld in (2,3):
    for pnl in (1,2):
        f = open('ROC_DAC_module_Pilt_BmI_D3_BLD%(bld)i_PNL%(pnl)i.dat' % locals(), 'wt')
        for roc in xrange(16):
            f.write(template % locals())
