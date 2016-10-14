if [ "$1" == "tkfec" ]; then
    ./fc7firmwarer.exe -i trkfec -s fc7_tkfec_rarp_en.bit
fi
for x in pxfec{09,10}; do
    ./fc7firmwarer.exe -i $x -s fc7_top1_0_9.bin
done
for x in fed{1294..1300}; do
    ./fc7firmwarer.exe -i $x -s 6.12.bin
done
