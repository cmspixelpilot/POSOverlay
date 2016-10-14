from JMTTools import *
from JMTROOTTools import *
from write_other_hc_configs import cable_map_parser, HC, module_sorter_by_portcard_phi
set_style(light=True)

if len(sys.argv) < 2:
    print 'usage: scurve.py in_fn out_fn'
    sys.exit(1)

in_fn = sys.argv[1]
out_fn = sys.argv[2]

td = trim_dat(in_fn)

cable_map = cable_map_parser()
default_l = [0]*4160
threshold_c, width_c = None, None
threshold_out_fn = out_fn + '.thresholds.pdf'
width_out_fn = out_fn + '.widths.pdf'

for module in cable_map.modules_from_rocs(td.ls.keys(), module_sorter_by_portcard_phi):
    print module.name
    thresholdses = []
    widthses = []
    for i in xrange(16):
        roc = '%s_ROC%i' % (module.name, i)
        es = td.ls.get(roc, None)
        if es is None:
            thresholds = default_l
            widths = default_l
        else:
            thresholds, widths = [], []
            for e in es:
                if e == 0:
                    thresholds.append(0)
                    widths.append(0)
                else:
                    thresholds.append(e.th)
                    widths.append(e.sg)
        thresholdses.append(thresholds)
        widthses.append(widths)

    h_thresholdses = flat_to_module('thresholds', module.name, thresholdses)
    h_thresholds, fc, pt = fnal_pixel_plot(h_thresholdses, module.name, module.name, (20, 50), threshold_c)
    if threshold_c is None:
        threshold_c = fc
        threshold_c.SaveAs(threshold_out_fn + '[')
    threshold_c.SaveAs(threshold_out_fn)

    h_widthses = flat_to_module('widths', module.name, widthses)
    h_widths, fc, pt = fnal_pixel_plot(h_widthses, module.name, module.name, (0, 10), width_c)
    if width_c is None:
        width_c = fc
        width_c.SaveAs(width_out_fn + '[')
    width_c.SaveAs(width_out_fn)

threshold_c.SaveAs(threshold_out_fn + ']')
width_c.SaveAs(width_out_fn + ']')
