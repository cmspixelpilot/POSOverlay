banger=${BUILD_HOME}/pixel/bin/banger.exe

amc13_t2_ip=$1
pxfec_ip=$2

amc13_d50=$($banger chtcp-2.0://localhost:10203?target=${amc13_t2_ip}:50001 /opt/cactus/etc/amc13/AMC13XG_T2.xml read STATUS.TTC.CLK_FREQ | tail -1)
amc13=$((amc13_d50 * 50))
pxfec=$($banger chtcp-2.0://localhost:10203?target=${pxfec_ip}:50001 ${BUILD_HOME}/pixel/BPixelTools/utca/address_table.xml readaddr 0x40000021 | tail -1)

echo amc13T2 says: $amc13
echo pxfec says: $pxfec
echo pxfec - amc13T2: $((pxfec - amc13))
