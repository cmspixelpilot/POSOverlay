
for ip in pxfec{09,10}; do 
    $BUILD_HOME/pixel/bin/banger.exe \
        chtcp-2.0://localhost:10203?target=${ip}:50001 \
        ${BUILD_HOME}/pixel/BPixelTools/utca/address_table.xml \
        write ctrl.ttc_xpoint_A_out3 0

    $BUILD_HOME/pixel/bin/banger.exe \
        chtcp-2.0://localhost:10203?target=${ip}:50001 \
        ${BUILD_HOME}/pixel/PixelFECInterface/dat/address_table.xml \
        write Board0.TTCrst 1

    sleep 1

    $BUILD_HOME/pixel/bin/banger.exe \
        chtcp-2.0://localhost:10203?target=${ip}:50001 \
        ${BUILD_HOME}/pixel/PixelFECInterface/dat/address_table.xml \
        write Board0.TTCrst 0
done

