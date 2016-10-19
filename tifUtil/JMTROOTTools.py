import sys, os, tempfile
from array import array

if os.environ.has_key('JMT_ROOTTOOLS_NOBATCHMODE'):
    import ROOT
else:
    sys.argv.append('-b')     # Start ROOT in batch mode;
    import ROOT; ROOT.TCanvas # make sure libGui gets initialized while '-b' is specified;
    sys.argv.remove('-b')     # and don't mess up sys.argv.

# don't import this until after the above since it does a from ROOT import *
import moduleSummaryPlottingTools as FNAL

class plot_saver:
    i = 0
    
    def __init__(self, plot_dir=None, html=True, log=True, root=True, pdf=False, pdf_log=False, C=False, C_log=False, size=(820,630), per_page=-1, canvas_margins=None):
        self.c = ROOT.TCanvas('c%i' % plot_saver.i, '', *size)
        if canvas_margins is not None:
            if type(canvas_margins) == int or type(canvas_margins) == float:
                top, bottom, left, right = tuple(canvas_margins for x in xrange(4))
            else:
                top, bottom, left, right = canvas_margins
            self.c.SetTopMargin(top)
            self.c.SetBottomMargin(bottom)
            self.c.SetLeftMargin(left)
            self.c.SetRightMargin(right)
        plot_saver.i += 1
        self.saved = []
        self.html = html
        self.set_plot_dir(plot_dir)
        self.log = log
        self.root = root
        self.pdf = pdf
        self.pdf_log = pdf_log
        self.C = C
        self.C_log = C_log
        self.per_page = per_page

    def __del__(self):
        self.write_index()

    def update_canvas(self):
        self.c.Update()

    def anchor_name(self, fn):
        return os.path.splitext(os.path.basename(fn))[0].replace('.', '_').replace('/', '_')
    
    def write_index(self):
        if not self.saved or not self.html:
            return
        html = open(os.path.join(self.plot_dir, 'index.html'), 'wt')
        if self.per_page > 0:
            nsaved = len(self.saved)
            ndxs = range(0, nsaved, self.per_page)
            npages = len(ndxs)
            for page, ndx in enumerate(ndxs):
                self.write_index_page(self.saved[ndx:ndx+self.per_page], page, npages)
        else:
            self.write_index_page(self.saved, 0, 1)
            
    def write_index_page(self, saved, page, num_pages):
        def html_fn(page):
            if page == 0:
                return 'index.html'
            else:
                return 'index_%i.html' % page
            return 
        html = open(os.path.join(self.plot_dir, html_fn(page)), 'wt')
        html.write('<html><body><pre>\n')
        def write_pages_line():
            html.write('pages: ')
            for i in xrange(num_pages):
                if i == page:
                    html.write('<b>%i</b>  ' % i)
                else:
                    html.write('<a href="%s">%i</a>  ' % (html_fn(i), i))
            html.write('\n')
        if num_pages > 1:
            write_pages_line()
        html.write('<a href="..">.. (parent directory)</a>\n')
        for i, save in enumerate(saved):
            if type(save) == str:
                # this is just a directory link
                html.write('<a href="%s">%10i%32s%s</a>\n' % (save, i, 'change directory: ', save))
                continue

            fn, log, root, pdf, pdf_log, C, C_log = save

            bn = os.path.basename(fn)
            html.write('<a href="#%s">%10i</a> ' % (self.anchor_name(fn), i))
            if log:
                html.write(' <a href="%s">log</a>' % os.path.basename(log))
            else:
                html.write('    ')
            if root:
                html.write(' <a href="%s">root</a>' % os.path.basename(root))
            else:
                html.write('     ')
            if pdf:
                html.write(' <a href="%s">pdf</a>' % os.path.basename(pdf))
            else:
                html.write('     ')
            if pdf_log:
                html.write(' <a href="%s">pdf_log</a>' % os.path.basename(pdf_log))
            else:
                html.write('     ')
            if C:
                html.write(' <a href="%s">C</a>' % os.path.basename(C))
            else:
                html.write('     ')
            if C_log:
                html.write(' <a href="%s">C_log</a>' % os.path.basename(C_log))
            else:
                html.write('     ')
            html.write('  <a href="%s">%s</a>' % (bn, bn))
            html.write('\n')
        html.write('<br><br>')
        for i, save in enumerate(saved):
            if type(save) == str:
                continue # skip dir entries
            fn, log, root, pdf, pdf_log, C, C_log = save
            bn = os.path.basename(fn)
            rootlink = ', <a href="%s">root</a>' % os.path.basename(root) if root else ''
            html.write('<h4 id="%s"><a href="#%s">%s</a>%s</h4><br>\n' % (self.anchor_name(fn), self.anchor_name(fn), bn.replace('.png', ''), rootlink))
            if log:
                html.write('<img src="%s"><img src="%s"><br><br>\n' % (bn, os.path.basename(log)))
            else:
                html.write('<img src="%s"><br><br>\n' % bn)
        if num_pages > 1:
            write_pages_line()
        html.write('</pre></body></html>\n')
        
    def set_plot_dir(self, plot_dir):
        self.write_index()
        self.saved = []
        if plot_dir is not None and '~' in plot_dir:
            plot_dir = os.path.expanduser(plot_dir)
        self.plot_dir = plot_dir
        if plot_dir is not None:
            os.system('mkdir -p %s' % self.plot_dir)

    def save_dir(self, n):
        if self.plot_dir is None:
            raise ValueError('save_dir called before plot_dir set!')
        self.saved.append(n)

    def save(self, n, log=None, root=None, pdf=None, pdf_log=None, C=None, C_log=None, logz=None, other_c=None):
        can = self.c if other_c is None else other_c

        if logz:
            logfcn = can.SetLogz
        else:
            logfcn = can.SetLogy

        log = self.log if log is None else log
        root = self.root if root is None else root
        pdf = self.pdf if pdf is None else pdf
        pdf_log = self.pdf_log if pdf_log is None else pdf_log
        C = self.C if C is None else C
        C_log = self.C_log if C_log is None else C_log
        
        if self.plot_dir is None:
            raise ValueError('save called before plot_dir set!')
        can.SetLogy(0)
        fn = os.path.join(self.plot_dir, n + '.png')
        can.SaveAs(fn)
        if root:
            root = os.path.join(self.plot_dir, n + '.root')
            can.SaveAs(root)
        if log:
            logfcn(1)
            log = os.path.join(self.plot_dir, n + '_log.png')
            can.SaveAs(log)
            logfcn(0)
        if pdf:
            pdf = os.path.join(self.plot_dir, n + '.pdf')
            can.SaveAs(pdf)
        if pdf_log:
            logfcn(1)
            pdf_log = os.path.join(self.plot_dir, n + '_log.pdf')
            can.SaveAs(pdf_log)
            logfcn(0)
        if C:
            C = os.path.join(self.plot_dir, n + '.C')
            can.SaveAs(C_fn)
        if C_log:
            logfcn(1)
            C_log = os.path.join(self.plot_dir, n + '_log.C')
            can.SaveAs(C_log)
            logfcn(0)
        self.saved.append((fn, log, root, pdf, pdf_log, C, C_log))

def set_style(light=False, date_pages=False):
    ROOT.gStyle.SetPadTickX(1)
    ROOT.gStyle.SetPadTickY(1)
    ROOT.gErrorIgnoreLevel = 1001 # Suppress TCanvas::SaveAs messages.
    if not light:
        ROOT.gROOT.SetStyle('Plain')
        ROOT.gStyle.SetFillColor(0)
        if date_pages:
            ROOT.gStyle.SetOptDate()
        ROOT.gStyle.SetOptStat(1222222)
        ROOT.gStyle.SetOptFit(2222)
        ROOT.gStyle.SetPadTickX(1)
        ROOT.gStyle.SetPadTickY(1)
        ROOT.gStyle.SetMarkerSize(.1)
        ROOT.gStyle.SetMarkerStyle(8)
        ROOT.gStyle.SetGridStyle(3)
        ROOT.gStyle.SetStatW(0.25)
        ROOT.gStyle.SetStatFormat('6.4g')
        ROOT.gStyle.SetPalette(1)
        ROOT.gStyle.SetTitleFont(42, 'XYZ')
        ROOT.gStyle.SetLabelFont(42, 'XYZ')
        ROOT.gStyle.SetStatFont(42)
        ROOT.gStyle.SetLegendFont(42)

def differentiate_stat_box(hist, movement=1, new_color=None, new_size=None, color_from_hist=True, offset=None):
    """Move hist's stat box and change its line/text color. If
    movement is just an int, that number specifies how many units to
    move the box downward. If it is a 2-tuple of ints (m,n), the stat
    box will be moved to the left m units and down n units. A unit is
    the width or height of the stat box.
    Call TCanvas::Update first (and use TH1::Draw('sames') if
    appropriate) or else the stat box will not exist."""

    s = hist.FindObject('stats')

    if color_from_hist:
        new_color = hist.GetLineColor()

    if new_color is not None:
        s.SetTextColor(new_color)
        s.SetLineColor(new_color)

    if type(movement) == int:
        movement = (0,movement)
    m,n = movement
    
    x1,x2 = s.GetX1NDC(), s.GetX2NDC()
    y1,y2 = s.GetY1NDC(), s.GetY2NDC()

    if new_size is not None:
        x1 = x2 - new_size[0]
        y1 = y2 - new_size[1]

    if offset is None:
        ox, oy = 0, 0
    else:
        ox, oy = offset

    s.SetX1NDC(x1 - (x2-x1)*m + ox)
    s.SetX2NDC(x2 - (x2-x1)*m + ox)
    s.SetY1NDC(y1 - (y2-y1)*n + oy)
    s.SetY2NDC(y2 - (y2-y1)*n + oy)

def flat_to_module(label, module_name, lists, xform=None):
    ''' Take a list of 16 lists, each of which has 4160 values, one
    per pixel in row major format, and make TH2Fs suitable for shoving
    into the fnal_pixel_plot function below.

    label is an extra tag for the histograms so they can be unique

    xform is an optional transformation function of the form
    xform(label, module_name, rocnum, col, row, val) -> new_val.

    Values can be None if skipping desired.
    '''

    assert len(lists) == 16
    hs = []
    for iroc,l in enumerate(lists):
        roc = module_name + '_ROC' + str(iroc)
        h = ROOT.TH2F('h_%s_%s' % (label, roc), label + ' : ' + roc, 52, 0, 52, 80, 0, 80)
        h.SetStats(0)
        hs.append(h)
        for i,val in enumerate(l):
            col = i / 80
            row = i % 80
            if xform is not None:
                val = xform(label, module_name, iroc, col, row, val)
            if val is not None:
                h.SetBinContent(col+1, row+1, float(val))
    return hs

def fnal_pixel_plot(hs, module_name, title, z_range=None, existing_c=None):
    h = FNAL.makeMergedPlot(hs, 'pos')
    if z_range == 'auto':
        z_range = FNAL.findZRange(hs)
    if z_range is not None:
        FNAL.setZRange(h, z_range)

    fc = FNAL.setupSummaryCanvas(h, moduleName=module_name)
    if existing_c is not None:
        existing_c.cd()
        existing_c.Clear()
        fc.DrawClonePad()

    pt = ROOT.TPaveText(-100,405,1395,432)
    pt.AddText(title)
    pt.SetTextAlign(12)
    pt.SetTextFont(42)
    pt.SetFillColor(0)
    pt.SetBorderSize(0)
    pt.SetFillStyle(0)
    pt.Draw()

    return h, fc, pt
