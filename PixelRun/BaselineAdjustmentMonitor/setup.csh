#!/bin/tcsh
set PROJECT_DIR=`pwd`
setenv ORACLE_HOME /opt/oracle/app/oracle/product/10.2.0/db_1/
setenv ROOTSYS     /usr/local/root_v5.17.06/
setenv TNS_ADMIN   ${PROJECT_DIR}

setenv PATH        ${ROOTSYS}/bin:${PATH}

setenv LD_LIBRARY_PATH ${ROOTSYS}/lib:${ORACLE_HOME}/lib:${PROJECT_DIR}/lib
