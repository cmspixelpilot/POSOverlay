from JMTTools import *
from JMTROOTTools import *
set_style(True)
ROOT.gStyle.SetOptStat(111111)

def convert_doug(fn):
    x = cPickle.load(open(fn, 'rb'))
    d = fn.replace('.p', '')
    try:
        os.mkdir(d)
    except OSError:
        pass
    for roc, v in x.iteritems():
        seen = set()
        newl = [0]*4160
        for i, (c,r, th,sg) in enumerate(v):
            assert 0 <= c <= 51
            assert 0 <= r <= 79
            assert 0 <= th
            assert 0 <= sg
            assert (c,r) not in seen
            seen.add((c,r))
            newl[c*80 + r] = (th,sg)
        newfn = os.path.join(d, roc)
        cPickle.dump(newl, open(newfn, 'wb'), -1)
        
def convert_trimdat(fn):
    d = fn.replace('.dat', '')
    try:
        os.mkdir(d)
    except OSError:
        pass
    newls = defaultdict(lambda: [0]*4160)
    seens = defaultdict(set)
    for line in open(fn):
        line = line.strip()
        if line:
            line = line.split()
            assert line[0] == '[PixelSCurveHistoManager::fit()]RocName='
            assert line[1].startswith('FPix_')
            roc = line[1]
            seen = seens[roc]
            newl = newls[roc]
            r, c = int(line[2]), int(line[3])
            assert 0 <= c <= 51
            assert 0 <= r <= 79
            assert (c,r) not in seen
            seen.add((c,r))
            sg, th = float(line[4]), float(line[5])
            assert 0 <= th
            assert 0 <= sg
            newl[c*80 + r] = (th, sg)

    for roc, newl in newls.iteritems(): 
        newfn = os.path.join(d, roc)
        cPickle.dump(newl, open(newfn, 'wb'), -1)


def comp(daq_dir, modtests_dir):
    for ifn, daq_fn in enumerate(sorted(glob(os.path.join(daq_dir, 'FPix*')))):
        #if ifn > 20: break
        roc = os.path.basename(daq_fn)
        print roc

        modtests_fn = os.path.join(modtests_dir, roc)
        assert os.path.isfile(modtests_fn)

        daq_l   = cPickle.load(open(daq_fn,   'rb'))
        modtests_l = cPickle.load(open(modtests_fn, 'rb'))

        h_th_daq   = ROOT.TH1F('h_th_daq',   roc + ';threshold (vcal low);pixels/0.3', 100, 20, 50)
        h_sg_daq   = ROOT.TH1F('h_sg_daq',   roc + ';width (vcal low);pixels/0.06',    100, 0, 6)
        h_th_modtests = ROOT.TH1F('h_th_modtests', roc + ';threshold (vcal low);pixels/0.3', 100, 20, 50)
        h_sg_modtests = ROOT.TH1F('h_sg_modtests', roc + ';width (vcal low);pixels/0.06',    100, 0, 6)
        for h in (h_th_modtests, h_sg_modtests, h_th_daq, h_sg_daq):
            h.SetLineWidth(2)
        for h in (h_th_modtests, h_sg_modtests):
            h.SetLineColor(2)
        h_th_daq_v_modtests = ROOT.TH2F('h_th_daq_v_modtests', roc + ';modtests threshold (vcal low);daq threshold (vcal low)', 100, 20, 50, 100, 20, 50)
        h_sg_daq_v_modtests = ROOT.TH2F('h_sg_daq_v_modtests', roc + ';modtests width (vcal low);daq width (vcal low)', 100, 0, 6, 100, 0, 6)
        h_th_daq_m_modtests = ROOT.TH1F('h_th_daq_m_modtests', roc + ';daq threshold - modtests threshold  (vcal low);pixels/0.3', 100, -15, 15)
        h_sg_daq_m_modtests = ROOT.TH1F('h_sg_daq_m_modtests', roc + ';daq width - modtests width (vcal low);pixels/0.06', 100, -3, 3)
        hists = [h_th_daq, h_sg_daq, h_th_modtests, h_sg_modtests, h_th_daq_v_modtests, h_sg_daq_v_modtests, h_th_daq_m_modtests, h_sg_daq_m_modtests]

        for i, (daq, modtests) in enumerate(izip(daq_l, modtests_l)):
            if type(daq) != tuple:
                daq = (0.,0.)
            assert type(modtests) == tuple

            col = i / 80
            row = i % 80

            daq_th, daq_sg = daq
            modtests_th, modtests_sg = modtests

            h_th_daq.Fill(daq_th)
            h_sg_daq.Fill(daq_sg)

            h_th_modtests.Fill(modtests_th)
            h_sg_modtests.Fill(modtests_sg)

            h_th_daq_v_modtests.Fill(modtests_th, daq_th)
            h_sg_daq_v_modtests.Fill(modtests_sg, daq_sg)

            h_th_daq_m_modtests.Fill(-modtests_th + daq_th)
            h_sg_daq_m_modtests.Fill(-modtests_sg + daq_sg)

        for h in (h_th_modtests, h_th_daq):
            h.GetYaxis().SetRangeUser(0.1, 800)
        for h in (h_sg_modtests, h_sg_daq):
            h.GetYaxis().SetRangeUser(0.1, 1100)
        h_th_daq_m_modtests.GetYaxis().SetRangeUser(0, 1000)
        h_sg_daq_m_modtests.GetYaxis().SetRangeUser(0, 350)

        c = ROOT.TCanvas('c', '', 1920, 1000)
        c.Divide(4,2)
        p = c.cd(1)
        h_th_modtests.Draw()
        h_th_daq.Draw('sames')
        p.Update()
        differentiate_stat_box(h_th_modtests, 0)
        differentiate_stat_box(h_th_daq, 1)
        p = c.cd(2)
        p.SetLogy()
        h_th_modtests.Draw()
        h_th_daq.Draw('sames')
        p.Update()
        differentiate_stat_box(h_th_modtests, 0)
        differentiate_stat_box(h_th_daq, 1)
        p = c.cd(3)
        h_sg_modtests.Draw()
        h_sg_daq.Draw('sames')
        p.Update()
        differentiate_stat_box(h_sg_modtests, 0)
        differentiate_stat_box(h_sg_daq, 1)
        p = c.cd(4)
        p.SetLogy()
        h_sg_modtests.Draw()
        h_sg_daq.Draw('sames')
        p.Update()
        differentiate_stat_box(h_sg_modtests, 0)
        differentiate_stat_box(h_sg_daq, 1)
        c.cd(5)
        h_th_daq_v_modtests.Draw('colz')
        c.cd(6)
        h_sg_daq_v_modtests.Draw('colz')
        c.cd(7)
        h_th_daq_m_modtests.Draw()
        c.cd(8)
        h_sg_daq_m_modtests.Draw()
        c.cd(0)
        c.SaveAs(roc + '.png')

        del c
        for h in hists:
            del h

def to_pdf(HC, disk):
    assert HC == 'BmI' and disk == 3
    for pcnum,l in by_portcard.iteritems():
        these = [x + '_ROC%i.png' % roc for roc in xrange(16) for x in l.split()]
        these2 = [x for x in these if os.path.isfile(x)]
        print 'missing'
        pprint(sorted(set(these) - set(these2)))
        cmd = 'convert ' + ' '.join(these2) + ' %s_D%s_PRT%i.pdf' % (HC, disk, pcnum)
        print cmd
        os.system(cmd)

comp('/home/fnaltest/SCurveInfo/disk3_run1177', '/home/fnaltest/SCurveInfo/BmI_Configs_m20_SCurveInfo')
#to_pdf('BmI', 3)
