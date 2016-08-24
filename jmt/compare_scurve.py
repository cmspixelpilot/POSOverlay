from JMTTools import *
from JMTROOTTools import *
set_style(True)
ROOT.gStyle.SetOptStat(111111)

from write_other_hc_configs import doer, HC, module_sorter_by_portcard_phi

modtests_dir = '/home/fnaltest/SCurveInfo/BmI_Configs_m20_SCurveInfo'
#disk, daq_dir = 1, '/home/fnaltest/SCurveInfo/BmI_disk1_runs-1227-1238'
#disk, daq_dir = 2, '/home/fnaltest/SCurveInfo/BmI_disk2_run1223'
disk, daq_dir = 3, '/home/fnaltest/SCurveInfo/BmI_disk3_run1177'

the_doer = doer(disk)

def convert_doug(fn):
    print "unpacking doug's pickle"
    x = cPickle.load(open(fn, 'rb'))
    print 'done'
    os.system('top -abn 1 | head')
    d = fn.replace('.p', '')
    mkdir_p(d)
    print 'n = ', len(x)
    for iroc, (roc, v) in enumerate(x.iteritems()):
        if iroc % 7*16 == 0:
            print 'roc #', iroc
        seen = set()
        newl = [0]*4160
        for i, (c,r, th,sg) in enumerate(v):
            assert 0 <= c <= 51
            assert 0 <= r <= 79
            if th <= 0 or sg <= 0:
                print c,r,th,sg
            assert (c,r) not in seen
            seen.add((c,r))
            newl[c*80 + r] = (th,sg)
        newfn = os.path.join(d, roc)
        cPickle.dump(newl, open(newfn, 'wb'), -1)
        
def convert_trimdat(fn):
    d = fn.replace('.dat', '')
    mkdir_p(d)
    t = trim_dat(fn)
    for roc, l in t.ls.iteritems(): 
        l = [(e.th, e.sg) for e in l]
        newfn = os.path.join(d, roc)
        cPickle.dump(l, open(newfn, 'wb'), -1)

def comp(out_fn, daq_dir, modtests_dir):
    summaries = []
    class summary:
        def __init__(self):
            pass

    c = ROOT.TCanvas('c', '', 1920, 1000)
    c.Divide(4,2)
    c.cd(0)
    if not out_fn.endswith('.pdf'):
        out_fn += '.pdf'
    c.SaveAs(out_fn + '[')

    daq_fns = []
    modules = [m.name for m in sorted(the_doer.modules, key=module_sorter_by_portcard_phi) if the_doer.moduleOK(m)]
    for m in modules:
        for r in xrange(16):
            roc = m + '_ROC' + str(r)
            fn = os.path.join(daq_dir, roc)
            if os.path.isfile(fn):
                daq_fns.append(fn)

    for iroc, daq_fn in enumerate(daq_fns):
        #if iroc > 50: break
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
        #for h in (h_th_modtests, h_sg_modtests, h_th_daq, h_sg_daq):
        #    h.SetLineWidth(2)
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

            if daq_th != 0.:
                h_th_daq.Fill(daq_th)
            if daq_sg != 0.:
                h_sg_daq.Fill(daq_sg)

            if modtests_th != 0.:
                h_th_modtests.Fill(modtests_th)
            if modtests_sg != 0.:
                h_sg_modtests.Fill(modtests_sg)

            h_th_daq_v_modtests.Fill(modtests_th, daq_th)
            h_sg_daq_v_modtests.Fill(modtests_sg, daq_sg)

            if daq_th != 0. and modtests_th != 0.:
                h_th_daq_m_modtests.Fill(daq_th - modtests_th)
            if daq_sg != 0. and modtests_sg != 0.:
                h_sg_daq_m_modtests.Fill(daq_sg - modtests_sg)

        s = summary()
        s.roc = roc
        s.daq_th_entries = h_th_daq.GetEntries()
        s.daq_th_mean = h_th_daq.GetMean()
        s.daq_th_rms = h_th_daq.GetRMS()
        s.daq_sg_entries = h_sg_daq.GetEntries()
        s.daq_sg_mean = h_sg_daq.GetMean()
        s.daq_sg_rms = h_sg_daq.GetRMS()
        s.modtests_th_entries = h_th_modtests.GetEntries()
        s.modtests_th_mean = h_th_modtests.GetMean()
        s.modtests_th_rms = h_th_modtests.GetRMS()
        s.modtests_sg_entries = h_sg_modtests.GetEntries()
        s.modtests_sg_mean = h_sg_modtests.GetMean()
        s.modtests_sg_rms = h_sg_modtests.GetRMS()
        summaries.append(s)
        
        for h in (h_th_modtests, h_th_daq):
            h.GetYaxis().SetRangeUser(0.1, 800)
        for h in (h_sg_modtests, h_sg_daq):
            h.GetYaxis().SetRangeUser(0.1, 1100)
        h_th_daq_m_modtests.GetYaxis().SetRangeUser(0, 1000)
        h_sg_daq_m_modtests.GetYaxis().SetRangeUser(0, 350)

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
        c.SaveAs(out_fn)

    c.SaveAs(out_fn + ']')

    return summaries

def draw_summaries(out_fn, summaries):
    hs = defaultdict(dict)
    def _h(s, name, title, *binning):
        mod = the_doer.modules_by_name[s.roc.split('_ROC')[0]]
        pc = mod.portcard.replace('FPix_', '')
        hn = name + '_' + pc
        ht = pc + title
        if not hs[pc].has_key(name):
            assert len(binning) in (3,6)
            H = ROOT.TH1F if len(binning) == 3 else ROOT.TH2F
            h = H(hn, ht, *binning)
            if H == ROOT.TH2F:
                h.SetStats(0)
            if 'modtests' in hn:
                h.SetLineColor(2)
            h.SetLineWidth(2)
            hs[pc][name] = h
        return hs[pc][name]

    for s in summaries:
        nback = 10
        _h(s, 'h_entries_daq_v_modtests_th', ';modtests threshold entries;daq threshold entries', nback, 4161-nback, 4161, nback, 4161-nback, 4161).Fill(s.modtests_th_entries, s.daq_th_entries)
        _h(s, 'h_entries_daq_v_modtests_sg', ';modtests width entries;daq width entries',         nback, 4161-nback, 4161, nback, 4161-nback, 4161).Fill(s.modtests_sg_entries, s.daq_sg_entries)
        _h(s, 'h_mean_daq_th',      ';threshold mean;rocs', 80, 10, 60).Fill(s.daq_th_mean)
        _h(s, 'h_mean_modtests_th', ';threshold mean;rocs', 80, 10, 60).Fill(s.modtests_th_mean)
        _h(s, 'h_mean_daq_sg',      ';width mean;rocs', 80, 0, 10).Fill(s.daq_sg_mean)
        _h(s, 'h_mean_modtests_sg', ';width mean;rocs', 80, 0, 10).Fill(s.modtests_sg_mean)
        _h(s, 'h_rms_daq_th',      ';threshold rms;rocs', 80, 0, 3).Fill(s.daq_th_rms)
        _h(s, 'h_rms_modtests_th', ';threshold rms;rocs', 80, 0, 3).Fill(s.modtests_th_rms)
        _h(s, 'h_rms_daq_sg',      ';width rms;rocs', 80, 0, 3).Fill(s.daq_sg_rms)
        _h(s, 'h_rms_modtests_sg', ';width rms;rocs', 80, 0, 3).Fill(s.modtests_sg_rms)

    #hs = dict(hs)
    #return hs

    c = ROOT.TCanvas('c', '', 1920, 1000)
    c.Divide(3,2)
    if not out_fn.endswith('.pdf'):
        out_fn += '.pdf'
    c.SaveAs(out_fn + '[')

    for pc, hpc in sorted(hs.items()):
        c.cd(1)
        hpc['h_entries_daq_v_modtests_th'].Draw('colz text00')
        c.cd(4)
        hpc['h_entries_daq_v_modtests_sg'].Draw('colz text00')

        for i,x in enumerate(['mean', 'rms']):
            for j,y in enumerate(['th', 'sg']):
                p = c.cd(3*j+2+i)
                hpc['h_%s_modtests_%s' % (x,y)].Draw()
                hpc['h_%s_daq_%s' % (x,y)].Draw('sames')
                p.Update()
                differentiate_stat_box(hpc['h_%s_daq_%s' % (x,y)], 0)
                differentiate_stat_box(hpc['h_%s_modtests_%s' % (x,y)], 1)

        c.cd(0)
        c.SaveAs(out_fn)

    c.SaveAs(out_fn + ']')

#convert_doug('BpO_Configs_m20_SCurveInfo.p')
#convert_trimdat('disk1_runs-1227-1238.dat')
summaries = comp('comp_disk%i' % disk, daq_dir, modtests_dir)
draw_summaries('summary_disk%i' % disk, summaries)
              
#to_pdf()
