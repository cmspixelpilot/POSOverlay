#*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
#*-*
#*-*  This program creates :
#*-*    - a one dimensional histogram
#*-*    - a two dimensional histogram
#*-*    - a profile histogram
#*-*    - a memory-resident ntuple
#*-*
#*-*  These objects are filled with some random numbers and saved on a file.
#*-*
#*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*

from ROOT import TCanvas, TFile, TProfile, TH1F, TH2F
from ROOT import gROOT, gBenchmark, gRandom, gSystem, Double


gROOT.Reset()

# Create a new canvas, and customize it.
c1 = TCanvas( 'c1', 'Dynamic Filling Example', 200, 10, 700, 500 )

# Create a new ROOT binary machine independent file.
# Note that this file may contain any kind of ROOT objects, histograms,
# pictures, graphics objects, detector geometries, tracks, events, etc..
# This file is now becoming the current directory.

hfile = gROOT.FindObject( 'hsimple.root' )
if hfile:
   hfile.Close()
hfile = TFile( 'hsimple.root', 'RECREATE', 'Demo ROOT file with histograms' )

# Create some histograms, a profile histogram 
hpx    = TH1F( 'hpx', 'This is the px distribution', 100, -4, 4 )


# Set canvas/frame attributes.
hpx.SetFillColor( 48 )



# Initialize random number generator.
gRandom.SetSeed()
rannor, rndm = gRandom.Rannor, gRandom.Rndm

# For speed, bind and cache the Fill member functions,
histos = [ 'hpx' ]
for name in histos:
   exec '%sFill = %s.Fill' % (name,name)

# Fill histograms randomly.
px = Double()
 # Fill histograms.
hpxFill( px )
hpx.Draw()

c1.Modified()
c1.Update()

 


# Save all objects in this file.

hfile.Write()
c1.Modified()
c1.Update()
  
# Note that the file is automatically closed when application terminates
# or when the file destructor is called.
