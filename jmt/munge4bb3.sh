cp -n calib.dat calib.dat.real
sed -i.bak -e 's/VcThr/Vcal/' -e '/Set: Vcal 200/d' calib.dat
