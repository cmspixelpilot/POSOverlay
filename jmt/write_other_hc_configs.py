# original map:
# doh
# 8 A -> fec 9 mfec 1 ch 1
# 8 B -> fec 9 mfec 1 ch 2
# 14 A -> fec 10 mfec 1 ch 1
# 14 B -> fec 10 mfec 1 ch 2
# poh
# 0041 -> fed id 1294 receiver b
# 0045 -> fed id 1294 receiver t
# 0047 -> fed id 1295 receiver b
# 0046 -> fed id 1295 receiver t
# 0043 -> fed id 1296 receiver b
# 0028 -> fed id 1296 receiver t
# 0035 -> fed id 1297 receiver b
# 0044 -> fed id 1297 receiver t
# 0027 -> fed id 1298 receiver b
# 0039 -> fed id 1298 receiver t
# 0052 -> fed id 1299 receiver b
# 0048 -> fed id 1299 receiver t
# 0081 -> fed id 1300 receiver b
# 0051 -> fed id 1300 receiver t

# hack:
# group 0, don't need to move anything, just plug in only DOH 8 A and B into fec 9 mfec 1 ch 1 and 2, and feds 1294-1296 + 0044 in 1297 receiver t
# group 1:
#  move mfec from fec 10 to fec 9
#  move feds 1298-1300 to 1294-1296 (keep 0035 in 1297 receiver b constant)

import sys, csv, os
from collections import defaultdict

group = 0
doug_dir = 'HC1_m20'
HC = 'BmI'
tkfecid = 'tkfec1'
tkfecring = 0

fiber_colors = ['pink-11', 'white-6', 'black-8', 'turquoise-12', 'green-3', 'brown-4', 'blue-1', 'gray-5', 'violet-10', 'yellow-9', 'red-7', 'orange-2']

# should-haves for sanity check
module_names_check = []
for disk in [1,2,3]:
    for pnl in [1,2]:
        for rng in [1,2]:
            if rng == 1:
                max_blade = 11
            else:
                max_blade = 17
            for bld in range(1, max_blade+1):
                module_names_check.append('FPix_%(HC)s_D%(disk)i_BLD%(bld)i_PNL%(pnl)i_RNG%(rng)i' % locals())
assert len(module_names_check) == 168
module_names_check.sort()

fn = sys.argv[1]
group = int(sys.argv[2])
assert group in [0,1]
f = open(fn)
r = csv.reader(f)
rows = list(r)
header = rows.pop(0)

class Module:
    def __init__(self, r):
        d = dict(zip(header, r))

        self.name = d['Official name of position']

        n = self.name.split('_')
        assert len(n) == 6
        assert n[1] == HC
        self.hc = n[1]

        def chompit(s, prefix, legal):
            assert s.startswith(prefix)
            v = int(s.replace(prefix, ''))
            assert(v in legal)
            return v

        self.disk = chompit(n[2], 'D', [1,2,3])
        self.rng  = chompit(n[5], 'RNG', [1,2])
        self.pnl  = chompit(n[4], 'PNL', [1,2])

        legal_blds = range(1, (17 if self.rng == 2 else 11)+1)
        self.bld  = chompit(n[3], 'BLD', legal_blds)
        
        n = self.internal_name = d['internal naming per Disk']
        n, n2 = n[:-1], n[-1]
        assert int(n) == self.bld
        assert (n2 == 'T' and self.pnl == 2) or (n2 == 'B' and self.pnl == 1)

        self.module = d['Module']
        self.hubid = int(d['HubID'])
        assert(0 <= self.hubid <= 15)

        hj = self.portcard_hj = d['PC position Mirror']
        assert int(hj[0]) == self.disk
        assert hj[1] in 'TB'
        assert hj[2] in 'ABCD'

        self.portcard_connection = int(d['PC connection'])
        self.portcard_identifier = d['PC identifier']

        self.dcdc = d['DCDC']
        self.dcdc_j = d['DCDC J11/J12']
        assert self.dcdc_j in ['J11', 'J12']

        self.doh_bundle = d['DOH bundle']
        self.doh = d['DOH A/B']
        assert self.doh in 'AB'

        self.ccu = '0x%02x' % int(d['CCU'], 16)
        assert self.ccu in '0x7b 0x7c 0x7d 0x7e'.split()
        self.ccu_channel = '0x%02x' % int(d['channel'], 16)
        assert self.ccu_channel in '0x10 0x11 0x12'.split()

        #self.poh_num = int(d['internal POH no'])
        self.poh_num = self.portcardstack * 7 + self.portcard_connection
        self.poh_fiber_str = d['POH fiber color']
        assert self.poh_fiber_str in fiber_colors
        self.poh_fiber = int(self.poh_fiber_str.split('-')[1])
        self.poh_bundle = int(d['POH Bundle'])
        self.poh_sn = d['POH SN']
        self.fed_channel_str = d['FED channel']
        self.fed_position = int(d['FED position']) # slot ?
        self.fed_receiver = 'bt'.index(d['FED receiver'])
        self.crate = d['FEC/FED crate']
        self.fed_id = int(d['FED ID'])
        self.fed_crate = 1
        assert self.fed_id - self.fed_position == 1293

        a,b = self.fed_channel_str.split('/')
        a,b = int(a), int(b)
        assert 1 <= a <= 48 and a % 2 == 1
        assert 1 <= b <= 48 and b % 2 == 0
        self.fed_channel = (a,b)

        # recv b 0 = fiber 1 -> fed ch 25/26
        b = (1-self.fed_receiver)*24
        n = self.poh_fiber
        self.xxx = (b + n*2 - 1, b + n*2)

        self.fec = int(d['FEC position'])
        assert self.fec in [9,10]
        self.mfec = int(d['mfec'])
        if self.fec == 9:
            assert self.mfec in [1,2,3,4]
        else:
            assert self.mfec in [1,2]
        self.mfecchannel = int(d['mfecchannel'])
        assert self.mfecchannel in [1,2]
        self.fec_crate = 1

    @property
    def portcardstack(self):
        return 'BT'.index(self.portcard_hj[1])

    @property
    def portcardnum(self):
        return 'ABCD'.index(self.portcard_hj[2]) + 1

    @property
    def portcard(self):
        return 'FPix_%s_D%i_PRT%i' % (self.hc, self.disk, self.portcardnum)

    @property
    def fec_ip(self):
        return 'pxfec%02i' % self.fec

    @property
    def fec_uri(self):
        return 'chtcp-2.0://localhost:10203?target=%s:50001' % self.fec_ip

    @property
    def fed_ip(self):
        return 'fed%02i' % self.fed_id

    @property
    def fed_uri(self):
        return 'chtcp-2.0://localhost:10203?target=%s:50001' % self.fed_ip


modules = [] 
modules_by_portcard_hj = defaultdict(list)
modules_by_portcard = defaultdict(list)
hubids_by_portcard = defaultdict(list)

# D1, 5 feds, 1 fec mezzanine hack
# and disconnected modules from too-short cables
def portcardOK(pc):
    #return True
    if group == 0:
        return '_D1_' in pc and ('_PRT1' in pc or '_PRT2' in pc)
    elif group == 1:
        return '_D1_' in pc and ('_PRT3' in pc or '_PRT4' in pc)
def moduleOK(m):
    #return True
    if m.disk != 1:
        return False
    grp = m.poh_bundle not in [1,2,3,4,5,6,7]
    if grp != group:
        return False
    not_connected = {
        'TB': [2,6],
        'TC': [6],
        'TD': [2,4],
        }        
    return m.portcard_connection not in not_connected.get(m.portcard_hj[-2:], [])

seen_poh = set()
seen_doh = set()
        
for r in rows:
    if not r[0].strip():
        assert(all(not x.strip() for x in r))
        continue

    m = Module(r)

    if m.poh_sn not in seen_poh:
        print m.poh_sn, '->', 'fed id %i receiver %s' % (m.fed_id, 'bt'[m.fed_receiver])
        seen_poh.add(m.poh_sn)
    if (m.doh_bundle, m.doh) not in seen_doh:
        print m.doh_bundle, m.doh, '->', 'fec %i mfec %i ch %i' % (m.fec, m.mfec, m.mfecchannel)
        seen_doh.add((m.doh_bundle, m.doh))

    # D1, 5 feds, 1 fec mezzanine hack
    if moduleOK(m):
        if m.fec == 10:
            assert group == 1
            m.fec = 9
            m.mfec = 2
        if m.fed_id in [1298, 1299, 1300]:
            m.fed_id -= 4
        
    modules.append(m)
    modules_by_portcard_hj[m.portcard_hj].append(m)
    modules_by_portcard[m.portcard].append(m)
    hubids_by_portcard[m.portcard].append(m.hubid)

modules.sort(key=lambda m: (m.disk, m.bld, m.pnl, m.rng))

# did we get all the right modules
module_names = [m.name for m in modules]
assert set(module_names_check) == set(module_names)

# check modules are unique
assert len(set(m.module for m in modules)) == len(modules)

# check number of portcards found
assert len(modules_by_portcard_hj) == 2 * 12
assert len(modules_by_portcard) == 12

# check number of modules per portcards found
assert set(len(x) for x in modules_by_portcard_hj.itervalues()) == set([7])
assert set(len(x) for x in modules_by_portcard.itervalues()) == set([14])

# are hubids unique per portcard stack
for pc, hubids in hubids_by_portcard.iteritems():
    assert len(set(hubids)) == 14

# portcard matching ok?
assert len(set((m.portcard_hj, m.portcard_identifier) for m in modules)) == 24
portcards = [(m.portcard, m.ccu, m.ccu_channel) for m in modules]
assert len(set(portcards)) == 12

# poh numbers unique?
assert len(set((m.portcard, m.poh_num) for m in modules)) == 12*14

#feds = sorted(set([(m.fed_id, m.fed_crate, m.fed_uri) for m in modules]))
#assert len(feds) == 7
feds_used = sorted(set([(m.fed_id, m.fed_crate, m.fed_uri) for m in modules if moduleOK(m)]))
assert len(feds_used) == 4

#fecs = sorted(set([(m.fec, m.fec_crate, m.fec_uri) for m in modules]))
#assert len(fecs) == 2
fecs_used = sorted(set([(m.fec, m.fec_crate, m.fec_uri) for m in modules if moduleOK(m)]))
assert len(fecs_used) == 1

t_portcard = '''Name: %(portcard)s
Type: p1fpix
TKFECID: %(tkfecid)s
ringAddress: %(tkfecring)i
ccuAddress: %(ccu)s
channelAddress: %(ccu_channel)s
i2cSpeed: 0x64
Delay25_GCR: 0xf0
Delay25_SCL: 0x43
Delay25_TRG: 0x6b
Delay25_SDA: 0x4a
Delay25_RCL: 0x63
Delay25_RDA: 0x4e
bPOH_Bias1: 0x22
bPOH_Bias2: 0x22
bPOH_Bias3: 0x22
bPOH_Bias4: 0x22
bPOH_Bias5: 0x22
bPOH_Bias6: 0x22
bPOH_Bias7: 0x22
bPOH_Gain123: 0x2a
bPOH_Gain4: 0x3f
bPOH_Gain567: 0x2a
tPOH_Bias1: 0x22
tPOH_Bias2: 0x22
tPOH_Bias3: 0x22
tPOH_Bias4: 0x22
tPOH_Bias5: 0x22
tPOH_Bias6: 0x22
tPOH_Bias7: 0x22
tPOH_Gain123: 0x2a
tPOH_Gain4: 0x3f
tPOH_Gain567: 0x2a
'''

path = os.path.join(HC, 'portcard')
if not os.path.isdir(path):
    os.makedirs(path)
for portcard, ccu, ccu_channel in portcards:
    fn = os.path.join(path, 'portcard_%s.dat' % portcard)
    open(fn, 'wt').write(t_portcard % locals())

t_dcdc = '''Enabled: no
CCUAddressEnable: 0x7e
CCUAddressPgood: 0x7d
PIAChannelAddress: 0x30
PortNumber: 0
'''

path = os.path.join(HC, 'dcdc')
if not os.path.isdir(path):
    os.makedirs(path)
for portcard, ccu, ccu_channel in portcards:
    fn = os.path.join(path, 'dcdc_%s.dat' % portcard)
    open(fn, 'wt').write(t_dcdc % locals())

path = os.path.join(HC, 'portcardmap')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'portcardmap.dat')
f = open(fn, 'wt')
f.write('# Portcard              Module                     AOH channel\n')
portcardmap = [(m,(m.portcard, m.name, m.poh_num)) for m in modules]
portcardmap.sort(key=lambda x: (x[1][0],x[1][2]))
for mm,m in portcardmap:
    if not moduleOK(mm) or not portcardOK(m[0]): continue
    f.write('%s\t%s A\t%s\n' % m)
    f.write('%s\t%s B\t%s\n' % m)
f.close()

path = os.path.join(HC, 'nametranslation')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'translation.dat')
f = open(fn, 'wt')
fmt = \
    '%-40s' \
    '%-6s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s' \
    '%-10s\n'
header = tuple('#name A/B FEC mfec mfecch hubid port rocid FEDid FEDch roc#'.split())
f.write(fmt % header)
for m in modules:
    if not moduleOK(m): continue
    for iroc in xrange(16):
        fields = (
            m.name + '_ROC' + str(iroc), 
            'AB'[iroc / 8],
            m.fec,
            m.mfec,
            m.mfecchannel,
            m.hubid,
            iroc / 4,
            iroc,
            m.fed_id,
            m.fed_channel[iroc / 8],
            iroc % 8,
            )
        line = fmt % fields
        f.write(line)
    f.write('\n')
f.close()

path = os.path.join(HC, 'detconfig')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'detectconfig.dat')
f = open(fn, 'wt')
f.write('Rocs:\n')
for m in modules:
    if not moduleOK(m): continue
    for iroc in xrange(16):
        f.write('%s_ROC%i\n' % (m.name, iroc))
f.close()

path = os.path.join(HC, 'maxvsf')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'maxvsf.dat')
f = open(fn, 'wt')
for m in modules:
    f.write('%s\t180\n' % m.name)
f.close()

path = os.path.join(HC, 'lowvoltagemap')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'lowvoltagemap.dat')
f = open(fn, 'wt')
for m in modules:
    f.write('%s\tCAEN/CMS_TRACKER_SY1527_5/branchControllerXX/easyCrateY/easyBoardZZ   channel00W   channel00V\n' % m.name)
f.close()

path = os.path.join(HC, 'fedconfig')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'fedconfig.dat')
f = open(fn, 'wt')
f.write('#FED number     crate     vme base address     type    URI\n')
for fed_id, fed_crate, fed_uri in feds_used:
    f.write('%s\t%s\t%s\tCTA\t%s\n' % (fed_id, fed_crate, fed_id, fed_uri))
f.close()

path = os.path.join(HC, 'fecconfig')
if not os.path.isdir(path):
    os.makedirs(path)
fn = os.path.join(path, 'fecconfig.dat')
f = open(fn, 'wt')
f.write('#FEC number     crate     vme base address     type    URI\n')
for fec_id, fec_crate, fec_uri in fecs_used:
    f.write('%s\t%s\t%s\tCTA\t%s\n' % (fec_id, fec_crate, fec_id, fec_uri))
f.close()

t_fedcard = '''Type: CTA
Control bits: 0xFFFFFFFFFFFFFFFF
Control bits override: 0
Transparent+scope channel: 12
PACKET_NB: 1
Which FMC (lower = 0): 0
Fitel channel order swapped: 1
Timeout checking enabled: 0
Timeout counter start: 10
Timeout number OOS threshold: 255
FED Base address                         :0x%(fed_id)x
FEDID Number                             :0x%(fed_id)x
Number of ROCs Chnl 1:8
Number of ROCs Chnl 2:8
Number of ROCs Chnl 3:8
Number of ROCs Chnl 4:8
Number of ROCs Chnl 5:8
Number of ROCs Chnl 6:8
Number of ROCs Chnl 7:8
Number of ROCs Chnl 8:8
Number of ROCs Chnl 9:8
Number of ROCs Chnl 10:8
Number of ROCs Chnl 11:8
Number of ROCs Chnl 12:8
Number of ROCs Chnl 13:8
Number of ROCs Chnl 14:8
Number of ROCs Chnl 15:8
Number of ROCs Chnl 16:8
Number of ROCs Chnl 17:8
Number of ROCs Chnl 18:8
Number of ROCs Chnl 19:8
Number of ROCs Chnl 20:8
Number of ROCs Chnl 21:8
Number of ROCs Chnl 22:8
Number of ROCs Chnl 23:8
Number of ROCs Chnl 24:8
Number of ROCs Chnl 25:8
Number of ROCs Chnl 26:8
Number of ROCs Chnl 27:8
Number of ROCs Chnl 28:8
Number of ROCs Chnl 29:8
Number of ROCs Chnl 30:8
Number of ROCs Chnl 31:8
Number of ROCs Chnl 32:8
Number of ROCs Chnl 33:8
Number of ROCs Chnl 34:8
Number of ROCs Chnl 35:8
Number of ROCs Chnl 36:8
Number of ROCs Chnl 37:8
Number of ROCs Chnl 38:8
Number of ROCs Chnl 39:8
Number of ROCs Chnl 40:8
Number of ROCs Chnl 41:8
Number of ROCs Chnl 42:8
Number of ROCs Chnl 43:8
Number of ROCs Chnl 44:8
Number of ROCs Chnl 45:8
Number of ROCs Chnl 46:8
Number of ROCs Chnl 47:8
Number of ROCs Chnl 48:8
Optical reciever 1  Capacitor Adjust(0-3):0
Optical reciever 2  Capacitor Adjust(0-3):0
Optical reciever 3  Capacitor Adjust(0-3):0
Optical reciever 1  Input Offset (0-15)  :10
Optical reciever 2  Input Offset (0-15)  :10
Optical reciever 3  Input Offset (0-15)  :10
Optical reciever 1 Output Offset (0-3)   :0
Optical reciever 2 Output Offset (0-3)   :0
Optical reciever 3 Output Offset (0-3)   :0
Offset DAC channel 1:0
Offset DAC channel 2:0
Offset DAC channel 3:0
Offset DAC channel 4:0
Offset DAC channel 5:0
Offset DAC channel 6:0
Offset DAC channel 7:0
Offset DAC channel 8:0
Offset DAC channel 9:0
Offset DAC channel 10:0
Offset DAC channel 11:0
Offset DAC channel 12:0
Offset DAC channel 13:0
Offset DAC channel 14:0
Offset DAC channel 15:0
Offset DAC channel 16:0
Offset DAC channel 17:0
Offset DAC channel 18:0
Offset DAC channel 19:0
Offset DAC channel 20:0
Offset DAC channel 21:0
Offset DAC channel 22:0
Offset DAC channel 23:0
Offset DAC channel 24:0
Offset DAC channel 25:0
Offset DAC channel 26:0
Offset DAC channel 27:0
Offset DAC channel 28:0
Offset DAC channel 29:0
Offset DAC channel 30:0
Offset DAC channel 31:0
Offset DAC channel 32:0
Offset DAC channel 33:0
Offset DAC channel 34:0
Offset DAC channel 35:0
Offset DAC channel 36:0
Clock Phase Bits ch   1-9:0x1ff
Clock Phase Bits ch 10-18:0x1ff
Clock Phase Bits ch 19-27:0x1ff
Clock Phase Bits ch 28-36:0x1ff
Black HiThold ch 1:400 
Black LoThold ch 1:250 
ULblack Thold ch 1:120 
Black HiThold ch 2:400 
Black LoThold ch 2:250 
ULblack Thold ch 2:120 
Black HiThold ch 3:400 
Black LoThold ch 3:250 
ULblack Thold ch 3:120 
Black HiThold ch 4:400 
Black LoThold ch 4:250 
ULblack Thold ch 4:120 
Black HiThold ch 5:400 
Black LoThold ch 5:250 
ULblack Thold ch 5:120 
Black HiThold ch 6:400 
Black LoThold ch 6:250 
ULblack Thold ch 6:120 
Black HiThold ch 7:400 
Black LoThold ch 7:250 
ULblack Thold ch 7:120 
Black HiThold ch 8:400 
Black LoThold ch 8:250 
ULblack Thold ch 8:120 
Black HiThold ch 9:400 
Black LoThold ch 9:250 
ULblack Thold ch 9:120 
Black HiThold ch 10:400 
Black LoThold ch 10:250 
ULblack Thold ch 10:120 
Black HiThold ch 11:400 
Black LoThold ch 11:250 
ULblack Thold ch 11:120 
Black HiThold ch 12:400 
Black LoThold ch 12:250 
ULblack Thold ch 12:120 
Black HiThold ch 13:400 
Black LoThold ch 13:250 
ULblack Thold ch 13:120 
Black HiThold ch 14:400 
Black LoThold ch 14:250 
ULblack Thold ch 14:120 
Black HiThold ch 15:400 
Black LoThold ch 15:250 
ULblack Thold ch 15:120 
Black HiThold ch 16:400 
Black LoThold ch 16:250 
ULblack Thold ch 16:120 
Black HiThold ch 17:400 
Black LoThold ch 17:250 
ULblack Thold ch 17:120 
Black HiThold ch 18:400 
Black LoThold ch 18:250 
ULblack Thold ch 18:120 
Black HiThold ch 19:400 
Black LoThold ch 19:250 
ULblack Thold ch 19:120 
Black HiThold ch 20:400 
Black LoThold ch 20:250 
ULblack Thold ch 20:120 
Black HiThold ch 21:400 
Black LoThold ch 21:250 
ULblack Thold ch 21:120 
Black HiThold ch 22:400 
Black LoThold ch 22:250 
ULblack Thold ch 22:120 
Black HiThold ch 23:400 
Black LoThold ch 23:250 
ULblack Thold ch 23:120 
Black HiThold ch 24:400 
Black LoThold ch 24:250 
ULblack Thold ch 24:120 
Black HiThold ch 25:400 
Black LoThold ch 25:250 
ULblack Thold ch 25:120 
Black HiThold ch 26:400 
Black LoThold ch 26:250 
ULblack Thold ch 26:120 
Black HiThold ch 27:400 
Black LoThold ch 27:250 
ULblack Thold ch 27:120 
Black HiThold ch 28:400 
Black LoThold ch 28:250 
ULblack Thold ch 28:120 
Black HiThold ch 29:400 
Black LoThold ch 29:250 
ULblack Thold ch 29:120 
Black HiThold ch 30:400 
Black LoThold ch 30:250 
ULblack Thold ch 30:120 
Black HiThold ch 31:400 
Black LoThold ch 31:250 
ULblack Thold ch 31:120 
Black HiThold ch 32:400 
Black LoThold ch 32:250 
ULblack Thold ch 32:120 
Black HiThold ch 33:400 
Black LoThold ch 33:250 
ULblack Thold ch 33:120 
Black HiThold ch 34:400 
Black LoThold ch 34:250 
ULblack Thold ch 34:120 
Black HiThold ch 35:400 
Black LoThold ch 35:250 
ULblack Thold ch 35:120 
Black HiThold ch 36:400 
Black LoThold ch 36:250 
ULblack Thold ch 36:120 
Delay channel 1(0-15):3
Delay channel 2(0-15):3
Delay channel 3(0-15):3
Delay channel 4(0-15):3
Delay channel 5(0-15):3
Delay channel 6(0-15):3
Delay channel 7(0-15):3
Delay channel 8(0-15):3
Delay channel 9(0-15):3
Delay channel 10(0-15):3
Delay channel 11(0-15):3
Delay channel 12(0-15):3
Delay channel 13(0-15):3
Delay channel 14(0-15):3
Delay channel 15(0-15):3
Delay channel 16(0-15):3
Delay channel 17(0-15):3
Delay channel 18(0-15):3
Delay channel 19(0-15):3
Delay channel 20(0-15):3
Delay channel 21(0-15):3
Delay channel 22(0-15):3
Delay channel 23(0-15):3
Delay channel 24(0-15):3
Delay channel 25(0-15):3
Delay channel 26(0-15):3
Delay channel 27(0-15):3
Delay channel 28(0-15):3
Delay channel 29(0-15):3
Delay channel 30(0-15):3
Delay channel 31(0-15):3
Delay channel 32(0-15):3
Delay channel 33(0-15):3
Delay channel 34(0-15):3
Delay channel 35(0-15):3
Delay channel 36(0-15):3
TBM level 0 Channel  1:270
TBM level 1 Channel  1:400
TBM level 2 Channel  1:500
TBM level 3 Channel  1:600
TBM level 4 Channel  1:700
ROC0 level 0 Channel  1 :270
ROC0 level 1 Channel  1 :400
ROC0 level 2 Channel  1 :500
ROC0 level 3 Channel  1 :600
ROC0 level 4 Channel  1 :700
ROC1 level 0 Channel  1 :270
ROC1 level 1 Channel  1 :400
ROC1 level 2 Channel  1 :500
ROC1 level 3 Channel  1 :600
ROC1 level 4 Channel  1 :700
ROC2 level 0 Channel  1 :270
ROC2 level 1 Channel  1 :400
ROC2 level 2 Channel  1 :500
ROC2 level 3 Channel  1 :600
ROC2 level 4 Channel  1 :700
ROC3 level 0 Channel  1 :270
ROC3 level 1 Channel  1 :400
ROC3 level 2 Channel  1 :500
ROC3 level 3 Channel  1 :600
ROC3 level 4 Channel  1 :700
ROC4 level 0 Channel  1 :270
ROC4 level 1 Channel  1 :400
ROC4 level 2 Channel  1 :500
ROC4 level 3 Channel  1 :600
ROC4 level 4 Channel  1 :700
ROC5 level 0 Channel  1 :270
ROC5 level 1 Channel  1 :400
ROC5 level 2 Channel  1 :500
ROC5 level 3 Channel  1 :600
ROC5 level 4 Channel  1 :700
ROC6 level 0 Channel  1 :270
ROC6 level 1 Channel  1 :400
ROC6 level 2 Channel  1 :500
ROC6 level 3 Channel  1 :600
ROC6 level 4 Channel  1 :700
ROC7 level 0 Channel  1 :270
ROC7 level 1 Channel  1 :400
ROC7 level 2 Channel  1 :500
ROC7 level 3 Channel  1 :600
ROC7 level 4 Channel  1 :700
TRLR level 0 Channel 1:270
TRLR level 1 Channel 1:400
TRLR level 2 Channel 1:500
TRLR level 3 Channel 1:600
TRLR level 4 Channel 1:700
TBM level 0 Channel  2:270
TBM level 1 Channel  2:400
TBM level 2 Channel  2:500
TBM level 3 Channel  2:600
TBM level 4 Channel  2:700
ROC0 level 0 Channel  2 :270
ROC0 level 1 Channel  2 :400
ROC0 level 2 Channel  2 :500
ROC0 level 3 Channel  2 :600
ROC0 level 4 Channel  2 :700
ROC1 level 0 Channel  2 :270
ROC1 level 1 Channel  2 :400
ROC1 level 2 Channel  2 :500
ROC1 level 3 Channel  2 :600
ROC1 level 4 Channel  2 :700
ROC2 level 0 Channel  2 :270
ROC2 level 1 Channel  2 :400
ROC2 level 2 Channel  2 :500
ROC2 level 3 Channel  2 :600
ROC2 level 4 Channel  2 :700
ROC3 level 0 Channel  2 :270
ROC3 level 1 Channel  2 :400
ROC3 level 2 Channel  2 :500
ROC3 level 3 Channel  2 :600
ROC3 level 4 Channel  2 :700
ROC4 level 0 Channel  2 :270
ROC4 level 1 Channel  2 :400
ROC4 level 2 Channel  2 :500
ROC4 level 3 Channel  2 :600
ROC4 level 4 Channel  2 :700
ROC5 level 0 Channel  2 :270
ROC5 level 1 Channel  2 :400
ROC5 level 2 Channel  2 :500
ROC5 level 3 Channel  2 :600
ROC5 level 4 Channel  2 :700
ROC6 level 0 Channel  2 :270
ROC6 level 1 Channel  2 :400
ROC6 level 2 Channel  2 :500
ROC6 level 3 Channel  2 :600
ROC6 level 4 Channel  2 :700
ROC7 level 0 Channel  2 :270
ROC7 level 1 Channel  2 :400
ROC7 level 2 Channel  2 :500
ROC7 level 3 Channel  2 :600
ROC7 level 4 Channel  2 :700
TRLR level 0 Channel 2:270
TRLR level 1 Channel 2:400
TRLR level 2 Channel 2:500
TRLR level 3 Channel 2:600
TRLR level 4 Channel 2:700
TBM level 0 Channel  3:270
TBM level 1 Channel  3:400
TBM level 2 Channel  3:500
TBM level 3 Channel  3:600
TBM level 4 Channel  3:700
ROC0 level 0 Channel  3 :270
ROC0 level 1 Channel  3 :400
ROC0 level 2 Channel  3 :500
ROC0 level 3 Channel  3 :600
ROC0 level 4 Channel  3 :700
ROC1 level 0 Channel  3 :270
ROC1 level 1 Channel  3 :400
ROC1 level 2 Channel  3 :500
ROC1 level 3 Channel  3 :600
ROC1 level 4 Channel  3 :700
ROC2 level 0 Channel  3 :270
ROC2 level 1 Channel  3 :400
ROC2 level 2 Channel  3 :500
ROC2 level 3 Channel  3 :600
ROC2 level 4 Channel  3 :700
ROC3 level 0 Channel  3 :270
ROC3 level 1 Channel  3 :400
ROC3 level 2 Channel  3 :500
ROC3 level 3 Channel  3 :600
ROC3 level 4 Channel  3 :700
ROC4 level 0 Channel  3 :270
ROC4 level 1 Channel  3 :400
ROC4 level 2 Channel  3 :500
ROC4 level 3 Channel  3 :600
ROC4 level 4 Channel  3 :700
ROC5 level 0 Channel  3 :270
ROC5 level 1 Channel  3 :400
ROC5 level 2 Channel  3 :500
ROC5 level 3 Channel  3 :600
ROC5 level 4 Channel  3 :700
ROC6 level 0 Channel  3 :270
ROC6 level 1 Channel  3 :400
ROC6 level 2 Channel  3 :500
ROC6 level 3 Channel  3 :600
ROC6 level 4 Channel  3 :700
ROC7 level 0 Channel  3 :270
ROC7 level 1 Channel  3 :400
ROC7 level 2 Channel  3 :500
ROC7 level 3 Channel  3 :600
ROC7 level 4 Channel  3 :700
TRLR level 0 Channel 3:270
TRLR level 1 Channel 3:400
TRLR level 2 Channel 3:500
TRLR level 3 Channel 3:600
TRLR level 4 Channel 3:700
TBM level 0 Channel  4:270
TBM level 1 Channel  4:400
TBM level 2 Channel  4:500
TBM level 3 Channel  4:600
TBM level 4 Channel  4:700
ROC0 level 0 Channel  4 :270
ROC0 level 1 Channel  4 :400
ROC0 level 2 Channel  4 :500
ROC0 level 3 Channel  4 :600
ROC0 level 4 Channel  4 :700
ROC1 level 0 Channel  4 :270
ROC1 level 1 Channel  4 :400
ROC1 level 2 Channel  4 :500
ROC1 level 3 Channel  4 :600
ROC1 level 4 Channel  4 :700
ROC2 level 0 Channel  4 :270
ROC2 level 1 Channel  4 :400
ROC2 level 2 Channel  4 :500
ROC2 level 3 Channel  4 :600
ROC2 level 4 Channel  4 :700
ROC3 level 0 Channel  4 :270
ROC3 level 1 Channel  4 :400
ROC3 level 2 Channel  4 :500
ROC3 level 3 Channel  4 :600
ROC3 level 4 Channel  4 :700
ROC4 level 0 Channel  4 :270
ROC4 level 1 Channel  4 :400
ROC4 level 2 Channel  4 :500
ROC4 level 3 Channel  4 :600
ROC4 level 4 Channel  4 :700
ROC5 level 0 Channel  4 :270
ROC5 level 1 Channel  4 :400
ROC5 level 2 Channel  4 :500
ROC5 level 3 Channel  4 :600
ROC5 level 4 Channel  4 :700
ROC6 level 0 Channel  4 :270
ROC6 level 1 Channel  4 :400
ROC6 level 2 Channel  4 :500
ROC6 level 3 Channel  4 :600
ROC6 level 4 Channel  4 :700
ROC7 level 0 Channel  4 :270
ROC7 level 1 Channel  4 :400
ROC7 level 2 Channel  4 :500
ROC7 level 3 Channel  4 :600
ROC7 level 4 Channel  4 :700
TRLR level 0 Channel 4:270
TRLR level 1 Channel 4:400
TRLR level 2 Channel 4:500
TRLR level 3 Channel 4:600
TRLR level 4 Channel 4:700
TBM level 0 Channel  5:270
TBM level 1 Channel  5:400
TBM level 2 Channel  5:500
TBM level 3 Channel  5:600
TBM level 4 Channel  5:700
ROC0 level 0 Channel  5 :270
ROC0 level 1 Channel  5 :400
ROC0 level 2 Channel  5 :500
ROC0 level 3 Channel  5 :600
ROC0 level 4 Channel  5 :700
ROC1 level 0 Channel  5 :270
ROC1 level 1 Channel  5 :400
ROC1 level 2 Channel  5 :500
ROC1 level 3 Channel  5 :600
ROC1 level 4 Channel  5 :700
ROC2 level 0 Channel  5 :270
ROC2 level 1 Channel  5 :400
ROC2 level 2 Channel  5 :500
ROC2 level 3 Channel  5 :600
ROC2 level 4 Channel  5 :700
ROC3 level 0 Channel  5 :270
ROC3 level 1 Channel  5 :400
ROC3 level 2 Channel  5 :500
ROC3 level 3 Channel  5 :600
ROC3 level 4 Channel  5 :700
ROC4 level 0 Channel  5 :270
ROC4 level 1 Channel  5 :400
ROC4 level 2 Channel  5 :500
ROC4 level 3 Channel  5 :600
ROC4 level 4 Channel  5 :700
ROC5 level 0 Channel  5 :270
ROC5 level 1 Channel  5 :400
ROC5 level 2 Channel  5 :500
ROC5 level 3 Channel  5 :600
ROC5 level 4 Channel  5 :700
ROC6 level 0 Channel  5 :270
ROC6 level 1 Channel  5 :400
ROC6 level 2 Channel  5 :500
ROC6 level 3 Channel  5 :600
ROC6 level 4 Channel  5 :700
ROC7 level 0 Channel  5 :270
ROC7 level 1 Channel  5 :400
ROC7 level 2 Channel  5 :500
ROC7 level 3 Channel  5 :600
ROC7 level 4 Channel  5 :700
TRLR level 0 Channel 5:270
TRLR level 1 Channel 5:400
TRLR level 2 Channel 5:500
TRLR level 3 Channel 5:600
TRLR level 4 Channel 5:700
TBM level 0 Channel  6:270
TBM level 1 Channel  6:400
TBM level 2 Channel  6:500
TBM level 3 Channel  6:600
TBM level 4 Channel  6:700
ROC0 level 0 Channel  6 :270
ROC0 level 1 Channel  6 :400
ROC0 level 2 Channel  6 :500
ROC0 level 3 Channel  6 :600
ROC0 level 4 Channel  6 :700
ROC1 level 0 Channel  6 :270
ROC1 level 1 Channel  6 :400
ROC1 level 2 Channel  6 :500
ROC1 level 3 Channel  6 :600
ROC1 level 4 Channel  6 :700
ROC2 level 0 Channel  6 :270
ROC2 level 1 Channel  6 :400
ROC2 level 2 Channel  6 :500
ROC2 level 3 Channel  6 :600
ROC2 level 4 Channel  6 :700
ROC3 level 0 Channel  6 :270
ROC3 level 1 Channel  6 :400
ROC3 level 2 Channel  6 :500
ROC3 level 3 Channel  6 :600
ROC3 level 4 Channel  6 :700
ROC4 level 0 Channel  6 :270
ROC4 level 1 Channel  6 :400
ROC4 level 2 Channel  6 :500
ROC4 level 3 Channel  6 :600
ROC4 level 4 Channel  6 :700
ROC5 level 0 Channel  6 :270
ROC5 level 1 Channel  6 :400
ROC5 level 2 Channel  6 :500
ROC5 level 3 Channel  6 :600
ROC5 level 4 Channel  6 :700
ROC6 level 0 Channel  6 :270
ROC6 level 1 Channel  6 :400
ROC6 level 2 Channel  6 :500
ROC6 level 3 Channel  6 :600
ROC6 level 4 Channel  6 :700
ROC7 level 0 Channel  6 :270
ROC7 level 1 Channel  6 :400
ROC7 level 2 Channel  6 :500
ROC7 level 3 Channel  6 :600
ROC7 level 4 Channel  6 :700
TRLR level 0 Channel 6:270
TRLR level 1 Channel 6:400
TRLR level 2 Channel 6:500
TRLR level 3 Channel 6:600
TRLR level 4 Channel 6:700
TBM level 0 Channel  7:270
TBM level 1 Channel  7:400
TBM level 2 Channel  7:500
TBM level 3 Channel  7:600
TBM level 4 Channel  7:700
ROC0 level 0 Channel  7 :270
ROC0 level 1 Channel  7 :400
ROC0 level 2 Channel  7 :500
ROC0 level 3 Channel  7 :600
ROC0 level 4 Channel  7 :700
ROC1 level 0 Channel  7 :270
ROC1 level 1 Channel  7 :400
ROC1 level 2 Channel  7 :500
ROC1 level 3 Channel  7 :600
ROC1 level 4 Channel  7 :700
ROC2 level 0 Channel  7 :270
ROC2 level 1 Channel  7 :400
ROC2 level 2 Channel  7 :500
ROC2 level 3 Channel  7 :600
ROC2 level 4 Channel  7 :700
ROC3 level 0 Channel  7 :270
ROC3 level 1 Channel  7 :400
ROC3 level 2 Channel  7 :500
ROC3 level 3 Channel  7 :600
ROC3 level 4 Channel  7 :700
ROC4 level 0 Channel  7 :270
ROC4 level 1 Channel  7 :400
ROC4 level 2 Channel  7 :500
ROC4 level 3 Channel  7 :600
ROC4 level 4 Channel  7 :700
ROC5 level 0 Channel  7 :270
ROC5 level 1 Channel  7 :400
ROC5 level 2 Channel  7 :500
ROC5 level 3 Channel  7 :600
ROC5 level 4 Channel  7 :700
ROC6 level 0 Channel  7 :270
ROC6 level 1 Channel  7 :400
ROC6 level 2 Channel  7 :500
ROC6 level 3 Channel  7 :600
ROC6 level 4 Channel  7 :700
ROC7 level 0 Channel  7 :270
ROC7 level 1 Channel  7 :400
ROC7 level 2 Channel  7 :500
ROC7 level 3 Channel  7 :600
ROC7 level 4 Channel  7 :700
TRLR level 0 Channel 7:270
TRLR level 1 Channel 7:400
TRLR level 2 Channel 7:500
TRLR level 3 Channel 7:600
TRLR level 4 Channel 7:700
TBM level 0 Channel  8:270
TBM level 1 Channel  8:400
TBM level 2 Channel  8:500
TBM level 3 Channel  8:600
TBM level 4 Channel  8:700
ROC0 level 0 Channel  8 :270
ROC0 level 1 Channel  8 :400
ROC0 level 2 Channel  8 :500
ROC0 level 3 Channel  8 :600
ROC0 level 4 Channel  8 :700
ROC1 level 0 Channel  8 :270
ROC1 level 1 Channel  8 :400
ROC1 level 2 Channel  8 :500
ROC1 level 3 Channel  8 :600
ROC1 level 4 Channel  8 :700
ROC2 level 0 Channel  8 :270
ROC2 level 1 Channel  8 :400
ROC2 level 2 Channel  8 :500
ROC2 level 3 Channel  8 :600
ROC2 level 4 Channel  8 :700
ROC3 level 0 Channel  8 :270
ROC3 level 1 Channel  8 :400
ROC3 level 2 Channel  8 :500
ROC3 level 3 Channel  8 :600
ROC3 level 4 Channel  8 :700
ROC4 level 0 Channel  8 :270
ROC4 level 1 Channel  8 :400
ROC4 level 2 Channel  8 :500
ROC4 level 3 Channel  8 :600
ROC4 level 4 Channel  8 :700
ROC5 level 0 Channel  8 :270
ROC5 level 1 Channel  8 :400
ROC5 level 2 Channel  8 :500
ROC5 level 3 Channel  8 :600
ROC5 level 4 Channel  8 :700
ROC6 level 0 Channel  8 :270
ROC6 level 1 Channel  8 :400
ROC6 level 2 Channel  8 :500
ROC6 level 3 Channel  8 :600
ROC6 level 4 Channel  8 :700
ROC7 level 0 Channel  8 :270
ROC7 level 1 Channel  8 :400
ROC7 level 2 Channel  8 :500
ROC7 level 3 Channel  8 :600
ROC7 level 4 Channel  8 :700
TRLR level 0 Channel 8:270
TRLR level 1 Channel 8:400
TRLR level 2 Channel 8:500
TRLR level 3 Channel 8:600
TRLR level 4 Channel 8:700
TBM level 0 Channel  9:270
TBM level 1 Channel  9:400
TBM level 2 Channel  9:500
TBM level 3 Channel  9:600
TBM level 4 Channel  9:700
ROC0 level 0 Channel  9 :270
ROC0 level 1 Channel  9 :400
ROC0 level 2 Channel  9 :500
ROC0 level 3 Channel  9 :600
ROC0 level 4 Channel  9 :700
ROC1 level 0 Channel  9 :270
ROC1 level 1 Channel  9 :400
ROC1 level 2 Channel  9 :500
ROC1 level 3 Channel  9 :600
ROC1 level 4 Channel  9 :700
ROC2 level 0 Channel  9 :270
ROC2 level 1 Channel  9 :400
ROC2 level 2 Channel  9 :500
ROC2 level 3 Channel  9 :600
ROC2 level 4 Channel  9 :700
ROC3 level 0 Channel  9 :270
ROC3 level 1 Channel  9 :400
ROC3 level 2 Channel  9 :500
ROC3 level 3 Channel  9 :600
ROC3 level 4 Channel  9 :700
ROC4 level 0 Channel  9 :270
ROC4 level 1 Channel  9 :400
ROC4 level 2 Channel  9 :500
ROC4 level 3 Channel  9 :600
ROC4 level 4 Channel  9 :700
ROC5 level 0 Channel  9 :270
ROC5 level 1 Channel  9 :400
ROC5 level 2 Channel  9 :500
ROC5 level 3 Channel  9 :600
ROC5 level 4 Channel  9 :700
ROC6 level 0 Channel  9 :270
ROC6 level 1 Channel  9 :400
ROC6 level 2 Channel  9 :500
ROC6 level 3 Channel  9 :600
ROC6 level 4 Channel  9 :700
ROC7 level 0 Channel  9 :270
ROC7 level 1 Channel  9 :400
ROC7 level 2 Channel  9 :500
ROC7 level 3 Channel  9 :600
ROC7 level 4 Channel  9 :700
TRLR level 0 Channel 9:270
TRLR level 1 Channel 9:400
TRLR level 2 Channel 9:500
TRLR level 3 Channel 9:600
TRLR level 4 Channel 9:700
TBM level 0 Channel  10:270
TBM level 1 Channel  10:400
TBM level 2 Channel  10:500
TBM level 3 Channel  10:600
TBM level 4 Channel  10:700
ROC0 level 0 Channel  10 :270
ROC0 level 1 Channel  10 :400
ROC0 level 2 Channel  10 :500
ROC0 level 3 Channel  10 :600
ROC0 level 4 Channel  10 :700
ROC1 level 0 Channel  10 :270
ROC1 level 1 Channel  10 :400
ROC1 level 2 Channel  10 :500
ROC1 level 3 Channel  10 :600
ROC1 level 4 Channel  10 :700
ROC2 level 0 Channel  10 :270
ROC2 level 1 Channel  10 :400
ROC2 level 2 Channel  10 :500
ROC2 level 3 Channel  10 :600
ROC2 level 4 Channel  10 :700
ROC3 level 0 Channel  10 :270
ROC3 level 1 Channel  10 :400
ROC3 level 2 Channel  10 :500
ROC3 level 3 Channel  10 :600
ROC3 level 4 Channel  10 :700
ROC4 level 0 Channel  10 :270
ROC4 level 1 Channel  10 :400
ROC4 level 2 Channel  10 :500
ROC4 level 3 Channel  10 :600
ROC4 level 4 Channel  10 :700
ROC5 level 0 Channel  10 :270
ROC5 level 1 Channel  10 :400
ROC5 level 2 Channel  10 :500
ROC5 level 3 Channel  10 :600
ROC5 level 4 Channel  10 :700
ROC6 level 0 Channel  10 :270
ROC6 level 1 Channel  10 :400
ROC6 level 2 Channel  10 :500
ROC6 level 3 Channel  10 :600
ROC6 level 4 Channel  10 :700
ROC7 level 0 Channel  10 :270
ROC7 level 1 Channel  10 :400
ROC7 level 2 Channel  10 :500
ROC7 level 3 Channel  10 :600
ROC7 level 4 Channel  10 :700
TRLR level 0 Channel 10:270
TRLR level 1 Channel 10:400
TRLR level 2 Channel 10:500
TRLR level 3 Channel 10:600
TRLR level 4 Channel 10:700
TBM level 0 Channel  11:270
TBM level 1 Channel  11:400
TBM level 2 Channel  11:500
TBM level 3 Channel  11:600
TBM level 4 Channel  11:700
ROC0 level 0 Channel  11 :270
ROC0 level 1 Channel  11 :400
ROC0 level 2 Channel  11 :500
ROC0 level 3 Channel  11 :600
ROC0 level 4 Channel  11 :700
ROC1 level 0 Channel  11 :270
ROC1 level 1 Channel  11 :400
ROC1 level 2 Channel  11 :500
ROC1 level 3 Channel  11 :600
ROC1 level 4 Channel  11 :700
ROC2 level 0 Channel  11 :270
ROC2 level 1 Channel  11 :400
ROC2 level 2 Channel  11 :500
ROC2 level 3 Channel  11 :600
ROC2 level 4 Channel  11 :700
ROC3 level 0 Channel  11 :270
ROC3 level 1 Channel  11 :400
ROC3 level 2 Channel  11 :500
ROC3 level 3 Channel  11 :600
ROC3 level 4 Channel  11 :700
ROC4 level 0 Channel  11 :270
ROC4 level 1 Channel  11 :400
ROC4 level 2 Channel  11 :500
ROC4 level 3 Channel  11 :600
ROC4 level 4 Channel  11 :700
ROC5 level 0 Channel  11 :270
ROC5 level 1 Channel  11 :400
ROC5 level 2 Channel  11 :500
ROC5 level 3 Channel  11 :600
ROC5 level 4 Channel  11 :700
ROC6 level 0 Channel  11 :270
ROC6 level 1 Channel  11 :400
ROC6 level 2 Channel  11 :500
ROC6 level 3 Channel  11 :600
ROC6 level 4 Channel  11 :700
ROC7 level 0 Channel  11 :270
ROC7 level 1 Channel  11 :400
ROC7 level 2 Channel  11 :500
ROC7 level 3 Channel  11 :600
ROC7 level 4 Channel  11 :700
TRLR level 0 Channel 11:270
TRLR level 1 Channel 11:400
TRLR level 2 Channel 11:500
TRLR level 3 Channel 11:600
TRLR level 4 Channel 11:700
TBM level 0 Channel  12:270
TBM level 1 Channel  12:400
TBM level 2 Channel  12:500
TBM level 3 Channel  12:600
TBM level 4 Channel  12:700
ROC0 level 0 Channel  12 :270
ROC0 level 1 Channel  12 :400
ROC0 level 2 Channel  12 :500
ROC0 level 3 Channel  12 :600
ROC0 level 4 Channel  12 :700
ROC1 level 0 Channel  12 :270
ROC1 level 1 Channel  12 :400
ROC1 level 2 Channel  12 :500
ROC1 level 3 Channel  12 :600
ROC1 level 4 Channel  12 :700
ROC2 level 0 Channel  12 :270
ROC2 level 1 Channel  12 :400
ROC2 level 2 Channel  12 :500
ROC2 level 3 Channel  12 :600
ROC2 level 4 Channel  12 :700
ROC3 level 0 Channel  12 :270
ROC3 level 1 Channel  12 :400
ROC3 level 2 Channel  12 :500
ROC3 level 3 Channel  12 :600
ROC3 level 4 Channel  12 :700
ROC4 level 0 Channel  12 :270
ROC4 level 1 Channel  12 :400
ROC4 level 2 Channel  12 :500
ROC4 level 3 Channel  12 :600
ROC4 level 4 Channel  12 :700
ROC5 level 0 Channel  12 :270
ROC5 level 1 Channel  12 :400
ROC5 level 2 Channel  12 :500
ROC5 level 3 Channel  12 :600
ROC5 level 4 Channel  12 :700
ROC6 level 0 Channel  12 :270
ROC6 level 1 Channel  12 :400
ROC6 level 2 Channel  12 :500
ROC6 level 3 Channel  12 :600
ROC6 level 4 Channel  12 :700
ROC7 level 0 Channel  12 :270
ROC7 level 1 Channel  12 :400
ROC7 level 2 Channel  12 :500
ROC7 level 3 Channel  12 :600
ROC7 level 4 Channel  12 :700
TRLR level 0 Channel 12:270
TRLR level 1 Channel 12:400
TRLR level 2 Channel 12:500
TRLR level 3 Channel 12:600
TRLR level 4 Channel 12:700
TBM level 0 Channel  13:270
TBM level 1 Channel  13:400
TBM level 2 Channel  13:500
TBM level 3 Channel  13:600
TBM level 4 Channel  13:700
ROC0 level 0 Channel  13 :270
ROC0 level 1 Channel  13 :400
ROC0 level 2 Channel  13 :500
ROC0 level 3 Channel  13 :600
ROC0 level 4 Channel  13 :700
ROC1 level 0 Channel  13 :270
ROC1 level 1 Channel  13 :400
ROC1 level 2 Channel  13 :500
ROC1 level 3 Channel  13 :600
ROC1 level 4 Channel  13 :700
ROC2 level 0 Channel  13 :270
ROC2 level 1 Channel  13 :400
ROC2 level 2 Channel  13 :500
ROC2 level 3 Channel  13 :600
ROC2 level 4 Channel  13 :700
ROC3 level 0 Channel  13 :270
ROC3 level 1 Channel  13 :400
ROC3 level 2 Channel  13 :500
ROC3 level 3 Channel  13 :600
ROC3 level 4 Channel  13 :700
ROC4 level 0 Channel  13 :270
ROC4 level 1 Channel  13 :400
ROC4 level 2 Channel  13 :500
ROC4 level 3 Channel  13 :600
ROC4 level 4 Channel  13 :700
ROC5 level 0 Channel  13 :270
ROC5 level 1 Channel  13 :400
ROC5 level 2 Channel  13 :500
ROC5 level 3 Channel  13 :600
ROC5 level 4 Channel  13 :700
ROC6 level 0 Channel  13 :270
ROC6 level 1 Channel  13 :400
ROC6 level 2 Channel  13 :500
ROC6 level 3 Channel  13 :600
ROC6 level 4 Channel  13 :700
ROC7 level 0 Channel  13 :270
ROC7 level 1 Channel  13 :400
ROC7 level 2 Channel  13 :500
ROC7 level 3 Channel  13 :600
ROC7 level 4 Channel  13 :700
TRLR level 0 Channel 13:270
TRLR level 1 Channel 13:400
TRLR level 2 Channel 13:500
TRLR level 3 Channel 13:600
TRLR level 4 Channel 13:700
TBM level 0 Channel  14:270
TBM level 1 Channel  14:400
TBM level 2 Channel  14:500
TBM level 3 Channel  14:600
TBM level 4 Channel  14:700
ROC0 level 0 Channel  14 :270
ROC0 level 1 Channel  14 :400
ROC0 level 2 Channel  14 :500
ROC0 level 3 Channel  14 :600
ROC0 level 4 Channel  14 :700
ROC1 level 0 Channel  14 :270
ROC1 level 1 Channel  14 :400
ROC1 level 2 Channel  14 :500
ROC1 level 3 Channel  14 :600
ROC1 level 4 Channel  14 :700
ROC2 level 0 Channel  14 :270
ROC2 level 1 Channel  14 :400
ROC2 level 2 Channel  14 :500
ROC2 level 3 Channel  14 :600
ROC2 level 4 Channel  14 :700
ROC3 level 0 Channel  14 :270
ROC3 level 1 Channel  14 :400
ROC3 level 2 Channel  14 :500
ROC3 level 3 Channel  14 :600
ROC3 level 4 Channel  14 :700
ROC4 level 0 Channel  14 :270
ROC4 level 1 Channel  14 :400
ROC4 level 2 Channel  14 :500
ROC4 level 3 Channel  14 :600
ROC4 level 4 Channel  14 :700
ROC5 level 0 Channel  14 :270
ROC5 level 1 Channel  14 :400
ROC5 level 2 Channel  14 :500
ROC5 level 3 Channel  14 :600
ROC5 level 4 Channel  14 :700
ROC6 level 0 Channel  14 :270
ROC6 level 1 Channel  14 :400
ROC6 level 2 Channel  14 :500
ROC6 level 3 Channel  14 :600
ROC6 level 4 Channel  14 :700
ROC7 level 0 Channel  14 :270
ROC7 level 1 Channel  14 :400
ROC7 level 2 Channel  14 :500
ROC7 level 3 Channel  14 :600
ROC7 level 4 Channel  14 :700
TRLR level 0 Channel 14:270
TRLR level 1 Channel 14:400
TRLR level 2 Channel 14:500
TRLR level 3 Channel 14:600
TRLR level 4 Channel 14:700
TBM level 0 Channel  15:270
TBM level 1 Channel  15:400
TBM level 2 Channel  15:500
TBM level 3 Channel  15:600
TBM level 4 Channel  15:700
ROC0 level 0 Channel  15 :270
ROC0 level 1 Channel  15 :400
ROC0 level 2 Channel  15 :500
ROC0 level 3 Channel  15 :600
ROC0 level 4 Channel  15 :700
ROC1 level 0 Channel  15 :270
ROC1 level 1 Channel  15 :400
ROC1 level 2 Channel  15 :500
ROC1 level 3 Channel  15 :600
ROC1 level 4 Channel  15 :700
ROC2 level 0 Channel  15 :270
ROC2 level 1 Channel  15 :400
ROC2 level 2 Channel  15 :500
ROC2 level 3 Channel  15 :600
ROC2 level 4 Channel  15 :700
ROC3 level 0 Channel  15 :270
ROC3 level 1 Channel  15 :400
ROC3 level 2 Channel  15 :500
ROC3 level 3 Channel  15 :600
ROC3 level 4 Channel  15 :700
ROC4 level 0 Channel  15 :270
ROC4 level 1 Channel  15 :400
ROC4 level 2 Channel  15 :500
ROC4 level 3 Channel  15 :600
ROC4 level 4 Channel  15 :700
ROC5 level 0 Channel  15 :270
ROC5 level 1 Channel  15 :400
ROC5 level 2 Channel  15 :500
ROC5 level 3 Channel  15 :600
ROC5 level 4 Channel  15 :700
ROC6 level 0 Channel  15 :270
ROC6 level 1 Channel  15 :400
ROC6 level 2 Channel  15 :500
ROC6 level 3 Channel  15 :600
ROC6 level 4 Channel  15 :700
ROC7 level 0 Channel  15 :270
ROC7 level 1 Channel  15 :400
ROC7 level 2 Channel  15 :500
ROC7 level 3 Channel  15 :600
ROC7 level 4 Channel  15 :700
TRLR level 0 Channel 15:270
TRLR level 1 Channel 15:400
TRLR level 2 Channel 15:500
TRLR level 3 Channel 15:600
TRLR level 4 Channel 15:700
TBM level 0 Channel  16:270
TBM level 1 Channel  16:400
TBM level 2 Channel  16:500
TBM level 3 Channel  16:600
TBM level 4 Channel  16:700
ROC0 level 0 Channel  16 :270
ROC0 level 1 Channel  16 :400
ROC0 level 2 Channel  16 :500
ROC0 level 3 Channel  16 :600
ROC0 level 4 Channel  16 :700
ROC1 level 0 Channel  16 :270
ROC1 level 1 Channel  16 :400
ROC1 level 2 Channel  16 :500
ROC1 level 3 Channel  16 :600
ROC1 level 4 Channel  16 :700
ROC2 level 0 Channel  16 :270
ROC2 level 1 Channel  16 :400
ROC2 level 2 Channel  16 :500
ROC2 level 3 Channel  16 :600
ROC2 level 4 Channel  16 :700
ROC3 level 0 Channel  16 :270
ROC3 level 1 Channel  16 :400
ROC3 level 2 Channel  16 :500
ROC3 level 3 Channel  16 :600
ROC3 level 4 Channel  16 :700
ROC4 level 0 Channel  16 :270
ROC4 level 1 Channel  16 :400
ROC4 level 2 Channel  16 :500
ROC4 level 3 Channel  16 :600
ROC4 level 4 Channel  16 :700
ROC5 level 0 Channel  16 :270
ROC5 level 1 Channel  16 :400
ROC5 level 2 Channel  16 :500
ROC5 level 3 Channel  16 :600
ROC5 level 4 Channel  16 :700
ROC6 level 0 Channel  16 :270
ROC6 level 1 Channel  16 :400
ROC6 level 2 Channel  16 :500
ROC6 level 3 Channel  16 :600
ROC6 level 4 Channel  16 :700
ROC7 level 0 Channel  16 :270
ROC7 level 1 Channel  16 :400
ROC7 level 2 Channel  16 :500
ROC7 level 3 Channel  16 :600
ROC7 level 4 Channel  16 :700
TRLR level 0 Channel 16:270
TRLR level 1 Channel 16:400
TRLR level 2 Channel 16:500
TRLR level 3 Channel 16:600
TRLR level 4 Channel 16:700
TBM level 0 Channel  17:270
TBM level 1 Channel  17:400
TBM level 2 Channel  17:500
TBM level 3 Channel  17:600
TBM level 4 Channel  17:700
ROC0 level 0 Channel  17 :270
ROC0 level 1 Channel  17 :400
ROC0 level 2 Channel  17 :500
ROC0 level 3 Channel  17 :600
ROC0 level 4 Channel  17 :700
ROC1 level 0 Channel  17 :270
ROC1 level 1 Channel  17 :400
ROC1 level 2 Channel  17 :500
ROC1 level 3 Channel  17 :600
ROC1 level 4 Channel  17 :700
ROC2 level 0 Channel  17 :270
ROC2 level 1 Channel  17 :400
ROC2 level 2 Channel  17 :500
ROC2 level 3 Channel  17 :600
ROC2 level 4 Channel  17 :700
ROC3 level 0 Channel  17 :270
ROC3 level 1 Channel  17 :400
ROC3 level 2 Channel  17 :500
ROC3 level 3 Channel  17 :600
ROC3 level 4 Channel  17 :700
ROC4 level 0 Channel  17 :270
ROC4 level 1 Channel  17 :400
ROC4 level 2 Channel  17 :500
ROC4 level 3 Channel  17 :600
ROC4 level 4 Channel  17 :700
ROC5 level 0 Channel  17 :270
ROC5 level 1 Channel  17 :400
ROC5 level 2 Channel  17 :500
ROC5 level 3 Channel  17 :600
ROC5 level 4 Channel  17 :700
ROC6 level 0 Channel  17 :270
ROC6 level 1 Channel  17 :400
ROC6 level 2 Channel  17 :500
ROC6 level 3 Channel  17 :600
ROC6 level 4 Channel  17 :700
ROC7 level 0 Channel  17 :270
ROC7 level 1 Channel  17 :400
ROC7 level 2 Channel  17 :500
ROC7 level 3 Channel  17 :600
ROC7 level 4 Channel  17 :700
TRLR level 0 Channel 17:270
TRLR level 1 Channel 17:400
TRLR level 2 Channel 17:500
TRLR level 3 Channel 17:600
TRLR level 4 Channel 17:700
TBM level 0 Channel  18:270
TBM level 1 Channel  18:400
TBM level 2 Channel  18:500
TBM level 3 Channel  18:600
TBM level 4 Channel  18:700
ROC0 level 0 Channel  18 :270
ROC0 level 1 Channel  18 :400
ROC0 level 2 Channel  18 :500
ROC0 level 3 Channel  18 :600
ROC0 level 4 Channel  18 :700
ROC1 level 0 Channel  18 :270
ROC1 level 1 Channel  18 :400
ROC1 level 2 Channel  18 :500
ROC1 level 3 Channel  18 :600
ROC1 level 4 Channel  18 :700
ROC2 level 0 Channel  18 :270
ROC2 level 1 Channel  18 :400
ROC2 level 2 Channel  18 :500
ROC2 level 3 Channel  18 :600
ROC2 level 4 Channel  18 :700
ROC3 level 0 Channel  18 :270
ROC3 level 1 Channel  18 :400
ROC3 level 2 Channel  18 :500
ROC3 level 3 Channel  18 :600
ROC3 level 4 Channel  18 :700
ROC4 level 0 Channel  18 :270
ROC4 level 1 Channel  18 :400
ROC4 level 2 Channel  18 :500
ROC4 level 3 Channel  18 :600
ROC4 level 4 Channel  18 :700
ROC5 level 0 Channel  18 :270
ROC5 level 1 Channel  18 :400
ROC5 level 2 Channel  18 :500
ROC5 level 3 Channel  18 :600
ROC5 level 4 Channel  18 :700
ROC6 level 0 Channel  18 :270
ROC6 level 1 Channel  18 :400
ROC6 level 2 Channel  18 :500
ROC6 level 3 Channel  18 :600
ROC6 level 4 Channel  18 :700
ROC7 level 0 Channel  18 :270
ROC7 level 1 Channel  18 :400
ROC7 level 2 Channel  18 :500
ROC7 level 3 Channel  18 :600
ROC7 level 4 Channel  18 :700
TRLR level 0 Channel 18:270
TRLR level 1 Channel 18:400
TRLR level 2 Channel 18:500
TRLR level 3 Channel 18:600
TRLR level 4 Channel 18:700
TBM level 0 Channel  19:270
TBM level 1 Channel  19:400
TBM level 2 Channel  19:500
TBM level 3 Channel  19:600
TBM level 4 Channel  19:700
ROC0 level 0 Channel  19 :270
ROC0 level 1 Channel  19 :400
ROC0 level 2 Channel  19 :500
ROC0 level 3 Channel  19 :600
ROC0 level 4 Channel  19 :700
ROC1 level 0 Channel  19 :270
ROC1 level 1 Channel  19 :400
ROC1 level 2 Channel  19 :500
ROC1 level 3 Channel  19 :600
ROC1 level 4 Channel  19 :700
ROC2 level 0 Channel  19 :270
ROC2 level 1 Channel  19 :400
ROC2 level 2 Channel  19 :500
ROC2 level 3 Channel  19 :600
ROC2 level 4 Channel  19 :700
ROC3 level 0 Channel  19 :270
ROC3 level 1 Channel  19 :400
ROC3 level 2 Channel  19 :500
ROC3 level 3 Channel  19 :600
ROC3 level 4 Channel  19 :700
ROC4 level 0 Channel  19 :270
ROC4 level 1 Channel  19 :400
ROC4 level 2 Channel  19 :500
ROC4 level 3 Channel  19 :600
ROC4 level 4 Channel  19 :700
ROC5 level 0 Channel  19 :270
ROC5 level 1 Channel  19 :400
ROC5 level 2 Channel  19 :500
ROC5 level 3 Channel  19 :600
ROC5 level 4 Channel  19 :700
ROC6 level 0 Channel  19 :270
ROC6 level 1 Channel  19 :400
ROC6 level 2 Channel  19 :500
ROC6 level 3 Channel  19 :600
ROC6 level 4 Channel  19 :700
ROC7 level 0 Channel  19 :270
ROC7 level 1 Channel  19 :400
ROC7 level 2 Channel  19 :500
ROC7 level 3 Channel  19 :600
ROC7 level 4 Channel  19 :700
TRLR level 0 Channel 19:270
TRLR level 1 Channel 19:400
TRLR level 2 Channel 19:500
TRLR level 3 Channel 19:600
TRLR level 4 Channel 19:700
TBM level 0 Channel  20:270
TBM level 1 Channel  20:400
TBM level 2 Channel  20:500
TBM level 3 Channel  20:600
TBM level 4 Channel  20:700
ROC0 level 0 Channel  20 :270
ROC0 level 1 Channel  20 :400
ROC0 level 2 Channel  20 :500
ROC0 level 3 Channel  20 :600
ROC0 level 4 Channel  20 :700
ROC1 level 0 Channel  20 :270
ROC1 level 1 Channel  20 :400
ROC1 level 2 Channel  20 :500
ROC1 level 3 Channel  20 :600
ROC1 level 4 Channel  20 :700
ROC2 level 0 Channel  20 :270
ROC2 level 1 Channel  20 :400
ROC2 level 2 Channel  20 :500
ROC2 level 3 Channel  20 :600
ROC2 level 4 Channel  20 :700
ROC3 level 0 Channel  20 :270
ROC3 level 1 Channel  20 :400
ROC3 level 2 Channel  20 :500
ROC3 level 3 Channel  20 :600
ROC3 level 4 Channel  20 :700
ROC4 level 0 Channel  20 :270
ROC4 level 1 Channel  20 :400
ROC4 level 2 Channel  20 :500
ROC4 level 3 Channel  20 :600
ROC4 level 4 Channel  20 :700
ROC5 level 0 Channel  20 :270
ROC5 level 1 Channel  20 :400
ROC5 level 2 Channel  20 :500
ROC5 level 3 Channel  20 :600
ROC5 level 4 Channel  20 :700
ROC6 level 0 Channel  20 :270
ROC6 level 1 Channel  20 :400
ROC6 level 2 Channel  20 :500
ROC6 level 3 Channel  20 :600
ROC6 level 4 Channel  20 :700
ROC7 level 0 Channel  20 :270
ROC7 level 1 Channel  20 :400
ROC7 level 2 Channel  20 :500
ROC7 level 3 Channel  20 :600
ROC7 level 4 Channel  20 :700
TRLR level 0 Channel 20:270
TRLR level 1 Channel 20:400
TRLR level 2 Channel 20:500
TRLR level 3 Channel 20:600
TRLR level 4 Channel 20:700
TBM level 0 Channel  21:270
TBM level 1 Channel  21:400
TBM level 2 Channel  21:500
TBM level 3 Channel  21:600
TBM level 4 Channel  21:700
ROC0 level 0 Channel  21 :270
ROC0 level 1 Channel  21 :400
ROC0 level 2 Channel  21 :500
ROC0 level 3 Channel  21 :600
ROC0 level 4 Channel  21 :700
ROC1 level 0 Channel  21 :270
ROC1 level 1 Channel  21 :400
ROC1 level 2 Channel  21 :500
ROC1 level 3 Channel  21 :600
ROC1 level 4 Channel  21 :700
ROC2 level 0 Channel  21 :270
ROC2 level 1 Channel  21 :400
ROC2 level 2 Channel  21 :500
ROC2 level 3 Channel  21 :600
ROC2 level 4 Channel  21 :700
ROC3 level 0 Channel  21 :270
ROC3 level 1 Channel  21 :400
ROC3 level 2 Channel  21 :500
ROC3 level 3 Channel  21 :600
ROC3 level 4 Channel  21 :700
ROC4 level 0 Channel  21 :270
ROC4 level 1 Channel  21 :400
ROC4 level 2 Channel  21 :500
ROC4 level 3 Channel  21 :600
ROC4 level 4 Channel  21 :700
ROC5 level 0 Channel  21 :270
ROC5 level 1 Channel  21 :400
ROC5 level 2 Channel  21 :500
ROC5 level 3 Channel  21 :600
ROC5 level 4 Channel  21 :700
ROC6 level 0 Channel  21 :270
ROC6 level 1 Channel  21 :400
ROC6 level 2 Channel  21 :500
ROC6 level 3 Channel  21 :600
ROC6 level 4 Channel  21 :700
ROC7 level 0 Channel  21 :270
ROC7 level 1 Channel  21 :400
ROC7 level 2 Channel  21 :500
ROC7 level 3 Channel  21 :600
ROC7 level 4 Channel  21 :700
TRLR level 0 Channel 21:270
TRLR level 1 Channel 21:400
TRLR level 2 Channel 21:500
TRLR level 3 Channel 21:600
TRLR level 4 Channel 21:700
TBM level 0 Channel  22:270
TBM level 1 Channel  22:400
TBM level 2 Channel  22:500
TBM level 3 Channel  22:600
TBM level 4 Channel  22:700
ROC0 level 0 Channel  22 :270
ROC0 level 1 Channel  22 :400
ROC0 level 2 Channel  22 :500
ROC0 level 3 Channel  22 :600
ROC0 level 4 Channel  22 :700
ROC1 level 0 Channel  22 :270
ROC1 level 1 Channel  22 :400
ROC1 level 2 Channel  22 :500
ROC1 level 3 Channel  22 :600
ROC1 level 4 Channel  22 :700
ROC2 level 0 Channel  22 :270
ROC2 level 1 Channel  22 :400
ROC2 level 2 Channel  22 :500
ROC2 level 3 Channel  22 :600
ROC2 level 4 Channel  22 :700
ROC3 level 0 Channel  22 :270
ROC3 level 1 Channel  22 :400
ROC3 level 2 Channel  22 :500
ROC3 level 3 Channel  22 :600
ROC3 level 4 Channel  22 :700
ROC4 level 0 Channel  22 :270
ROC4 level 1 Channel  22 :400
ROC4 level 2 Channel  22 :500
ROC4 level 3 Channel  22 :600
ROC4 level 4 Channel  22 :700
ROC5 level 0 Channel  22 :270
ROC5 level 1 Channel  22 :400
ROC5 level 2 Channel  22 :500
ROC5 level 3 Channel  22 :600
ROC5 level 4 Channel  22 :700
ROC6 level 0 Channel  22 :270
ROC6 level 1 Channel  22 :400
ROC6 level 2 Channel  22 :500
ROC6 level 3 Channel  22 :600
ROC6 level 4 Channel  22 :700
ROC7 level 0 Channel  22 :270
ROC7 level 1 Channel  22 :400
ROC7 level 2 Channel  22 :500
ROC7 level 3 Channel  22 :600
ROC7 level 4 Channel  22 :700
TRLR level 0 Channel 22:270
TRLR level 1 Channel 22:400
TRLR level 2 Channel 22:500
TRLR level 3 Channel 22:600
TRLR level 4 Channel 22:700
TBM level 0 Channel  23:270
TBM level 1 Channel  23:400
TBM level 2 Channel  23:500
TBM level 3 Channel  23:600
TBM level 4 Channel  23:700
ROC0 level 0 Channel  23 :270
ROC0 level 1 Channel  23 :400
ROC0 level 2 Channel  23 :500
ROC0 level 3 Channel  23 :600
ROC0 level 4 Channel  23 :700
ROC1 level 0 Channel  23 :270
ROC1 level 1 Channel  23 :400
ROC1 level 2 Channel  23 :500
ROC1 level 3 Channel  23 :600
ROC1 level 4 Channel  23 :700
ROC2 level 0 Channel  23 :270
ROC2 level 1 Channel  23 :400
ROC2 level 2 Channel  23 :500
ROC2 level 3 Channel  23 :600
ROC2 level 4 Channel  23 :700
ROC3 level 0 Channel  23 :270
ROC3 level 1 Channel  23 :400
ROC3 level 2 Channel  23 :500
ROC3 level 3 Channel  23 :600
ROC3 level 4 Channel  23 :700
ROC4 level 0 Channel  23 :270
ROC4 level 1 Channel  23 :400
ROC4 level 2 Channel  23 :500
ROC4 level 3 Channel  23 :600
ROC4 level 4 Channel  23 :700
ROC5 level 0 Channel  23 :270
ROC5 level 1 Channel  23 :400
ROC5 level 2 Channel  23 :500
ROC5 level 3 Channel  23 :600
ROC5 level 4 Channel  23 :700
ROC6 level 0 Channel  23 :270
ROC6 level 1 Channel  23 :400
ROC6 level 2 Channel  23 :500
ROC6 level 3 Channel  23 :600
ROC6 level 4 Channel  23 :700
ROC7 level 0 Channel  23 :270
ROC7 level 1 Channel  23 :400
ROC7 level 2 Channel  23 :500
ROC7 level 3 Channel  23 :600
ROC7 level 4 Channel  23 :700
TRLR level 0 Channel 23:270
TRLR level 1 Channel 23:400
TRLR level 2 Channel 23:500
TRLR level 3 Channel 23:600
TRLR level 4 Channel 23:700
TBM level 0 Channel  24:270
TBM level 1 Channel  24:400
TBM level 2 Channel  24:500
TBM level 3 Channel  24:600
TBM level 4 Channel  24:700
ROC0 level 0 Channel  24 :270
ROC0 level 1 Channel  24 :400
ROC0 level 2 Channel  24 :500
ROC0 level 3 Channel  24 :600
ROC0 level 4 Channel  24 :700
ROC1 level 0 Channel  24 :270
ROC1 level 1 Channel  24 :400
ROC1 level 2 Channel  24 :500
ROC1 level 3 Channel  24 :600
ROC1 level 4 Channel  24 :700
ROC2 level 0 Channel  24 :270
ROC2 level 1 Channel  24 :400
ROC2 level 2 Channel  24 :500
ROC2 level 3 Channel  24 :600
ROC2 level 4 Channel  24 :700
ROC3 level 0 Channel  24 :270
ROC3 level 1 Channel  24 :400
ROC3 level 2 Channel  24 :500
ROC3 level 3 Channel  24 :600
ROC3 level 4 Channel  24 :700
ROC4 level 0 Channel  24 :270
ROC4 level 1 Channel  24 :400
ROC4 level 2 Channel  24 :500
ROC4 level 3 Channel  24 :600
ROC4 level 4 Channel  24 :700
ROC5 level 0 Channel  24 :270
ROC5 level 1 Channel  24 :400
ROC5 level 2 Channel  24 :500
ROC5 level 3 Channel  24 :600
ROC5 level 4 Channel  24 :700
ROC6 level 0 Channel  24 :270
ROC6 level 1 Channel  24 :400
ROC6 level 2 Channel  24 :500
ROC6 level 3 Channel  24 :600
ROC6 level 4 Channel  24 :700
ROC7 level 0 Channel  24 :270
ROC7 level 1 Channel  24 :400
ROC7 level 2 Channel  24 :500
ROC7 level 3 Channel  24 :600
ROC7 level 4 Channel  24 :700
TRLR level 0 Channel 24:270
TRLR level 1 Channel 24:400
TRLR level 2 Channel 24:500
TRLR level 3 Channel 24:600
TRLR level 4 Channel 24:700
TBM level 0 Channel  25:270
TBM level 1 Channel  25:400
TBM level 2 Channel  25:500
TBM level 3 Channel  25:600
TBM level 4 Channel  25:700
ROC0 level 0 Channel  25 :270
ROC0 level 1 Channel  25 :400
ROC0 level 2 Channel  25 :500
ROC0 level 3 Channel  25 :600
ROC0 level 4 Channel  25 :700
ROC1 level 0 Channel  25 :270
ROC1 level 1 Channel  25 :400
ROC1 level 2 Channel  25 :500
ROC1 level 3 Channel  25 :600
ROC1 level 4 Channel  25 :700
ROC2 level 0 Channel  25 :270
ROC2 level 1 Channel  25 :400
ROC2 level 2 Channel  25 :500
ROC2 level 3 Channel  25 :600
ROC2 level 4 Channel  25 :700
ROC3 level 0 Channel  25 :270
ROC3 level 1 Channel  25 :400
ROC3 level 2 Channel  25 :500
ROC3 level 3 Channel  25 :600
ROC3 level 4 Channel  25 :700
ROC4 level 0 Channel  25 :270
ROC4 level 1 Channel  25 :400
ROC4 level 2 Channel  25 :500
ROC4 level 3 Channel  25 :600
ROC4 level 4 Channel  25 :700
ROC5 level 0 Channel  25 :270
ROC5 level 1 Channel  25 :400
ROC5 level 2 Channel  25 :500
ROC5 level 3 Channel  25 :600
ROC5 level 4 Channel  25 :700
ROC6 level 0 Channel  25 :270
ROC6 level 1 Channel  25 :400
ROC6 level 2 Channel  25 :500
ROC6 level 3 Channel  25 :600
ROC6 level 4 Channel  25 :700
ROC7 level 0 Channel  25 :270
ROC7 level 1 Channel  25 :400
ROC7 level 2 Channel  25 :500
ROC7 level 3 Channel  25 :600
ROC7 level 4 Channel  25 :700
TRLR level 0 Channel 25:270
TRLR level 1 Channel 25:400
TRLR level 2 Channel 25:500
TRLR level 3 Channel 25:600
TRLR level 4 Channel 25:700
TBM level 0 Channel  26:270
TBM level 1 Channel  26:400
TBM level 2 Channel  26:500
TBM level 3 Channel  26:600
TBM level 4 Channel  26:700
ROC0 level 0 Channel  26 :270
ROC0 level 1 Channel  26 :400
ROC0 level 2 Channel  26 :500
ROC0 level 3 Channel  26 :600
ROC0 level 4 Channel  26 :700
ROC1 level 0 Channel  26 :270
ROC1 level 1 Channel  26 :400
ROC1 level 2 Channel  26 :500
ROC1 level 3 Channel  26 :600
ROC1 level 4 Channel  26 :700
ROC2 level 0 Channel  26 :270
ROC2 level 1 Channel  26 :400
ROC2 level 2 Channel  26 :500
ROC2 level 3 Channel  26 :600
ROC2 level 4 Channel  26 :700
ROC3 level 0 Channel  26 :270
ROC3 level 1 Channel  26 :400
ROC3 level 2 Channel  26 :500
ROC3 level 3 Channel  26 :600
ROC3 level 4 Channel  26 :700
ROC4 level 0 Channel  26 :270
ROC4 level 1 Channel  26 :400
ROC4 level 2 Channel  26 :500
ROC4 level 3 Channel  26 :600
ROC4 level 4 Channel  26 :700
ROC5 level 0 Channel  26 :270
ROC5 level 1 Channel  26 :400
ROC5 level 2 Channel  26 :500
ROC5 level 3 Channel  26 :600
ROC5 level 4 Channel  26 :700
ROC6 level 0 Channel  26 :270
ROC6 level 1 Channel  26 :400
ROC6 level 2 Channel  26 :500
ROC6 level 3 Channel  26 :600
ROC6 level 4 Channel  26 :700
ROC7 level 0 Channel  26 :270
ROC7 level 1 Channel  26 :400
ROC7 level 2 Channel  26 :500
ROC7 level 3 Channel  26 :600
ROC7 level 4 Channel  26 :700
TRLR level 0 Channel 26:270
TRLR level 1 Channel 26:400
TRLR level 2 Channel 26:500
TRLR level 3 Channel 26:600
TRLR level 4 Channel 26:700
TBM level 0 Channel  27:270
TBM level 1 Channel  27:400
TBM level 2 Channel  27:500
TBM level 3 Channel  27:600
TBM level 4 Channel  27:700
ROC0 level 0 Channel  27 :270
ROC0 level 1 Channel  27 :400
ROC0 level 2 Channel  27 :500
ROC0 level 3 Channel  27 :600
ROC0 level 4 Channel  27 :700
ROC1 level 0 Channel  27 :270
ROC1 level 1 Channel  27 :400
ROC1 level 2 Channel  27 :500
ROC1 level 3 Channel  27 :600
ROC1 level 4 Channel  27 :700
ROC2 level 0 Channel  27 :270
ROC2 level 1 Channel  27 :400
ROC2 level 2 Channel  27 :500
ROC2 level 3 Channel  27 :600
ROC2 level 4 Channel  27 :700
ROC3 level 0 Channel  27 :270
ROC3 level 1 Channel  27 :400
ROC3 level 2 Channel  27 :500
ROC3 level 3 Channel  27 :600
ROC3 level 4 Channel  27 :700
ROC4 level 0 Channel  27 :270
ROC4 level 1 Channel  27 :400
ROC4 level 2 Channel  27 :500
ROC4 level 3 Channel  27 :600
ROC4 level 4 Channel  27 :700
ROC5 level 0 Channel  27 :270
ROC5 level 1 Channel  27 :400
ROC5 level 2 Channel  27 :500
ROC5 level 3 Channel  27 :600
ROC5 level 4 Channel  27 :700
ROC6 level 0 Channel  27 :270
ROC6 level 1 Channel  27 :400
ROC6 level 2 Channel  27 :500
ROC6 level 3 Channel  27 :600
ROC6 level 4 Channel  27 :700
ROC7 level 0 Channel  27 :270
ROC7 level 1 Channel  27 :400
ROC7 level 2 Channel  27 :500
ROC7 level 3 Channel  27 :600
ROC7 level 4 Channel  27 :700
TRLR level 0 Channel 27:270
TRLR level 1 Channel 27:400
TRLR level 2 Channel 27:500
TRLR level 3 Channel 27:600
TRLR level 4 Channel 27:700
TBM level 0 Channel  28:270
TBM level 1 Channel  28:400
TBM level 2 Channel  28:500
TBM level 3 Channel  28:600
TBM level 4 Channel  28:700
ROC0 level 0 Channel  28 :270
ROC0 level 1 Channel  28 :400
ROC0 level 2 Channel  28 :500
ROC0 level 3 Channel  28 :600
ROC0 level 4 Channel  28 :700
ROC1 level 0 Channel  28 :270
ROC1 level 1 Channel  28 :400
ROC1 level 2 Channel  28 :500
ROC1 level 3 Channel  28 :600
ROC1 level 4 Channel  28 :700
ROC2 level 0 Channel  28 :270
ROC2 level 1 Channel  28 :400
ROC2 level 2 Channel  28 :500
ROC2 level 3 Channel  28 :600
ROC2 level 4 Channel  28 :700
ROC3 level 0 Channel  28 :270
ROC3 level 1 Channel  28 :400
ROC3 level 2 Channel  28 :500
ROC3 level 3 Channel  28 :600
ROC3 level 4 Channel  28 :700
ROC4 level 0 Channel  28 :270
ROC4 level 1 Channel  28 :400
ROC4 level 2 Channel  28 :500
ROC4 level 3 Channel  28 :600
ROC4 level 4 Channel  28 :700
ROC5 level 0 Channel  28 :270
ROC5 level 1 Channel  28 :400
ROC5 level 2 Channel  28 :500
ROC5 level 3 Channel  28 :600
ROC5 level 4 Channel  28 :700
ROC6 level 0 Channel  28 :270
ROC6 level 1 Channel  28 :400
ROC6 level 2 Channel  28 :500
ROC6 level 3 Channel  28 :600
ROC6 level 4 Channel  28 :700
ROC7 level 0 Channel  28 :270
ROC7 level 1 Channel  28 :400
ROC7 level 2 Channel  28 :500
ROC7 level 3 Channel  28 :600
ROC7 level 4 Channel  28 :700
TRLR level 0 Channel 28:270
TRLR level 1 Channel 28:400
TRLR level 2 Channel 28:500
TRLR level 3 Channel 28:600
TRLR level 4 Channel 28:700
TBM level 0 Channel  29:270
TBM level 1 Channel  29:400
TBM level 2 Channel  29:500
TBM level 3 Channel  29:600
TBM level 4 Channel  29:700
ROC0 level 0 Channel  29 :270
ROC0 level 1 Channel  29 :400
ROC0 level 2 Channel  29 :500
ROC0 level 3 Channel  29 :600
ROC0 level 4 Channel  29 :700
ROC1 level 0 Channel  29 :270
ROC1 level 1 Channel  29 :400
ROC1 level 2 Channel  29 :500
ROC1 level 3 Channel  29 :600
ROC1 level 4 Channel  29 :700
ROC2 level 0 Channel  29 :270
ROC2 level 1 Channel  29 :400
ROC2 level 2 Channel  29 :500
ROC2 level 3 Channel  29 :600
ROC2 level 4 Channel  29 :700
ROC3 level 0 Channel  29 :270
ROC3 level 1 Channel  29 :400
ROC3 level 2 Channel  29 :500
ROC3 level 3 Channel  29 :600
ROC3 level 4 Channel  29 :700
ROC4 level 0 Channel  29 :270
ROC4 level 1 Channel  29 :400
ROC4 level 2 Channel  29 :500
ROC4 level 3 Channel  29 :600
ROC4 level 4 Channel  29 :700
ROC5 level 0 Channel  29 :270
ROC5 level 1 Channel  29 :400
ROC5 level 2 Channel  29 :500
ROC5 level 3 Channel  29 :600
ROC5 level 4 Channel  29 :700
ROC6 level 0 Channel  29 :270
ROC6 level 1 Channel  29 :400
ROC6 level 2 Channel  29 :500
ROC6 level 3 Channel  29 :600
ROC6 level 4 Channel  29 :700
ROC7 level 0 Channel  29 :270
ROC7 level 1 Channel  29 :400
ROC7 level 2 Channel  29 :500
ROC7 level 3 Channel  29 :600
ROC7 level 4 Channel  29 :700
TRLR level 0 Channel 29:270
TRLR level 1 Channel 29:400
TRLR level 2 Channel 29:500
TRLR level 3 Channel 29:600
TRLR level 4 Channel 29:700
TBM level 0 Channel  30:270
TBM level 1 Channel  30:400
TBM level 2 Channel  30:500
TBM level 3 Channel  30:600
TBM level 4 Channel  30:700
ROC0 level 0 Channel  30 :270
ROC0 level 1 Channel  30 :400
ROC0 level 2 Channel  30 :500
ROC0 level 3 Channel  30 :600
ROC0 level 4 Channel  30 :700
ROC1 level 0 Channel  30 :270
ROC1 level 1 Channel  30 :400
ROC1 level 2 Channel  30 :500
ROC1 level 3 Channel  30 :600
ROC1 level 4 Channel  30 :700
ROC2 level 0 Channel  30 :270
ROC2 level 1 Channel  30 :400
ROC2 level 2 Channel  30 :500
ROC2 level 3 Channel  30 :600
ROC2 level 4 Channel  30 :700
ROC3 level 0 Channel  30 :270
ROC3 level 1 Channel  30 :400
ROC3 level 2 Channel  30 :500
ROC3 level 3 Channel  30 :600
ROC3 level 4 Channel  30 :700
ROC4 level 0 Channel  30 :270
ROC4 level 1 Channel  30 :400
ROC4 level 2 Channel  30 :500
ROC4 level 3 Channel  30 :600
ROC4 level 4 Channel  30 :700
ROC5 level 0 Channel  30 :270
ROC5 level 1 Channel  30 :400
ROC5 level 2 Channel  30 :500
ROC5 level 3 Channel  30 :600
ROC5 level 4 Channel  30 :700
ROC6 level 0 Channel  30 :270
ROC6 level 1 Channel  30 :400
ROC6 level 2 Channel  30 :500
ROC6 level 3 Channel  30 :600
ROC6 level 4 Channel  30 :700
ROC7 level 0 Channel  30 :270
ROC7 level 1 Channel  30 :400
ROC7 level 2 Channel  30 :500
ROC7 level 3 Channel  30 :600
ROC7 level 4 Channel  30 :700
TRLR level 0 Channel 30:270
TRLR level 1 Channel 30:400
TRLR level 2 Channel 30:500
TRLR level 3 Channel 30:600
TRLR level 4 Channel 30:700
TBM level 0 Channel  31:270
TBM level 1 Channel  31:400
TBM level 2 Channel  31:500
TBM level 3 Channel  31:600
TBM level 4 Channel  31:700
ROC0 level 0 Channel  31 :270
ROC0 level 1 Channel  31 :400
ROC0 level 2 Channel  31 :500
ROC0 level 3 Channel  31 :600
ROC0 level 4 Channel  31 :700
ROC1 level 0 Channel  31 :270
ROC1 level 1 Channel  31 :400
ROC1 level 2 Channel  31 :500
ROC1 level 3 Channel  31 :600
ROC1 level 4 Channel  31 :700
ROC2 level 0 Channel  31 :270
ROC2 level 1 Channel  31 :400
ROC2 level 2 Channel  31 :500
ROC2 level 3 Channel  31 :600
ROC2 level 4 Channel  31 :700
ROC3 level 0 Channel  31 :270
ROC3 level 1 Channel  31 :400
ROC3 level 2 Channel  31 :500
ROC3 level 3 Channel  31 :600
ROC3 level 4 Channel  31 :700
ROC4 level 0 Channel  31 :270
ROC4 level 1 Channel  31 :400
ROC4 level 2 Channel  31 :500
ROC4 level 3 Channel  31 :600
ROC4 level 4 Channel  31 :700
ROC5 level 0 Channel  31 :270
ROC5 level 1 Channel  31 :400
ROC5 level 2 Channel  31 :500
ROC5 level 3 Channel  31 :600
ROC5 level 4 Channel  31 :700
ROC6 level 0 Channel  31 :270
ROC6 level 1 Channel  31 :400
ROC6 level 2 Channel  31 :500
ROC6 level 3 Channel  31 :600
ROC6 level 4 Channel  31 :700
ROC7 level 0 Channel  31 :270
ROC7 level 1 Channel  31 :400
ROC7 level 2 Channel  31 :500
ROC7 level 3 Channel  31 :600
ROC7 level 4 Channel  31 :700
TRLR level 0 Channel 31:270
TRLR level 1 Channel 31:400
TRLR level 2 Channel 31:500
TRLR level 3 Channel 31:600
TRLR level 4 Channel 31:700
TBM level 0 Channel  32:270
TBM level 1 Channel  32:400
TBM level 2 Channel  32:500
TBM level 3 Channel  32:600
TBM level 4 Channel  32:700
ROC0 level 0 Channel  32 :270
ROC0 level 1 Channel  32 :400
ROC0 level 2 Channel  32 :500
ROC0 level 3 Channel  32 :600
ROC0 level 4 Channel  32 :700
ROC1 level 0 Channel  32 :270
ROC1 level 1 Channel  32 :400
ROC1 level 2 Channel  32 :500
ROC1 level 3 Channel  32 :600
ROC1 level 4 Channel  32 :700
ROC2 level 0 Channel  32 :270
ROC2 level 1 Channel  32 :400
ROC2 level 2 Channel  32 :500
ROC2 level 3 Channel  32 :600
ROC2 level 4 Channel  32 :700
ROC3 level 0 Channel  32 :270
ROC3 level 1 Channel  32 :400
ROC3 level 2 Channel  32 :500
ROC3 level 3 Channel  32 :600
ROC3 level 4 Channel  32 :700
ROC4 level 0 Channel  32 :270
ROC4 level 1 Channel  32 :400
ROC4 level 2 Channel  32 :500
ROC4 level 3 Channel  32 :600
ROC4 level 4 Channel  32 :700
ROC5 level 0 Channel  32 :270
ROC5 level 1 Channel  32 :400
ROC5 level 2 Channel  32 :500
ROC5 level 3 Channel  32 :600
ROC5 level 4 Channel  32 :700
ROC6 level 0 Channel  32 :270
ROC6 level 1 Channel  32 :400
ROC6 level 2 Channel  32 :500
ROC6 level 3 Channel  32 :600
ROC6 level 4 Channel  32 :700
ROC7 level 0 Channel  32 :270
ROC7 level 1 Channel  32 :400
ROC7 level 2 Channel  32 :500
ROC7 level 3 Channel  32 :600
ROC7 level 4 Channel  32 :700
TRLR level 0 Channel 32:270
TRLR level 1 Channel 32:400
TRLR level 2 Channel 32:500
TRLR level 3 Channel 32:600
TRLR level 4 Channel 32:700
TBM level 0 Channel  33:270
TBM level 1 Channel  33:400
TBM level 2 Channel  33:500
TBM level 3 Channel  33:600
TBM level 4 Channel  33:700
ROC0 level 0 Channel  33 :270
ROC0 level 1 Channel  33 :400
ROC0 level 2 Channel  33 :500
ROC0 level 3 Channel  33 :600
ROC0 level 4 Channel  33 :700
ROC1 level 0 Channel  33 :270
ROC1 level 1 Channel  33 :400
ROC1 level 2 Channel  33 :500
ROC1 level 3 Channel  33 :600
ROC1 level 4 Channel  33 :700
ROC2 level 0 Channel  33 :270
ROC2 level 1 Channel  33 :400
ROC2 level 2 Channel  33 :500
ROC2 level 3 Channel  33 :600
ROC2 level 4 Channel  33 :700
ROC3 level 0 Channel  33 :270
ROC3 level 1 Channel  33 :400
ROC3 level 2 Channel  33 :500
ROC3 level 3 Channel  33 :600
ROC3 level 4 Channel  33 :700
ROC4 level 0 Channel  33 :270
ROC4 level 1 Channel  33 :400
ROC4 level 2 Channel  33 :500
ROC4 level 3 Channel  33 :600
ROC4 level 4 Channel  33 :700
ROC5 level 0 Channel  33 :270
ROC5 level 1 Channel  33 :400
ROC5 level 2 Channel  33 :500
ROC5 level 3 Channel  33 :600
ROC5 level 4 Channel  33 :700
ROC6 level 0 Channel  33 :270
ROC6 level 1 Channel  33 :400
ROC6 level 2 Channel  33 :500
ROC6 level 3 Channel  33 :600
ROC6 level 4 Channel  33 :700
ROC7 level 0 Channel  33 :270
ROC7 level 1 Channel  33 :400
ROC7 level 2 Channel  33 :500
ROC7 level 3 Channel  33 :600
ROC7 level 4 Channel  33 :700
TRLR level 0 Channel 33:270
TRLR level 1 Channel 33:400
TRLR level 2 Channel 33:500
TRLR level 3 Channel 33:600
TRLR level 4 Channel 33:700
TBM level 0 Channel  34:270
TBM level 1 Channel  34:400
TBM level 2 Channel  34:500
TBM level 3 Channel  34:600
TBM level 4 Channel  34:700
ROC0 level 0 Channel  34 :270
ROC0 level 1 Channel  34 :400
ROC0 level 2 Channel  34 :500
ROC0 level 3 Channel  34 :600
ROC0 level 4 Channel  34 :700
ROC1 level 0 Channel  34 :270
ROC1 level 1 Channel  34 :400
ROC1 level 2 Channel  34 :500
ROC1 level 3 Channel  34 :600
ROC1 level 4 Channel  34 :700
ROC2 level 0 Channel  34 :270
ROC2 level 1 Channel  34 :400
ROC2 level 2 Channel  34 :500
ROC2 level 3 Channel  34 :600
ROC2 level 4 Channel  34 :700
ROC3 level 0 Channel  34 :270
ROC3 level 1 Channel  34 :400
ROC3 level 2 Channel  34 :500
ROC3 level 3 Channel  34 :600
ROC3 level 4 Channel  34 :700
ROC4 level 0 Channel  34 :270
ROC4 level 1 Channel  34 :400
ROC4 level 2 Channel  34 :500
ROC4 level 3 Channel  34 :600
ROC4 level 4 Channel  34 :700
ROC5 level 0 Channel  34 :270
ROC5 level 1 Channel  34 :400
ROC5 level 2 Channel  34 :500
ROC5 level 3 Channel  34 :600
ROC5 level 4 Channel  34 :700
ROC6 level 0 Channel  34 :270
ROC6 level 1 Channel  34 :400
ROC6 level 2 Channel  34 :500
ROC6 level 3 Channel  34 :600
ROC6 level 4 Channel  34 :700
ROC7 level 0 Channel  34 :270
ROC7 level 1 Channel  34 :400
ROC7 level 2 Channel  34 :500
ROC7 level 3 Channel  34 :600
ROC7 level 4 Channel  34 :700
TRLR level 0 Channel 34:270
TRLR level 1 Channel 34:400
TRLR level 2 Channel 34:500
TRLR level 3 Channel 34:600
TRLR level 4 Channel 34:700
TBM level 0 Channel  35:270
TBM level 1 Channel  35:400
TBM level 2 Channel  35:500
TBM level 3 Channel  35:600
TBM level 4 Channel  35:700
ROC0 level 0 Channel  35 :270
ROC0 level 1 Channel  35 :400
ROC0 level 2 Channel  35 :500
ROC0 level 3 Channel  35 :600
ROC0 level 4 Channel  35 :700
ROC1 level 0 Channel  35 :270
ROC1 level 1 Channel  35 :400
ROC1 level 2 Channel  35 :500
ROC1 level 3 Channel  35 :600
ROC1 level 4 Channel  35 :700
ROC2 level 0 Channel  35 :270
ROC2 level 1 Channel  35 :400
ROC2 level 2 Channel  35 :500
ROC2 level 3 Channel  35 :600
ROC2 level 4 Channel  35 :700
ROC3 level 0 Channel  35 :270
ROC3 level 1 Channel  35 :400
ROC3 level 2 Channel  35 :500
ROC3 level 3 Channel  35 :600
ROC3 level 4 Channel  35 :700
ROC4 level 0 Channel  35 :270
ROC4 level 1 Channel  35 :400
ROC4 level 2 Channel  35 :500
ROC4 level 3 Channel  35 :600
ROC4 level 4 Channel  35 :700
ROC5 level 0 Channel  35 :270
ROC5 level 1 Channel  35 :400
ROC5 level 2 Channel  35 :500
ROC5 level 3 Channel  35 :600
ROC5 level 4 Channel  35 :700
ROC6 level 0 Channel  35 :270
ROC6 level 1 Channel  35 :400
ROC6 level 2 Channel  35 :500
ROC6 level 3 Channel  35 :600
ROC6 level 4 Channel  35 :700
ROC7 level 0 Channel  35 :270
ROC7 level 1 Channel  35 :400
ROC7 level 2 Channel  35 :500
ROC7 level 3 Channel  35 :600
ROC7 level 4 Channel  35 :700
TRLR level 0 Channel 35:270
TRLR level 1 Channel 35:400
TRLR level 2 Channel 35:500
TRLR level 3 Channel 35:600
TRLR level 4 Channel 35:700
TBM level 0 Channel  36:270
TBM level 1 Channel  36:400
TBM level 2 Channel  36:500
TBM level 3 Channel  36:600
TBM level 4 Channel  36:700
ROC0 level 0 Channel  36 :270
ROC0 level 1 Channel  36 :400
ROC0 level 2 Channel  36 :500
ROC0 level 3 Channel  36 :600
ROC0 level 4 Channel  36 :700
ROC1 level 0 Channel  36 :270
ROC1 level 1 Channel  36 :400
ROC1 level 2 Channel  36 :500
ROC1 level 3 Channel  36 :600
ROC1 level 4 Channel  36 :700
ROC2 level 0 Channel  36 :270
ROC2 level 1 Channel  36 :400
ROC2 level 2 Channel  36 :500
ROC2 level 3 Channel  36 :600
ROC2 level 4 Channel  36 :700
ROC3 level 0 Channel  36 :270
ROC3 level 1 Channel  36 :400
ROC3 level 2 Channel  36 :500
ROC3 level 3 Channel  36 :600
ROC3 level 4 Channel  36 :700
ROC4 level 0 Channel  36 :270
ROC4 level 1 Channel  36 :400
ROC4 level 2 Channel  36 :500
ROC4 level 3 Channel  36 :600
ROC4 level 4 Channel  36 :700
ROC5 level 0 Channel  36 :270
ROC5 level 1 Channel  36 :400
ROC5 level 2 Channel  36 :500
ROC5 level 3 Channel  36 :600
ROC5 level 4 Channel  36 :700
ROC6 level 0 Channel  36 :270
ROC6 level 1 Channel  36 :400
ROC6 level 2 Channel  36 :500
ROC6 level 3 Channel  36 :600
ROC6 level 4 Channel  36 :700
ROC7 level 0 Channel  36 :270
ROC7 level 1 Channel  36 :400
ROC7 level 2 Channel  36 :500
ROC7 level 3 Channel  36 :600
ROC7 level 4 Channel  36 :700
TRLR level 0 Channel 36:270
TRLR level 1 Channel 36:400
TRLR level 2 Channel 36:500
TRLR level 3 Channel 36:600
TRLR level 4 Channel 36:700
Channel Enbable bits chnls 1-9  (on = 0):0x1ff
Channel Enbable bits chnls 10-18(on = 0):0x1ff
Channel Enbable bits chnls 19-27(on = 0):0x1ff
Channel Enbable bits chnls 28-36(on = 0):0x1ff
TTCrx Coarse Delay Register 2:7
TTCrc      ClkDes2 Register 3:0x13
TTCrc Fine Dlay ClkDes2 Reg 1:5
Center Chip Control Reg:0xf0010
Initial Slink DAQ mode:0
Channel ADC Gain bits chnls  1-12(1Vpp = 0):0x0
Channel ADC Gain bits chnls 13-20(1Vpp = 0):0x0
Channel ADC Gain bits chnls 21-28(1Vpp = 0):0x0
Channel ADC Gain bits chnls 29-36(1Vpp = 0):0x0
Channel Baseline Enbable chnls 1-9  (on = (0x1ff<<16)+):0x14a
Channel Baseline Enbable chnls 10-18(on = (0x1ff<<16)+):0x14a
Channel Baseline Enbable chnls 19-27(on = (0x1ff<<16)+):0x14a
Channel Baseline Enbable chnls 28-36(on = (0x1ff<<16)+):0x14a
TBM trailer mask chnls 1-9  (0xff = all masked):0x2
TBM trailer mask chnls 10-18(0xff = all masked):0x2
TBM trailer mask chnls 19-27(0xff = all masked):0x2
TBM trailer mask chnls 28-36(0xff = all masked):0x2
Private 8 bit word chnls 1-9  :0xb1
Private 8 bit word chnls 10-18:0xb2
Private 8 bit word chnls 19-27:0xb3
Private 8 bit word chnls 28-36:0xb4
Special Random testDAC mode (on = 0x1, off=0x0):0x0
Number of Consecutive (max 1023) Out of Syncs till TTs OOS set:64
Number of Consecutive (max 1023) Empty events till TTs ERR set:64
N Fifo-1 almost full level,sets TTs BUSY (max 1023):900
NC Fifo-1 almost full level,sets TTs BUSY (max 1023):900
SC Fifo-1 almost full level,sets TTs BUSY (max 1023):900
S Fifo-1 almost full level,sets TTs BUSY (max 1023):900
Fifo-3 almost full level,sets TTs WARN (max 8191):7680
FED Master delay 0=0,1=32,2=48,3=64:1
TTCrx Register 0 fine delay ClkDes1:14
Params FED file check word:20211
N fifo-1 hit limit (max 1023 (hard) 900 (soft):800
NC fifo-1 hit limit (max 1023 (hard) 900 (soft):800
SC fifo-1 hit limit (max 1023 (hard) 900 (soft):800
S fifo-1 hit limit (max 1023 (hard) 900 (soft):800
N  testreg:0x2303
NC testreg:0x80000000
SC testreg:0x0
S  testreg:0x80002303
Set BUSYWHENBEHIND by this many triggers with timeouts:8
D[0]=1 enable fed-stuck reset D[1]=1 disable ev# protect(dont):0x1
Limit for fifo-2 almost full (point for the TTS flag):0x16A8
Limit for consecutive timeout OR OOSs:200
Turn off filling of lastdac fifos(exc 1st ROC):0
Number of simulated hits per ROC for internal generator:0
Miniumum hold time for busy (changing definition):0
Trigger Holdoff in units of 25us(0=none):0
Spare fedcard input 1:0
Spare fedcard input 2:0
Spare fedcard input 3:0
Spare fedcard input 4:0
Spare fedcard input 5:0
Spare fedcard input 6:0
Spare fedcard input 7:0
Spare fedcard input 8:0
Spare fedcard input 9:0
Spare fedcard input 10:0
'''

path = os.path.join(HC, 'fedcard')
if not os.path.isdir(path):
    os.makedirs(path)
for fed_id, fed_crate, fed_uri in feds_used:
    fn = os.path.join(path, 'params_fed_%i.dat' % fed_id)
    open(fn, 'wt').write(t_fedcard % locals())


print 'now you need to run unpack_dougs_configs.sh!'
