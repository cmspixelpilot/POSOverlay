if [ "$1" == "noreset" ]; then
    /opt/cactus/bin/amc13/AMC13Tool2.exe -c amc13.xml
else
    /opt/cactus/bin/amc13/AMC13Tool2.exe -c amc13.xml -X reset_amc13
fi

