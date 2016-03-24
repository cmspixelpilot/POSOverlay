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

from ROOT import TCanvas, TFile, TProfile, TH1F, TH2F, TGraph, TMultiGraph, TLegend
from ROOT import gROOT, gBenchmark, gRandom, gSystem, Double
from array import array

gROOT.Reset()

# Create a new canvas, and customize it.
c1 = TCanvas( 'c1', 'Width ns vs bias', 200, 10, 700, 500 )
c2 = TCanvas( 'c2', 'Height mV vs bias', 200, 10, 700, 500 )
c3 = TCanvas( 'c3', 'Height/ref_Height vs bias ', 200, 10, 700, 500 )
# Create a new ROOT binary machine independent file.
# Note that this file may contain any kind of ROOT objects, histograms,
# pictures, graphics objects, detector geometries, tracks, events, etc..
# This file is now becoming the current directory.

hfile = gROOT.FindObject( 'hsimple.root' )
if hfile:
   hfile.Close()
hfile = TFile( 'hsimple.root', 'RECREATE', 'Demo ROOT file with histograms' )

# Create some histograms, a profile histogram 
# hpx    = TH1F( 'hpx', 'This is the px distribution', 100, -4, 4 )
# hpx.SetFillColor( 48 )

################################################################################################################################
# white fiber fed channel 7
# red fiber fed channel 6
#gain 3, bias scan of the eye quality
height=[]
width=[]
bias = array     ("d",  [10,   15,   20,   25,   30,   35,   40,   50,   60,   70,   80,   90,  100 ])
height.append(array("d",[0 ,  2.7,  4.9,  7.2,  8.5,  8.0,  9.4,  9.1,  8.7,  8.0,  7.0,  6.4,  4.2 ]))#mV, poh6 red fiber
height.append(array("d",[0 ,  3.5,  7.1,  9.9, 10.7, 10.4, 10.4,  9.5,  9.0,  8.1,  7.3,  6.5,  5.2 ]))#mV, poh6 white fiber
width.append(array("d" ,[0 , 0.47,  1.7,  1.6,  1.6, 1.51, 1.6,   1.6, 1.43, 1.57, 1.50, 1.46, 1.4 ]))#ns, poh6 red fiber
width.append(array("d" ,[0 , 1.11, 1.61, 1.37, 1.10, 1.16, 1.25, 1.31, 1.24, 1.17, 1.24, 1.18, 1.16 ]))#ns, poh6 white fiber
################################################################################################################################
#gain 3 bias 50
channel=[]
for i in range (0,13):
   channel.append(i)

        ### 12 11 10 9 8 7 6 5 4 3  2  1 FED
        ###  1  2 3 4 5 6 7 8 9 10 11 12 Fiber
        ### 12      11    10    9     8    7    6    5     4      3     2      1 FED
       ####  1      2     3     4     5    6    7    8     9     10    11     12 Fiber
       #### blue orange green brown gray white red black yellow violet pink acqua 
      ###   1     2     3     4     5     6    7     8     9    10  11 12 Fiber
heigth_ch=[-1,  5.9,  4.7,  6.5,  8.4,  7.8, 9.0,  7.1,  7.4,  -1, -1, -1]
widht_ch= [-1, 0.98, 1.54, 1.68, 1.49, 1.41, 1.6, 1.33, 1.29,  -1, -1, -1]

## fiber 1 and 10  and 11 badddddd
################################################################################################################################
# gain 3 bias 50 channel 7
# eye for different prescale
hz=array("d",[11223.0,1122.0,112.0, 11.2])
prescale=array("d",[1, 10, 100, 1000])
heigth_f=array("d",[7.9, 7.8, 7.7,7.8])
widht_f=array("d",[1.60, 1.63, 1.62,1.61])

################################################################################################################################
# red fiber, scan bias gain of the eye quality
bias1=array("d",[0, 25, 50, 75, 100])       
gain1=array("i",[0, 1, 2, 3])
heigth_b_g=[]           #bias 0   25   50   75 100
heigth_b_g.append(array("d",[ 0,   0,   0,   0, 0])) #mV gain 0
heigth_b_g.append(array("d",[ 0, 4.6, 3.1,   0, 0]))
heigth_b_g.append(array("d",[ 0, 7.2, 6.2, 3.9, 1.7]))
heigth_b_g.append(array("d",[ 0, 9.2, 8.6, 6.8, 4.2]))#mV gain 3
widht_b_g=[]           #bias 0   25   50   75 100
widht_b_g.append(array("d",[ 0,   0,   0,   0, 0]))#ns gain 0
widht_b_g.append(array("d",[ 0, 1.6, 1.2,   0, 0]))
widht_b_g.append(array("d",[ 0, 1.7, 1.7, 1.5, 1.1]))
widht_b_g.append(array("d",[ 0, 1.9, 1.6, 1.5, 1.4]))#ns gain 3
################################################################################################################################
color=[ "red", "white"]
kcolor=["kred","kgreen"]

width_g=[]
MGwidth=TMultiGraph()
LGwidth=TLegend(0.1,0.7,0.48,0.9)
for i in width :
    width_g.append(TGraph(int(len(bias)),bias,i))
k=1
for i in width_g :
   c1.cd()
   i.SetLineColor(k+1)
   MGwidth.Add(i)
   LGwidth.AddEntry(i,str(color[k-1])+ " fiber","alp")
   k=k+1
MGwidth.Draw("ApL")
LGwidth.Draw()
c1.Modified()
c1.Update()
c1.Write()

height_g=[]
MGheight=TMultiGraph()
LGheight=TLegend(0.1,0.7,0.48,0.9)
for i in height :
   height_g.append(TGraph(int(len(bias)),bias,i))
k=1
for i in height_g :
   c2.cd()
   i.SetLineColor(k+1)
   MGheight.Add(i)
   LGheight.AddEntry(i,str(color[k-1])+ " fiber","alp")
   k=k+1
MGheight.Draw("APL")
LGheight.Draw()
c2.Modified()
c2.Update()
c2.Write()


c3.cd()
ratio= TH1F("ratio","ratio",len(bias),0,100)
for i in range(len(bias)):
   if height[0][i]!= 0:
      if height[0][i]!= 0:
         ratio.Fill(bias[i],height[0][i]/height[1][i])
      
ratio.Draw()
c3.Modified()
c3.Update()
c3.Write()


 # hz=[11223,1122,112, 11.2]
# prescale=[1, 10, 100, 1000]
# heigth_f=[7.9, 7.8, 7.7,7.8]
# widht_f=[1.60, 1.63, 1.62,1.61]
c4= TCanvas()
# Save all objects in this file.
hf= TGraph(int(len(hz)),hz,heigth_f)
wf= TGraph(int(len(hz)),hz,widht_f)
c4.cd()
hf.Draw()
wf.Draw("same")
c4.Modified()
c4.Update()
c4.Write()



hfile.Write()
c1.Modified()
c1.Update()
c5= TCanvas()
c6= TCanvas()
# Note that the file is automatically closed when application terminates
# or when the file destructor is called.
# red fiber, scan bias gain of the eye quality
# bias1=[0, 25, 50, 75, 100]       
# gain1=[0, 1, 2, 3]
# heigth_b_g=[]           #bias 0   25   50   75 100
# heigth_b_g.append(array("d",[ 0,   0,   0,   0, 0])) #mV gain 0
# heigth_b_g.append(array("d",[ 0, 4.6, 3.1,   0, 0]))
# heigth_b_g.append(array("d",[ 0, 7.2, 6.2, 3.9, 1.7]))
# heigth_b_g.append(array("d",[ 0, 9.2, 8.6, 6.8, 4.2]))#mV gain 3
# widht_b_g=[]           #bias 0   25   50   75 100
# widht_b_g.append(array("d",[ 0,   0,   0,   0, 0]))#ns gain 0
# widht_b_g.append(array("d",[ 0, 1.6, 1.2,   0, 0]))
# widht_b_g.append(array("d",[ 0, 1.7, 1.7, 1.5, 1.1]))
# widht_b_g.append(array("d",[ 0, 1.9, 1.6, 1.5, 1.4]))#ns gain 3

# for i in height :
#    height_g.append(TGraph(int(len(bias)),bias,i))
# k=1
# for i in height_g :
#    c2.cd()
#    i.SetLineColor(k+1)
#    MGheight.Add(i)

l5=TLegend()
l6=TLegend()
# for i in height_g :
#    c2.cd()
#    i.SetLineColor(k+1)
#    MGheight.Add(i)
#    LGheight.AddEntry(i,str(color[k-1])+ " fiber","alp")
kk1=[]
kk2=[]
mm1=TMultiGraph()
mm2=TMultiGraph()
for i in gain1 :
   kk1.append(TGraph(int(len(bias1)),bias1, heigth_b_g[i]))
for i in gain1 :
   kk2.append(TGraph(int(len(bias1)),bias1, widht_b_g[i]))
k=0
for i in kk1 :
   k=k+1
   i.SetLineColor(k+1)
   l5.AddEntry(i, "gain "+str(k-1),"alp")
   mm1.Add(i)
k=0
for i in kk2 :
   k=k+1
   i.SetLineColor(k+1)
   l5.AddEntry(i, "gain "+str(k-1),"alp")
   mm2.Add(i)

c5.cd()
mm1.Draw("APL")
l5.Draw("")
c5.Modified()
c5.Update()
mm1.Write()
c5.Write()
c6.cd()
mm2.Draw("APL")
l6.Draw("")
mm2.Write()
c6.Modified()
c6.Update()
c6.Write()
