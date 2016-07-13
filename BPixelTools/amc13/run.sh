xml=amc13.xml
reset=1
externaltrig=0

while test $# -gt 0
do
    case "$1" in
        --xml)
            shift
            xml=$1
            ;;
        --noreset) reset=0
            ;;
        --externaltrig) externaltrig=1
            ;;
        --*) echo "bad option $1"
            ;;
        *) echo "argument $1 not parsed"
            ;;
    esac
    shift
done

if [ "$reset" != "1" ]; then
    /opt/cactus/bin/amc13/AMC13Tool2.exe -c $xml
elif [ "$externaltrig" == "1" ]; then
    /opt/cactus/bin/amc13/AMC13Tool2.exe -c $xml -X reset_amc13_externaltrig
else
    /opt/cactus/bin/amc13/AMC13Tool2.exe -c $xml -X reset_amc13
fi
