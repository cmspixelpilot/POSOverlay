
RNG=$1
shift
echo RNG will be $RNG
for x in $@; do
    base="${x%.*}"
    ext="${x##*.}"
    y=$(echo $base | sed s/Pilt/FPix/g)_RNG${RNG}.${ext}
    echo $x "    to    " $y
    mv $x $y

    xmod=$(echo $x | sed 's/.*Pilt\(.*\)\.dat/Pilt\1_PLQ1/')
    ymod=$(echo $y | sed 's/.*FPix\(.*\)\.dat/FPix\1/')
    echo $xmod "    to    " $ymod
    sed -i s/${xmod}/${ymod}/g $y
done
