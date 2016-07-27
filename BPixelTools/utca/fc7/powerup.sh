./fc7firmwarer.exe -i trkfec -s fc7_tkfec_rarp_en.bit
./fc7firmwarer.exe -i pxfec09 -s 101_2.bin
for x in fed{1294..1300}; do
    ./fc7firmwarer.exe -i $x -s fc7_top_v6.10.5.bin
done
