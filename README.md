#
# CMS POS
#
## TIF working branch
#


Setup TIF POS dir with the following:

git clone https://github.com/odysei/POSOverlay pixel<br />
cd pixel<br />
git checkout tif_pos_development

# TIF specific (for now):
ln -s ~/POS_config config<br />

ln -s POSRelease/RPM.version<br />
ln -s POSRelease/VERSION<br />
ln -s ../DiagSystem/gwtapplets/LogReaderWithFileServer/<br />
ln -s POSRelease/Makefile<br />

cd PixelSupervisor<br />
mv Makefile Makefile_default<br />
ln -s Makefile_TIF Makefile<br />
cd ..<br />

cd PixelTKFECSupervisor<br />
mv Makefile Makefile_default<br />
ln -s Makefile_TIF Makefile<br />
cd ..<br />

cd PixelFECSupervisor<br />
mv Makefile Makefile_default<br />
ln -s Makefile_TIF Makefile<br />
cd ..<br />

make install<br />
make Set=pixel<br />