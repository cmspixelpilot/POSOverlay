#!/usr/bin/env python

import sys, os
from JMTTools import *
from JMTROOTTools import *
set_style()

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

run = run_from_argv()
run_dir = run_dir(run)
in_fn = os.path.join(run_dir, 'delay25_1.root')
out_dir = os.path.join(run_dir, 'dump_delay25')
    
foo2(in_fn, out_dir)

if 'scp' in sys.argv:
    remote_dir = 'public_html/qwer/dump_delay25/%i' % run
    cmd = 'ssh jmt46@lnx201.lns.cornell.edu "mkdir -p %s"' % remote_dir
    print cmd
    os.system(cmd)
    cmd = 'scp -r %s/* jmt46@lnx201.lns.cornell.edu:%s' % (out_dir, remote_dir)
    print cmd
    os.system(cmd)
