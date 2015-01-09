{
//gDebug=7;
gROOT->ProcessLine(".L PixelConfigBase.cc+");
gROOT->ProcessLine(".L PixelCalibBase.cc+");
gROOT->ProcessLine(".L PixelROCName.cc+");
gROOT->ProcessLine(".L PixelModuleName.cc+");
gROOT->ProcessLine(".L PixelDACScanRange.cc+");
gROOT->ProcessLine(".L PixelCalib.cc+");
gROOT->ProcessLine(".L PixelSLinkData.cc+");
//gROOT->ProcessLine(".L pixelROC.cc+");
gROOT->ProcessLine(".L pixel_alive.c+");
//gROOT->ProcessLine(".L pixel_gaincalib.c+");
//gROOT->ProcessLine(".L my_scurve.C+");
gROOT->ProcessLine(".L pixelROCscurve.cc+");
gROOT->ProcessLine(".L pixel_scurve.c+");
gROOT->ProcessLine(".L delay25.c+");
gStyle->SetPalette(1,0);
}
