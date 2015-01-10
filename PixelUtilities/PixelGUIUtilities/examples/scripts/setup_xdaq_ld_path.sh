#!/bin/bash

#Make sure XDAQ_ROOT, XDAQ_OS, and XDAQ_PLATFORM are defined before calling this script!

# Setup the library path
export LD_LIBRARY_PATH
export LD_LIBRARY_PATH=./:$XDAQ_ROOT/daq/xdaq/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$XDAQ_ROOT/daq/extern/xerces/linuxx86/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xtuple/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/toolbox/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xoap/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/pt/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xdaq/executive/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xdaq/hyperdaq/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/pt/fifo/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/pt/http/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/log4cplus/linuxx86/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/cgicc/linuxx86/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/mimetic/linuxx86/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/asyncresolv/linuxx86/lib:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xdata/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xcept/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xgi/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/i2o/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xrelay/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/xmem/lib/$XDAQ_OS/$XDAQ_PLATFORM/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/log4cplus/udpappender/lib/$XDAQ_OS/$XDAQ_PLATFORM:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$XDAQ_ROOT/daq/extern/log4cplus/xmlappender/lib/$XDAQ_OS/$XDAQ_PLATFORM:$LD_LIBRARY_PATH
                                                                                                                             

