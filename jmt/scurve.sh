RUN=$1
if [ "$RUN" == "" ]; then
    RUN=$(echo $(pwd) | sed 's/.*_//')
    if ! [[ $RUN =~ $re ]] ; then
        echo "error: need run number" >&2
        exit 1
    fi
fi
LOGFN=${POS_LOG_DIR}/scurve.run${RUN}.log.gz
echo q | ${BUILD_HOME}/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc6/PixelAnalysis.exe SCurve $RUN 2>&1 | gzip > $LOGFN
echo $LOGFN

