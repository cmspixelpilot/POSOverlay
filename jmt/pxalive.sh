RUN=$1
if [ "$RUN" == "" ]; then
    RUN=$(echo $(pwd) | sed 's/.*_//')
    if ! [[ $RUN =~ $re ]] ; then
        echo "error: need run number" >&2
        exit 1
    fi
fi

LOGFN=${POS_LOG_DIR}/pixelalive.run${RUN}.log.gz
echo $LOGFN
echo q | ${BUILD_HOME}/pixel/PixelAnalysisTools/test/bin/linux/x86_64_slc6/PixelAnalysis.exe PixelAlive $RUN 2>&1 | gzip > $LOGFN
