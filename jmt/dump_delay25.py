#!/usr/bin/env python

import sys, os
from JMTucker.Tools.ROOTTools import *
set_style()

#in_fn = sys.argv[1]
#out_dir = sys.argv[2]

def foo(f, path, out_dir, html):
    path_mang = path.replace('/', '_')
    d = f.Get(path)
    info = []
    if not d:
        msg = 'no %s found in %s' % (path, f.GetName())
        print msg
        html.write('<h1>%s</h1>' % msg)
    else:
        header = None
        these = []
        keys = f.Get(path).GetListOfKeys()
        lkeys = len(keys)
        for i, key in enumerate(keys):
            c = key.ReadObj()
            if i % 6 == 0:
                header = c.GetName()
                if 'and all' in header:
                    assert i == lkeys - 1
                    header = header.replace('RDa vs. SDa for portcard ', '')
                    fn = header.replace(' and all modules', 'allmodules') + '.png'
                    these = [fn]
                    info.append((header,these))
                else:
                    portcard, module = header.replace('RDa vs. SDa for portcard ', '').split(' and module ')
                    mang = '%s+%s' % (portcard, module)
                    fn = mang + '_all.png'
                    these = [fn]
                c.SaveAs(os.path.join(out_dir, fn))
            else:
                fn = mang + '_cmd%i.png' % i
                these.append(fn)
                c.SaveAs(os.path.join(out_dir, fn))
            if i % 6 == 5:
                info.append((header,these))
            print i, fn, len(info), len(these)
        for header, these in info:
            html.write('''
<br>
<h1>%(header)s</h1>
''' % locals())
                
            if len(these) == 6:
                fnall, fn1, fn2, fn3, fn4, fn5 = these
                html.write('''
<table>
  <tr>
    <th rowspan="2"><img src="%(fnall)s"></th>
    <th><img src="%(fn1)s" height="386" width="348"></th>
    <th><img src="%(fn2)s" height="386" width="348"></th>
    <th><img src="%(fn3)s" height="386" width="348"></th>
  </tr>
  <tr>
    <td><img src="%(fn4)s" height="386" width="348"></td>
    <td><img src="%(fn5)s" height="386" width="348"></td>
    <td></td>
  </tr>
</table>
''' % locals())
            else:
                fnall = these[0]
                html.write('<img src="%(fnall)s">\n' % locals())



#foo('/uscms/home/tucker/afshome/delay25_1.root', 'Pilt/Pilt_BmI/Pilt_BmI_D3', '/uscms/home/tucker/asdf/plots/dump_delay25')

def foo2(in_fn, out_dir):
    print ("mkdir -p '%s'" % out_dir)
    os.system("mkdir -p '%s'" % out_dir)

    html_fn = os.path.join(out_dir, 'index.html')
    html = open(html_fn, 'wt')
    html.write('<html><body>\n')

    f = ROOT.TFile(in_fn)

    for which in 'IO':
        html.write('<h1>Bm%s</h1>\n' % which)
        foo(f, 'Pilt/Pilt_Bm%s/Pilt_Bm%s_D3' % (which, which), out_dir, html)
    
    html.write('</body></html>\n')
    html.close()

#foo2('/uscms/home/tucker/afshome/delay25_1.root', '/uscms/home/tucker/asdf/plots/dump_delay25')

if 0:
    for date, in_fn in [
        ('Jan  8 22.25', '/uscms/home/tucker/afshome/delay25_1.root'),
        ('Jan 10 11.23', '/uscms/home/tucker/afshome/delay25_1-1.root'),
        ('Jan 10 11.31', '/uscms/home/tucker/afshome/delay25_1-2.root'),
        ('Jan 10 12.24', '/uscms/home/tucker/afshome/delay25_1-3.root'),
        ('Jan 10 13.00', '/uscms/home/tucker/afshome/delay25_1-4.root'),
        ('Jan 15 09.59', '/uscms/home/tucker/afshome/delay25_1-5.root'),
        ('Jan 15 11.46', '/uscms/home/tucker/afshome/delay25_1-6.root'),
        ('Jan 15 11.54', '/uscms/home/tucker/afshome/delay25_1-7.root'),
        ('Jan 15 11.58', '/uscms/home/tucker/afshome/delay25_1-8.root'),
        ('Jan 16 13.34', '/uscms/home/tucker/afshome/delay25_1-9.root'),
        ]:
        print in_fn
        foo2(in_fn, '/uscms/home/tucker/asdf/plots/dump_delay25/%s' % date)

run = None
for x in sys.argv:
    try:
        run = int(x)
    except ValueError:
        pass
if run is None:
    raise ValueError('need run #')

foo2('delay25_1.root', '/uscms/home/tucker/asdf/plots/dump_delay25/Run%s' % run)
