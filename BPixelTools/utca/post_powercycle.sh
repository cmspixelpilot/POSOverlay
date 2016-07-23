todo="pxfec09 fed1294 fed1295 fed1296 fed1297 fed1298"
todo="fed1294 fed1295 fed1296 fed1297 fed1298"

for ip in $todo; do 
    $BUILD_HOME/pixel/PixelUtilities/PixeluTCAUtilities/test/src/common/banger.exe \
        board chtcp-2.0://localhost:10203?target=${ip}:50001 \
        file://${BUILD_HOME}/pixel/BPixelTools/utca_scripts/address_table.xml \
        write ctrl.ttc_xpoint_A_out3 0
done
