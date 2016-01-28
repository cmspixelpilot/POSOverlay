#
#source ~/setup_xdaq.csh
source ../setup.csh

if ( ! $?BPIXELTOOLS ) then
    set p=`pwd`
    setenv BPIXELTOOLS $p:h
    if ( $?PYTHONPATH ) then
	setenv PYTHONPATH ${PYTHONPATH}:${BPIXELTOOLS}/tools/python
    else
	setenv PYTHONPATH ${BPIXELTOOLS}/tools/python
    endif
endif



if ( ! $?LD_LIBRARY_PATH ) then
    setenv LD_LIBRARY_PATH ""
endif

#echo $LD_LIBRARY_PATH


setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${XDAQ_ROOT}/lib

setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${ENV_CMS_TK_FEC_ROOT}/lib

setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${BPIXELTOOLS}/tools/lib:/usr/lib

# echo $LD_LIBRARY_PATH
