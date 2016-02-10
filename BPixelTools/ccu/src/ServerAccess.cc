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

#include <iostream>

#include <stdio.h>    // fopen snprintf
#include <string.h>   // strcmp
#include <unistd.h>   // usleep
#include <sys/time.h> // time

// For socket
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// #include "FecVmeRingDevice.h"
// #include "FecPciRingDevice.h"
// #include "FecUsbRingDevice.h"

#if defined(BUSVMECAENPCI) || defined(BUSVMECAENUSB) || defined (BUSVMESBS)
#include "FecVmeRingDevice.h"
#endif
#if defined(BUSPCIFEC)
#include "FecPciRingDevice.h"
#endif
#if defined(BUSUSBFEC)
#include "FecUsbRingDevice.h"
#endif

#include "HashTable.h"
#include "ServerAccess.h"
#include "FecRingRegisters.h"

#define MAXERRORCOUNT 1
#define MAXCAR (DD_USER_MAX_MSG_LENGTH * 2 + 2)

// Default firmware version
#define MFECFIRMWARE 0x17
#define VMEFIRMWARE 0x16
#define TRIGGERFIRMWARE 0x14

using namespace std;

inline std::string IntToString(int t, int base = 16)
{
   std::ostringstream oss;
   if(base == 10) oss << t;
   else oss << std::hex << t;
   return oss.str();
}

// To display getchar or not
static bool noGetchar_ = false ;
void setNoGetchar( bool val ) {
  noGetchar_ = val ;
}

bool getNoGetchar ( ) {
  return noGetchar_ ;
}

char fileSuperName_[100] ;

void setSuperFileName ( char *fileName ) {

  strncpy (fileSuperName_,fileName,100) ;
}

// Global variable for displaying error
static bool errorCounterFlag_ = false ;
void setErrorCounterFlag ( bool val ) {

  errorCounterFlag_ = true ;
}

bool getErrorCounterFlag ( ) {

  return (errorCounterFlag_) ;
}

// Output file
static FILE *stdchan_ = stderr ;

void setStdchan ( FILE *chan ) {
  stdchan_ = chan ;
  }

FILE *getStdchan ( ) {

  return (stdchan_) ;
  }

// range for FEC and ring default = PCI
static unsigned int minFecSlot_ = 0 ;
static unsigned int maxFecSlot_ = 4 ;
static unsigned int minFecRing_ = 0 ;
static unsigned int maxFecRing_ = 0 ;

static enumFecBusType fecBusType_ ;

void setFecType ( enumFecBusType fecBusType ) {

  fecBusType_ = fecBusType ;
  switch (fecBusType) {
  case FECVME:
#if defined(BUSVMECAENPCI) || defined(BUSVMECAENUSB) || defined (BUSVMESBS)
    minFecSlot_ = FecVmeRingDevice::MINVMEFECSLOT ;
    maxFecSlot_ = FecVmeRingDevice::MAXVMEFECSLOT ;
    minFecRing_ = FecVmeRingDevice::getMinVmeFecRingValue() ;
    maxFecRing_ = FecVmeRingDevice::getMaxVmeFecRingValue() ;
#endif
    break ;
  case FECPCI:
#if defined(BUSPCIFEC)
    minFecSlot_ = FecPciRingDevice::MINPCIFECSLOT ;
    maxFecSlot_ = FecPciRingDevice::MAXPCIFECSLOT ;
    minFecRing_ = FecPciRingDevice::MINPCIFECRING ;
    maxFecRing_ = FecPciRingDevice::MAXPCIFECRING ;
#endif
    break ;
  case FECUSB:
#if defined(BUSUSBFEC)
    minFecSlot_ = FecUsbRingDevice::minUsbFecSlot ;
    maxFecSlot_ = FecUsbRingDevice::maxUsbFecSlot ;
    minFecRing_ = FecUsbRingDevice::minUsbFecRing ;
    maxFecRing_ = FecUsbRingDevice::maxUsbFecRing ;
#endif
    break;
  }
}



/** Create the FEC Access class depending on the version of the FecSoftware and the FEC type
 */
FecAccess *createFecAccess ( int argc, char **argv, int *cnt, int fecslot ) {

  //Lea
  //FecAccess *fecAccess = FecAccess::createFecAccess ( argc, argv, cnt, true, 100, false, fecslot) ;
  FecAccess *fecAccess = FecAccess::createFecAccess ( argc, argv, cnt, false ) ;

  if (!fecAccess) {
    std::cerr << "Creation of FecAccess failed! (fecAccess pointer null)" << std::endl ; 
    exit (EXIT_FAILURE) ; ; 
  }

  setFecType (fecAccess->getFecBusType()) ;
  
  std::list<keyType> *fecList = fecAccess->getFecList() ;
  
  if ((fecList == NULL) || (fecList->empty())) {
    std::cerr << "No FEC connected! (fec list null)" << std::endl ;
  }
  delete fecList;

  return (fecAccess) ;
}

/* ----------------------------------------------------------------------- */
/*                                                                         */
/*                       i2c channels methods                              */
/*                                                                         */
/* ----------------------------------------------------------------------- */

/** 
 * <p>Command: -i2c
 * \param fecAccess - FEC Access object
 * \param fecAddress - slot of the FEC.
 * \param ringAddress - slot of the ring.
 * \param ccuAddress - address of the first CCU of the ring.
 * \param channelAddress - address of the channel.
 * \param deviceAddress - address of the device.
 * \param modeType - type of device to access.
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setI2CDevice (FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     tscType8 ccuAddress,
		     tscType8 channelAddress,
		     tscType8 deviceAddress,
		     enumDeviceType modeType,
		     long loop, unsigned long tms,
		     unsigned int value ) {

  std::ostringstream o;
  std::ostringstream er;
  keyType index ;
  index = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,deviceAddress) ; 


  try {
    //create i2c access
    fecAccess->addi2cAccess (index, 
                             modeType,
                             MODE_SHARE) ;
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;

  } 

 
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {

      //read old value
      o << endl 
	<< "Set I2C Device (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << ", i2c address 0x" << std::hex << (int)deviceAddress << "):" << std::endl 
	<< "-->CR: Changed Value from 0x" << std::hex << (int)fecAccess->read(index) << " to 0x";
    
      //set new value
      fecAccess->write(index, value) ;
    
      //read new value
      o << std::hex << (int)fecAccess->read(index) << endl;
    }
    catch (FecExceptionHandler e) {
     
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove i2c access
    fecAccess->removei2cAccess (index) ;
  }
  catch (FecExceptionHandler e) {
  
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;

    return er.str();
    }
  
  return o.str() ;
}

/* new method to set and test i2cspeed */
void setI2CSpeed ( FecAccess *fecAccess, 
                   tscType8 fecAddress, 
                   tscType8 ringAddress, 
                   tscType8 ccuAddress, 
                   tscType8 channelAddress, 
                   unsigned int i2cSpeed ) { 

            keyType index = buildCompleteKey(fecAddress, ringAddress, ccuAddress,channelAddress, 0);
            tscType32 channelCRA = fecAccess->geti2cChannelCRA (index) ;
            int speed = 0;

            if( channelCRA == 0 ) speed = 100;
            else if( channelCRA == 1 ) speed = 200;
            else if( channelCRA == 2 ) speed = 400;
            else if( channelCRA == 3 ) speed = 1000;
            std::cout << "current i2c speed : " << std::dec << speed << " kHz "<< std::endl;

            channelCRA &= 0xFC;
            if( i2cSpeed == 100 ) channelCRA |= 0x0; 
            else if( i2cSpeed == 200 ) channelCRA |= 0x1;
            else if( i2cSpeed == 400 ) channelCRA |= 0x2;
            else if( i2cSpeed == 1000 ) channelCRA |= 0x3; 
            fecAccess->seti2cChannelCRA(index,channelCRA);

            tscType32 newCRA = fecAccess->geti2cChannelCRA (index) ;

            if( newCRA == channelCRA ) std::cout << "Written new speed " << std::dec << i2cSpeed << " kHz " << std::endl;

}

/** 
 * <p>Command: -i2c read
 * \param fecAccess - FEC Access object
 * \param fecAddress - slot of the FEC.
 * \param ringAddress - slot of the ring.
 * \param ccuAddress - address of the first CCU of the ring.
 * \param channelAddress - address of the channel.
 * \param deviceAddress - address of the device.
 * \param modeType - type of device to access.
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string getI2CDevice (FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     tscType8 ccuAddress,
		     tscType8 channelAddress,
		     tscType8 deviceAddress,
		     enumDeviceType modeType,
		     long loop, unsigned long tms) {

  std::ostringstream o;
  std::ostringstream er;
  keyType index ;
  index = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,deviceAddress) ; 


  try {
    //create i2c access
    fecAccess->addi2cAccess (index, 
                             modeType,
                             MODE_SHARE) ;
  }
  catch (FecExceptionHandler e) {
 
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;

  } 

 
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {

      //read old value
      o << endl 
	<< "Read I2C Device (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << ", i2c address 0x" << std::hex << (int)deviceAddress << "):" << std::endl 
	<< "---->  Value: 0x" << std::hex << (int)fecAccess->read(index) << endl;
    }
    catch (FecExceptionHandler e) {
     
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove i2c access
    fecAccess->removei2cAccess (index) ;
  }
  catch (FecExceptionHandler e) {
  
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;

    return er.str();
    }
  
  return o.str() ;
}


/** 
 * <p>command: -device pll
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string readPll ( FecAccess *fecAccess,
		 tscType8 fecAddress, 
		 tscType8 ringAddress,
		 tscType8 ccuAddress, 
		 tscType8 channelAddress,
		 tscType8 deviceAddress,
		 long loop, unsigned long tms ) {

 
  pllAccess *pll ;

  std::ostringstream o;
  std::ostringstream er;
 

  try {

    //create pll access
    pll = new pllAccess ( fecAccess,
                          fecAddress,
                          ringAddress,
                          ccuAddress,
                          channelAddress,
                          deviceAddress) ; // Open a pll access
  }
  catch (FecExceptionHandler e) { 
    

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }  
 
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) { 

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {   
      
      // read pll
      o << endl 
	<< "Read PLL (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->CNTR1  : 0x" << std::hex << (int)pll->getCNTRL1() << std::endl  
	<< "-->CNTR2  : 0x" << std::hex << (int)pll->getCNTRL2() << std::endl 
	<< "-->CNTR3  : 0x" << std::hex << (int)pll->getCNTRL3() << std::endl 
	<< "-->CNTR4  : 0x" << std::hex << (int)pll->getCNTRL4() << std::endl 
	<< "-->CNTR5  : 0x" << std::hex << (int)pll->getCNTRL5() << std::endl << std::endl 
	<< "-->Clock Phase   : " << std::dec << (int)pll->getClockPhase() << std::endl
	<< "-->Trigger Delay : " << std::dec << (int)pll->getTriggerDelay() << std::endl;
	if ( pll->getSeuStatus() ) o <<  "-->SEU detected  : yes" << std::endl ;
	else o << "-->SEU detected  : no" << std::endl ; 
	
    }
    
    catch (FecExceptionHandler e) {
      
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
 
    } 

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }
 
  try {
    //remove pll access
    delete pll ;
  }
  catch (FecExceptionHandler e) {
  

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;

    return er.str() ;

  }

   return o.str() ;
}

/** 
 * <p>command: -device pll
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string resetPll ( FecAccess *fecAccess,
		  tscType8 fecAddress, 
		  tscType8 ringAddress,
		  tscType8 ccuAddress, 
		  tscType8 channelAddress,
		  tscType8 deviceAddress,
		  long loop, unsigned long tms ) {

 
  pllAccess *pll ;

  std::ostringstream o;
  std::ostringstream er;
 

  try {
    //access pll
    pll = new pllAccess ( fecAccess,
                          fecAddress,
                          ringAddress,
                          ccuAddress,
                          channelAddress,
                          deviceAddress) ; // Open a pll access
    
  }
  catch (FecExceptionHandler e) {  
 

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }  
 
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) { 

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {

      //reset pll
      tscType8 reset;
      reset = pll->pllReset ( );    
   
      o << endl << endl 
	<< "PLL (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): reset" << std::endl << std::endl; 
	
    }
    
    catch (FecExceptionHandler e) {
     
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
  
    } 

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }
 
  try {
    //remove pll access
    delete pll ;
  }
  catch (FecExceptionHandler e) {
 

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;

    return er.str() ;

  }

   return o.str() ;
}


/** 
 * <p>command: -device pll
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setPll_ClockPhase ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value) {

  
  pllAccess *pll ;

  std::ostringstream o;
  std::ostringstream er;
 
  try {

    //access pll
    pll = new pllAccess ( fecAccess,
                          fecAddress,
                          ringAddress,
                          ccuAddress,
                          channelAddress,
                          deviceAddress) ; // Open a pll access
  }
  catch (FecExceptionHandler e) {
 

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  } 


  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try { 

      //read old value
      o << endl 
	<< "Set PLL (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->ClockPhase: Changed Value from " << std::dec << (int)pll->getClockPhase() << " to ";
     
      // Set the pll 
      pll->setClockPhase ((tscType8)value) ;

      // read new value
      o << std::dec << (int)pll->getClockPhase() << endl;

    }
    catch (FecExceptionHandler e) {
   

      o << endl << endl 
	<< "------------ Exception ----------" << std::endl << 
	e.what() << std::endl << "---------------------------------" << std::endl;      
      
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove pll access
    delete pll ;
  }
  catch (FecExceptionHandler e) {
   

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;

  }

  return o.str() ;
}

/** 
 * <p>command: -device pll
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setPll_TriggerDelay ( FecAccess *fecAccess,
			     tscType8 fecAddress, 
			     tscType8 ringAddress,
			     tscType8 ccuAddress, 
			     tscType8 channelAddress,
			     tscType8 deviceAddress,
			     long loop, unsigned long tms,
			     unsigned int value ) {

 
  pllAccess *pll ; 
  std::ostringstream o;
  std::ostringstream er;
 
  try {

    //access pll
    pll = new pllAccess ( fecAccess,
                          fecAddress,
                          ringAddress,
                          ccuAddress,
                          channelAddress,
                          deviceAddress) ; // Open a pll access
  }
  catch (FecExceptionHandler e) {
  

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  } 

 
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {

      // read old value
      o << endl
	<< "Set PLL (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->TriggerDelay: Changed Value from " << (int)pll->getTriggerDelay() << " to ";
         
      // Set the pll 
      pll->setTriggerDelay ((tscType8)value) ;

      // read new value
      o << (int)pll->getTriggerDelay() << std::endl ;
    
    }
    catch (FecExceptionHandler e) {
     

      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;      

    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove pll access
    delete pll ;
  }
  catch (FecExceptionHandler e) {
 

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

   return o.str() ;
}

/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string readDelay25 ( FecAccess *fecAccess,
		     tscType8 fecAddress, 
		     tscType8 ringAddress,
		     tscType8 ccuAddress, 
		     tscType8 channelAddress,
		     tscType8 deviceAddress,
		     long loop, unsigned long tms ) {
  
  delay25Access *delay25 ; 
 
  std::ostringstream o;
  std::ostringstream er;
  
  try {
    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access 
    
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
     
    return er.str() ;
  } 

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) { 

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {  
     
      //read delay25
      o << endl 
	<< "Read Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl; 
      o	<< "-->CR0    : 0x" << std::hex <<  (int)delay25->getCR0();
      if ( delay25->getCR0() & (1 << 6) ) o << "  (enabled)" << std::endl;
      else o << " (not enabled)" << std::endl;
      o << "-->CR1    : 0x" << std::hex << (int)delay25->getCR1();
      if ( delay25->getCR1() & (1 << 6) ) o << "  (enabled)" << std::endl;
      else o << " (not enabled)" << std::endl; 
      o	<< "-->CR2    : 0x" << std::hex << (int)delay25->getCR2();
      if ( delay25->getCR2() & (1 << 6) ) o << "  (enabled)" << std::endl;
      else o << " (not enabled)" << std::endl; 
      o	<< "-->CR3    : 0x" << std::hex << (int)delay25->getCR3(); 
      if ( delay25->getCR3() & (1 << 6) ) o << "  (enabled)" << std::endl;
      else o << " (not enabled)" << std::endl;
      o	<< "-->CR4    : 0x" << std::hex << (int)delay25->getCR4();
      if ( delay25->getCR4() & (1 << 6) ) o << "  (enabled)" << std::endl;
      else o << " (not enabled)" << std::endl;
      o	<< "-->GCR    : 0x" << std::hex << (int)delay25->getGCR() << std::endl << std::endl 
	<< "-->Delay0 : "   << std::dec << (int)delay25->getDelay0() << std::endl 
	<< "-->Delay1 : "   << std::dec << (int)delay25->getDelay1() << std::endl 
	<< "-->Delay2 : "   << std::dec << (int)delay25->getDelay2() << std::endl 
	<< "-->Delay3 : "   << std::dec << (int)delay25->getDelay3() << std::endl 
	<< "-->Delay4 : "   << std::dec << (int)delay25->getDelay4() << std::endl 
	<< "-->Mode   : "   << std::dec << (int)delay25->getFrequencyMode() << "MHz" << std::endl << std::endl ; 

      
    }
    
    catch (FecExceptionHandler e) { 
           
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     

    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove delay25 access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) { 
   

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;

    return er.str() ;
  }  
 

  return o.str() ;
}

/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string ResynchDLL ( FecAccess *fecAccess,
		    tscType8 fecAddress, 
		    tscType8 ringAddress,
		    tscType8 ccuAddress, 
		    tscType8 channelAddress,
		    tscType8 deviceAddress,
		    long loop, unsigned long tms ) {
 
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {
    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access

  }
  catch (FecExceptionHandler e) { 
    

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {

      //resynchronisation
      delay25->forceDLLResynchronisation ( ) ; 
      o << endl << endl 
	<< "Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): DLL resynchronisation done" << std::endl << std::endl; 
     

    }
    catch (FecExceptionHandler e) {
  
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {

    //remove i2c access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {
 
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDelay25_Delay0 ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value ) {
 
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {
    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access

  }
  catch (FecExceptionHandler e) { 

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {

      //read old value
      o << endl
	<< "Set Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->Delay (CR0): Changed Value from " << (int)delay25->getDelay0() << " to ";
     
      // Set the delay25 
      delay25->setDelay0 ((tscType8)value) ; 
      
      //read new value
      o << (int)delay25->getDelay0() << endl;

    }
    catch (FecExceptionHandler e) {

      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove delay25 access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {
  

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}



/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDelay25_Delay1 ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value ) {
  
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {
    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try { 

      // read old value
      o << endl 
	<< "Set Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->Delay (CR1): Changed Value from " << (int)delay25->getDelay1() << " to "; 

         
      // Set the delay25 
      delay25->setDelay1 ((tscType8)value) ; 
   
      // read new value
      o << (int)delay25->getDelay1() << endl;

    }
    catch (FecExceptionHandler e) {
  
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove i2c access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDelay25_Delay2 ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value ) {
 
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {
    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access
  }
  catch (FecExceptionHandler e) {
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 

    
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try { 
      
      //read old value
      o << endl 
	<< "Set Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->Delay (CR2): Changed Value from " << (int)delay25->getDelay2() << " to ";
     
      // Set the delay25 
      delay25->setDelay2 ((tscType8)value) ; 
     
      // read new value
      o << (int)delay25->getDelay2() << endl;

    }
    catch (FecExceptionHandler e) {
      
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove i2c access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {
  
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDelay25_Delay3 ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value ) {
 
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {

    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access
  }
  catch (FecExceptionHandler e) {
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 
   
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {
     
      // read old value 
      o << endl
	<< "Set Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->Delay (CR3): Changed Value from " << (int)delay25->getDelay3() << " to ";
        
      // Set the delay25 
      delay25->setDelay3 ((tscType8)value) ; 
      
      //read new value
      o << (int)delay25->getDelay3() << endl;

    }
    catch (FecExceptionHandler e) {
 
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove i2c access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device delay25
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDelay25_Delay4 ( FecAccess *fecAccess,
			   tscType8 fecAddress, 
			   tscType8 ringAddress,
			   tscType8 ccuAddress, 
			   tscType8 channelAddress,
			   tscType8 deviceAddress,
			   long loop, unsigned long tms,
			   unsigned int value ) {
  
  delay25Access *delay25 ; 
  std::ostringstream o;
  std::ostringstream er;
   
  try {

    //access delay25
    delay25 = new delay25Access ( fecAccess,
				  fecAddress,
				  ringAddress,
				  ccuAddress,
				  channelAddress,
				  deviceAddress) ; // Open a delay25 access
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    return er.str() ;
  } 

   
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;
    
    try {

      //read old value
      o << endl 
	<< "Set Delay25 (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl 
	<< "-->Delay (CR4): Changed Value from " << (int)delay25->getDelay4() << " to ";
      
          
      // Set the delay25 
      delay25->setDelay4 ((tscType8)value) ; 
     
      //read new value
      o << (int)delay25->getDelay4() << endl;

    }
    catch (FecExceptionHandler e) {
    
      o << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
     
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  try {
    //remove delay25 access
    delete delay25 ;
  }
  catch (FecExceptionHandler e) {
 
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device laserdriver
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string readLaserdriver ( FecAccess *fecAccess,
			 tscType8 fecAddress, 
			 tscType8 ringAddress,
			 tscType8 ccuAddress, 
			 tscType8 channelAddress,
			 tscType8 deviceAddress,
			 long loop, unsigned long tms) {

  laserdriverAccess *ld = NULL ;
  laserdriverDescription *mald = NULL ;

  std::ostringstream o;
  std::ostringstream er;
  
  try {

    ld = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress) ; // Open a laserdriver access
  }
  catch (FecExceptionHandler e) {
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {

      // Get its value back
      mald = ld->getValues() ;

      o << endl
	<< "Read AOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << ", deviceAddress 0x" << std::hex << (int)deviceAddress << "):" << std::endl;
      o << "-->Gain channel 0:  " << std::dec << (int)mald->getGain0() << std::endl ;
      o << "-->Gain channel 1:  " << std::dec << (int)mald->getGain1() << std::endl ;
      o << "-->Gain channel 2:  " << std::dec << (int)mald->getGain2() << std::endl ;

      tscType8 bias[MAXLASERDRIVERCHANNELS] ;
      mald->getBias (bias) ;

      for (int i = 0 ; i < MAXLASERDRIVERCHANNELS ; i ++) {
      
	o << "-->Bias channel " << i << ":  " << std::dec << (int)bias[i] << std::endl ;
      }

     
      if ( ld->getSeuStatus() ) o <<  "-->SEU detected  : yes" << std::endl ;
      else o << "-->SEU detected  : no" << std::endl ; 
      
      delete mald ;
    }
    catch (FecExceptionHandler e) {
    
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;      
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (ld != NULL) delete ld ;
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}

string readPoh ( FecAccess *fecAccess,
		 tscType8 fecAddress, 
		 tscType8 ringAddress,
		 tscType8 ccuAddress, 
		 tscType8 channelAddress,
		 tscType8 deviceAddress,
		 long loop, unsigned long tms) {

  laserdriverAccess *lda = NULL ;
  laserdriverDescription *malda = NULL ;
  laserdriverAccess *ldb = NULL ;
  laserdriverDescription *maldb = NULL ;

  std::ostringstream o;
  std::ostringstream er;
  
  try {

    lda = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress) ; // Open a laserdriver access
    ldb = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress+4) ; // Open a laserdriver access
  }
  catch (FecExceptionHandler e) {
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {

      // Get its value back
      malda = lda->getValues() ;
      maldb = ldb->getValues() ;

      o << endl
	<< "Read POH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << ", deviceAddress 0x" << std::hex << (int)deviceAddress << "):" << std::endl;
      o << "-->Gain channel 0:  " << std::dec << (int)malda->getGain0() << std::endl ;
      o << "-->Gain channel 1:  " << std::dec << (int)malda->getGain2() << std::endl ;
      o << "-->Gain channel 2:  " << std::dec << (int)maldb->getGain0() << std::endl ;
      o << "-->Gain channel 3:  " << std::dec << (int)maldb->getGain2() << std::endl ;

      tscType8 bias_a[MAXLASERDRIVERCHANNELS] ;
      malda->getBias (bias_a) ;
      tscType8 bias_b[MAXLASERDRIVERCHANNELS] ;
      maldb->getBias (bias_b) ;

      o << "-->Bias channel 0:  " << std::dec << (int)bias_a[0] << std::endl ;
      o << "-->Bias channel 1:  " << std::dec << (int)bias_a[2] << std::endl ;
      o << "-->Bias channel 2:  " << std::dec << (int)bias_b[0] << std::endl ;
      o << "-->Bias channel 3:  " << std::dec << (int)bias_b[2] << std::endl ;
     
      if ( lda->getSeuStatus() || ldb->getSeuStatus() ) o <<  "-->SEU detected  : yes" << std::endl ;
      else o << "-->SEU detected  : no" << std::endl ; 
      
      delete malda ;
      delete maldb ;

    }
    catch (FecExceptionHandler e) {
    
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;      
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (lda != NULL) delete lda ;
    if (ldb != NULL) delete ldb ;
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;

}

/** 
 * <p>command: -device laserdriver
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setLaserdriver ( FecAccess *fecAccess,
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
			unsigned int valuebias2) {

  laserdriverAccess *ld = NULL ;
  laserdriverDescription *mald1 = NULL ;
  laserdriverDescription *mald = NULL ;

  std::ostringstream o;
  std::ostringstream er;
 
  try {

    ld = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress) ; // Open a laserdriver access
  }
  catch (FecExceptionHandler e) { 
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {
      
      // Get its value back
      mald1 = ld->getValues() ;

      // Set the values
      if (setgain0) mald1->setGain(0,valuegain0);
      if (setgain1) mald1->setGain(1,valuegain1);
      if (setgain2) mald1->setGain(2,valuegain2);
      if (setbias0) mald1->setBias(0,valuebias0);
      if (setbias1) mald1->setBias(1,valuebias1);
      if (setbias2) mald1->setBias(2,valuebias2);
      
      // Set the aoh
      ld->setValues (*mald1) ;
           
      // Get its value back
      mald = ld->getValues() ;

      if (*mald == *mald1) { // Show debug message
	o << endl 
	   << "Write AOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): done" << std::endl;
	 
      }
      else {
	o << endl 
	  << "Write AOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): ERROR" << std::endl;
      }
      

      delete mald ;
    }

    catch (FecExceptionHandler e) {
     
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;        

      
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (ld != NULL) delete ld ;
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
    
  }

  return o.str() ;
}

string setPoh ( FecAccess *fecAccess,
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
		unsigned int valuebias3) {

  laserdriverAccess *lda = NULL ;
  laserdriverDescription *malda1 = NULL ;
  laserdriverDescription *malda = NULL ;

  laserdriverAccess *ldb = NULL ;
  laserdriverDescription *maldb1 = NULL ;
  laserdriverDescription *maldb = NULL ;

  std::ostringstream o;
  std::ostringstream er;
 
  try {

    lda = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress) ; // Open a laserdriver access
    ldb = new laserdriverAccess ( fecAccess,
                                 fecAddress,
                                 ringAddress,
                                 ccuAddress,
                                 channelAddress,
                                 deviceAddress+4) ; // Open a laserdriver access
  }
  catch (FecExceptionHandler e) { 
   
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {
      
      // Get its value back
      malda1 = lda->getValues() ;
      maldb1 = ldb->getValues() ;

      // Set the values
      if (setgain0) malda1->setGain(0,valuegain0);
      if (setgain1) malda1->setGain(2,valuegain1);
      if (setgain2) maldb1->setGain(0,valuegain2);
      if (setgain3) maldb1->setGain(2,valuegain3);
      if (setbias0) malda1->setBias(0,valuebias0);
      if (setbias1) malda1->setBias(2,valuebias1);
      if (setbias2) maldb1->setBias(0,valuebias2);
      if (setbias3) maldb1->setBias(2,valuebias3);
      
      // Set the poh
      lda->setValues (*malda1) ;
      ldb->setValues (*maldb1) ;
           
      // Get its value back
      malda = lda->getValues() ;
      maldb = ldb->getValues() ;

      if (*malda == *malda1 && *maldb == *maldb1) { // Show debug message
	o << endl 
	   << "Write POH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): done" << std::endl;
	 
      }
      else {
	o << endl 
	  << "Write AOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): ERROR" << std::endl;
      }
      

      delete malda ;
      delete maldb ;
    }

    catch (FecExceptionHandler e) {
     
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;        

      
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (lda != NULL) delete lda ;
    if (ldb != NULL) delete ldb ;
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
    
  }

  return o.str() ;
}

/** 
 * <p>command: -device doh
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string readDoh ( FecAccess *fecAccess,
		 tscType8 fecAddress, 
		 tscType8 ringAddress,
		 tscType8 ccuAddress, 
		 tscType8 channelAddress,
		 tscType8 deviceAddress,
		 long loop, unsigned long tms ) {

  
  DohAccess *doh = NULL ;
  laserdriverDescription *mald = NULL ;

  std::ostringstream o;
  std::ostringstream er; 

  try {

    doh = new DohAccess ( fecAccess,
			  fecAddress,
			  ringAddress,
			  ccuAddress,
			  channelAddress,
			  deviceAddress) ; // Open a laserdriver access
  }
  catch (FecExceptionHandler e) {
    
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {
      
      // Get its value back
      mald = doh->getValues() ; 

      o << endl
	<< "Read DOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "):" << std::endl; 
      o << "-->Gain channel 0:  " << std::dec << (int)mald->getGain0() << std::endl ;
      o << "-->Gain channel 1:  " << std::dec << (int)mald->getGain1() << std::endl ;
      o << "-->Gain channel 2:  " << std::dec << (int)mald->getGain2() << std::endl ;
     
     
      tscType8 bias[MAXLASERDRIVERCHANNELS] ;
      mald->getBias (bias) ;

      for (int i = 0 ; i < MAXLASERDRIVERCHANNELS ; i ++) {
      

	o << "-->Bias channel " << i << ":  " << std::dec << (int)bias[i] << std::endl ;
      } 

      if ( doh->getSeuStatus() ) o <<  "-->SEU detected  : yes" << std::endl ;
      else o << "-->SEU detected  : no" << std::endl ; 

      delete mald ;
    }

    catch (FecExceptionHandler e) {
     
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl; 
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (doh != NULL) delete doh ;
  }
  catch (FecExceptionHandler e) {

    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}


/** 
 * <p>command: -device doh
 * \param fecAccess - FEC Access object
 * \param fecAddress - FEC slot
 * \param ringAddress - ring slot
 * \param channelAddress - I2C channel number
 * \param deviceAddress - I2C device address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
string setDoh ( FecAccess *fecAccess,
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
		unsigned int valuebias2) {

  DohAccess *doh = NULL ;
  laserdriverDescription *mald1 = NULL ;
  laserdriverDescription *mald = NULL ;

  std::ostringstream o;
  std::ostringstream er; 

  try {

    doh = new DohAccess ( fecAccess,
			  fecAddress,
			  ringAddress,
			  ccuAddress,
			  channelAddress,
			  deviceAddress) ; // Open a laserdriber access
  }
  catch (FecExceptionHandler e) {
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    
    return er.str() ;
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    if (loop != 1)
      o << "----------------------- Loop " << loopI+1 << std::endl ;

    try {
      // Get its value back
      mald1 = doh->getValues() ;
     

      // Set the values
      if (setgain0) mald1->setGain(0,valuegain0);
      if (setgain1) mald1->setGain(1,valuegain1);
      if (setgain2) mald1->setGain(2,valuegain2);
      if (setbias0) mald1->setBias(0,valuebias0);
      if (setbias1) mald1->setBias(1,valuebias1);
      if (setbias2) mald1->setBias(2,valuebias2);
   
      // Set the doh
      doh->setValues (*mald1, true) ;
   
      // Get its value back
      mald = doh->getValues() ;

      if (*mald == *mald1) { // Show debug message
       
	o << endl 
	   << "Write DOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): done" << std::endl;
      }
      else {
       
	o << endl 
	  << "Write DOH (CCU 0x" << std::hex << (int)ccuAddress << ", channel 0x" << std::hex << (int)channelAddress << "): ERROR" << std::endl;
      }

      delete mald ;
    }

    catch (FecExceptionHandler e) {
      o << endl << endl 
      << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;    
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }


  try {
    if (doh != NULL) delete doh ;
  }
  catch (FecExceptionHandler e) {
 
    er << endl << endl 
       << "------------ Exception ----------" << std::endl << 
      e.what() << std::endl << "---------------------------------" << std::endl;
    
    return er.str() ;
  }

  return o.str() ;
}
		   

/**
 * \warning note that the counters must be available
 * \warning before display all status, operations must be done on CCUs.
 * Only CCUs which supported already operations are displayed.
 */
/*void displayStatus ( FecExceptionHandler *e, unsigned int count,
                     FecAccess *fecAccess, FILE *stdchan ) {

  time_t timestamp = time(NULL) ;
  errorType errorCode = 0 ;
  keyType index = 0xFFFFFFFF ;
  std::string dMsg = "No error";
  static unsigned int errorCounter = 1 ;

  if (e != NULL) {

    timestamp = e->getTimeStamp() ;
    errorCode = e->getErrorCode() ;
    index = 0xFFFFFFFF ;
    if (e->isHardwarePosition())
      index = e->getHardwarePosition() ;
    dMsg = e->getErrorDevelopperMessage ( ) ;
  }

  try {
    
    fprintf (stdchan_, "------------------------------------ Error %d\n", errorCounter) ;
    errorCounter ++ ;

    // Scan the PCI slot for device driver loaded
    std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
    if (fecSlotList != NULL) {

      fprintf (stdchan, "timestamp=%ld\n", timestamp) ;
      if (count != 0) 
        fprintf (stdchan, "counter=%d\n", count) ;

      if (e != NULL) {
        fprintf (stdchan, "errorCode=%ld\n", errorCode) ;
        fprintf (stdchan, "sourceErrorIndex=0x%04X\n", index) ;
        fprintf (stdchan, "dMsg=\"%s\"\n", dMsg.c_str()) ;
      }
      
      for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
        
        keyType index = *p ;        
        FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;
        
	fprintf (stdchan, "FEC %d ring %d\n", getFecKey(index), getRingKey(index)) ;

	// FEC control/status registers
	fprintf ( stdchan, "FEC_SR0=0x%04X\n", (tscType16)fecDevice->getFecRingSR0()) ;
	fprintf ( stdchan, "FEC_SR1=0x%04X\n", fecDevice->getFecRingSR1()) ;
	fprintf ( stdchan, "FEC_CR0=0x%04X\n", fecDevice->getFecRingCR0()) ;
	fprintf ( stdchan, "FEC_CR1=0x%04X\n", fecDevice->getFecRingCR1()) ;

	// For each CCU on that ring
	std::list<keyType> *ccuList = fecDevice->getCcuList() ;
	if (ccuList != NULL) {
	  for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	
	    keyType indexCcu = *p ;
	  
	    // Get CCU registers
	    unsigned int SRA = fecAccess->getCcuSRA(indexCcu) ;
	    unsigned int SRB = fecAccess->getCcuSRB(indexCcu) ;
	    unsigned int SRC = fecAccess->getCcuSRC(indexCcu) ;
	    unsigned int SRD = fecAccess->getCcuSRD(indexCcu) ;
	    unsigned int SRE = fecAccess->getCcuSRE(indexCcu) ;
	    unsigned int SRF = fecAccess->getCcuSRF(indexCcu) ;
	    unsigned int CRA = fecAccess->getCcuCRA(indexCcu) ;
	    unsigned int CRB = fecAccess->getCcuCRB(indexCcu) ;
	    unsigned int CRC = fecAccess->getCcuCRC(indexCcu) ;
	    unsigned int CRD = fecAccess->getCcuCRD(indexCcu) ;
	    unsigned int CRE = fecAccess->getCcuCRE(indexCcu) ;
	    //unsigned int CRF = fecAccess->getCcuCRF(indexCcu) ;
	  
	    unsigned int ccuAddress = getCcuKey (indexCcu) ;
	    fprintf ( stdchan, "CCU_0x%02X_SRA=0x%04X\n", ccuAddress, SRA) ; 
	    fprintf ( stdchan, "CCU_0x%02X_SRB=0x%04X\n", ccuAddress, SRB) ; 
	    fprintf ( stdchan, "CCU_0x%02X_SRC=0x%04X\n", ccuAddress, SRC) ; 
	    fprintf ( stdchan, "CCU_0x%02X_SRD=0x%04X\n", ccuAddress, SRD) ; 
	    fprintf ( stdchan, "CCU_0x%02X_SRE=0x%04X\n", ccuAddress, SRE) ; 
	    fprintf ( stdchan, "CCU_0x%02X_SRF=0x%04X\n", ccuAddress, SRF) ; 
	    fprintf ( stdchan, "CCU_0x%02X_CRA=0x%04X\n", ccuAddress, CRA) ; 
	    fprintf ( stdchan, "CCU_0x%02X_CRB=0x%04X\n", ccuAddress, CRB) ; 
	    fprintf ( stdchan, "CCU_0x%02X_CRC=0x%04X\n", ccuAddress, CRC) ; 
	    fprintf ( stdchan, "CCU_0x%02X_CRD=0x%04X\n", ccuAddress, CRD) ; 
	    fprintf ( stdchan, "CCU_0x%02X_CRE=0x%04X\n", ccuAddress, CRE) ; 
	  }
	}
      }
    }
    else {
      fprintf ( stdchan, "Unable to read the CCUs" ) ;
    }
  }
  catch (FecExceptionHandler e) {

    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;
  }
  }*/




/**
 * This method try to find all the device driver loaded for FECs and reset all PLX 
 * and FECs
 * <p>command: -reset
 * \param fecAccess - FEC access object
 * \param fecAddress - FEC slot to be reseted
 * \param ringAddress - ring to be reseted
 * \warning the loop is used only for the FEC reset it self
 */
string resetPlxFec ( FecAccess *fecAccess,
		     tscType8 fecAddress,
		     tscType8 ringAddress,
		     long loop, unsigned long tms ) {

  std::ostringstream o;

  try {

    // Build the index for the given ring
    keyType index = buildFecRingKey(fecAddress,ringAddress) ;

    // Make a reset
    //fecAccess->fecHardReset(index) ;

#ifdef TTCRx
    fecAccess->initTTCRx(index) ;
#endif

    // Wait for any alarms
    for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

      //if (fecSlotList != NULL) {
      //for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
      //keyType index = *p ;

      // Ring A
      fecAccess->fecRingReset(index) ;
      // Ring B
      fecAccess->fecRingResetB(index) ;
      // Reset the FSM machine
      fecAccess->fecRingResetFSM(index) ;
        
      tscType16 fecSR0 = fecAccess->getFecRingSR0(index) ;
     
      o << std::endl << "Value of the Status Register 0 of the FEC " << (int)getFecKey(index) << ", ring " << (int)getRingKey(index) << ": 0x" << std::hex << (int)fecSR0 << std::endl ;
      if (fecSR0 & 0x1)
	o << "\tFIFO transmit running" << std::endl ;
      if (fecSR0 & 0x2)
	o << "\tFIFO receive running" << std::endl ;
      if (fecSR0 & 0x4)
	o << "\tFIFO receive half full" << std::endl ;
      if (fecSR0 & 0x8)
	o << "\tFIFO receive full" << std::endl ;
      if (fecSR0 & 0x10)
	o << "\tFIFO receive empty" << std::endl ;
      if (fecSR0 & 0x20)
	o << "\tFIFO return half full" << std::endl ;
      if (fecSR0 & 0x40)
	o << "\tFIFO return full" << std::endl ;
      if (fecSR0 & 0x80)
	o << "\tFIFO return empty" << std::endl ;
      if (fecSR0 & 0x100)
	o << "\tFIFO transmit half full" << std::endl ;
      if (fecSR0 & 0x200)
	o << "\tFIFO transmit full" << std::endl ;
      if (fecSR0 & 0x400)
	o << "\tFIFO transmit empty" << std::endl ;
      if (fecSR0 & 0x800)
	o << "\tLink initialise" << std::endl ;
      if (fecSR0 & 0x1000) 
	o << "\tPending irg" << std::endl ;
      if (fecSR0 & 0x2000)
	o << "\tData to FEC" << std::endl ;
      if (fecSR0 & 0x4000)
	o << "\tTTCRx ok" << std::endl ;

      // Wait
      if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
    }
  }
  catch (FecExceptionHandler e) {
    
    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;

    o << std::endl << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;

  }

  return o.str();
}

/**
 * This method reset the crate
 * <p>command: -crateReset
 * <p>command: -testCrateReset
 * \param fecAccess - FEC access object
 */
string crateReset ( FecAccess *fecAccess, bool testCrateReset,
		  long loop, unsigned long tms ) {
  std::ostringstream o;
  #if defined(BUSVMECAENPCI) || defined(BUSVMECAENUSB) || defined (BUSVMESBS)
  // -----------------------------------------------------------------------------------
  // crate reset + reload the mFEC firmware

 
  try {
    if (fecAccess->getFecBusType() == FECVME) {
      fecAccess->crateReset() ;
    }
    else {
      o << "The crate reset is not available for other FEC than VME FEC" << std::endl ;
      std::cerr << "The crate reset is not available for other FEC than VME FEC" << std::endl ;
    }
  }
  catch (FecExceptionHandler &e) {
    
    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;  

    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // -----------------------------------------------------------------------------------
  // reset the crate then test the crate reset to see if firmware are correctly reloaded
  if (testCrateReset) { 

    std::ofstream fichier ( "/tmp/FecTestCrateReset.txt", std::ios_base::app ) ;
    if (!fichier) {
      std::cerr << "Cannot open the file /tmp/FecTestCrateReset.txt, cannot run the test" << std::endl ; 
      o << "Cannot open the file /tmp/FecTestCrateReset.txt, cannot run the test" << std::endl ;
      return o.str();
    }
      
    time_t coucou = time(NULL) ;
    fichier << std::endl << std::endl << "--------------------------------------------- test started on " << ctime(&coucou) ;
    fichier.flush() ;

    std::list<keyType> *listFecK = fecAccess->getFecList() ;
    std::list<keyType> listFecKey ;
    for (std::list<keyType>::iterator it = listFecK->begin() ; it != listFecK->end() ; it ++) {
      try {
	FecVmeRingDevice *fecVmeRing = (FecVmeRingDevice *)fecAccess->getFecRingDevice(*it) ;
	fecVmeRing->getVmeVersion () ;
	fecVmeRing->getFecRingSR0() ;
	listFecKey.push_back(*it) ;
      }
      catch (FecExceptionHandler &e) {
	try {
	  fichier << "Strange, the ring " << (int)getFecKey(*it) << "." << (int)getRingKey(*it) << " was existing before any crate reset and does exists anymore" ;
	  fichier.flush() ;
	  fecAccess->fecHardReset(*it,0x1) ;
	  fecAccess->getFecRingSR0(*it) ;
	  fichier << ": recovered" << std::endl ;
	}
	catch (FecExceptionHandler &e) { 
	  fichier << ": still in error, exiting" << std::endl ;
	  return o.str();
	}
      }
    }
    delete listFecK ;

    if (!listFecKey.size()) {
      coucou = time(NULL) ;
      fichier << "no FECs on that crate, stopping the test on " << ctime(&coucou) ;
      fichier.close() ;
    }
    else {
#define LOOPIDISPLAY 10000
      try {
	for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

	  //std::cout << loopI << ": " << (loopI >= 100) << " " << ((loopI % 100)==0) << std::endl ;
	  if ( (loopI >= LOOPIDISPLAY) && ((loopI % LOOPIDISPLAY) == 0) ) {
	    coucou = time(NULL) ;
	    fichier << "| " << loopI << " times | " << ctime(&coucou) ;
	    fichier.flush() ;
	  }

	  // crate reset withtout reload the firmware
	  FecVmeRingDevice::crateReset ( );
	  
	  for (std::list<keyType>::iterator it = listFecKey.begin() ; it != listFecKey.end() ; it ++) {
	    try {
	      FecVmeRingDevice *fecVmeRing = (FecVmeRingDevice *)fecAccess->getFecRingDevice(*it) ;
	      if ( (loopI >= LOOPIDISPLAY) && ((loopI % LOOPIDISPLAY) == 0) ) {
		o << loopI << " times" << " on FEC " << (int)getFecKey(*it) << " ring " << (int)getRingKey(*it) ;
		fichier << "| " << loopI << " times | " << "FEC | " << (int)getFecKey(*it) << " | Ring | " << (int)getRingKey(*it) ;
	      }

	      try {
		if ( (loopI >= LOOPIDISPLAY) && ((loopI % LOOPIDISPLAY) == 0) ) {
		  o << ", VME firmware version = " << fecVmeRing->getVmeVersion( ) ;
		  fichier <<  " | VME firmware version | " << fecVmeRing->getVmeVersion( ) ;
		}
	      }
	      catch (FecExceptionHandler &e) {
		o << std::endl ;
		coucou = time(NULL) ;
		fichier << "ERROR on reading the VME firmware on " << (int)getFecKey(*it) ;
		fichier.flush() ;
	      }
	      
	      try {
		if ( (loopI >= LOOPIDISPLAY) && ((loopI % LOOPIDISPLAY) == 0) ) {
		  o << ", SR0 = 0x" << std::hex << fecVmeRing->getFecRingSR0() << std::dec << std::endl ;
		  fichier <<  " | SR0 | 0x" << std::hex << fecVmeRing->getFecRingSR0() << std::dec << " | " ;
		}
	      }
	      catch (FecExceptionHandler &e) {
		o << std::endl ;
		fichier << "ERROR on reading the SR0 on ring " << (int)getFecKey(*it) << "." << (int)getRingKey(*it) << std::endl ;
		fichier.flush() ;
	      }
	    }
	    catch (FecExceptionHandler &e) {
	      fichier << "No FEC found on " << (int)getFecKey(*it) << std::endl ;
	      fichier.flush() ;
	    }

	    if ( (loopI >= LOOPIDISPLAY) && ((loopI % LOOPIDISPLAY) == 0) ) {
	      fichier << std::endl ;
	    }
	  }

	  // Wait
	  if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
	}
      }
      catch (FecExceptionHandler &e) {
	
	std::cerr << "*********** ERROR ********************************" << std::endl ; 
	std::cerr << "An error occurs during hardware access" << std::endl ;
	std::cerr << e.what()  << std::endl ;
	std::cerr << "**************************************************" << std::endl ;
	
	o << "*********** ERROR ********************************" << std::endl ; 
	o << "An error occurs during hardware access" << std::endl ;
	o << e.what()  << std::endl ;
	o << "**************************************************" << std::endl ;

	coucou = time(NULL) ;
	fichier << "Problem during the crate reset: " << e.what() << ", stopping the test on " << ctime(&coucou) ;
	fichier.close() ;
      }
    }

    coucou = time(NULL) ;
    fichier << "Number of loops done = " << loop << ", test stop at " << ctime(&coucou) ;
    fichier.close() ;
  }
  #endif
  return o.str();
}

//##################3
/**
This method performs a test on the DC-DC converters
*/

string DCDCenableTest (  FecAccess   *fecAccess  , 
			 tscType8     fecAddress ,
			 tscType8     ringAddress, 
			 tscType8     ccuAddress,
			 unsigned int dcdcAddress,
			 bool         noBroadcast )  {
 
  std::ostringstream o;
  char textBuffer[ 4096 ];


  /// This is the initCCUs method from Aachen software/// 
  keyType ccuAddressKey = buildCompleteKey(fecAddress,ringAddress,ccuAddress,0,0) ; // Build the key
  keyType ccuChannelKey;
  tscType32 ccuCRE;
  tscType16 ddr, piaData, piaIndex;
  tscType16 piaChannel[ 4 ] = { 0x30, 0x31, 0x32, 0x33 };
  tscType16 piaDDR[ 4 ]     = { 0x06, 0x39, 0x8E, 0x67 };

  try {
 
      fecAccess->setCcuCRE( ccuAddressKey, 0xF0000 );
      ccuCRE = fecAccess->getCcuCRE( ccuAddressKey );

      if( ccuCRE != 0xF0000 ) {
        printf( "CCU 0x%02X: CRE=0x%05X (0xF0000)\n", ccuAddress, ccuCRE );
        return o.str();
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        fecAccess->setPiaChannelDataReg( ccuChannelKey, 0 );
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );

        fecAccess->setPiaChannelDDR( ccuChannelKey, piaDDR[ piaIndex ] );
        ddr = fecAccess->getPiaChannelDDR( ccuChannelKey );

        if( ddr != piaDDR[ piaIndex ] ) {
          printf( "CCU 0x%02X DDR%c=0x%02X (0x%02X)\n", ccuAddress, 'A' + piaIndex, ddr, piaDDR[ piaIndex ] );
          return o.str();
        }
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        piaData = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];

        if( piaData != 0 ) {
          printf( "CCU 0x%02X PIA%c=0x%02X (0x00)\n", ccuAddress, 'A' + piaIndex, piaData );
          return o.str();
        }
      }
    
  }
  catch( FecExceptionHandler e ) {
    sprintf( textBuffer, "\n------------<<<< Exception >>>>------------\n" );
    sprintf( textBuffer, "%sAn error occurs during CCU Channels Enable\n", textBuffer );
    sprintf( textBuffer, "%s-------------------------------------------\n", textBuffer );
    sprintf( textBuffer, "%s%s\n", textBuffer, e.what().c_str( ) );
    sprintf( textBuffer, "%s-------------------------------------------\n", textBuffer );

    std::cerr << textBuffer;
    exit( EXIT_FAILURE );
  }

  o << " CCU DCDC Converter Enable Test started " << std::endl;

  tscType16 piaValue;
  tscType16 piaInMask[ 4 ]  = { 0xC0, 0xC6, 0x71, 0x98 };
  tscType16 piaENPattern[ 4 ], piaPGPattern[ 4 ];
  tscType16 i, mask, retry;

  char bitPattern[ 14 ] = { 0 };

  bool pgError[ 4 ] = { false };
  bool error, printed;

  struct { tscType16 pia;
           tscType16 bit; } en[ 13 ] = { { 2, 0x08 },   //ENC1_0
                                         { 2, 0x80 },   //ENC1_1
                                         { 3, 0x04 },   //ENC1_2
                                         { 3, 0x01 },   //ENC1_3
                                         { 3, 0x02 },   //ENC1_4
                                         { 3, 0x20 },   //ENC1_5
                                         { 3, 0x40 },   //ENC1_6
                                         { 2, 0x04 },   //ENC2_1
                                         { 2, 0x02 },   //ENC2_2
                                         { 1, 0x20 },   //ENC2_3
                                         { 1, 0x10 },   //ENC2_4
                                         { 1, 0x08 },   //ENC2_5
                                         { 1, 0x01 } }; //ENC2_6

  struct { tscType16 pia;
           tscType16 bit; } pg[ 13 ] = { { 1, 0x02 },   //PGC1_0
                                         { 2, 0x01 },   //PGC1_1
                                         { 2, 0x10 },   //PGC1_2
                                         { 2, 0x40 },   //PGC1_3
                                         { 3, 0x08 },   //PGC1_4
                                         { 3, 0x10 },   //PGC1_5
                                         { 3, 0x80 },   //PGC1_6
                                         { 1, 0x80 },   //PGC2_1
                                         { 2, 0x20 },   //PGC2_2
                                         { 1, 0x40 },   //PGC2_3
                                         { 1, 0x04 },   //PGC2_4
                                         { 0, 0x40 },   //PGC2_5
                                         { 0, 0x80 } }; //PGC2_6
  
  std::string bitPatterns[ 14 ] = { "1111111111111", 
                                    "1111111111110", 
				    "1111111111101", 
				    "1111111111011", 
				    "1111111110111", 
				    "1111111101111",
                                    "1111111011111",
                                    "1111110111111",
				    "1111101111111",
				    "1111011111111",
				    "1110111111111",
				    "1101111111111",
				    "1011111111111",
				    "0111111111111"};

  try {
   

    for( int enPattern = 0x1FFF; enPattern >= 0; enPattern-- ) {
      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        piaENPattern[ piaIndex ] = 0;
        piaPGPattern[ piaIndex ] = 0;
      }
      
      for( mask = 0x1000, i = 0; i < 13; i++, mask >>= 1 ){
	if( enPattern & mask ) {
	  piaENPattern[ en[ i ].pia ] |= en[ i ].bit;
	  piaPGPattern[ pg[ i ].pia ] |= pg[ i ].bit;
	  bitPattern[ i ] = '1';
	}
	else
	  bitPattern[ i ] = '0';
      }
      
      bool skip = true;
      if( (std::string)bitPattern == bitPatterns[13-dcdcAddress] ) skip = false;
      
      if( skip ) continue;

      o << "Pattern " << bitPattern << std::endl;
      printed = false;

      for( piaIndex = 1; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        retry = 0;

        do {
          error = false;
          fecAccess->setPiaChannelDataReg( ccuChannelKey, piaENPattern[ piaIndex ] );
          piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];

          if( piaValue != piaENPattern[ piaIndex ] ) {
            if( !printed ) {
              o << " Enable Selection = " << bitPattern << std::endl;
              printed = true;
            }

          
            error = true;
            retry++;
          }

        } while( error && ( retry < 10 ) );
      }

      if( printed )
        o << std::endl;

      printed = false;
      sleep( 1 );

      int ngood=0;
      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaInMask[ piaIndex ];

        if( piaValue != piaPGPattern[ piaIndex ] ) {
          if( !printed ) {
            o << " Enable Selection = " << bitPattern << std::endl;
            printed = true;
          }

          pgError[ piaIndex ] = true;
        }
        else {
          if( pgError[ piaIndex ] ) {
            printf(  "Error for PIA%c\n", 'A' + piaIndex );
	    
	  }
          else {
	    ngood++;
	  }
        }

        
      }


      if (ngood==4) {
	o << "DCDC: Disable OK " << std::endl;
      }   
      else {
	o << "DCDC: Disable ERROR " << std::endl;
      }
    }

  
  
  //////////////////////////////////////////////////////////////
      int enPattern = 0x1FFF;

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        piaENPattern[ piaIndex ] = 0;
        piaPGPattern[ piaIndex ] = 0;
      }

      for( mask = 0x1000, i = 0; i < 13; i++, mask >>= 1 ){
        if( enPattern & mask ) {
          piaENPattern[ en[ i ].pia ] |= en[ i ].bit;
          piaPGPattern[ pg[ i ].pia ] |= pg[ i ].bit;
          bitPattern[ i ] = '1';
        }
        else
          bitPattern[ i ] = '0';
      }

      o << "Pattern " << bitPattern  << std::endl;
      printed = false;

      for( piaIndex = 1; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        retry = 0;

        do {
          error = false;
          fecAccess->setPiaChannelDataReg( ccuChannelKey, piaENPattern[ piaIndex ] );
          piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];

          if( piaValue != piaENPattern[ piaIndex ] ) {
            if( !printed ) {
              o << " Enable Selection = " << bitPattern << std::endl;
              printed = true;
            }

            error = true;
            retry++;
          }

        } while( error && ( retry < 10 ) );
      }

      if( printed )
        o << std::endl;

      printed = false;
      sleep( 1 );

      int ngood=0;
      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaInMask[ piaIndex ];

        if( piaValue != piaPGPattern[ piaIndex ] ) {
          if( !printed ) {
            o << " Enable Selection = " << bitPattern << std::endl;
            printed = true;
          }

          pgError[ piaIndex ] = true;
        }
        else {
          if( pgError[ piaIndex ] )
            printf( "Error for PIA%c\n", 'A' + piaIndex );
          else 
            ngood++;
        }

        
      }

      if (ngood==4) {
	o << "DCDC: Enable OK" << endl;
      } 
      else {
	o << "DCDC: Enable ERROR" << endl;
      }
     ////////////////////////////////////////////////    
    
  }
  catch( FecExceptionHandler e ) {
    sprintf( textBuffer, "\n-------------<<<< Exception >>>>------------\n" );
    sprintf( textBuffer, "%sAn error occurs during DCDC Converter Enable\n", textBuffer );
    sprintf( textBuffer, "%s--------------------------------------------\n", textBuffer );
    sprintf( textBuffer, "%s%s\n", textBuffer, e.what( ).c_str( ) );
    sprintf( textBuffer, "%s--------------------------------------------\n", textBuffer );

    std::cerr << textBuffer;
    exit( EXIT_FAILURE );
  }

  return o.str();

}
/**
This method performs a test on the DC-DC converters
*/

/*string DCDCenableTest (  FecAccess   *fecAccess  , 
			 tscType8     fecAddress ,
			 tscType8     ringAddress, 
			 tscType8     ccuAddress,
			 unsigned int dcdcAddress,
			 bool         noBroadcast ) {

  std::ostringstream o;
  char textBuffer[ 4096 ];


  /// This is the initCCUs method from Aachen software/// 
  keyType ccuAddressKey = buildCompleteKey(fecAddress,ringAddress,ccuAddress,0,0) ; // Build the key
  keyType ccuChannelKey;
  //keyType   ccuAddressKey, ccuChannelKey;
  tscType32 ccuCRE;
  // tscType16 ccuAddress, ddr, piaData, piaIndex;
  tscType16 ddr, piaData, piaIndex;
  tscType16 piaChannel[ 4 ] = { 0x30, 0x31, 0x32, 0x33 };
  tscType16 piaDDR[ 4 ]     = { 0x06, 0x39, 0x8E, 0x67 };

  try {


      fecAccess->setCcuCRE( ccuAddressKey, 0xF0000 );
      ccuCRE = fecAccess->getCcuCRE( ccuAddressKey );

      if( ccuCRE != 0xF0000 ) {
        printf( "CCU 0x%02X: CRE=0x%05X (0xF0000)\n", ccuAddress, ccuCRE );
	o << "Error writing ccuCRE" << std::endl;
        return o.str();
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        fecAccess->setPiaChannelDataReg( ccuChannelKey, 0 );
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );

        fecAccess->setPiaChannelDDR( ccuChannelKey, piaDDR[ piaIndex ] );
        ddr = fecAccess->getPiaChannelDDR( ccuChannelKey );

        if( ddr != piaDDR[ piaIndex ] ) {
          printf( "CCU 0x%02X DDR%c=0x%02X (0x%02X)\n", ccuAddress, 'A' + piaIndex, ddr, piaDDR[ piaIndex ] );
	  o << "Error writing CCU DDR" << std::endl;
          return o.str();
        }
      }

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        piaData = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];

        if( piaData != 0 ) {
          printf( "CCU 0x%02X PIA%c=0x%02X (0x00)\n", ccuAddress, 'A' + piaIndex, piaData );
	  o << "Error reading CCU DDR" << std::endl;
          return o.str();
        }
      }
      // }
  }
  catch( FecExceptionHandler e ) {
    o << std::endl << "\n------------<<<< Exception >>>>------------"  << std::endl;
    o << "An error occurs during CCU Channels Enable" << std::endl;
    o << e.what() << std::endl;

    std::cerr << textBuffer;
    exit( EXIT_FAILURE );
  }

 

  // tscType16 ccu, ccuIndex, piaValue;
  tscType16 piaValue;
  tscType16 piaInMask[ 4 ]  = { 0xC0, 0xC6, 0x71, 0x98 };
  tscType16 piaENPattern[ 4 ], piaPGPattern[ 4 ];
  tscType16 i, mask, retry;

  char bitPattern[ 14 ] = { 0 };

  bool error, printed;

  struct { tscType16 pia;
           tscType16 bit; } en[ 13 ] = { { 2, 0x08 },   //ENC1_0
                                         { 2, 0x80 },   //ENC1_1
                                         { 3, 0x04 },   //ENC1_2
                                         { 3, 0x01 },   //ENC1_3
                                         { 3, 0x02 },   //ENC1_4
                                         { 3, 0x20 },   //ENC1_5
                                         { 3, 0x40 },   //ENC1_6
                                         { 2, 0x04 },   //ENC2_1
                                         { 2, 0x02 },   //ENC2_2
                                         { 1, 0x20 },   //ENC2_3
                                         { 1, 0x10 },   //ENC2_4
                                         { 1, 0x08 },   //ENC2_5
                                         { 1, 0x01 } }; //ENC2_6

  struct { tscType16 pia;
           tscType16 bit; } pg[ 13 ] = { { 1, 0x02 },   //PGC1_0
                                         { 2, 0x01 },   //PGC1_1
                                         { 2, 0x10 },   //PGC1_2
                                         { 2, 0x40 },   //PGC1_3
                                         { 3, 0x08 },   //PGC1_4
                                         { 3, 0x10 },   //PGC1_5
                                         { 3, 0x80 },   //PGC1_6
                                         { 1, 0x80 },   //PGC2_1
                                         { 2, 0x20 },   //PGC2_2
                                         { 1, 0x40 },   //PGC2_3
                                         { 1, 0x04 },   //PGC2_4
                                         { 0, 0x40 },   //PGC2_5
                                         { 0, 0x80 } }; //PGC2_6
  
  std::string bitPatterns[ 14 ] = { "1111111111111", 
                                    "1111111111110", 
				    "1111111111101", 
				    "1111111111011", 
				    "1111111110111", 
				    "1111111101111",
                                    "1111111011111",
                                    "1111110111111",
				    "1111101111111",
				    "1111011111111",
				    "1110111111111",
				    "1101111111111",
				    "1011111111111",
				    "0111111111111"};

  try {

     //////////////////////////////////////////////////////////////TEST ENABLING
      int enPattern = 0x1FFF;

      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        piaENPattern[ piaIndex ] = 0;
        piaPGPattern[ piaIndex ] = 0;
      }

      for( mask = 0x1000, i = 0; i < 13; i++, mask >>= 1 ){
        if( enPattern & mask ) {
          piaENPattern[ en[ i ].pia ] |= en[ i ].bit;
          piaPGPattern[ pg[ i ].pia ] |= pg[ i ].bit;
          bitPattern[ i ] = '1';
        }
        else
          bitPattern[ i ] = '0';
      }

      for( piaIndex = 1; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        cout << "piaindex " << piaIndex << endl;
	error = false;
	fecAccess->setPiaChannelDataReg( ccuChannelKey, piaENPattern[ piaIndex ] );
	//write
	piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];
	cout << "write " << piaValue << endl;
	cout << "write " << piaENPattern[piaIndex] << endl;
	if( piaValue != piaENPattern[ piaIndex ] ) {
	  cout << "write " << endl;
	  cout << std::hex << piaValue << endl;
	  cout << std::hex << piaENPattern[piaIndex] << endl;
	  error = true;
	}
	//read
	piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaInMask[ piaIndex ];
	cout << "read  " << piaValue << endl;
	cout << "read  " << piaPGPattern[piaIndex] << endl;
	if( piaValue != piaPGPattern[ piaIndex ] ) {
	  error = true;  
	  cout << "read " << endl;
	  cout << std::hex << piaValue << endl;
	  cout << std::hex << piaPGPattern[piaIndex] << endl;
	}
     }

      if(error )
        o << "Enabling DCDC " << std::dec << dcdcAddress << ": ERROR"  << std::endl;
      else
	o << "Enabling DCDC " << std::dec << dcdcAddress << ": OK"  << std::endl;
      
     ////////////////////////////////////////////////TEST DISABLING    
    for( int enPattern = 0x1FFF; enPattern >= 0; enPattern-- ) {
   
      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        piaENPattern[ piaIndex ] = 0;
        piaPGPattern[ piaIndex ] = 0;
      }

         for( mask = 0x1000, i = 0; i < 13; i++, mask >>= 1 ){
           if( enPattern & mask ) {
             piaENPattern[ en[ i ].pia ] |= en[ i ].bit;
             piaPGPattern[ pg[ i ].pia ] |= pg[ i ].bit;
             bitPattern[ i ] = '1';
           }
           else
             bitPattern[ i ] = '0';
         }

      bool skip = true;
      if( dcdcAddress == 99 ){
      for( int i = 0; i < 14; ++i ){
          if( (std::string)bitPattern == bitPatterns[i] ){
             skip = false;
             break;
          } 
      }
      }
      else if( (std::string)bitPattern == bitPatterns[13-dcdcAddress] ) skip = false;
      
      if( skip ) continue;

      for( piaIndex = 1; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        retry = 0;

        do {
          error = false;
          fecAccess->setPiaChannelDataReg( ccuChannelKey, piaENPattern[ piaIndex ] );
          piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaDDR[ piaIndex ];

	  //check write error
          if( piaValue != piaENPattern[ piaIndex ] ) {
            
            error = true;
            retry++;
          }

        } while( error && ( retry < 10 ) );
      }

      if (error)
	o << "ERROR writing piaChannel" << std::endl;
     

      printed = false;
      sleep( 1 );

      bool errordis = false;
      for( piaIndex = 0; piaIndex < 4; piaIndex++ ) {
        ccuChannelKey = ccuAddressKey | setChannelKey( piaChannel[ piaIndex ] );
        piaValue = fecAccess->getPiaChannelDataReg( ccuChannelKey ) & piaInMask[ piaIndex ];
   
        if( piaValue != piaPGPattern[ piaIndex ] ) {
	  errordis=true;
        }
      }
      if( errordis ) {
	o << "Disabling DCDC " << dcdcAddress << ": ERROR"  << std::endl;
      } 
      else {
	o << "Disabling DCDC " << dcdcAddress << ": OK"  << std::endl;
      }
      
    }
  

    
  }
  catch( FecExceptionHandler e ) {
    o << std::endl <<  "-------------<<<< Exception >>>>------------" << std::endl;
    o << "An error occurs during DCDC Converter Enable" << std::endl;
    exit( EXIT_FAILURE );
  }

  return o.str();

  }*/

/**
 * This method try to find all the device driver loaded for FECs and scan the ring
 * for each FEC to find all the CCUs.
 * <p>command: -scanccu (in place of -scanring)
 * \param fecAccess - FEC access object
 * \param noBroadcast - CCU broadcast mode used (false) or not (true) [OPTION parameter]
 * \warning the loop is not used
 */
string testScanCCU ( FecAccess *fecAccess, 
		   tscType8 fecAddress,
		   tscType8 ringAddress,
		   bool noBroadcast ) {
 
  std::ostringstream o;
 
  std::list<keyType> *ccuList = NULL ;
  try { 
	     

    // Scan the ring for each FECs and display the CCUs
    //keyType indexFEC = buildFecRingKey(fecAddress,ringAddress);
    //ccuList = fecAccess->getCcuList ( indexFEC, noBroadcast ) ;
    ccuList = fecAccess->getCcuList ( buildFecRingKey(fecAddress,ringAddress), noBroadcast ) ;
        
    if (ccuList != NULL) {

      for (std::list<keyType>::iterator p=ccuList->begin();p!=ccuList->end();p++) {
      
	keyType index = *p ;

	std::cout << std::hex << "FEC 0x" << (int)getFecKey(index) 
	  << " Ring 0x" << (int)getRingKey(index) 
	  << " CCU 0x" << (int)getCcuKey(index)
	  << " found" << std::endl ;
      }
    }
    else {
     
      o << std::endl << "No CCU found on FECs" << std::endl ;
    }  
  }
  catch (FecExceptionHandler e) {

    o << std::endl << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // Delete = already store in the table of FEC device class
  delete ccuList ;

  return o.str();
}


/**
 * <p>command: -status
 * \param fecAccess - FEC Access object
 * \param noBroadcast - CCU broadcast mode used (false, default) or not (true) [OPTION parameter]
 * \warning the loop is not used
 */
string newtestScanCCU ( FecAccess *fecAccess,
			bool noBroadcast ) {

  std::ostringstream o;
   
  std::list<keyType> *ccuList = NULL ;
  try {
 
    for (tscType8 fecAddress = minFecSlot_ ; fecAddress <= maxFecSlot_  ; fecAddress ++) {

      for (tscType8 ringAddress = minFecRing_ ; ringAddress <= maxFecRing_ ; ringAddress++) {
	
	// Find the FECs
	try {
	 
	  keyType index = buildFecRingKey(fecAddress,ringAddress) ;        
	 
	  // Start the scanning of the ring for each FEC device
	  FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;

	  fecDevice->fecScanRingBroadcast ( ) ;
	}
	catch (FecExceptionHandler e) {

	}
      }
    }

    // Scan the PCI or VME slot for device driver loaded
    std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
    if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
      for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
        
        keyType indexFEC = *p ;        
	
	try {
	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFEC, noBroadcast ) ;

	  if (ccuList != NULL) {
	    
	    for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	      
	      keyType index = *p ;
	      
	      o << std::hex << "FEC 0x" << (int)getFecKey(index) 
		<< " Ring 0x" << (int)getRingKey(index) 
		<< " CCU 0x" << (int)getCcuKey(index)
			<< " found" << std::endl ;
	    }
	  }
	  else { 
	    o << std::endl << "No CCU found on FECs" << std::endl ;
	  }
	     
	}
	catch (FecExceptionHandler e) {
	 	 
	  o << "No CCU on FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	}
      }
    }
    else {
    
      o << "No FEC found" << std::endl ;
    }  
  }
  catch (FecExceptionHandler e) {
  
    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // Delete = already store in the table of FEC device class
  delete ccuList ;
  return o.str();
}


/**
 * This method scan the ring for all the I2C address (0x0 -> 0xFF)
 * <p>command: -scanringedevice
 * \param fecAddress - FEC address
 * \param ringSlot   - ring slot
 * \warning the precompilation flag must be set
 * \warning the loop is not used
 */
string testScanRingEDevice ( FecAccess *fecAccess,
			     tscType8 fecAddress, tscType8 ringAddress ) { 

  std::ostringstream o;
  std::ostringstream er;

  FecRingDevice *fecDevice ;
  try {

    fecDevice = fecAccess->getFecRingDevice (buildFecRingKey(fecAddress,ringAddress)) ;
    fecDevice->fecHardReset() ;
    // Disable PLX interrupts
    fecDevice->setIRQ (false) ;
  }
  catch (FecExceptionHandler e) {
   

    er << std::endl << "------------ Exception ----------" << std::endl ;
    er << "Did not find the FEC" << std::endl ;
    er << e.what()  << std::endl ;
    er << "---------------------------------" << std::endl ;

    return er.str();
  }

  //std::list<keyType> *deviceList = fecDevice->scanRingForI2CDevice ( true ) ;
  std::list<keyType> *deviceList = fecDevice->scanRingForI2CDevice ( true, true );//add printout d.k.
  o << std::endl;
  if (deviceList != NULL) {

    for (std::list<keyType>::iterator p=deviceList->begin();p!=deviceList->end();p++) {
      
      keyType index = *p ;
   
      char msg[100] ;
      decodeKey (msg, index) ;

      o << "Device " << msg << " found" << std::endl ;
    }
  }
  else {
  
    o << "No device found on the CCU and channels" << std::endl ;
  }

  delete deviceList ;

  return o.str();
}


/**
 * <p>Command: no command
 * read the SR0 of the FEC
 * \param fecAddress - FEC address
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */
void readFecRegisterSR0 (FecAccess *fecAccess,
			 tscType8 fecAddress,
			 tscType8 ringAddress, 
                         long loop, unsigned long tms ) {

  std::list<keyType> *fecSlotList = NULL ;

  try {
    // Scan the PCI or VME slot for device driver loaded
    fecSlotList = fecAccess->getFecList( ) ;
  }
  catch (FecExceptionHandler e) {
    
    std::cout << "------------ Exception ----------" << std::endl ;
    std::cout << e.what()  << std::endl ;
    std::cout << "---------------------------------" << std::endl ;
    
    return;
  }

  if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
    for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
            
      keyType indexFEC = *p ;
      std::cout << "####################################################" << std::endl ;
      std::cout << "FEC " << std::dec << (int)getFecKey(indexFEC) << " Ring " << (int)getRingKey(indexFEC) << std::endl ;
      if (fecAccess->getFecBusType() == FECVME) {
	FecVmeRingDevice *fecVmeRing = (FecVmeRingDevice *)fecAccess->getFecRingDevice (indexFEC) ;
	try {
	  std::cout << "FEC hardware ID = " << fecVmeRing->getFecHardwareId() << std::endl ;
	  std::cout << "VME Firmware version = " << fecVmeRing->getVmeVersion() << std::endl ;
	}
	catch (FecExceptionHandler e) {
	  std::cerr << "Unable to read the firmware version from the VME FEC" << std::endl ;
	  std::cerr << e.what() << std::endl ;
	}
      }
      std::cout << "FEC firmware version = " << std::hex << fecAccess->getFecFirmwareVersion(indexFEC) << std::endl ;

      for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {
	
	if (loop != 1)
	  std::cout << "----------------------- Loop " << loopI+1 << std::endl ;
	
	try {

	  //std::cout << std::hex << std::endl ; 
	  fecAccess->getFecRingSR0(indexFEC) ;
	  tscType32 fecSR0 = fecAccess->getFecRingSR0(indexFEC) ;

	  std::cout << "Value of the Status Register 0 of the fec : 0x" << std::hex << fecSR0 << std::endl ;
	  if (fecSR0 & 0x1)
	    std::cout << "\tFIFO transmit running" << std::endl ;
	  if (fecSR0 & 0x2)
	    std::cout << "\tFIFO receive running" << std::endl ;
	  if (fecSR0 & 0x4)
	    std::cout << "\tFIFO receive half full" << std::endl ;
	  if (fecSR0 & 0x8)
	    std::cout << "\tFIFO receive full" << std::endl ;
	  if (fecSR0 & 0x10)
	    std::cout << "\tFIFO receive empty" << std::endl ;
	  if (fecSR0 & 0x20)
	    std::cout << "\tFIFO return half full" << std::endl ;
	  if (fecSR0 & 0x40)
	    std::cout << "\tFIFO return full" << std::endl ;
	  if (fecSR0 & 0x80)
	    std::cout << "\tFIFO return empty" << std::endl ;
	  if (fecSR0 & 0x100)
	    std::cout << "\tFIFO transmit half full" << std::endl ;
	  if (fecSR0 & 0x200)
	    std::cout << "\tFIFO transmit full" << std::endl ;
	  if (fecSR0 & 0x400)
	    std::cout << "\tFIFO transmit empty" << std::endl ;
	  if (fecSR0 & 0x800)
	    std::cout << "\tLink initialise" << std::endl ;
	  if (fecSR0 & 0x1000) 
	    std::cout << "\tPending irg" << std::endl ;
	  if (fecSR0 & 0x2000)
	    std::cout << "\tData to FEC" << std::endl ;
	  if (fecSR0 & 0x4000)
	    std::cout << "\tTTCRx ok" << std::endl ;
	}
	catch (FecExceptionHandler e) {
	  
	  std::cout << "------------ Exception ----------" << std::endl ;
	  std::cout << e.what()  << std::endl ;
	  std::cout << "---------------------------------" << std::endl ;
	}

	// Wait
	if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
      }
    }
  }
  else {
    std::cout << "Did not find any FEC" << std::endl ;
  }

  delete fecSlotList ;
}



/** 
 * This method scan the ring for device Tracker APV (6), MUX, PLL, DCU, LASERDRIVER, DOH.
 * <p>command: -scantrackerdevice (in place of -scanringdevice)
 * <p>command: -calibrateI2CSpeed test the i2c speed for the devices
 * \param fecAddress - FEC address
 * \warning the test is read and try to read back the force acknowledge for the base address
 * in the rigth mode. Not all the registers are accessed.
 * \warning the loop is not used
 */
// Very tracker dependent. In many places the i2c device address is hardwired.
// Needs cleaning. d.k. 3/07 
std::string testScanPixelDevice ( FecAccess *fecAccess, 
				  tscType8 fecAddress,
				  tscType8 ringAddress,
				  long loop, unsigned long tms) { 

  std::ostringstream o;
  std::ostringstream er;

  // Make a PIA reset to detect all the devices
  //std::cout << "Make a PIA reset on all FEC and all CCUs" << std::endl ;
  allCCUsPiaReset (fecAccess, fecAddress, ringAddress) ;

  FecRingDevice *fecDevice = NULL ;
  
  // Find the FECs
  try {
    // Scan the PCI slot for device driver loaded
    fecDevice = fecAccess->getFecRingDevice (buildFecRingKey(fecAddress,ringAddress)) ;
  }
  catch (FecExceptionHandler e) {  
  
    er << std::endl << "*********** ERROR ********************************" << std::endl ; 
    er << "Cannot find the FEC " << std::dec << (int)fecAddress << " ring " << (int)ringAddress << ": " << e.what() << std::endl ; 
    er << "**************************************************" << std::endl ;
    
    return er.str() ;
  }

  // Modify for BPix - IT DOES NOT SEEM TO BE USED LATER?
  int sizeValues = 11 ;
  keyType
    deviceValues[10][2] = { 
      {poh1deviceAddress, NORMALMODE}, // POH-1
      {poh2deviceAddress, NORMALMODE}, // POH-2
      {poh3deviceAddress, NORMALMODE}, // POH-3
      {poh4deviceAddress, NORMALMODE}, // POH-4
      {poh5deviceAddress, NORMALMODE}, // POH-5
      {poh6deviceAddress, NORMALMODE}, // POH-6
      {poh7deviceAddress, NORMALMODE}, // POH-7
      {dohdeviceAddress, NORMALMODE}, // DOH
      {plldeviceAddress, NORMALMODE}, // PLL
      {delay25deviceAddress, NORMALMODE}  // DELAY25  
    } ;


  pllAccessedType pllSet ;
  laserdriverAccessedType laserdriverSet ;
  dohAccessedType dohSet ;
  delay25AccessedType delay25Set ;

  //o << std::endl << "Scan ring for CCU and try to find devices" << std::endl ;

  // Test error on the scan ring
  try {

    for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

      if (loop != 1)
	o << "----------------------- Loop " << loopI+1 << std::endl ;

      
      // false => use broadcast
      // true  => display
      std::list<keyType> *deviceList = fecDevice->scanRingForI2CDevice ( (keyType *)deviceValues, sizeValues, false, false ) ;

      Sgi::hash_map<keyType, bool *> trackerModule ;

      if (deviceList != NULL) {
	bool *module ;

	for (std::list<keyType>::iterator p=deviceList->begin();p!=deviceList->end();p++) {
      
	  keyType index = *p ;
   
	  char msg[100] ;
	  decodeKey (msg, index) ;

	  o << "Device " << msg << " found" << std::endl;

	  keyType indexChannel = buildCompleteKey(getFecKey(index),getRingKey(index),getCcuKey(index),getChannelKey(index),0) ;
	  if (trackerModule[indexChannel] == NULL) {
	    trackerModule[indexChannel] = new bool[13] ;
	    module = trackerModule[indexChannel] ;
	    for (int i = 0 ; i < 13 ; i ++) module[i] = false ;
	  }
	  else module = trackerModule[indexChannel] ;

	  switch (getAddressKey(index)) {
	  case poh1deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	     
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[0] = true ;
	    
	    //o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh2deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	    
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[1] = true ;
	   
	    //o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh3deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[2] = true; 
	  
	    //o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh4deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	    
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[3] = true ;
	    //o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh5deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[4] = true; 
	    // o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh6deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	   
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[5] = true ;
	 
	    // o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case poh7deviceAddress:
	    try {
	      laserdriverSet[index] = new laserdriverAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	    
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[6] = true ;
	   
	    // o << "AOH     " << msg << " found" << std::endl ;
	    break ;
	  case plldeviceAddress:
	    try {
	      pllSet[index] = new pllAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	    
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[7] = true ;
	  
	    //o << "PLL     " << msg << " found" << std::endl ;
	    break ;
	  case dohdeviceAddress:
	    try {
	      dohSet[index] = new DohAccess (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	     
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[8] = true ;
	  
	    // o << "DOH     " << msg << " found" << std::endl ;
	    break ;
	  case delay25deviceAddress:
	    try {
	      delay25Set[index] = new delay25Access (fecAccess, index) ;
	    }
	    catch (FecExceptionHandler e) {
	  
	      o << "Cannot create the access to a device" << std::endl ;
	      o << e.what() << std::endl ;
	    }
	    module[9]= true ;
	  
	    //o << "DELAY25 " << msg << " found" << std::endl ;
	    break ;
	  }
	}

	tscType16 currentCcuAddress = 0;
	tscType16 channelccu = 0x10;
	tscType16 channel1 = 0x11;
	tscType16 channel3 = 0x13;

	for (Sgi::hash_map<keyType, bool *>::iterator p=trackerModule.begin();p!=trackerModule.end();p++) {
	  
	  keyType indexF = p->first ;
	  module = p->second ;
	  if (module != NULL) {
	    
	    //bool aohF  = module[0] || module[1] || module[2] || module[3] ||  module[4] || module[5] || module[6] || module[7];
	    bool pllF  = module[7] ;
	    bool dohF  = module[8] ;
	   	  
	    bool delay25F = module[9] ;
	    
	    if(currentCcuAddress == 0) {
	      currentCcuAddress = getCcuKey(indexF);
	      //o << "<CCU = " << IntToString(currentCcuAddress) << ">" << endl;
	    }
	    
	    if(currentCcuAddress != getCcuKey(indexF)) {
	      //o << "</CCU>" << endl;
	      currentCcuAddress = getCcuKey(indexF);
	      //o << "<CCU = " << IntToString(currentCcuAddress) << ">" << endl;
	    }
	    
	    //Layer 1&2
	    if ( getChannelKey(indexF) == channel1 ) {
	      if (module[0] && module[1] && module[2] && module[3] && module[4] && module[5] && module[6] && module[7] && module[8]) {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "all devices found" << endl;
		continue;
	      }
	      else {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "ERROR: not all devices found" << endl;
	      }
	    }
	    //Layer 3
	    else if ( getChannelKey(indexF) == channel3 ) {
	      if (module[0] && module[1] && module[2] && module[3] && module[4] && module[5] && module[6] && module[7] && module[8]) {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "all devices found" << endl;
		continue;
	      }
	      else {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "ERROR: not all devices found" << endl;
	      }
	    }
	    //Tracker DOH
	    else if ( getChannelKey(indexF) == channelccu ) {
	      if (module[8]) {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "CCU DOH found" << endl;
		continue;
	      }
	      else {
		o << std::endl << "FEC " << std::dec << (int)getFecKey(indexF) << " Ring " << (int)getRingKey(indexF) << " CCU 0x" << std::hex << (int)getCcuKey(indexF) << " channel 0x" << IntToString(getChannelKey(indexF)) << std::endl ;
		o << "ERROR: CCU DOH not found" << endl;
	      }
	    }

	    // Check each part
	    if (!module[0]) {
	      o << "\t" << "Miss a POH-1 at address 0x" << std::hex << poh1deviceAddress << std::endl ;
	    }
	    if (!module[1]) {
	      o << "\t" << "Miss a POH-2 at address 0x" << std::hex << poh2deviceAddress << std::endl ;
	    }
	    if (!module[2]) {
	      o << "\t" << "Miss a POH-3 at address 0x" << std::hex << poh3deviceAddress << std::endl ;
	    }
	    if (!module[3]) {
	      o << "\t" << "Miss a POH-4 at address 0x" << std::hex << poh4deviceAddress << std::endl ;
	    }
	    if (!module[4]) {
	      o << "\t" << "Miss a POH-5 at address 0x" << std::hex << poh5deviceAddress << std::endl ;
	    }
	    if (!module[5]) {
	      o << "\t" << "Miss a POH-6 at address 0x" << std::hex << poh6deviceAddress << std::endl ;
	    }
	    if (!module[6]) {
	      o << "\t" << "Miss a POH-7 at address 0x" << std::hex << poh7deviceAddress << std::endl ;
	    }
	    if (!pllF) {
	      o << "\t" << "Miss a  PLL at address 0x" << std::hex << plldeviceAddress << std::endl ;
	    }
	    if (!delay25F) {
	      o << "\t" << "Miss a DELAY25 at address 0x" << std::hex << delay25deviceAddress << std::endl ;
	    }
	    if (!dohF) {
	      std::cout << "\t" << "Miss a DOH at address 0x" << std::hex << dohdeviceAddress << std::endl ;
	    } 
	  }
	}

	// delete hash_map
	for (pllAccessedType::iterator p=pllSet.begin();p!=pllSet.end();p++) {
	  try { delete p->second ; }
	  catch (FecExceptionHandler e) {	}
	}
	pllSet.clear() ;
	for (delay25AccessedType::iterator p=delay25Set.begin();p!=delay25Set.end();p++) {
	  try { delete p->second ; }
	  catch (FecExceptionHandler e) {	}
	}
	delay25Set.clear() ;
	for (laserdriverAccessedType::iterator p=laserdriverSet.begin();p!=laserdriverSet.end();p++) {
	  try { delete p->second ; }
	  catch (FecExceptionHandler e) {	}
	}
	laserdriverSet.clear() ;
	for (dohAccessedType::iterator p=dohSet.begin();p!=dohSet.end();p++) {
	  try { delete p->second ; }
	  catch (FecExceptionHandler e) {	}
	}
	dohSet.clear() ;
	for (Sgi::hash_map<keyType, bool *>::iterator p=trackerModule.begin();p!=trackerModule.end();p++) {
	  delete[] p->second ;
	}
	trackerModule.clear() ;
      }
      else {

	o << "No device found on the CCU and channels" << std::endl ;
      }

      // Wait
      if ( (loop != 1) && (tms > 0) ) usleep (tms) ;

      delete deviceList ;
    }
  }
  catch (FecExceptionHandler e) {

    o << std::endl << "------------ Exception ----------" << std::endl ;
    o << "Cannot scan the ring for i2c devices" << std::endl ;
    o << e.what()  << std::endl ;
    o << "---------------------------------" << std::endl ;
  }
  //o << "</CCU>" << endl;

  // Switch all the APVs
  //std::cout << "Make a PIA reset on all FEC and all CCUs" << std::endl ;
  allCCUsPiaReset (fecAccess, fecAddress, ringAddress) ;

  return o.str() ;
}


/** <p> no command:
 * make a pia reset on all CCUs that can be found and make a PIA reset on all PIA channels
 * \param fecAccess - FEC Access object
 */
void allCCUsPiaReset ( FecAccess *fecAccess, tscType8 fecAddress, tscType8 ringAddress ) {

  //std::cout << "Making a PIA reset for the FEC " << std::dec << (int)fecAddress << " ring " << (int)ringAddress << " on all CCUs on all PIA channels" << std::endl ;
  
  // Scan the PCI or VME slot for device driver loaded
  //std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
  //if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
  //for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
            
  keyType indexFEC = buildFecRingKey(fecAddress,ringAddress) ;
  //std::cout << "-------------------------------------------------" << std::endl ;
  //std::cout << "Scan the ring to find CCUs for FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
      
  try {
    // Scan the ring for each FECs and display the CCUs
    std::list<keyType> *ccuList = fecAccess->getCcuList ( indexFEC ) ;
    
    if (ccuList != NULL) {
      
      for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	
	keyType index = *p ;
	
	// For each channel
	for (tscType8 channel = 0x30 ; channel <= 0x30 ; channel ++) {
	  
	  try {
	    keyType indexPIA = buildCompleteKey(getFecKey(index),getRingKey(index),getCcuKey(index),channel,0) ;
		
	    char msg[80] ;
	    decodeKey(msg, indexPIA) ;
	    //std::cout << "Making a PIA reset on channel " << msg << std::endl ;
	    
	    PiaResetAccess piaResetAccess(fecAccess, indexPIA, 0xFF) ;
	    piaResetDescription piaD ( index, (unsigned long)1000, (unsigned long)100, (tscType8)0xFF ) ;
	    piaResetAccess.setValues (piaD) ;
	    //piaResetDescription *piaD1 = piaResetAccess.getValues ( ) ;
	    
	    //if (piaD == *piaD1)
	    //std::cout << "Pia reset comparison ok" << std::endl ;
	    //else
	    //std::cout << "Pia reset comparison error" << std::endl ;
	  }
	  catch (FecExceptionHandler e) {

	    std::cout << "------------ Exception ----------" << std::endl ;
	    std::cout << e.what()  << std::endl ;
	    std::cout << "---------------------------------" << std::endl ;

	    // display all registers
	    if (e.getFecRingRegisters() != NULL ) FecRingRegisters::displayAllRegisters ( *(e.getFecRingRegisters()) ) ;
	    // Original frame
	    if (e.getRequestFrame() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getRequestFrame()) << std::endl ;	    
	    if (e.getDAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getDAck()) << std::endl ;
	    if (e.getFAck() != NULL) std::cerr << FecRingRegisters::decodeFrame (e.getFAck()) << std::endl ;
	  }
	}
      }
    }
    else {
      std::cerr << "Did not found CCU on the FEC " << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
    }
    
    delete ccuList ;
  }
  catch (FecExceptionHandler e) {
    
    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;
  }
  
  
}


/**
 * <p>command: -status
 * \param fecAccess - FEC Access object
 * \param noBroadcast - CCU broadcast mode used (false, default) or not (true) [OPTION parameter]
 * \warning the loop is not used
 */
string testStatus ( FecAccess *fecAccess,
		    bool noBroadcast ) {

  std::ostringstream o;
   
  std::list<keyType> *ccuList = NULL ;
  try {

    // Message
    
    o << std::endl << "-------------------------------------------------" << std::endl ;
    o << "Scan the device driver FECs" << std::endl ;

    for (tscType8 fecAddress = minFecSlot_ ; fecAddress <= maxFecSlot_  ; fecAddress ++) {

      for (tscType8 ringAddress = minFecRing_ ; ringAddress <= maxFecRing_ ; ringAddress++) {
	
	// Find the FECs
	try {
	  // Scan the PCI slot for device driver loaded
	  fecAccess->getFecRingDevice (buildFecRingKey(fecAddress,ringAddress)) ;

	  keyType index = buildFecRingKey(fecAddress,ringAddress) ;        
	  tscType32 fecSR0 = fecAccess->getFecRingSR0 (index) ;
	  

	  o << "Found a FEC device driver on slot " << std::dec << (int)fecAddress << "." << (int)ringAddress << std::endl ;
	  o << "\tFEC status register  0: 0x" << std::hex << fecSR0 << std::endl ;
	  if (fecSR0 & 0x1) {
	    o << "\t\tFIFO transmit running" << std::endl ;
	  
	  }
	  if (fecSR0 & 0x2) {
	    
	    o << "\t\tFIFO receive running" << std::endl ;
	  }
	  if (fecSR0 & 0x4) {
	   
	    o << "\t\tFIFO receive half full" << std::endl ;
	  }
	  if (fecSR0 & 0x8) {
	   
	    o << "\t\tFIFO receive full" << std::endl ;
	  }
	  if (fecSR0 & 0x10) {
	   
	    o << "\t\tFIFO receive empty" << std::endl ;
	  }
	  if (fecSR0 & 0x20) {
	   
	    o << "\t\tFIFO return half full" << std::endl ;
	  }
	  if (fecSR0 & 0x40) {
	  
	    o << "\t\tFIFO return full" << std::endl ;
	  }
	  if (fecSR0 & 0x80) {
	    
	    o << "\t\tFIFO return empty" << std::endl ;
	  }
	  if (fecSR0 & 0x100) {
	   
	    o << "\t\tFIFO transmit half full" << std::endl ;
	  }
	  if (fecSR0 & 0x200) {
	   
	    o << "\t\tFIFO transmit full" << std::endl ;
	  }
	  if (fecSR0 & 0x400) {
	   
	    o << "\t\tFIFO transmit empty" << std::endl ;
	  }
	  if (fecSR0 & 0x800) {
	  
	    o << "\t\tLink initialise" << std::endl ;
	  }
	  if (fecSR0 & 0x1000) { 
	  
	    o << "\t\tPending irg" << std::endl ;
	  }
	  if (fecSR0 & 0x2000) {
	  
	    o << "\t\tData to FEC" << std::endl ;
	  }
	  if (fecSR0 & 0x4000) {
	   
	    o << "\t\tTTCRx ok" << std::endl ;
	  }
	
	  tscType32 fecSR1 = fecAccess->getFecRingSR1 (index) ;
	 
	  o << "\tFEC status register  1: 0x" << std::hex << fecSR1 << std::endl ;
	  if (fecSR1 & 0x1) {
	  
	    o << "\t\tIllegal data" << std::endl ;
	  }
	  if (fecSR1 & 0x2) {
	   
	    o << "\t\tIllegal sequence" << std::endl ;
	  }
	  if (fecSR1 & 0x4) { 
	   
	    o << "\t\tCRC error" << std::endl ;
	  }
	  if (fecSR1 & 0x8) { 
	   
	    o << "\t\tData copied" << std::endl ;
	  }
	  if (fecSR1 & 0x10) { 
	 
	    o << "\t\tAddress seen" << std::endl ;
	  }
	  if (fecSR1 & 0x20) { 
	  
	    o << "\t\tError" << std::endl ;
	  }
	
	  tscType32 fecCR0 = fecAccess->getFecRingCR0 (index) ;
	 
	  o << "\tFEC control register 0: 0x" << std::hex << fecCR0 << std::endl ;
	  if (fecCR0 & 0x1) {
	   
	    o << "\t\tFEC enable" << std::endl ;
	  }
	  else {
	   
	    o << "\t\tFEC disable" << std::endl ;
	  }
	  //if (fecCR0 & 0x2)
	  //  std::cout << "\t\tSend" << std::endl ;
	  //if (fecCR0 & 0x4)
	  //  std::cout << "\t\tResend" << std::endl ;
	  if (fecCR0 & 0x8) {
	   
	    o << "\t\tOutput ring B" << std::endl ;
	  }
	  else {
	   
	    o << "\t\tOutput ring A" << std::endl ;
	  }
	  if (fecCR0 & 0x10) {
	   
	    o << "\t\tInput ring B" << std::endl ;
	  }
	  else {
	  
	    o << "\t\tInput ring A" << std::endl ;
	  }
	
	  o << "\t\tSel irq mode = " << std::dec << (fecCR0 & 0x20) << std::endl ;
	  //std::cout << "\t\tReset TTCRx = " << std::dec << (fecCR0 & 0x20) << std::endl ;
	  
	  tscType32 fecCR1 = fecAccess->getFecRingCR1 (index) ;
	  
	  o << "\tFEC control register 1: " << std::hex << fecCR1 << std::endl ;
	
	  // Start the scanning of the ring for each FEC device
	  //cout << "the following two lines may have to be disabled in case of crashes " << endl;
	  FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;
	  fecDevice->fecScanRingBroadcast ( ) ;

	}
	catch (FecExceptionHandler e) {

	}
      }
    }

    // Scan the PCI or VME slot for device driver loaded
    std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
    if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
      for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
        
        keyType indexFEC = *p ;        
  
	o << "-------------------------------------------------" << std::endl ;
	o << "Scan the ring to find CCUs for FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	
	try {
	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFEC, noBroadcast ) ;

	  if (ccuList != NULL) {
	    
	    for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	      
	      keyType index = *p ;
	    	      
	      o << std::dec << "FEC 0x" << (int)getFecKey(index) 
		<<std::hex << " Ring 0x " << (int)getRingKey(index) 
		<< " CCU 0x" << (int)getCcuKey(index)
		<< " found" << std::dec << std::endl ;
	    
	      // Get CCU registers
	      tscType32 SRA = fecAccess->getCcuSRA(index) ;
	      tscType32 SRB = fecAccess->getCcuSRB(index) ;
	      tscType32 SRC = fecAccess->getCcuSRC(index) ;
	      tscType32 SRD = fecAccess->getCcuSRD(index) ;
	      tscType32 SRE = fecAccess->getCcuSRE(index) ;
	      tscType32 SRF = fecAccess->getCcuSRF(index) ;
	      tscType32 CRA = fecAccess->getCcuCRA(index) ;
	      tscType32 CRB = fecAccess->getCcuCRB(index) ;
	      tscType32 CRC = fecAccess->getCcuCRC(index) ;
	      tscType32 CRD = fecAccess->getCcuCRD(index) ;
	      tscType32 CRE = fecAccess->getCcuCRE(index) ;
	      //tscType32 CRF = fecAccess->getCcuCRF(index) ;
	      
	     

	      o << "\tCCU status register  A: 0x" << std::hex << SRA << std::endl ; displayCCUSRA(SRA) ;
	      o << "\tCCU status register  B: 0x" << std::hex << SRB << std::endl ; displayCCUSRB(SRB) ;
	      o << "\tCCU status register  C: 0x" << std::hex << SRC << std::endl ; displayCCUSRC(SRC) ;
	      o << "\tCCU status register  D: 0x" << std::hex << SRD << std::endl ; displayCCUSRD(SRD) ;
	      o << "\tCCU status register  E: 0x" << std::hex << SRE << std::endl ; displayCCUSRE(SRE) ;
	      o << "\tCCU status register  F: 0x" << std::hex << SRF << std::endl ; displayCCUSRF(SRF) ;
	      o << "\tCCU control register A: 0x" << std::hex << CRA << std::endl ; displayCCUCRA(CRA) ;
	      o << "\tCCU control register B: 0x" << std::hex << CRB << std::endl ; displayCCUCRB(CRB) ;
	      o << "\tCCU control register C: 0x" << std::hex << CRC << std::endl ; displayCCUCRC(CRC) ;
	      o << "\tCCU control register D: 0x" << std::hex << CRD << std::endl ; displayCCUCRD(CRD) ;
	      o << "\tCCU control register E: 0x" << std::hex << CRE << std::endl ; displayCCUCRE(CRE) ;
	      //o << "\tCCU control register F: 0x" << std::hex << CRF << std::endl ; 
	     
	    }
	  }
	  else {
	    
	    o << "No CCU on FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	  }
	}
	catch (FecExceptionHandler e) {
	 	 
	  o << "No CCU on FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	}
      }
    }
    else {
    
      o << "No FEC found" << std::endl ;
    }  
  }
  catch (FecExceptionHandler e) {
  
    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // Delete = already store in the table of FEC device class
  delete ccuList ;
  return o.str();
}


/**
 * <p>command: -status
 * \param fecAccess - FEC Access object
 * \param noBroadcast - CCU broadcast mode used (false, default) or not (true) [OPTION parameter]
 */
string testMap ( FecAccess *fecAccess,
		 bool noBroadcast ) {

  std::ostringstream o;
   
  std::list<keyType> *ccuList = NULL ;
  try {

    // Message
    for (tscType8 fecAddress = minFecSlot_ ; fecAddress <= maxFecSlot_  ; fecAddress ++) {

      for (tscType8 ringAddress = minFecRing_ ; ringAddress <= maxFecRing_ ; ringAddress++) {
	
	// Find the FECs
	try {
	  // Scan the PCI slot for device driver loaded
	  fecAccess->getFecRingDevice (buildFecRingKey(fecAddress,ringAddress)) ;

	  keyType index = buildFecRingKey(fecAddress,ringAddress) ;        
	  //tscType32 fecSR0 = fecAccess->getFecRingSR0 (index) ;
	  

	  o << "Found a FEC device driver on slot " << std::dec << (int)fecAddress << "." << (int)ringAddress << std::endl ;
	 
	  // Start the scanning of the ring for each FEC device
	  FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;

	  fecDevice->fecScanRingBroadcast ( ) ;
	}
	catch (FecExceptionHandler e) {

	}
      }
    }

    // Scan the PCI or VME slot for device driver loaded
    /*std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
    if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
      for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
        
        keyType indexFEC = *p ;        
  
	o << "-------------------------------------------------" << std::endl ;
	o << "Scan the ring to find CCUs for FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	
	try {
	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFEC, noBroadcast ) ;

	  if (ccuList != NULL) {
	    
	    for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	      
	      keyType index = *p ;
	    	      
	      o << std::hex << "FEC 0x" << getFecKey(index) 
			<< " Ring 0x " << getRingKey(index) 
			<< " CCU 0x" << getCcuKey(index)
			<< " found" << std::endl ;
	    }
	  }
	  else {
	    
	    o << "No CCU on FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	  }
	}
	catch (FecExceptionHandler e) {
	  o << "No CCU on FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	}
      }
    }
    else {
    
      o << "No FEC found" << std::endl ;
    }*/  
  }
  catch (FecExceptionHandler e) {
  
    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // Delete = already store in the table of FEC device class
  delete ccuList ;
  return o.str();
}


/**
 * <p>command: -status
 * \param fecAccess - FEC Access object
 * \param noBroadcast - CCU broadcast mode used (false, default) or not (true) [OPTION parameter]
 */
string testMapCCU ( FecAccess *fecAccess,
		    bool noBroadcast ) {

  std::ostringstream o;
   
  std::list<keyType> *ccuList = NULL ;
  try {

    // Message
    /*for (tscType8 fecAddress = minFecSlot_ ; fecAddress <= maxFecSlot_  ; fecAddress ++) {

      for (tscType8 ringAddress = minFecRing_ ; ringAddress <= maxFecRing_ ; ringAddress++) {
	
	// Find the FECs
	try {
	  // Scan the PCI slot for device driver loaded
	  fecAccess->getFecRingDevice (buildFecRingKey(fecAddress,ringAddress)) ;

	  keyType index = buildFecRingKey(fecAddress,ringAddress) ;        
	  //tscType32 fecSR0 = fecAccess->getFecRingSR0 (index) ;

	  // Start the scanning of the ring for each FEC device
	  FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;

	  fecDevice->fecScanRingBroadcast ( ) ;
	}
	catch (FecExceptionHandler e) {

	}
      }
      }*/

    // Scan the PCI or VME slot for device driver loaded
    std::list<keyType> *fecSlotList = fecAccess->getFecList( ) ;
    
    if ((fecSlotList != NULL) && (!fecSlotList->empty())) {
      for (std::list<keyType>::iterator p=fecSlotList->begin() ; p!=fecSlotList->end() ; p++) {
        
        keyType indexFEC = *p ;        
  
	o << "-------------------------------------------------" << std::endl ;
	//o << "Scan the ring to find CCUs for FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << std::endl ;
	
	try {
	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFEC, noBroadcast ) ;

	  if (ccuList != NULL) {
	    
	    for (std::list<keyType>::iterator p=ccuList->begin() ; p!=ccuList->end() ; p++) {
	      
	      keyType index = *p ;
	    	      
	      o << std::dec << "FEC " << (int)getFecKey(index) 
		<< std::dec	<< " ring 0x" << (int)getRingKey(index) 
		<< std::hex << " : CCU 0x" << (int)getCcuKey(index)
			<< " found" << std::endl ;
	    }
	  }
	  else {
	    
	    o << "FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << " : no CCU found " << std::endl ;
	  }
	}
	catch (FecExceptionHandler e) {
	  o << "FEC " << std::dec << (int)getFecKey(indexFEC) << " ring " << (int)getRingKey(indexFEC) << " : no CCU found " << std::endl ;
	}
      }
    }
    else {
    
      o << "No FEC found" << std::endl ;
    }  
  }
  catch (FecExceptionHandler e) {
  
    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }

  // Delete = already store in the table of FEC device class
  delete ccuList ;
  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register A
 */
string displayCCUSRA ( tscType8 SRA ) {

  std::ostringstream o;

  o << "\t\tCCU status register A = 0x" << std::hex << (int)SRA << std::endl ;
  if (SRA & 0x1)
    o << "\t\t(PED)     Error bit: CRC error" << std::endl ;
  if (SRA & 0x2) 
    o << "\t\t(IE)      Internal error: node controller state machine error" << std::endl ;
  if (SRA & 0x4)
    o << "\t\t(ALSET)   One or more ALARM inputs are currently active" << std::endl ;
  if (SRA & 0x8)
    o << "\t\t(CCUPERR) Parity error in the internal CCU registers" << std::endl ;
  if (SRA & 0x10)
    o << "\t\t(CHAPERR) Parity error in any of the CCU channels" << std::endl ;
  if (SRA & 0x20)
    o << "\t\t(ILLSEQ)  Illegal sequence" << std::endl ;
  if (SRA & 0x40)
    o << "\t\t(INVMCD)  Invalid command" << std::endl ;
  if (SRA & 0x80)
    o << "\t\t(GE)      Global error" << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register B
 */
string displayCCUSRB ( tscType8 SRB ) {

  std::ostringstream o;
  
  o << "\t\tLast correctly received transaction number (SRB) " << (int)SRB << "(0x" << std::hex << (int)SRB << ")" << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register C
 */
string displayCCUSRC ( tscType8 SRC ) { 
  
  std::ostringstream o;
  
  o << "\t\tCCU status register C = 0x" << std::hex << (int)SRC << std::endl ;
  o << "\t\tInput " << (SRC & 0x1 ? 'B' : 'A') << " Ouput " << (SRC & 0x2 ? 'B' : 'A') << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register D
 */
string displayCCUSRD ( tscType8 SRD ) { 
  
  std::ostringstream o;

  o << "\t\tSource field for the last ring message addressed to this CCU: 0x" << std::hex << (int)SRD << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register E
 */
string displayCCUSRE ( tscType32 SRE ) {
  
  std::ostringstream o;

  o << "\t\tCCU status register E = 0x" << std::hex << (unsigned int)SRE << std::endl ;
  ushort channelBusy[18] = {0} ;

  for (ushort i = 0 ; i < 18 ; i ++) {
    
    if (SRE & (0x1 << i)) channelBusy[i] = 1 ;
  }

  o << "\t\tList of busy channels:\n\t\t- i2c: " << std::endl ;
  for (ushort i = 0 ; i < 16 ; i ++) 
    if (channelBusy[i]) o << i+1 << std::endl ;
  o << std::endl ;
  if (channelBusy[16] == 1) o << "\t\t- Trigger channel busy" << std::endl ;
  if (channelBusy[17] == 1) o << "\t\t- Memory channel busy" << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU status register F
 */
string displayCCUSRF ( tscType16 SRF ) {
  
  std::ostringstream o;
  
  o << "\t\tNumber of parity error since the last reset: 0x" << std::hex << SRF << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU control register A
 */
string displayCCUCRA ( tscType8 CRA ) {
  
  std::ostringstream o;

  o << "\t\tCCU control register A = 0x" << std::hex << CRA << std::endl ;
  if (CRA & 0x20)
    o << "\t\t(EXTRES) Generate external reset" << std::endl ;
  if (CRA & 0x40)
    o << "\t\t(CLRE)   Clear error" << std::endl ;
  if (CRA & 0x80)
    o << "\t\t(RES)    Reset all channels" << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU control register B
 */
string displayCCUCRB ( tscType8 CRB ) {
  
  std::ostringstream o;
  
  o << "\t\tCCU control register B = 0x" << std::hex << (int)CRB << std::endl ;
  if (CRB & 0x1)
    o << "\t\t(ENAL1) Enable ALARM1 interrupt" << std::endl ;
  if (CRB & 0x2) 
    o << "\t\t(ENAL2) Enable ALARM2 interrupt" << std::endl ;
  if (CRB & 0x4)
    o << "\t\t(ENAL3) Enable ALARM3 interrupt" << std::endl ;
  if (CRB & 0x8)
    o << "\t\t(ENAL4) Enable ALARM4 interrupt" << std::endl ;
  
  o << "\t\t(RTRY) Retry count: " << (((CRB & 0x3) == 0x3) ? 4 : CRB & 0x3) << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU control register C
 */
string displayCCUCRC ( tscType8 CRC ) {
  
  std::ostringstream o;
  
  o << "\t\tCCU control register C = 0x" << std::hex << (int)CRC << std::endl ;
  o << "\t\tInput " << (CRC & 0x1 ? 'B' : 'A') << " Ouput " << (CRC & 0x2 ? 'B' : 'A') << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU control register D
 */
string displayCCUCRD ( tscType8 CRD ) {
  
  std::ostringstream o;

  o << "\t\tCCU control register D = 0x" << std::hex << (int)CRD << std::endl ;
  o << "\t\tBroadcast class: 0x" << std::hex << (int)(CRD & 0x7F) << std::endl ;

  return o.str();
}

/**
 * <p>command: no directly accessisble
 * \param CCU control register E
 */
string displayCCUCRE ( tscType32 CRE ) {
  
  std::ostringstream o;

  o << "\t\tCCU control register E = 0x" << std::hex << (unsigned int)CRE << std::endl ;
  ushort channelEnable[23] = {0}, i = 0 ;
  bool i2cDisplay = false, piaDisplay = false ;
  
  for (i = 0 ; i < 23 ; i ++) {
    channelEnable[i] = CRE & (0x1 << i) ;
    if (channelEnable[i]) {
      if (i < 16) i2cDisplay = true ;
      else
	if (i < 20) piaDisplay = true ;
    }
  }
  
  o << "\t\tList of enable channels:" << std::endl ;
  
  o << "\t\t- i2c: " << std::endl ;
  if (! i2cDisplay) o << "None" << std::endl ;
  for (ushort i = 0 ; i < 16 ; i ++) 
    if (channelEnable[i]) o << i+1 ;

  o << "\n\t\t- PIA: " << std::endl ;
  if (! piaDisplay) o << "None" << std::endl ;
  
  for (; i < 20 ; i ++) 
    if (channelEnable[i]) o << i+1 ;
  o << std::endl ;

  if (channelEnable[20]) o << "\t\t- Memory channel enable" << std::endl ;
  else o << "\t\t- Memory channel not enabled" << std::endl ;
  if (channelEnable[21]) o << "\t\t- Trigger channel enable" << std::endl ;
  else o << "\t\t- Trigger channel not enabled" << std::endl ;
  if (channelEnable[22]) o << "\t\t- JTAG channel enable" << std::endl ;
  else o << "\t\t- JTAG channel not enabled" << std::endl ;

  return o.str();
}


/** 
 * <p>Command: -piaReset
 * \param fecAccess - FEC Access object
 * \param  fecAddress - slot of the FEC.
 * \param  ringAddress - slot of the ring.
 * \param  ccuAddress - address of the first CCU of the ring.
 * \param  channelAddress - address of the channel.
 * \param delayActiveReset - delay where the reset is active in microseconds
 * \param intervalDelayReset - delay between two reset
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \warning if the access cannot be performed, the loop is not used
 */

std::string testPIAResetfunctions (FecAccess *fecAccess,
				   tscType8 fecAddress,
				   tscType8 ringAddress,
				   tscType8 ccuAddress,
				   tscType8 channelAddress,
				   tscType16 value,
				   unsigned long delayActiveReset,
				   unsigned long intervalDelayReset,
				   long loop, unsigned long tms ) { 
  
  std::string result;

  PiaResetAccess *piaResetAccess ;
  //keyType myKey = (setFecKey(fecAddress) | setRingKey(ringAddress) | setCcuKey(ccuAddress) | setChannelKey(channelAddress) ) ; // Build the key
  keyType myKey = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,0) ; // Build the key

  try {

                   
    piaResetAccess = new PiaResetAccess(fecAccess, myKey, 0xFF) ;
  }
  catch (FecExceptionHandler e) {

    //if (errorCounterFlag_) displayStatus ( &e, 0, fecAccess, stdchan_ ) ;

    return "Pia access creation error";
  }

  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {
      
    try {

      // Create the piaResetDescription
      piaResetDescription piaD ( myKey, (unsigned long)delayActiveReset, (unsigned long)intervalDelayReset, (tscType8)value ) ; 

      piaResetAccess->setValues (piaD) ;

      piaResetDescription *piaD1 = piaResetAccess->getValues ( ) ;
    

      if (piaD == *piaD1) {
        //result = "Pia reset comparison ok";
	result = "Pia reset";
	
      }
      else {
        //result = "Pia reset comparison error";	
	result = "Pia reset";

	
      }

      delete piaD1 ;
    }

    catch (FecExceptionHandler e) {
    

      //if (errorCounterFlag_) displayStatus ( &e, loopI, fecAccess, stdchan_ ) ;

      result = "Access to hardware error";
    }

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  // delete the PIA reset
  try {
    delete piaResetAccess ;
  }
  catch (FecExceptionHandler e) {    
    std::cout << "------------ Exception ----------" << std::endl ;
    std::cout << e.what()  << std::endl ;
    std::cout << "---------------------------------" << std::endl ;
  }

  return result;
}


/** This method is able to switch ring A to ring B for the FEC and 
 * each CCU
 * <p>command: -redundancy
 * \param fecAccess - FEC access object
 * \param fecAddress - FEC slot
 * \param ccuAddresses - array of CCU addresses (or FEC)
 * \param numberOfCCU - number of CCU to be reconfigured
 * \bug the command is removed and replaced by -bypassed
 */
string testRedundancyRing ( FecAccess *fecAccess, 
			    tscType8 fecAddress,
			    tscType8 ringAddress,
			    uint ccuAddresses[][3], 
			    uint numberOfCCU ) { 

  std::ostringstream o;
  std::ostringstream er;
  
  keyType indexFecRing = buildFecRingKey(fecAddress,ringAddress) ;
  
  // Find the FECs
  try {
    // Scan the PCI slot for device driver loaded
    fecAccess->getFecRingDevice (indexFecRing) ;
  }
  catch (FecExceptionHandler e) {  
  

    er << "*********** ERROR ********************************" << std::endl ; 
    er << "Cannot find any FECs on the ring: " << e.what() << std::endl ; 
    er << "**************************************************" << std::endl ;

    return er.str();
  }

  // Ask to not initialise the Fec Device at the start point (FecRingDevice
  // object creation
  bool fecDeviceInit = fecAccess->getFecRingDeviceInit () ;
  if (fecDeviceInit == true) fecAccess->setFecRingDeviceInit (false) ;

  // disable the IRQ
 
  o << "Disable the IRQ " << std::endl ;  
  fecAccess->getFecRingDevice (indexFecRing)->setIRQ(false) ;

  try {
    //   FEC 0        CCU 0x10      CCU 0x18      CCU 0x7F
    //         Input A
    //            Ouput B
    // { {0x0, 0, 1}, {0x10, 0, 0}, {0x18, 1, 0}, {0x7F, 0, 0} 
                                     
    for (unsigned int i = 0 ; i < numberOfCCU ; i++) {  

      // FEC
      if (ccuAddresses[i][0] == 0x0) {

	// Retreive the CR0 value from cache
	tscType32 fecCR0 = fecAccess->getFecRingCR0(indexFecRing) ;

	// Make an or with the value => input
	if (ccuAddresses[i][1] == 0) fecCR0 &= 0xFFEF ;
	else
	  fecCR0 = (fecCR0 & 0xFFEF) | 0x0010 ;

	// Make an or with the value => output
	if (ccuAddresses[i][2] == 0) fecCR0 &= 0xFFF7 ;
	else
	  fecCR0 = (fecCR0 & 0xFFF7) | 0x0008 ;
	
	try {
	  fecAccess->setFecRingCR0 (indexFecRing, fecCR0) ;
	}
	catch (FecExceptionHandler e) { }

      }
      else { // CCU
	      
	keyType index = buildCompleteKey(fecAddress,ringAddress,ccuAddresses[i][0],0,0) ; 
	
	tscType32 CRC = 0 ;
	
	// Make an or with the value => input
	if (ccuAddresses[i][1] == 0) CRC &= 0xFE ;
	else
	  CRC = (CRC & 0xFE) | 0x01 ;

	// Make an or with the value => output
	if (ccuAddresses[i][2] == 0) CRC &= 0xFD ;
	else
	  CRC = (CRC & 0xFD) | 0x02 ;

	try {
	  
	  fecAccess->setCcuCRC (index, CRC) ;
	  	  
	}
	catch (FecExceptionHandler e) { 
	}
      }
    }

    try {
      // Twice ?
      fecAccess->fecRingRelease( indexFecRing ) ;
     
      o << "FEC SR0 = 0x" << std::hex << fecAccess->getFecRingSR0( indexFecRing ) << std::endl ;
    }
    catch (FecExceptionHandler e) {
    
    }
 
    
  }
  catch (FecExceptionHandler e) { 

    o << "*********** ERROR ********************************" << std::endl ; 
    o << "An error occurs during hardware access" << std::endl ;
    o << e.what()  << std::endl ;
    o << "**************************************************" << std::endl ;
  }
 
 
  // Initialise the Fec object or not at the start point
  fecAccess->setFecRingDeviceInit (fecDeviceInit) ; 
  
  
  // enable the IRQ
 
  o << "Reenable the IRQ" << std::endl ;
  fecAccess->getFecRingDevice (indexFecRing)->setIRQ(true) ; 

  /* try {
    
    keyType index1 = buildFecRingKey(fecAddress,ringAddress) ;
    
    // Start the scanning of the ring for each FEC device
    FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index1 ) ;
    fecDevice->fecScanRingBroadcast ( ) ;
    
  }
  catch (FecExceptionHandler e) {
    
  }
  try {
    
    keyType index2 = buildFecRingKey(fecAddress,ringAddress) ;
           
    // Start the scanning of the ring for each FEC device
    FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index2 ) ;
    fecDevice->fecScanRingBroadcast ( ) ;
    
  }
  catch (FecExceptionHandler e) {
    
  }*/

  return o.str();
}



/** This method is able to switch ring A to ring B for the FEC and 
 * each CCU
 * <p>command: -autoredundancy
 * \param fecAccess - FEC access object
 * \param fecAddress - slot of the FEC.
 * \param ringAddress - slot of the ring.
 * \param loop - a loop (if negative => for ever ^C needed to end the process)
 * \param tms - time before two loop
 * \param This method, only check if the CCU bypass is correct without any CCU on ring
 */
std::vector<std::string> autoRedundancyRing ( FecAccess *fecAccess,
					      tscType8 fecAddress,
					      tscType8 ringAddress,
					      long loop, unsigned long tms ) { 

  keyType indexFecRing = buildFecRingKey(fecAddress,ringAddress) ;

  std::vector<std::string> results;

  unsigned int transactionNumber = 0 ;
  unsigned int totalTransactionNumber = 0 ;
  bool error = false ;

  // Find the FECs
  try {
    // Scan the PCI slot for device driver loaded
    fecAccess->getFecRingDevice (indexFecRing) ;
  }
  catch (FecExceptionHandler e) {  

    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "Cannot find any FECs on the ring: " << e.what() << std::endl ; 
    std::cerr << "**************************************************" << std::endl ;

    return results;
  }

  // While the loop is not finished
  for (long loopI = 0 ; (loopI < loop) || (loop < 0) ; loopI ++) {

    transactionNumber = 0 ;

    std::cout << "--------------------------------> Test " << std::dec << loopI << " start at " << time(NULL) << std::endl ;

    // Display the time
    struct timeval time1, time2 ;
    struct timezone zone1, zone2 ;

    // Find all the CCUs
    std::list<keyType> *ccuList = NULL ;

    try {
      
      unsigned int timeout = 10 ;
      do {

	// -----------------------------------------------------------------------------------------------------------
	//
	// Make a reset and check the CCUs on the ring
	//
	// -----------------------------------------------------------------------------------------------------------
	// Make a reset
	fecAccess->fecRingReset (indexFecRing) ;
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {
	  
	  std::cout << "PARASITIC: After reset => SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << std::endl ;
	  emptyFIFOs ( fecAccess, indexFecRing, true ) ;
	}
	
	// Log message
	std::cout << "PLX and FEC reset Performed (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;
	
	timeout -- ;
	
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) usleep (tms) ;
      }
      while (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing)) && timeout > 0) ;
      
      // get the time
      gettimeofday(&time1, &zone1);
      

      // Scan the ring for each FECs and display the CCUs
      ccuList = fecAccess->getCcuList ( indexFecRing ) ;
      if (ccuList != NULL) 
	clearFecCcuErrorRegisters (fecAccess, fecAddress, ringAddress, *ccuList) ;

      // 1 transaction for each scan ring
      transactionNumber += 1 ;
      
      if (ccuList != NULL) {
	
	// Store all the addresses
	keyType ccuAddresses[127] ;
	unsigned int nbCcu = 0 ;

	// Log Message
	std::cout << "After Reset: CCU found " ;

	for (std::list<keyType>::iterator p=ccuList->begin();p!=ccuList->end();p++) {

	  // 1 transaction for the detection of the CCU type
	  transactionNumber += 1 ;
	  
	  keyType index = *p ;
	  ccuAddresses[nbCcu++] = index ;

	  // Log Message
	  std::cout << std::hex << "0x" << getCcuKey(index) << " " ;
	}

	// Log message
	std::cout << std::endl ;

	// Delete = already store in the table of FEC device class
	delete ccuList ;

	// disable the IRQ
	fecAccess->getFecRingDevice (indexFecRing)->setIRQ(false) ;

	// -----------------------------------------------------------------------------------------------------------
	//
	// Make a reset and check the bypass of the 1st CCU
	//
	// -----------------------------------------------------------------------------------------------------------
	// Reset the Ring A and B
	//fecAccess->fecHardReset ( indexFecRing ) ;
	fecAccess->fecRingReset ( indexFecRing ) ;
	fecAccess->fecRingResetB ( indexFecRing ) ;
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {

	  std::cout << "PARASITIC: After reset => SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << std::endl ;
	  emptyFIFOs ( fecAccess, indexFecRing, true ) ;
	}

	// disable the IRQ
	fecAccess->getFecRingDevice (indexFecRing)->setIRQ(false) ;

	// Log message
	std::cout << "PLX and FEC reset Performed (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;

	// Bypass the 1st CCU
	fecAccess->setFecRingCR0 ( ccuAddresses[0], FEC_CR0_SELSEROUT ) ;
	fecAccess->setCcuCRC ( ccuAddresses[1], CCU_CRC_ALTIN) ;
	fecAccess->fecRingRelease ( indexFecRing ) ;

	// 1 transation for the CRC
	transactionNumber += 1 ;  

	results.push_back("<CCUAddress = 0x" + IntToString(getCcuKey(ccuAddresses[0])) + ">");
	
	// If the status is not correct then
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[0])))
	  emptyFIFOs ( fecAccess, ccuAddresses[0] ) ;

	if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[0]))) { 

	  // Log Message
	  std::cout << "ERROR when I bypass the 1st CCU 0x" << std::hex << getCcuKey(ccuAddresses[0]) << ", SR0 = 0x" <<  fecAccess->getFecRingSR0(ccuAddresses[0]) << std::endl ; 
	  error = true ;
	  results.push_back("   <Bypass>Nok</Bypass>");
	}
	else {

	  results.push_back("   <Bypass>Ok</Bypass>");

	  // Log Message
	  std::cout << "Bypass the 1st CCU 0x" << std::hex << getCcuKey(ccuAddresses[0]) << " Ok (SR0 = 0x" << fecAccess->getFecRingSR0(ccuAddresses[0]) << ")" << std::endl ;
	  
	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFecRing ) ;
	  if (ccuList != NULL) 
	    clearFecCcuErrorRegisters (fecAccess, fecAddress, ringAddress, *ccuList) ;

	  // 1 transaction for each scan ring
	  transactionNumber += 1 ;

	  if (ccuList != NULL) {
	    
	    // Log Message
	    std::cout << "Bypass of the 1st CCU: CCU found " ;

	    for (std::list<keyType>::iterator p=ccuList->begin();p!=ccuList->end();p++) {
	      
	      // 1 transaction for the detection of the CCU type
	      transactionNumber += 1 ;

	      keyType index = *p ;
	      
	      // Log Message
	      std::cout << std::hex << "0x" << getCcuKey(index) << " " ;
	    }
	    
	    // Log Message
	    std::cout << std::endl ;

	    // Read during a certain amount of time the CCU and the DCU
	    // transactionNumber += readDcuCcuMinutes (fecAccess, fecAddress, ringAddress, ccuList, dcuChannel, ccuDummyIndex, results, dcuTest) ;

	    // Delete = already store in the table of FEC device class
	    delete ccuList ;
	  }
	  else {
	    std::cout << "ERROR: no CCU found after the bypassed of the 1st CCU 0x"  << std::hex << getCcuKey(ccuAddresses[0]) << std::endl ;
	  }
	}

	results.push_back("</CCUAddress>");

	// -----------------------------------------------------------------------------------------------------------
	//
	// Make a reset and check the bypass of each CCU between the 2nd CCU and the before last one
	//
	// -----------------------------------------------------------------------------------------------------------
	// Reset the Ring A and B
	//fecAccess->fecHardReset ( indexFecRing ) ;
	fecAccess->fecRingReset ( indexFecRing ) ;
	fecAccess->fecRingResetB ( indexFecRing ) ;
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {

	  std::cout << "PARASITIC: After reset => SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << std::endl ;
	  emptyFIFOs ( fecAccess, indexFecRing, true ) ;	
	}

	// Log message
	std::cout << "PLX and FEC reset Performed (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;
	
	// disable the IRQ
	fecAccess->getFecRingDevice (indexFecRing)->setIRQ(false) ;
	
	// Bypass the other CCU except the last one
	uint numberCCU = nbCcu - 1 ;

	for (uint ccuI = 1 ; ccuI < numberCCU ; ccuI ++) {

	  fecAccess->setCcuCRC (ccuAddresses[ccuI-1], CCU_CRC_SSP) ;
	  fecAccess->setCcuCRC (ccuAddresses[ccuI+1], CCU_CRC_ALTIN) ;
	  fecAccess->fecRingRelease ( indexFecRing ) ;

	  results.push_back("<CCUAddress = 0x" + IntToString(getCcuKey(ccuAddresses[ccuI])) + ">");

	  // 1 transation for the CRC
	  transactionNumber += 2 ;
	  
	  // If the status is not correct then
	  if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[ccuI])))
	    emptyFIFOs ( fecAccess, ccuAddresses[ccuI] ) ;

	  if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[ccuI]))) { 
	    
	    std::cout << "ERROR when I bypass the " << std::dec << ccuI+1 << "th CCU 0x" << std::hex << getCcuKey(ccuAddresses[ccuI])
		 << ", SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(ccuAddresses[ccuI]) << std::endl ;
	    error = true ;
	    
	    results.push_back("   <Bypass>Nok</Bypass>");
	  }
	  else {

	    results.push_back("   <Bypass>Ok</Bypass>");
	    
	    std::cout << "Bypass of the " << std::dec << ccuI+1 << "th CCU 0x" << std::hex << getCcuKey(ccuAddresses[ccuI]) << " Ok (SR0 = 0x" << fecAccess->getFecRingSR0(ccuAddresses[0]) << ")" << std::endl ;

	    // Scan the ring for each FECs and display the CCUs
	    ccuList = fecAccess->getCcuList ( indexFecRing ) ;
	    if (ccuList != NULL) 
	      clearFecCcuErrorRegisters (fecAccess, fecAddress, ringAddress, *ccuList) ;

            // 1 transaction for each scan ring
	    transactionNumber += 1 ;

	    if (ccuList != NULL) {

	      // Log Message
	      std::cout << "Bypass of the " << std::dec << ccuI+1 << "th CCU 0x" << std::hex << getCcuKey(ccuAddresses[ccuI]) << ": CCU found " ;

	      for (std::list<keyType>::iterator p=ccuList->begin();p!=ccuList->end();p++) {

		// 1 transaction for the detection of the CCU type
		transactionNumber += 1 ;   

		keyType index = *p ;
		
		// Log Message
		std::cout << std::hex << "0x" << getCcuKey(index) << " " ;
	      }

	      // Log Message
	      std::cout << std::endl ;

	      // Read during a certain amount of time the CCU and the DCU
	      // transactionNumber += readDcuCcuMinutes (fecAccess, fecAddress, ringAddress, ccuList, dcuChannel, ccuDummyIndex, results, dcuTest) ;

	      // Delete = already store in the table of FEC device class
	      delete ccuList ;
	    }
	    else {
	      std::cout << "ERROR: no CCU found after the bypassed of the "  << std::dec << ccuI+1 << "th CCU 0x" << std::hex << getCcuKey(ccuAddresses[ccuI]) << std::endl ;
	    }
	    
	    results.push_back("</CCUAddress>");

	  }

	  // Reset the Ring A and B
	  //fecAccess->fecHardReset ( indexFecRing ) ;
	  fecAccess->fecRingReset ( indexFecRing ) ;
	  fecAccess->fecRingResetB ( indexFecRing ) ;
	  if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {

	    std::cout << "PARASITIC: After reset => SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << std::endl ;
	    emptyFIFOs ( fecAccess, indexFecRing, true ) ;
	  }

	  // disable the IRQ
	  fecAccess->getFecRingDevice (indexFecRing)->setIRQ(false) ;

	  // Log message
	  std::cout << "PLX and FEC reset Performed (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;
	}

	// -----------------------------------------------------------------------------------------------------------
	//
	// Make a reset and check the bypass of each the last CCU
	//
	// -----------------------------------------------------------------------------------------------------------
	// Bypass the last CCU
	results.push_back("<CCUAddress = 0x" + IntToString(getCcuKey(ccuAddresses[nbCcu-1])) + ">");

	// Bypass the last CCU
	//fecAccess->setCcuCRC (ccuAddresses[nbCcu-2], CCU_CRC_SSP) ;
	fecAccess->setCcuCRC (ccuAddresses[nbCcu-1], CCU_CRC_SSP) ;
	std::cout << "Output B for CCU 0x" << std::hex << getCcuKey(ccuAddresses[nbCcu-1]) << std::endl;


	////new
	keyType ccuAddressDummy = buildCompleteKey(fecAddress,ringAddress,0x76,0,0) ; 
	//fecAccess->setCcuCRC (ccuAddressDummy, CCU_CRC_ALTIN) ;
	//std::cout << "Input B for dummy CCU" << std::endl;
	fecAccess->setCcuCRC (ccuAddressDummy, CCU_CRC_SSP) ;
	std::cout << "Output B for dummy CCU" << std::endl;
	/////////

	//fecAccess->setFecRingCR0 ( indexFecRing, FEC_CR0_SELSERIN ) ;
	//std::cout << "Input B for FEC" << std::endl;

	//displayFecStatus(fecAccess,buildFecRingKey(fecAddress,ringAddress)) ;
	std::cout << "SR0 register: 0x" << fecAccess->getFecRingSR0(ccuAddresses[nbCcu-1]) << std::endl ;

	// 1 transation for the CRC
	transactionNumber += 1 ;

	// If the status is not correct then
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[nbCcu-1]))){ 
	  std::cout << "emptyFIFOS!!!" << endl;
	  emptyFIFOs ( fecAccess, ccuAddresses[nbCcu-1] ) ;
	 
	}

	std::cout << "SR0 register: 0x" << fecAccess->getFecRingSR0(ccuAddresses[nbCcu-1]) << std::endl ;

	if (! isFecSR0Correct(fecAccess->getFecRingSR0(ccuAddresses[nbCcu-1]))) { 
	  
	  std::cout << "ERROR when I bypass the last CCU 0x" << std::hex << getCcuKey(ccuAddresses[nbCcu-1]) 
	       << " without the dummy CCU, SR0 = 0x" << fecAccess->getFecRingSR0(ccuAddresses[nbCcu-2]) << std::endl ;
	  error = true ;

	  results.push_back("   <Bypass>Nok</Bypass>");
	  
	}
	else {

	  results.push_back("   <Bypass>Ok</Bypass>");

	  std::cout << "Bypass of the last CCU 0x" << std::hex << getCcuKey(ccuAddresses[nbCcu-1]) << " Ok without the dummy CCU, (SR0 = 0x" << fecAccess->getFecRingSR0(ccuAddresses[nbCcu-1]) << ")" << std::endl ;
	 

	  // Scan the ring for each FECs and display the CCUs
	  ccuList = fecAccess->getCcuList ( indexFecRing ) ;
	  if (ccuList != NULL) 
	    clearFecCcuErrorRegisters (fecAccess, fecAddress, ringAddress, *ccuList) ;

	  // 1 transaction for each scan ring
	  transactionNumber += 1 ;
	    
	  if (ccuList != NULL) {
	      
	    // Log Message
	    std::cout << "Bypass of the last CCU 0x" << std::hex << getCcuKey(ccuAddresses[nbCcu-2]) << " CCU found " ;

	    for (std::list<keyType>::iterator p=ccuList->begin();p!=ccuList->end();p++) {
	      
	      // 1 transaction for the detection of the CCU type
	      transactionNumber += 1 ;

	      keyType index = *p ;
	      
	      // Log Message
	      std::cout << std::hex << "0x" << getCcuKey(index) << " " ;
	    }
	    
	    // Log Message
	    std::cout << std::endl ;

	    // Read during a certain amount of time the CCU and the DCU
	    // transactionNumber += readDcuCcuMinutes (fecAccess, fecAddress, ringAddress, ccuList, dcuChannel, ccuDummyIndex, results, dcuTest) ;
	    
	    // Delete = already store in the table of FEC device class
	    delete ccuList ;
	  }
	  else {
	    std::cout << "ERROR: no CCU found after the bypassed of the last CCU 0x"  << std::hex << getCcuKey(ccuAddresses[nbCcu-1]) << " without dummy CCU" << std::endl ;
	  }
	}
	
	results.push_back("</CCUAddress>");

	// Display the final message
	if (error) {
	  
	  std::cout << "Redundancy: Error when the CCUs are bypassed, Cannot use the redundancy" << std::endl ;
	}
	else {

	  std::cout << "Redundancy: Ring B is ok" << std::endl ;
	}

	// Reset the Ring A and B
	//fecAccess->fecHardReset ( indexFecRing ) ;
	fecAccess->fecRingReset ( indexFecRing ) ;
	fecAccess->fecRingResetB ( indexFecRing ) ;
	if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {

	  std::cout << "PARASITIC: After reset => SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << std::endl ;
	  emptyFIFOs ( fecAccess, indexFecRing, true ) ;
	}

	// Log message
	std::cout << "PLX and FEC reset Performed (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;
	
	// enable the IRQ
	fecAccess->getFecRingDevice (indexFecRing)->setIRQ(true) ;
      }
      else {

	// Log Message
	std::cout << "ERROR: No CCU found on the ring, (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;
      }
    }
    catch (FecExceptionHandler e) { 
      
      std::cerr << "*********** ERROR ********************************" << std::endl ; 
      std::cerr << "An error occurs during hardware access" << std::endl ;
      std::cerr << e.what()  << std::endl ;
      std::cerr << "**************************************************" << std::endl ;
    }

    // Display the time
    gettimeofday(&time2, &zone2);
    double timesec1 = time1.tv_sec + time1.tv_usec * 0.000001 ;
    double timesec2 = time2.tv_sec + time2.tv_usec * 0.000001 ;
    double timesec = timesec2 - timesec1 ;
    std::cout << "--------------------------------> Test " << std::dec << loopI << " stop and takes " << std::dec << timesec << " s for "
	 << transactionNumber << " transactions (SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(indexFecRing) << ")" << std::endl ;

    totalTransactionNumber += transactionNumber ;

    // Wait
    if ( (loop != 1) && (tms > 0) ) usleep (tms) ;
  }

  std::cout << "End of the methods, the total number of transaction is " << std::dec << totalTransactionNumber << std::endl ;

  return results;
}



/** Clear the error in the FECs and in the different CCUs
 */
void clearFecCcuErrorRegisters ( FecAccess *fecAccess,
				 tscType8 fecAddress,
				 tscType8 ringAddress,
				 std::list<keyType> ccuAddresses ) {

  keyType indexFecRing = buildFecRingKey(fecAddress,ringAddress) ;

  try {
    // Clear the FEC register 1 to have a clear view of the errors coming
    fecAccess->setFecRingCR1(indexFecRing, FEC_CR1_CLEARIRQ | FEC_CR1_CLEARERRORS) ;
  }
  catch (FecExceptionHandler e) {
    std::cerr << "Error during the set of the FEC CR1: " << e.what() << std::endl;
  }

  if (! isFecSR0Correct(fecAccess->getFecRingSR0(indexFecRing))) {
    std::cerr << "SR0 is not correct: clear only the SR1 of the FEC" << std::endl ;
  }
  else {
    //std::cerr << "Clear errors on CCU SRA and FEC SR1" << std::endl ;
    keyType previous = 0 ;
    for (std::list<keyType>::iterator it = ccuAddresses.begin() ; it != ccuAddresses.end() ; it ++) {
      
      try {
	keyType index = *it ;
	//std::cout << "Clear errors on CCU 0x" << std::hex << index << std::endl ;
	fecAccess->setCcuCRA (index, CCU_CRA_CLRE) ;
      }
      catch (FecExceptionHandler e) {
	std::cerr << "Error during the set of CCU CRA: " << e.what() << std::endl ;
	displayFecStatus(fecAccess,buildFecRingKey(fecAddress,ringAddress)) ;
	if (previous != 0) {
	  std::cout << "CCU " << getCcuKey(previous) << std::endl;  
	  displayCCUSRA (fecAccess->getCcuSRA(previous)) ;
	  displayCCUCRC (fecAccess->getCcuCRC(previous)) ;
	  displayCCUSRC (fecAccess->getCcuSRC(previous)) ;
	}
// 	std::list<keyType>::iterator it1 = it ;
// 	it1++ ;
// 	if (it1 != ccuAddresses.end()) {
// 	  std::cout << "CCU " << getCcuKey(*it) << std::endl;  
// 	  displayCCUSRA (fecAccess->getCcuSRA(*it1)) ;
// 	  displayCCUSRC (fecAccess->getCcuSRC(*it1)) ;
// 	  displayCCUSRC (fecAccess->getCcuCRC(*it1)) ;
// 	}
	std::cout << "CCU " << getCcuKey(*it) << std::endl;  
	displayCCUSRB (fecAccess->getCcuSRB(*it)) ;
	displayCCUSRA (fecAccess->getCcuSRA(*it)) ;
	displayCCUCRC (fecAccess->getCcuCRC(*it)) ;
	displayCCUSRC (fecAccess->getCcuSRC(*it)) ;
	getchar() ;

	try {
	  keyType index = *it ;
	  //std::cout << "Clear errors on CCU 0x" << std::hex << index << std::endl ;
	  fecAccess->setCcuCRA (index, CCU_CRA_CLRE) ;
	}
	catch (FecExceptionHandler e) {
	  std::cerr << "Error during the set of CCU CRA: " << e.what() << std::endl ;
	}
      }
      previous = *it ;
    }
  }
}





/** This method try to recover the ring until the status register 0 is ok
 */
void emptyFIFOs ( FecAccess *fecAccess, keyType index, bool display ) {

  unsigned int receiveFifoDepth = 0 ;
  unsigned int transmitFifoDepth = 0 ;
  unsigned int returnFifoDepth = 0 ;

  //std::cout << "Start emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
  try {
    
    // Start the scanning of the ring for each FEC device
    FecRingDevice *fecDevice = fecAccess->getFecRingDevice ( index ) ;

    receiveFifoDepth = fecAccess->getReceiveFifoDepth(index) ;
    transmitFifoDepth = fecAccess->getTransmitFifoDepth(index) ;
    returnFifoDepth = fecAccess->getReturnFifoDepth(index) ;

    // Is the status is correct
    if (! isFecSR0Correct(fecAccess->getFecRingSR0(index))) {

      uint fecSR0 = fecAccess->getFecRingSR0(index) ;

      //std::cout << "      emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
      fecSR0 =  fecAccess->getFecRingSR0(index) ;

      // FIFO receive
      if (! (fecSR0 & FEC_SR0_RECEMPTY)) {
        
        // std::cout << "FIFO receive not empty" << std::endl ;
        tscType32 value = 0 ;
        tscType16 FecSR0 ; 
	
        unsigned int loopCnt = 0 ; 
        
        while (
               (!((FecSR0=fecAccess->getFecRingSR0(index)) & FEC_SR0_RECEMPTY) || (FecSR0 & FEC_SR0_RECRUN)) 
               && (loopCnt < receiveFifoDepth)
               && (value != 0xFFFF) ) {
	  

	  if (display) 
	    std::cout << "FIFO receive word(" << std::dec << loopCnt << ") = " 
		 <<  fecDevice->getFifoReceive()
		 << " (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(index)
		 << ")" << std::endl ;
	  else
	    fecDevice->getFifoReceive() ;
          
          loopCnt++; 
          
        } // end of while loop 
      }
      
      // Clear the errors and display the status
      // fecAccess->setFecRingCR1 (index, FEC_CR1_CLEARIRQ | FEC_CR1_CLEARERRORS) ;

      //std::cout << "      emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
      fecSR0 =  fecAccess->getFecRingSR0(index) ;

      // FIFO return
      if (! (fecSR0 & FEC_SR0_RETEMPTY)) {

	// std::cout << "FIFO return not empty" << std::endl ;
	tscType32 value = 0 ;
	tscType16 FecSR0 ; 

	unsigned int loopCnt = 0 ; 

	while (
	       (!((FecSR0=fecAccess->getFecRingSR0(index)) & FEC_SR0_RETEMPTY)) 
	       && (loopCnt < returnFifoDepth)
	       && (value != 0xFFFF) ) {

	  if (display) 
	    std::cout << "FIFO return word(" << std::dec << loopCnt << ") = " 
		 <<  fecDevice->getFifoReturn()
		 << " (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(index)
		 << ")" << std::endl ;
	  else
	    fecDevice->getFifoReturn() ;
          
	  loopCnt++; 
	} // end of while loop 
      }

      // Clear the errors and display the status
      // fecAccess->setFecRingCR1 (index, FEC_CR1_CLEARIRQ | FEC_CR1_CLEARERRORS) ;

      //std::cout << "      emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
      fecSR0 =  fecAccess->getFecRingSR0(index) ;

      // FIFO transmit
      if (! (fecSR0 & FEC_SR0_TRAEMPTY)) {

	// std::cout << "FIFO transmit not empty" << std::endl ;
	tscType32 value = 0 ;
	tscType16 FecSR0 ; 

	unsigned int loopCnt = 0 ; 

	while (
	       (!((FecSR0=fecAccess->getFecRingSR0(index)) & FEC_SR0_TRAEMPTY) || (FecSR0 & FEC_SR0_TRARUN)) 
	       && (loopCnt < transmitFifoDepth)
	       && (value != 0xFFFF) ) {

	  if (display) 
	    std::cout << "FIFO transmit word(" << std::dec << loopCnt << ") = " 
		 <<  fecDevice->getFifoTransmit()
		 << " (SR0 = 0x" << std::hex <<  fecAccess->getFecRingSR0(index)
		 << ")" << std::endl ;
	  else
	    fecDevice->getFifoTransmit() ;
     
	  loopCnt++; 
	} // end of while loop 
      }

      //std::cout << "      emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
      fecSR0 =  fecAccess->getFecRingSR0(index) ;

      // Pending IRQ
      if (fecSR0 & FEC_SR0_PENDINGIRQ) {

	//std::cout << "setFecRingCR1 (" << std::hex << (FEC_CR1_CLEARIRQ) << ")" << std::endl ;
	fecAccess->setFecRingCR1 (index, FEC_CR1_CLEARIRQ) ;
      }
    }
  }
  catch (FecExceptionHandler e) {
    
    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;
  }

  std::cout << "Stop emptyFIFOs: SR0 = 0x" << std::hex << fecAccess->getFecRingSR0(index) << std::endl ;
}


/**
 * This method display all FEC status for a given ring
 * \param fecAccess - FEC access object
 * \param findex - which FEC and ring
 * \param ringAddress - ring slot
 */
void displayFecStatus ( FecAccess *fecAccess,
                        keyType index ) {

  try {

    // Display all status
    tscType16 fecSR0 = fecAccess->getFecRingSR0 (index) ;
    std::cout << "\tFEC status register 0: 0x" << std::hex << fecSR0 << std::endl ;
    if (fecSR0 & FEC_SR0_TRARUN)
      std::cout << "\t\tFIFO transmit running" << std::endl ;
    if (fecSR0 & FEC_SR0_RECRUN)
      std::cout << "\t\tFIFO receive running" << std::endl ;
    if (fecSR0 & FEC_SR0_RECFULL)
      std::cout << "\t\tFIFO receive full" << std::endl ;
    if (fecSR0 & FEC_SR0_RECEMPTY)
      std::cout << "\t\tFIFO receive empty" << std::endl ;
    if (fecSR0 & FEC_SR0_RETFULL)
      std::cout << "\t\tFIFO return full" << std::endl ;
    if (fecSR0 & FEC_SR0_RETEMPTY)
      std::cout << "\t\tFIFO return empty" << std::endl ;
    if (fecSR0 & FEC_SR0_TRAFULL)
      std::cout << "\t\tFIFO transmit full" << std::endl ;
    if (fecSR0 &  FEC_SR0_TRAEMPTY)
      std::cout << "\t\tFIFO transmit empty" << std::endl ;
    if (fecSR0 & FEC_SR0_LINKINITIALIZED)
      std::cout << "\t\tLink initialise" << std::endl ;
    if (fecSR0 & FEC_SR0_PENDINGIRQ) 
      std::cout << "\t\tPending irg" << std::endl ;
    if (fecSR0 & FEC_SR0_DATATOFEC)
      std::cout << "\t\tData to FEC" << std::endl ;
    if (fecSR0 & FEC_SR0_TTCRXREADY)
      std::cout << "\t\tTTCRx ok" << std::endl ;
    
    tscType16 fecSR1 = fecAccess->getFecRingSR1 (index) ;
    std::cout << "\tFEC status register 1: 0x" << std::hex << fecSR1 << std::endl ;
    if (fecSR1 & FEC_SR1_ILLDATA)
      std::cout << "\t\tIllegal data" << std::endl ;
    if (fecSR1 & FEC_SR1_ILLSEQ)
      std::cout << "\t\tIllegal sequence" << std::endl ;
    if (fecSR1 & FEC_SR1_CRCERROR) 
      std::cout << "\t\tCRC error" << std::endl ;
    if (fecSR1 & FEC_SR1_DATACOPIED) 
      std::cout << "\t\tData copied" << std::endl ;
    if (fecSR1 & FEC_SR1_ADDRSEEN) 
      std::cout << "\t\tAddress seen" << std::endl ;
    if (fecSR1 & FEC_SR1_ERROR) 
      std::cout << "\t\tError" << std::endl ;
    if (fecSR1 & FEC_SR1_TIMEOUT) 
      std::cout << "\t\tTimeout" << std::endl ;
    if (fecSR1 & FEC_SR1_CLOCKERROR) 
      std::cout << "\t\tClock error" << std::endl ;

    tscType16 fecCR0 = fecAccess->getFecRingCR0 (index) ;
    std::cout << "\tFEC control register 0: 0x" << std::hex << fecCR0 << std::endl ;
    if (fecCR0 & FEC_CR0_ENABLEFEC)
      std::cout << "\t\tFEC enable" << std::endl ;
    else
      std::cout << "\t\tFEC disable" << std::endl ;
    //if (fecCR0 & FEC_CR0_SEND)
    //  std::cout << "\t\tSend" << std::endl ;
    //if (fecCR0 & 0x4)
    //  std::cout << "\t\tResend" << std::endl ;
    if (fecCR0 & FEC_CR0_XTALCLOCK) 
      std::cout << "\t\tXtal clock" << std::endl ;
    if (fecCR0 & FEC_CR0_SELSEROUT)
      std::cout << "\t\tOutput ring B" << std::endl ;
    else
      std::cout << "\t\tOutput ring A" << std::endl ;
    if (fecCR0 & FEC_CR0_SELSERIN)
      std::cout << "\t\tInput ring B" << std::endl ;
    else
      std::cout << "\t\tInput ring A" << std::endl ;
    
    // The CR1 cannot be read
    //tscType16 fecCR1 = fecAccess->getFecRingCR1 (index) ;
    //std::cout << "\tFEC control register 1: 0x%04X\n", fecCR1) ;
  }
  catch (FecExceptionHandler e) {
    
    std::cerr << "*********** ERROR ********************************" << std::endl ; 
    std::cerr << "An error occurs during hardware access" << std::endl ;
    std::cerr << e.what()  << std::endl ;
    std::cerr << "**************************************************" << std::endl ;
  }
}






std::string PIOrw (FecAccess *fecAccess,
		      tscType8 fecAddress,
		      tscType8 ringAddress,
		      tscType8 ccuAddress,
		      tscType8 channelAddress, // only 0x30 used for the barrel pixel ccu board
		   int value                   // <0 means read, 0..255 means write
		      ) { 
  
  std::ostringstream o;
  std::string result="";

  PiaChannelAccess *piaChannelAccess ;
  keyType myKey = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,0) ; // Build the key

  try {

    piaChannelAccess = new PiaChannelAccess(fecAccess, myKey) ;

  }catch (FecExceptionHandler e) {
    return "Pia access creation error in PIOrw";
  }


  try {
    keyType accessKey=piaChannelAccess->getKey();
    if(value<0){
      tscType8 aGCR=fecAccess->getPiaChannelGCR(accessKey);
      fecAccess->setPiaChannelGCR(accessKey, aGCR & 0xf7);        // clear bit 3 of the GCR (no strobe)
      fecAccess->setPiaChannelDDR (accessKey, 0) ;                // set Data Direction Register to read 
      usleep(10);                                                 // wait a little, line may have been held by ccu
      tscType8 data=fecAccess->getPiaChannelDataReg(accessKey);   
      o << "pio data reads " << hex << (int) data<< dec;
      result=o.str();
    }else if((value>=0)&&(value<256)){
      fecAccess->setPiaChannelDDR (accessKey, 0xFF);               // set Data Direction Register, all write
      fecAccess->setPiaChannelDataReg(accessKey, (tscType8) value);
      //o << "pio data set to " << hex << value<< dec; result=o.str();
      result="ok ";
    }else{
      result= "illegal value for pio write";
    }


  } catch (FecExceptionHandler e) {
    result = "Access to hardware error";
  }


  // delete the PIA access
  try {
    delete piaChannelAccess ;
  }
  catch (FecExceptionHandler e) {    
    std::cout << "------------ Exception ----------" << std::endl ;
    std::cout << e.what()  << std::endl ;
    std::cout << "---------------------------------" << std::endl ;
  }

  return result;
}





std::string CtrlRegE (FecAccess *fecAccess,
		      tscType8 fecAddress,
		      tscType8 ringAddress,
		      tscType8 ccuAddress,
		      tscType8 channelAddress, 
		   int value                   // <0 means read, 0..255 means write
		      ) { 
  
  std::ostringstream o;

  keyType myKey = buildCompleteKey(fecAddress,ringAddress,ccuAddress,channelAddress,0) ; // Build the key
  tscType24 cre=fecAccess->getCcuCRE(myKey);
  o << "CRE=" << cre << endl;
  if (value>0){
    cre =  (cre & 0xff0000 ) | (value& 0xFF);
    fecAccess->setCcuCRE(myKey,cre);
    o << "wrote CRE=" << cre << endl;
  }
  return o.str();
}

