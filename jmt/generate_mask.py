from JMTTools import dec
from JMTROOTTools import *

detconfig = {
    'Pilt_BmI_D3_BLD2_PNL1_PLQ1_ROC4':   'noInit',
    'Pilt_BmI_D3_BLD3_PNL2_PLQ1_ROC0':   'noInit',
    'Pilt_BmO_D3_BLD11_PNL2_PLQ1_ROC13': 'noInit',
    }

nametranslation = {
1:  ('Pilt_BmO_D3_BLD11_PNL1_PLQ1', 0),
2:  ('Pilt_BmO_D3_BLD11_PNL1_PLQ1', 8),
3:  ('Pilt_BmO_D3_BLD11_PNL2_PLQ1', 0),
4:  ('Pilt_BmO_D3_BLD11_PNL2_PLQ1', 8),
7:  ('Pilt_BmO_D3_BLD10_PNL1_PLQ1', 0),
8:  ('Pilt_BmO_D3_BLD10_PNL1_PLQ1', 8),
9:  ('Pilt_BmO_D3_BLD10_PNL2_PLQ1', 0),
10: ('Pilt_BmO_D3_BLD10_PNL2_PLQ1', 8),
25: ('Pilt_BmI_D3_BLD2_PNL1_PLQ1', 0),
26: ('Pilt_BmI_D3_BLD2_PNL1_PLQ1', 8),
27: ('Pilt_BmI_D3_BLD2_PNL2_PLQ1', 0),
28: ('Pilt_BmI_D3_BLD2_PNL2_PLQ1', 8),
31: ('Pilt_BmI_D3_BLD3_PNL1_PLQ1', 0),
32: ('Pilt_BmI_D3_BLD3_PNL1_PLQ1', 8),
33: ('Pilt_BmI_D3_BLD3_PNL2_PLQ1', 0),
34: ('Pilt_BmI_D3_BLD3_PNL2_PLQ1', 8),
}

s = [x.strip() for x in '''
      1 Channel- 10 ROC- 3 DCOL- 0 Pixel- 102
      1 Channel- 10 ROC- 3 DCOL- 0 Pixel- 142
      1 Channel- 10 ROC- 3 DCOL- 0 Pixel- 18
      1 Channel- 10 ROC- 3 DCOL- 0 Pixel- 44
      1 Channel- 10 ROC- 3 DCOL- 1 Pixel- 2
      1 Channel- 10 ROC- 3 DCOL- 20 Pixel- 37
      1 Channel- 10 ROC- 3 DCOL- 20 Pixel- 53
      1 Channel- 10 ROC- 3 DCOL- 21 Pixel- 78
      1 Channel- 10 ROC- 3 DCOL- 23 Pixel- 63
      1 Channel- 10 ROC- 3 DCOL- 24 Pixel- 3
      1 Channel- 10 ROC- 3 DCOL- 25 Pixel- 135
      1 Channel- 10 ROC- 3 DCOL- 25 Pixel- 27
      1 Channel- 10 ROC- 3 DCOL- 25 Pixel- 29
      1 Channel- 10 ROC- 3 DCOL- 25 Pixel- 59
      1 Channel- 10 ROC- 3 DCOL- 4 Pixel- 2
      1 Channel- 10 ROC- 3 DCOL- 6 Pixel- 59
      1 Channel- 10 ROC- 5 DCOL- 22 Pixel- 3
      1 Channel- 10 ROC- 5 DCOL- 25 Pixel- 80
      1 Channel- 10 ROC- 6 DCOL- 24 Pixel- 76
      1 Channel- 27 ROC- 0 DCOL- 8 Pixel- 51
      1 Channel- 27 ROC- 1 DCOL- 0 Pixel- 61
      1 Channel- 27 ROC- 1 DCOL- 0 Pixel- 74
      1 Channel- 27 ROC- 1 DCOL- 0 Pixel- 96
      1 Channel- 27 ROC- 1 DCOL- 1 Pixel- 75
      1 Channel- 27 ROC- 1 DCOL- 10 Pixel- 42
      1 Channel- 27 ROC- 1 DCOL- 13 Pixel- 77
      1 Channel- 27 ROC- 1 DCOL- 19 Pixel- 48
      1 Channel- 27 ROC- 1 DCOL- 20 Pixel- 105
      1 Channel- 27 ROC- 1 DCOL- 25 Pixel- 19
      1 Channel- 27 ROC- 1 DCOL- 25 Pixel- 45
      1 Channel- 27 ROC- 1 DCOL- 25 Pixel- 59
      1 Channel- 27 ROC- 1 DCOL- 25 Pixel- 63
      1 Channel- 27 ROC- 1 DCOL- 3 Pixel- 15
      1 Channel- 27 ROC- 1 DCOL- 4 Pixel- 54
      1 Channel- 27 ROC- 1 DCOL- 8 Pixel- 51
      1 Channel- 27 ROC- 3 DCOL- 20 Pixel- 49
      1 Channel- 27 ROC- 3 DCOL- 25 Pixel- 143
      1 Channel- 27 ROC- 3 DCOL- 25 Pixel- 59
      1 Channel- 27 ROC- 5 DCOL- 25 Pixel- 35
      1 Channel- 27 ROC- 5 DCOL- 9 Pixel- 55
      1 Channel- 27 ROC- 6 DCOL- 15 Pixel- 82
      1 Channel- 27 ROC- 7 DCOL- 22 Pixel- 57
      1 Channel- 28 ROC- 0 DCOL- 22 Pixel- 20
      1 Channel- 28 ROC- 5 DCOL- 19 Pixel- 43
      1 Channel- 28 ROC- 5 DCOL- 21 Pixel- 66
      1 Channel- 28 ROC- 5 DCOL- 22 Pixel- 97
      1 Channel- 28 ROC- 6 DCOL- 25 Pixel- 83
      1 Channel- 9 ROC- 3 DCOL- 21 Pixel- 30
      1 Channel- 9 ROC- 5 DCOL- 24 Pixel- 19
      1 Channel- 9 ROC- 6 DCOL- 16 Pixel- 49
      1 Channel- 9 ROC- 7 DCOL- 25 Pixel- 28
      2 Channel- 10 ROC- 3 DCOL- 20 Pixel- 35
      2 Channel- 10 ROC- 3 DCOL- 25 Pixel- 13
      2 Channel- 10 ROC- 3 DCOL- 25 Pixel- 145
      2 Channel- 10 ROC- 3 DCOL- 25 Pixel- 81
      2 Channel- 10 ROC- 4 DCOL- 3 Pixel- 34
      2 Channel- 27 ROC- 1 DCOL- 0 Pixel- 87
      2 Channel- 27 ROC- 1 DCOL- 1 Pixel- 95
      2 Channel- 27 ROC- 1 DCOL- 25 Pixel- 57
      2 Channel- 27 ROC- 1 DCOL- 25 Pixel- 85
      2 Channel- 27 ROC- 3 DCOL- 23 Pixel- 81
      2 Channel- 27 ROC- 3 DCOL- 25 Pixel- 85
      2 Channel- 27 ROC- 5 DCOL- 12 Pixel- 31
      2 Channel- 27 ROC- 6 DCOL- 21 Pixel- 43
      2 Channel- 28 ROC- 4 DCOL- 20 Pixel- 82
      2 Channel- 28 ROC- 5 DCOL- 22 Pixel- 128
      2 Channel- 28 ROC- 6 DCOL- 17 Pixel- 3
      2 Channel- 28 ROC- 6 DCOL- 21 Pixel- 43
      2 Channel- 28 ROC- 6 DCOL- 24 Pixel- 3
      2 Channel- 9 ROC- 0 DCOL- 16 Pixel- 43
      2 Channel- 9 ROC- 6 DCOL- 21 Pixel- 132
      3 Channel- 10 ROC- 3 DCOL- 22 Pixel- 56
      3 Channel- 10 ROC- 3 DCOL- 23 Pixel- 31
      3 Channel- 10 ROC- 3 DCOL- 25 Pixel- 21
      3 Channel- 27 ROC- 3 DCOL- 1 Pixel- 25
      3 Channel- 27 ROC- 5 DCOL- 24 Pixel- 73
      3 Channel- 28 ROC- 6 DCOL- 15 Pixel- 119
      3 Channel- 28 ROC- 6 DCOL- 6 Pixel- 45
      4 Channel- 10 ROC- 3 DCOL- 25 Pixel- 107
      4 Channel- 10 ROC- 3 DCOL- 25 Pixel- 75
      4 Channel- 10 ROC- 5 DCOL- 24 Pixel- 78
      4 Channel- 27 ROC- 1 DCOL- 0 Pixel- 99
      4 Channel- 27 ROC- 1 DCOL- 2 Pixel- 41
      4 Channel- 27 ROC- 1 DCOL- 25 Pixel- 91
      4 Channel- 27 ROC- 1 DCOL- 3 Pixel- 63
      4 Channel- 27 ROC- 1 DCOL- 8 Pixel- 44
      4 Channel- 27 ROC- 3 DCOL- 4 Pixel- 59
      4 Channel- 27 ROC- 5 DCOL- 24 Pixel- 88
      4 Channel- 28 ROC- 5 DCOL- 21 Pixel- 13
      4 Channel- 28 ROC- 6 DCOL- 13 Pixel- 14
      4 Channel- 28 ROC- 6 DCOL- 19 Pixel- 78
      4 Channel- 28 ROC- 6 DCOL- 25 Pixel- 27
      5 Channel- 10 ROC- 3 DCOL- 24 Pixel- 44
      5 Channel- 10 ROC- 5 DCOL- 23 Pixel- 22
      5 Channel- 27 ROC- 1 DCOL- 10 Pixel- 87
      5 Channel- 28 ROC- 6 DCOL- 24 Pixel- 43
      5 Channel- 9 ROC- 0 DCOL- 24 Pixel- 60
      5 Channel- 9 ROC- 3 DCOL- 22 Pixel- 103
      6 Channel- 27 ROC- 3 DCOL- 16 Pixel- 41
      6 Channel- 28 ROC- 5 DCOL- 18 Pixel- 83
      6 Channel- 28 ROC- 6 DCOL- 22 Pixel- 42
      6 Channel- 9 ROC- 5 DCOL- 20 Pixel- 107
      7 Channel- 27 ROC- 3 DCOL- 24 Pixel- 107
      7 Channel- 27 ROC- 4 DCOL- 21 Pixel- 2
      7 Channel- 9 ROC- 0 DCOL- 17 Pixel- 104
      8 Channel- 10 ROC- 3 DCOL- 18 Pixel- 117
      8 Channel- 10 ROC- 3 DCOL- 18 Pixel- 3
      8 Channel- 10 ROC- 3 DCOL- 21 Pixel- 28
      8 Channel- 10 ROC- 3 DCOL- 25 Pixel- 47
      8 Channel- 10 ROC- 3 DCOL- 25 Pixel- 49
      8 Channel- 10 ROC- 4 DCOL- 0 Pixel- 84
      8 Channel- 27 ROC- 3 DCOL- 7 Pixel- 81
      8 Channel- 28 ROC- 5 DCOL- 16 Pixel- 97
      8 Channel- 28 ROC- 6 DCOL- 8 Pixel- 25
      9 Channel- 10 ROC- 3 DCOL- 0 Pixel- 14
      9 Channel- 10 ROC- 3 DCOL- 18 Pixel- 38
      9 Channel- 10 ROC- 3 DCOL- 19 Pixel- 24
      9 Channel- 10 ROC- 3 DCOL- 25 Pixel- 43
      9 Channel- 10 ROC- 5 DCOL- 17 Pixel- 10
      9 Channel- 10 ROC- 5 DCOL- 24 Pixel- 137
      9 Channel- 27 ROC- 1 DCOL- 25 Pixel- 72
      9 Channel- 28 ROC- 5 DCOL- 17 Pixel- 98
      9 Channel- 28 ROC- 6 DCOL- 21 Pixel- 9
     10 Channel- 28 ROC- 5 DCOL- 1 Pixel- 33
     10 Channel- 28 ROC- 6 DCOL- 22 Pixel- 104
     14 Channel- 10 ROC- 3 DCOL- 18 Pixel- 131
     14 Channel- 10 ROC- 3 DCOL- 25 Pixel- 69
     14 Channel- 27 ROC- 1 DCOL- 15 Pixel- 73
     14 Channel- 28 ROC- 6 DCOL- 25 Pixel- 42
     15 Channel- 10 ROC- 5 DCOL- 18 Pixel- 90
     15 Channel- 27 ROC- 3 DCOL- 22 Pixel- 70
     15 Channel- 28 ROC- 6 DCOL- 25 Pixel- 29
     15 Channel- 9 ROC- 7 DCOL- 14 Pixel- 22
     16 Channel- 10 ROC- 4 DCOL- 25 Pixel- 42
     17 Channel- 10 ROC- 3 DCOL- 25 Pixel- 127
     17 Channel- 27 ROC- 2 DCOL- 4 Pixel- 91
     17 Channel- 28 ROC- 5 DCOL- 25 Pixel- 100
     17 Channel- 9 ROC- 5 DCOL- 23 Pixel- 52
     19 Channel- 10 ROC- 3 DCOL- 25 Pixel- 35
     23 Channel- 27 ROC- 1 DCOL- 6 Pixel- 99
     23 Channel- 27 ROC- 5 DCOL- 17 Pixel- 24
     23 Channel- 9 ROC- 3 DCOL- 17 Pixel- 90
     24 Channel- 10 ROC- 3 DCOL- 1 Pixel- 125
     24 Channel- 10 ROC- 3 DCOL- 25 Pixel- 31
     26 Channel- 9 ROC- 0 DCOL- 6 Pixel- 40
     27 Channel- 27 ROC- 6 DCOL- 17 Pixel- 43
     27 Channel- 28 ROC- 0 DCOL- 17 Pixel- 47
     28 Channel- 10 ROC- 3 DCOL- 22 Pixel- 49
     29 Channel- 27 ROC- 3 DCOL- 25 Pixel- 3
     29 Channel- 28 ROC- 6 DCOL- 24 Pixel- 108
     30 Channel- 28 ROC- 0 DCOL- 23 Pixel- 32
     31 Channel- 10 ROC- 3 DCOL- 25 Pixel- 37
     36 Channel- 27 ROC- 4 DCOL- 0 Pixel- 2
     36 Channel- 9 ROC- 6 DCOL- 2 Pixel- 61
     38 Channel- 9 ROC- 3 DCOL- 20 Pixel- 115
     43 Channel- 27 ROC- 1 DCOL- 20 Pixel- 32
     45 Channel- 10 ROC- 3 DCOL- 25 Pixel- 125
     47 Channel- 10 ROC- 3 DCOL- 1 Pixel- 37
     48 Channel- 28 ROC- 0 DCOL- 15 Pixel- 61
     50 Channel- 27 ROC- 3 DCOL- 22 Pixel- 79
     50 Channel- 9 ROC- 4 DCOL- 3 Pixel- 61
     53 Channel- 10 ROC- 1 DCOL- 7 Pixel- 145
     61 Channel- 28 ROC- 7 DCOL- 15 Pixel- 89
     62 Channel- 9 ROC- 3 DCOL- 22 Pixel- 114
     63 Channel- 9 ROC- 3 DCOL- 4 Pixel- 93
     68 Channel- 10 ROC- 1 DCOL- 22 Pixel- 96
     68 Channel- 10 ROC- 4 DCOL- 0 Pixel- 14
     74 Channel- 10 ROC- 3 DCOL- 19 Pixel- 108
     74 Channel- 10 ROC- 5 DCOL- 22 Pixel- 137
     74 Channel- 28 ROC- 0 DCOL- 4 Pixel- 9
     74 Channel- 28 ROC- 6 DCOL- 25 Pixel- 68
     76 Channel- 27 ROC- 3 DCOL- 25 Pixel- 67
     80 Channel- 9 ROC- 2 DCOL- 22 Pixel- 51
     85 Channel- 27 ROC- 5 DCOL- 3 Pixel- 61
     99 Channel- 28 ROC- 6 DCOL- 13 Pixel- 74
    104 Channel- 27 ROC- 5 DCOL- 14 Pixel- 103
    124 Channel- 27 ROC- 6 DCOL- 0 Pixel- 76
    127 Channel- 10 ROC- 3 DCOL- 14 Pixel- 101
    139 Channel- 27 ROC- 5 DCOL- 3 Pixel- 49
    168 Channel- 10 ROC- 3 DCOL- 25 Pixel- 33
    171 Channel- 28 ROC- 6 DCOL- 5 Pixel- 16
    214 Channel- 28 ROC- 4 DCOL- 2 Pixel- 2
    227 Channel- 10 ROC- 2 DCOL- 19 Pixel- 7
    242 Channel- 28 ROC- 5 DCOL- 20 Pixel- 14
    246 Channel- 10 ROC- 3 DCOL- 25 Pixel- 63
    274 Channel- 28 ROC- 3 DCOL- 15 Pixel- 71
    294 Channel- 27 ROC- 5 DCOL- 25 Pixel- 85
    309 Channel- 10 ROC- 3 DCOL- 19 Pixel- 49
    314 Channel- 27 ROC- 4 DCOL- 23 Pixel- 10
    320 Channel- 10 ROC- 3 DCOL- 25 Pixel- 39
    330 Channel- 9 ROC- 4 DCOL- 21 Pixel- 36
    394 Channel- 10 ROC- 2 DCOL- 19 Pixel- 9
    431 Channel- 28 ROC- 5 DCOL- 21 Pixel- 46
    530 Channel- 10 ROC- 3 DCOL- 2 Pixel- 131
    539 Channel- 27 ROC- 1 DCOL- 0 Pixel- 82
   1063 Channel- 27 ROC- 1 DCOL- 20 Pixel- 49
   1088 Channel- 28 ROC- 5 DCOL- 25 Pixel- 67
   1179 Channel- 10 ROC- 3 DCOL- 21 Pixel- 34
   2117 Channel- 10 ROC- 4 DCOL- 6 Pixel- 76
   3309 Channel- 9 ROC- 6 DCOL- 21 Pixel- 130
   3343 Channel- 9 ROC- 7 DCOL- 23 Pixel- 59
   3487 Channel- 28 ROC- 0 DCOL- 18 Pixel- 42
   4324 Channel- 9 ROC- 6 DCOL- 21 Pixel- 134
   5687 Channel- 9 ROC- 5 DCOL- 24 Pixel- 87
'''.split('\n') if x.strip()]
to_mask = [[int(x) for x in line.split()[::2]] for line in s]

s = [x.strip() for x in '''
999964 27_4_23_12
999977 26_4_23_154
999981 10_1_7_145
130 3_2_24_73
170 33_0_23_73
181 33_0_16_95
251 33_0_0_102
329 33_0_16_63
359 26_4_19_154
375 33_0_21_91
407 33_0_8_50
591 33_0_5_110
684 25_4_10_5
1061 33_0_5_90
1130 33_0_14_85
1305 9_6_21_130
1336 33_0_2_97
1496 33_0_13_85
1868 33_0_7_84
1946 25_4_10_17
1997 33_0_8_26
2181 33_0_11_96
2319 33_0_4_79
376 31_2_20_109
377 9_6_21_134
471 27_6_4_69
519 28_3_15_69
'''.split('\n') if x.strip()]
to_mask = []
for line in s:
    a,b = line.split(' ')
    line = [a] + b.split('_')
    to_mask.append([int(x) for x in line])

hs = {}
def hist(roc):
    if hs.has_key(roc):
        return hs[roc]
    h = ROOT.TH2F(roc, roc, 52, 0, 52, 80, 0, 80)
    hs[roc] = h
    return h

def y():
    for count, channel, rocn, dc, pxl in to_mask:
        rocb, rocbn = nametranslation[channel]
        this_roc = rocb + '_ROC' + str(rocbn + rocn)
        this_col, this_row = dec(dc, pxl)
        yield count, channel, rocn, dc, pxl, this_roc, this_col, this_row

if 0:
    for count, channel, rocn, dc, pxl, this_roc, this_col, this_row in y():
        hist(this_roc).Fill(this_col, this_row, float(count))
    c = ROOT.TCanvas('c', '', 600, 600)
    for hn, h in hs.items():
        h.Draw('colz')
        c.SaveAs(hn + '.png')

def mask(roc_, col_, row_):
    for count, channel, rocn, dc, pxl, roc, col, row in y():
        if count >= 100 and roc_ == roc and col_ == col and row_ == row:
            return '0'
    return '1'

print 'loop'
for hc in 'OI':
    blds = (2,3) if hc == 'I' else (10,11)
    for bld in blds:
        for pnl in (1,2):
            f = open('ROC_Masks_module_Pilt_Bm%(hc)s_D3_BLD%(bld)i_PNL%(pnl)i.dat' % locals(), 'wt')
            for roc in xrange(16):
                roc_s = 'Pilt_Bm%(hc)s_D3_BLD%(bld)i_PNL%(pnl)i_PLQ1_ROC%(roc)i' % locals()
                roc_off = 'noInit' in detconfig.get(roc_s, '')
                print roc_s
                cols = []
                for colnum in xrange(52):
                    col = []
                    if roc_off:
                        col = '0'*80
                    else:
                        for row in xrange(80):
                            col.append(mask(roc_s, colnum, row))
                        col = ''.join(col)
                    col = 'col%02i:   %s' % (colnum, col)
                    cols.append(col)
                cols = '\n'.join(cols)
                f.write('ROC: %(roc_s)s\n%(cols)s\n' % locals())
