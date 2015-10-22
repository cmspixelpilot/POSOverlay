import sys, os, tempfile
from array import array

if os.environ.has_key('JMT_ROOTTOOLS_NOBATCHMODE'):
    import ROOT
else:
    sys.argv.append('-b')     # Start ROOT in batch mode;
    import ROOT; ROOT.TCanvas # make sure libGui gets initialized while '-b' is specified;
    sys.argv.remove('-b')     # and don't mess up sys.argv.

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

def set_style(date_pages=False):
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
    ROOT.gErrorIgnoreLevel = 1001 # Suppress TCanvas::SaveAs messages.
