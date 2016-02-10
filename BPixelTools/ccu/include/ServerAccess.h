/*
This file is part of Fec Software project.

Fec Software is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Fec Software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fec Software; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

Copyright 2002 - 2003, Frederic DROUHIN - Universite de Haute-Alsace, Mulhouse-France
*/
#ifndef PIXELACCESS_H
#define PIXELACCESS_H

#ifndef PIXEL
#define PIXEL
#endif // PIXEL

#include "PixelDeviceAddresses.h"

#include "keyType.h"

#include "FecExceptionHandler.h"

#include "i2cAccess.h"
#include "piaAccess.h"

#include "FecAccess.h"

#include "pllDescription.h"
#include "pllAccess.h"

#include "delay25Description.h"
#include "delay25Access.h"

#include "laserdriverDescription.h"
#include "laserdriverAccess.h"
#include "DohAccess.h"

#include "FecRingRegisters.h"

const unsigned int init_delay25_delay0 = 28; //RCK
const unsigned int init_delay25_delay1 = 10; //CTR
const unsigned int init_delay25_delay2 = 4; //SDA
const unsigned int init_delay25_delay3 = 0; //RDA
const unsigned int init_delay25_delay4 = 0; //clk

const unsigned int init_pll_clk = 0;
const unsigned int init_pll_tr  = 0;

const unsigned int init_aoh1a_gain0 = 0;
const unsigned int init_aoh1a_gain1 = 1;
const unsigned int init_aoh1a_gain2 = 1;
const unsigned int init_aoh1a_bias0 = 29;
const unsigned int init_aoh1a_bias1 = 30;
const unsigned int init_aoh1a_bias2 = 29;

const unsigned int init_aoh1b_gain0 = 1;
const unsigned int init_aoh1b_gain1 = 1;
const unsigned int init_aoh1b_gain2 = 0;
const unsigned int init_aoh1b_bias0 = 27;
const unsigned int init_aoh1b_bias1 = 33;
const unsigned int init_aoh1b_bias2 = 33;

const unsigned int init_aoh2a_gain0 = 0;
const unsigned int init_aoh2a_gain1 = 1;
const unsigned int init_aoh2a_gain2 = 1;
const unsigned int init_aoh2a_bias0 = 29;
const unsigned int init_aoh2a_bias1 = 30;
const unsigned int init_aoh2a_bias2 = 29;

const unsigned int init_aoh2b_gain0 = 1;
const unsigned int init_aoh2b_gain1 = 1;
const unsigned int init_aoh2b_gain2 = 0;
const unsigned int init_aoh2b_bias0 = 27;
const unsigned int init_aoh2b_bias1 = 33;
const unsigned int init_aoh2b_bias2 = 33;

const unsigned int init_aoh3a_gain0 = 0;
const unsigned int init_aoh3a_gain1 = 1;
const unsigned int init_aoh3a_gain2 = 1;
const unsigned int init_aoh3a_bias0 = 29;
const unsigned int init_aoh3a_bias1 = 30;
const unsigned int init_aoh3a_bias2 = 29;

const unsigned int init_aoh3b_gain0 = 1;
const unsigned int init_aoh3b_gain1 = 1;
const unsigned int init_aoh3b_gain2 = 0;
const unsigned int init_aoh3b_bias0 = 27;
const unsigned int init_aoh3b_bias1 = 33;
const unsigned int init_aoh3b_bias2 = 33;

const unsigned int init_aoh4a_gain0 = 0;
const unsigned int init_aoh4a_gain1 = 0;
const unsigned int init_aoh4a_gain2 = 0;
const unsigned int init_aoh4a_bias0 = 31;
const unsigned int init_aoh4a_bias1 = 29;
const unsigned int init_aoh4a_bias2 = 27;

const unsigned int init_aoh4b_gain0 = 0;
const unsigned int init_aoh4b_gain1 = 0;
const unsigned int init_aoh4b_gain2 = 0;
const unsigned int init_aoh4b_bias0 = 25;
const unsigned int init_aoh4b_bias1 = 26;
const unsigned int init_aoh4b_bias2 = 26;

const unsigned int init_poh1_gain0 = 0;
const unsigned int init_poh1_gain1 = 1;
const unsigned int init_poh1_gain2 = 1;
const unsigned int init_poh1_gain3 = 1;
const unsigned int init_poh1_bias0 = 29;
const unsigned int init_poh1_bias1 = 30;
const unsigned int init_poh1_bias2 = 29;
const unsigned int init_poh1_bias3 = 29;

const unsigned int init_doh_gain0 = 1;
const unsigned int init_doh_gain1 = 0;
const unsigned int init_doh_gain2 = 0;
const unsigned int init_doh_bias0 = 30;
const unsigned int init_doh_bias1 = 25;
const unsigned int init_doh_bias2 = 25;

/** to set or the getchar, default false getchar are displayed
 */
void setNoGetchar(bool) ;
bool getNoGetchar() ;

/** To display error
 */
void setErrorCounterFlag ( bool val ) ;
bool getErrorCounterFlag ( ) ;

/** For output
 */
void setStdchan ( FILE *chan ) ;
FILE *getStdchan ( ) ; 

/** For different FEC bus
*/
void setFecType ( enumFecBusType fecBusType ) ;

/** Create the FEC Access class depending on the version of the FecSoftware and the FEC type
 */
FecAccess *createFecAccess ( int argc, char **argv, int *cnt , int fecslot= 0xFFFFFFFF);


/** \brief read PLL
 */
std::string readPll ( FecAccess *fec,
		      tscType8 fecAddress, 
		      tscType8 ringAddress,
		      tscType8 ccuAddress, 
		      tscType8 channelAddress,
		      tscType8 deviceAddress,
		      long loop, unsigned long tms ) ;

/** \brief reset PLL
 */
std::string resetPll ( FecAccess *fecAccess,
		       tscType8 fecAddress, 
		       tscType8 ringAddress,
		       tscType8 ccuAddress, 
		       tscType8 channelAddress,
		       tscType8 deviceAddress,
		       long loop, unsigned long tms ) ;

/** \brief set PLL
 */
std::string setPll_ClockPhase ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief set PLL
 */
std::string setPll_TriggerDelay ( FecAccess *fec,
				  tscType8 fecAddress, 
				  tscType8 ringAddress,
				  tscType8 ccuAddress, 
				  tscType8 channelAddress,
				  tscType8 deviceAddress,
				  long loop, unsigned long tms,
				  unsigned int value ) ;

/** \brief read DELAY25
 */
std::string readDelay25 ( FecAccess *fec,
			  tscType8 fecAddress, 
			  tscType8 ringAddress,
			  tscType8 ccuAddress, 
			  tscType8 channelAddress,
			  tscType8 deviceAddress,
			  long loop, unsigned long tms ) ;

/** \brief set delay0 of a DELAY25
 */
std::string setDelay25_Delay0 ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief set delay1 of a DELAY25
 */
std::string setDelay25_Delay1 ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief set delay2 of a DELAY25
 */
std::string setDelay25_Delay2 ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief set delay3 of a DELAY25
 */
std::string setDelay25_Delay3 ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief set delay4 of a DELAY25
 */
std::string setDelay25_Delay4 ( FecAccess *fec,
				tscType8 fecAddress, 
				tscType8 ringAddress,
				tscType8 ccuAddress, 
				tscType8 channelAddress,
				tscType8 deviceAddress,
				long loop, unsigned long tms,
				unsigned int value ) ;

/** \brief resynch dll 
 */
std::string ResynchDLL ( FecAccess *fecAccess,
			 tscType8 fecAddress, 
			 tscType8 ringAddress,
			 tscType8 ccuAddress, 
			 tscType8 channelAddress,
			 tscType8 deviceAddress,
			 long loop, unsigned long tms ) ;
 

/** \brief Basic test for accessing a LaserDriver
 */
std::string readLaserdriver ( FecAccess *fec,
			      tscType8 fecAddress, 
			      tscType8 ringAddress,
			      tscType8 ccuAddress, 
			      tscType8 channelAddress,
			      tscType8 deviceAddress,
			      long loop, unsigned long tms) ;

std::string readPoh ( FecAccess *fec,
		      tscType8 fecAddress, 
		      tscType8 ringAddress,
		      tscType8 ccuAddress, 
		      tscType8 channelAddress,
		      tscType8 deviceAddress,
	              long loop, unsigned long tms) ;

/** \brief Basic test for accessing a LaserDriver
 */
std::string setLaserdriver ( FecAccess *fecAccess,
			     tscType8 fecAddress, 
			     tscType8 ringAddress,
			     tscType8 ccuAddress, 
			     tscType8 channelAddress,
			     tscType8 deviceAddress,
			     long loop, unsigned long tms,
			     bool setgain0,
			     unsigned int valuegain0,
			     bool setgain1,
			     unsigned int valuegain1,
			     bool setgain2,
			     unsigned int valuegain2,
			     bool setbias0,
			     unsigned int valuebias0,
			     bool setbias1,
			     unsigned int valuebias1,
			     bool setbias2,
			     unsigned int valuebias2);

std::string setPoh ( FecAccess *fecAccess,
		     tscType8 fecAddress, 
		     tscType8 ringAddress,
		     tscType8 ccuAddress, 
		     tscType8 channelAddress,
		     tscType8 deviceAddress,
		     long loop, unsigned long tms,
		     bool setgain0,
		     unsigned int valuegain0,
		     bool setgain1,
		     unsigned int valuegain1,
	             bool setgain2,
	             unsigned int valuegain2,
	             bool setgain3,
	             unsigned int valuegain3,
		     bool setbias0,
		     unsigned int valuebias0,
		     bool setbias1,
		     unsigned int valuebias1,
		     bool setbias2,
		     unsigned int valuebias2,
		     bool setbias3,
		     unsigned int valuebias3);
		
/** \brief Basic test for accessing a DOH
 */
std::string readDoh ( FecAccess *fecAccess,
		      tscType8 fecAddress, 
		      tscType8 ringAddress,
		      tscType8 ccuAddress, 
		      tscType8 channelAddress,
		      tscType8 deviceAddress,
		      long loop, unsigned long tms) ;

/** \brief Basic test for accessing a DOH
 */
std::string setDoh  ( FecAccess *fecAccess,
		      tscType8 fecAddress, 
		      tscType8 ringAddress,
		      tscType8 ccuAddress, 
		      tscType8 channelAddress,
		      tscType8 deviceAddress,
		      long loop, unsigned long tms,
		      bool setgain0,
		      unsigned int valuegain0,
		      bool setgain1,
		      unsigned int valuegain1,
		      bool setgain2,
		      unsigned int valuegain2,
		      bool setbias0,
		      unsigned int valuebias0,
		      bool setbias1,
		      unsigned int valuebias1,
		      bool setbias2,
		      unsigned int valuebias2);


/** \brief display all the counters
 */
void displayStatus ( FecExceptionHandler *e, unsigned int count, FecAccess *fecAccess, FILE *stdchan ) ;

/** \brief display all FEC/CCU registers in case of error
 */
//void displayStatus ( FecAccess *fecAccess, tscType8 fecAddress, tscType8 ringAddress, tscType8 ccuAddress, tscType8 channelAddress, FecExceptionHandler *e) ;

/** \brief Reset all PLX/FEC
 */
std::string resetPlxFec ( FecAccess *fecAccess, 
			  tscType8 fecAddress,
			  tscType8 ringAddress,
			  long loop, unsigned long tms ) ;

/** \brief Crate reset for VME FEC
 */
std::string crateReset ( FecAccess *fecAccess, bool testCrateReset, long loop, unsigned long tms ) ;

/** \brief test of DC-DC converters
 */
std::string DCDCenableTest ( FecAccess *fecAccess, 
			     tscType8 fecAddress,
			     tscType8 ringAddress, 
			     tscType8 ccuAddress,
			     unsigned int dcdcAddress,
			     bool noBroadcast );

/** \brief scan for ccu
 */
std::string testScanCCU ( FecAccess *fecAccess, 
			  tscType8 fecAddress,
			  tscType8 ringAddress,
			  bool noBroadcast );

/** \brief scan for ccu
 */
std::string newtestScanCCU ( FecAccess *fecAccess,
			     bool noBroadcast );

/** \brief scan for the devices
 */
std::string testScanRingEDevice ( FecAccess *fecAccess,

				  tscType8 fecAddress, tscType8 ringAddress );
/** \brief scan for the devices
 */
std::string testScanPixelDevice ( FecAccess *fecAccess, 
				  tscType8 fecAddress,
				  tscType8 ringAddress,
				  long loop, unsigned long tms); 


void allCCUsPiaReset ( FecAccess *fecAccess, tscType8 fecAddress, tscType8 ringAddress );

/** \brief status
 */
std::string testStatus ( FecAccess *fecAccess,
			 bool noBroadcast );

/** \brief map (fec)
 */
std::string testMap ( FecAccess *fecAccess,
		      bool noBroadcast );

/** \brief map (fec and ccu)
 */
std::string testMapCCU ( FecAccess *fecAccess,
		    bool noBroadcast );

/** \brief redundancy
 */
std::string testRedundancyRing ( FecAccess *fecAccess, 
				 tscType8 fecAddress,
				 tscType8 ringAddress,
				 uint ccuAddresses[][3], 
				 uint numberOfCCU );
     
/** \brief Access by write/read a register on i2c channel
 */
std::string setI2CDevice (FecAccess *fecAccess,
			  tscType8 fecAddress,
			  tscType8 ringAddress,
			  tscType8 ccuAddress,
			  tscType8 channelAddress,
			  tscType8 deviceAddress,
			  enumDeviceType modeType,
			  long loop, unsigned long tms,
			  unsigned int value ) ;



std::string getI2CDevice (FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     tscType8 ccuAddress,
		     tscType8 channelAddress,
		     tscType8 deviceAddress,
		     enumDeviceType modeType,
		     long loop, unsigned long tms) ;

/* new method to set i2c speed */
void setI2CSpeed ( FecAccess *fecAccess, 
                   tscType8 fecAddress, 
                   tscType8 ringAddress, 
                   tscType8 ccuAddress, 
                   tscType8 channelAddress, 
                   unsigned int i2cSpeed );

/** \brief Default test, just read the FEC CR0
 */
void readFecRegisterSR0 (FecAccess *fecAccess, 
			 tscType8 fecAddress, 
			 tscType8 ringAddress,
                         long loop, unsigned long tms ) ;


/** \brief display a FEC status register
 */
std::string displayFECSR0 ( tscType16 fecSR0 ) ;

/** \brief display a FEC status register
 */
std::string displayFECSR1 ( tscType16 fecSR1 ) ;

/** \brief display a FEC control register
 */
std::string displayFECCR0 ( tscType16 fecCR0 ) ;

/** \brief display a FEC control register
 */
std::string displayFECCR1 ( tscType16 fecCR1 ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRA ( tscType8 SRA ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRB ( tscType8 SRB ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRC ( tscType8 SRC ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRD ( tscType8 SRD ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRE ( tscType32 SRE ) ;

/** \brief display a CCU status register
 */
std::string displayCCUSRF ( tscType16 SRF ) ;

/** \brief display a CCU control register
 */
std::string displayCCUCRA ( tscType8 CRA ) ;

/** \brief display a CCU control register
 */
std::string displayCCUCRB ( tscType8 CRB ) ;

/** \brief display a CCU control register
 */
std::string displayCCUCRC ( tscType8 CRC ) ;

/** \brief display a CCU control register
 */
std::string displayCCUCRD ( tscType8 CRD ) ;

/** \brief display a CCU control register
 */
std::string displayCCUCRE ( tscType32 CRE ) ;

/** \brief pia reset
 */
std::string testPIAResetfunctions (FecAccess *fecAccess,
				   tscType8 fecAddress,
				   tscType8 ringAddress,
				   tscType8 ccuAddress,
				   tscType8 channelAddress,
				   tscType16 value,
				   unsigned long delayActiveReset,
				   unsigned long intervalDelayReset,
				   long loop, unsigned long tms );

/* \brief autoredundancy
 */
std::vector<std::string> autoRedundancyRing ( FecAccess *fecAccess,
					      tscType8 fecAddress,
					      tscType8 ringAddress,
					      long loop, unsigned long tms ) ;

/** Clear the error in the FECs and in the different CCUs
 */
void clearFecCcuErrorRegisters ( FecAccess *fecAccess,
				 tscType8 fecAddress,
				 tscType8 ringAddress,
				 std::list<keyType> ccuAddresses ) ;

/** This method try to recover the ring until the status register 0 is ok
 */
void emptyFIFOs ( FecAccess *fecAccess, keyType index, bool display = false) ;

/** \brief display fec status
 */
void displayFecStatus ( FecAccess *fecAccess,
                        keyType index ) ;

std::string PIOrw (FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     tscType8 ccuAddress,
		    tscType8 channelAddress=0x30, 
		    int value=0xFF);

std::string CtrlRegE (FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     tscType8 ccuAddress,
		    tscType8 channelAddress=0x30, 
		    int value=0xFF);

#endif

