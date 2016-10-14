#!/usr/bin/env python

import sys, os
from JMTTools import *
from JMTROOTTools import *
set_style()

class Graph:
    def _eat(self, prefix, to_int=False):
        x = self.lines.pop(0)
        assert x.startswith(prefix)
        x = x.split()[-1]
        if to_int:
            x = int(x)
        return x
        
    def __init__(self, fn):
        self.fn = fn
        self.lines = [line.strip() for line in open(fn) if line.strip()]

        self.portcard = self._eat('Portcard: ')
        self.module = self._eat('Module: ')
        self.sda_origin = self._eat('SDaOrigin: ', True)
        self.rda_origin = self._eat('RDaOrigin: ', True)
        self.sda_range = self._eat('SDaRange: ', True)
        self.rda_range = self._eat('RDaRange: ', True)
        self.grid_size = self._eat('GridSize: ', True)
        self.ntests = self._eat('Tests: ', True)
        assert self.lines.pop(0) == 'GridScan:'

        self.graph = {}

        line = self.lines.pop(0)
        while not line.startswith('SelectedPoint:') and not line.startswith('NoStableRegion'):
            line = line.split()
            assert len(line) == 3
            sda, rda, nsuccess = line
            sda, rda, nsuccess = int(sda), int(rda), int(nsuccess)
            assert nsuccess <= self.ntests
            self.graph[(sda, rda)] = nsuccess
            line = self.lines.pop(0)
        if line != 'NoStableRegion':
            selected = self.lines.pop(0)
            assert not self.lines
            selected = selected.split()
            assert len(selected) == 3
            self.selected_sda      = int(selected[0])
            self.selected_rda      = int(selected[1])
            self.selected_nsuccess = int(selected[2])

    def plot(self):
        h = ROOT.TH2F(self.module, self.module + ';SDa;RDa',
                      self.sda_range / self.grid_size, self.sda_origin, self.sda_origin + self.sda_range,
                      self.rda_range / self.grid_size, self.rda_origin, self.rda_origin + self.rda_range)
        h.SetStats(0)
        for key, val in self.graph.iteritems():
            sda, rda = key
            bin = h.FindBin(sda, rda)
            h.SetBinContent(bin, float(val) / self.ntests)
        return h


c = ROOT.TCanvas('c', '', 1000, 1000)
graphs = [(x, Graph(x)) for x in sys.argv if os.path.isfile(x) and x.startswith('graph')]
for fn, g in graphs:
    print fn
    fn = fn.replace('.dat', '.png')
    h = g.plot()
    h.Draw('colz')
    c.SaveAs(fn)
