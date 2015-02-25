RUN=$1
echo q | /nfshome0/pixelpilot/build/TriDAS/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc6/PixelAnalysis.exe SCurve $RUN | gzip > ~/Logs/scurve.run${RUN}.log.gz
