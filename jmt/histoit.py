import sys
from ROOT import TH1F

d = {}

for p in sys.argv[1:]:
    a,b = p.split(':')
    a,b = int(a), int(b)
    d[a] = b

mn = min(d.keys())
mx = max(d.keys())

h = TH1F('h', '', mx-mn+1, mn, mx+1)
for a,b in d.iteritems():
    h.Fill(a, b)

h.Draw()

