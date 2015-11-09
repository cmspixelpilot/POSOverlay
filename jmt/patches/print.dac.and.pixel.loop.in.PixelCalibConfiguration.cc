diff --git a/CalibFormats/SiPixelObjects/src/PixelCalibConfiguration.cc b/CalibFormats/SiPixelObjects/src/PixelCalibConfiguration.cc
index b31babb..a0bdbdd 100644
--- a/CalibFormats/SiPixelObjects/src/PixelCalibConfiguration.cc
+++ b/CalibFormats/SiPixelObjects/src/PixelCalibConfiguration.cc
@@ -1163,10 +1163,9 @@ void PixelCalibConfiguration::nextFECState(std::map<unsigned int, PixelFECConfig
 
       int dacvalue = scanValue(ii, state, rocs_[i]);
 
-      //cout << "dacname ii:"<<dacs_[ii].name()<<" "<<ii<<endl;
+      cout << "roc " << rocs_[i] << " dacname ii:"<<dacs_[ii].name()<<" "<<ii<<" value " << dacvalue << endl;
       
       if (dacs_[ii].relative()){
-
 	//We have to find the default DAC setting so that we can
 	//add the offset as we are in relative mode.
 
@@ -1175,6 +1174,7 @@ void PixelCalibConfiguration::nextFECState(std::map<unsigned int, PixelFECConfig
 	dacvalue+=rocInfo_[i].defaultDACs_[ii].second;
 	//cout << "[PixelCalibConfiguration::nextFECState] ROC="<<rocs_[i]
 	//     << " dac="<<dacs_[ii].name()<<" new value="<<dacvalue<<endl;
+	cout << " is relative " << dacvalue <<endl;
       }
 
       if (dacs_[ii].isTBM()) {
@@ -1271,7 +1271,7 @@ void PixelCalibConfiguration::nextFECState(std::map<unsigned int, PixelFECConfig
         if (icol>=ncol) icol=ncol-1;
         unsigned int row=rows_[i_row][irow];
         unsigned int col=cols_[i_col][icol];
-
+	cout << "col " << col << " row " << row << endl;
         pixelFECs[theROC.fecnumber()]->calpix(theROC.mfec(),
 					      theROC.mfecchannel(),
 					      theROC.hubaddress(),
