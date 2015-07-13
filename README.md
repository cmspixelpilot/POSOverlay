#
# CMS POS
#
## TIF working branch
#


Setup TIF POS dir with the following:

git clone https://github.com/odysei/POSOverlay pixel
cd pixel
git checkout tif_pos_development

# TIF specific (for now):
ln -s ~/POS_config config

ln -s POSRelease/RPM.version
ln -s POSRelease/VERSION
ln -s ../DiagSystem/gwtapplets/LogReaderWithFileServer/
ln -s POSRelease/Makefile

cd PixelSupervisor
mv Makefile Makefile_default
ln -s Makefile_TIF Makefile
cd ..

cd PixelTKFECSupervisor
mv Makefile Makefile_default
ln -s Makefile_TIF Makefile
cd ..

cd PixelFECSupervisor
mv Makefile Makefile_default
ln -s Makefile_TIF Makefile
cd ..

make install
make Set=pixel