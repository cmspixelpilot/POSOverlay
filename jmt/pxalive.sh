RUN=$1
LOGFN=~/Logs/pixelalive.run${RUN}.log.gz
echo q | \
${BUILD_HOME}/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc6/PixelAnalysis.exe \
PixelAlive $RUN | gzip > $LOGFN

echo $LOGFN

