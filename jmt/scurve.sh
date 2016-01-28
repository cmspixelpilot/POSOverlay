RUN=$1
LOGFN=~/Logs/scurve.run${RUN}.log.gz
echo q | ${BUILD_HOME}/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc6/PixelAnalysis.exe SCurve $RUN 2>&1 | gzip > $LOGFN
echo $LOGFN

