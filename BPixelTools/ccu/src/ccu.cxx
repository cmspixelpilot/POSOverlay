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
#include <sstream>
#include <fstream>

using std::cout;
using std::endl;

#include <stdio.h>    // fopen snprintf
#include <stdlib.h>
#include <string.h>   // strcmp
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
#include <string>
#include <time.h>



#include "ServerAccess.h"
#include "SimpleCommand.h"
#include "MultiplexingServer.h"
#include "VMELock.h"

//port address
const int portaddress = 2002; 


using namespace std;

// address table
//map< string, int > fecAddressMap;               // key = sector
map< string, unsigned int > fecAddressMap;       // key = sector
map< string, unsigned int > ringAddressMap;
map< string, unsigned int > ccuAddressMap;
map< string, unsigned int > channelAddressMap;  // key = group


/* helper function that chops a string into a vector of strings */
void tokenize(const string& str,
              vector<string>& tokens,
              const string& symbols)
{
  const string whitespace=" \n";
  const string doublequotes="\"";
  string delimiters=symbols+whitespace+doublequotes;

  // Skip whitespaces at beginning.
  string::size_type lastPos = str.find_first_not_of(whitespace, 0);
  // Find first delimiter
  string::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (string::npos != pos || string::npos != lastPos)
    {
      if(lastPos==pos){
        // found a single character symbol
        // quotes?
        if(str.substr(lastPos,1)==doublequotes){
          pos = str.find_first_of(doublequotes, lastPos+1);
          // FIXME: check for end of string here
          tokens.push_back(str.substr(lastPos+1,pos-lastPos-1));
        }else{
          tokens.push_back(str.substr(lastPos, 1));
        }
        pos++;  // move on to next character
      }else{
        // Found a string, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
      }
      // Skip whitespace.  Note the "not_of"
      lastPos = str.find_first_not_of(whitespace, pos);
      // Find next "non-delimiter"
      pos = str.find_first_of(delimiters, lastPos);
    }
}

string printAddressMap(){
  string line; 
  map<string, unsigned int>::const_iterator itr;
  std::ostringstream o;
  for(itr = fecAddressMap.begin(); itr != fecAddressMap.end(); ++itr){
    string sector=itr->first;
    //   cout <<  (*itr).first << " Value: " << (*itr).second;
    o << sector << " " <<fecAddressMap[sector] << " "
      << ringAddressMap[sector] << " " << ccuAddressMap[sector] << endl;
  }
  return o.str();
}



void readAddressMaps(const string filename){
  string line; 
  ifstream myfile (filename.c_str());
  if (myfile.is_open()) {
    
    while (! myfile.eof() ) {
      getline (myfile,line);
      if (line[0]=='#' || line=="") continue;
      vector<string> column;  // must clear when declared outside of the loop
      tokenize(line ,column,"");

      if(column.size()==5){
	string group=column[4].c_str();
	string sector=group.substr(0,3);
	fecAddressMap[sector]=atoi(column[0].c_str());
	ringAddressMap[sector]=strtol (column[1].c_str(),NULL,16);
	ccuAddressMap[sector]=strtol (column[2].c_str(),NULL,16);
	channelAddressMap[group]=strtol (column[3].c_str(),NULL,16);
	//cout << line << " ===>"   << group << " " << sector << " " << fecAddressMap[sector] << endl;
      }
    }
  }else{
    cout << "error reading address table " << filename << endl;
    exit(1);
  }
  printAddressMap();
}




/* this function handles client requests passed on to it as a string and
   returns a response string that is going to be sent back to the client */
string handle(vector<string> tokens, string sector, string group, FecAccess *fecAccess, unsigned int fecAddress, unsigned int ringAddress, unsigned int ccuAddress, unsigned int channelAddress, long loop ){
  ////////////////////////////////////
  //Default settings
  unsigned int deviceAddress = 0x0 ;

  std::map<std::string,unsigned int> pohAddressesMap;
  pohAddressesMap["poh1"] = poh1deviceAddress;
  pohAddressesMap["poh2"] = poh2deviceAddress;
  pohAddressesMap["poh3"] = poh3deviceAddress;
  pohAddressesMap["poh4"] = poh4deviceAddress;
  pohAddressesMap["poh5"] = poh5deviceAddress;
  pohAddressesMap["poh6"] = poh6deviceAddress;
  pohAddressesMap["poh7"] = poh7deviceAddress;
 
  // To loop on a command
  unsigned long tms  = 0 ;  // wait tms microseconds
  enumDeviceType modeType = PHILIPS ;
  /////////////////////////////////////////
 
  std::ostringstream o;

  //************************************HELP
  //************************************
  if (tokens[0]=="help"){
    
    o << "which addresses\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the fec, ring, ccu and channel address\n\n" ;

    o << "fec <FEC Address>\n";
    o << "\t <FEC Address>: FEC address in dec\n" ; 
    o << "\t\t set the fec address\n\n" ;

    o << "which fec\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the fec address\n\n" ; 

    o << "ring <ring Address>\n";
    o << "\t <ring Address>: ring address in hex\n" ; 
    o << "\t\t set the ring address\n\n" ;

    o << "which ring\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the ring address\n\n" ; 

    o << "ccu <CCU Address>\n";
    o << "\t <CCU Address>: CCU address in hex\n" ; 
    o << "\t\t set the ccu address\n\n" ;

    o << "which ccu\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the ccu address\n\n" ;

    o << "channel <channel Address>\n";
    o << "\t <channel Address>: channel address in hex\n" ; 
    o << "\t\t set the channel address\n\n" ;

    o << "which channel\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the channel address\n\n" ;

    o << "group <group description>\n";
    o << "\t <group description>: -6PL12, -6PL3, ...\n" ; 
    o << "\t\t set the group\n\n" ;

    o << "which group\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the group description\n\n" ;

    o << "sector <sector description>\n";
    o << "\t <sector description>: -6P, +6N, ...\n" ; 
    o << "\t\t set the sector\n\n" ;

    o << "which sector\n";
    o << "\t no option needed\n" ; 
    o << "\t\t get the sector description\n\n" ;

    o << "reset\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t reset the fec\n\n" ;

    o << "cratereset\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t reset the crate \n\n" ;

    o << "setup\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t setup all the devices (to some initial values)\n\n" ;

    o << "dcdcenabletest\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t test enabling/disabling DCDC converters\n\n" ;

    o << "scanccu\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t scan the fec for ccu\n\n" ;

    o << "scanringedevice\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t scan the fec for i2c devices connected\n\n" ;

    o << "scanpixeldevice\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t scan the fec for i2c devices connected\n\n" ; 

    o << "status\n";
    o << "\t no option is needed\n" ; 
    o << "\t\t print the status of the FEC\n\n" ;

    o << "piareset [option]\n";
    o << "\t options:\n" ; 
    o << "\t all\n" ; 
    o << "\t\t reset all\n" ;
    o << "\t roc\n" ; 
    o << "\t\t reset roc\n" ;
    o << "\t aoh\n" ; 
    o << "\t\t reset aoh\n" ;
    o << "\t doh\n" ; 
    o << "\t\t reset doh\n" ;
    o << "\t res1\n" ; 
    o << "\t\t reserve 1\n" ;
    o << "\t res2\n" ; 
    o << "\t\t reserve 2\n\n" ; 

    o << "redundancy fec <input> <output> ccu <CCU address> <input> <output>\n";
    o << "\t <input> : A, B\n" ; 
    o << "\t <output>: A, B\n" ; 
    o << "\t\t CCU ring redundancy: select input/output port of individual device\n\n" ; 

    o << "i2c <channeladdress> <deviceaddress> <value> \n";
    o << "\t <channeladdress> : channel address in hex : 0x11 or 0x13\n" ; 
    o << "\t <deviceaddress>: address of the register\n" ; 
    o << "\t <value>: value to write to the register\n" ; 
    o << "\t\t CCU ring redundancy: select input/output port of individual device\n\n" ;

    o << "delay25 [option]\n";
    o << "\t options:\n" ; 
    o << "\t read\n" ;
    o << "\t\t read registers of Delay25\n" ;
    o << "\t set <delay> <value>\n" ;
    o << "\t\t <delay> : options: d0, d1, d2, d3, d4\n" ; 
    o << "\t\t <value> : value to write to the delay" ; 
    o << "\t\t set delay values of Delay25\n" ; 
    o << "\t setall <value_0> <value_1> <value_2> <value_3> <value_4>\n" ;
    o << "\t\t <value_i> : delay to set in register i" ; 
    o << "\t\t set all the delay values of Delay25\n" ;
    o << "\t init\n" ;
    o << "\t\t initalise Delay25\n" ; 
    o << "\t reset\n" ;
    o << "\t\t software reset of Delay25\n" ; 
    o << "\t DLLresynch\n" ;
    o << "\t\t resynchronization of the DLL without resetting the chip\n" ;
    o << "\t i2c <register> <value>\n" ;
    o << "\t\t <register> : options: r0, r1, r2, r3, r4, r5 \n" ; 
    o << "\t\t <value> : value to write to the register" ; 
    o << "\t\t access i2c register of Delay25\n\n" ; 

    o << "pll [option]\n";
    o << "\t options:\n" ; 
    o << "\t read\n" ;
    o << "\t\t read registers of PLL\n" ;
    o << "\t set <delay> <value>\n" ;
    o << "\t\t <delay> : options: clk, tr\n" ; 
    o << "\t\t <value> : value to write to the PLL" ; 
    o << "\t\t set delay values of PLL (clock phase, trigger delay)\n" ; 
    o << "\t setall <value_clk> <value_tr>\n" ;
    o << "\t\t <value_clk> : clock phase, <value trigger> : trigger delay " ; 
    o << "\t\t set all the values of PLL\n" ;
    o << "\t init\n" ;
    o << "\t\t initalise PLL\n" ; 
    o << "\t reset\n" ;
    o << "\t\t i2c reset of PLL\n" ; 
    o << "\t i2c <register> <value>\n" ;
    o << "\t\t <register> : options: r0, r1, r2, r3 \n" ; 
    o << "\t\t <value> : value to write to the register" ; 
    o << "\t\t access i2c register of PLL\n\n" ; 


    o << "aoh1a [option]\n";
    o << "aoh1b [option]\n";
    o << "aoh2a [option]\n";
    o << "aoh2b [option]\n";
    o << "aoh3a [option]\n";
    o << "aoh3b [option]\n"; 
    o << "aoh4a [option]\n";
    o << "aoh4b [option]\n";
    o << "\t options:\n" ; 
    o << "\t read\n" ;
    o << "\t\t read registers of AOH\n" ;
    o << "\t set <gain/bias> <value>\n" ;
    o << "\t\t <gain/bias> : options: g0, g1, g2, b0, b1, b2\n" ; 
    o << "\t\t <value> : value to write to the AOH gain/bias" ; 
    o << "\t\t set gain/bias of AOH\n" ; 
    o << "\t setall <value_g0> <value_g1> <value_g2> <value_b0> <value_b1> <value_b2>\n" ;
    o << "\t\t <value_gi> : value to set to gain i, <value_bi> : value to set to bias i " ; 
    o << "\t\t set all the values of AOH\n" ;
    o << "\t init\n" ;
    o << "\t\t initalise AOH\n" ; 
    o << "\t i2c <register> <value>\n" ;
    o << "\t\t <register> : options: r0, r1, r2, r3 \n" ; 
    o << "\t\t <value> : value to write to the register" ; 
    o << "\t\t access i2c register of AOH\n\n" ;

    o << "doh [option]\n";
    o << "\t options:\n" ; 
    o << "\t read\n" ;
    o << "\t\t read registers of DOH\n" ;
    o << "\t set <gain/bias> <value>\n" ;
    o << "\t\t <gain/bias> : options: g0, g1, g2, b0, b1, b2\n" ; 
    o << "\t\t <value> : value to write to the DOH gain/bias" ; 
    o << "\t\t set gain/bias of DOH\n" ; 
    o << "\t setall <value_g0> <value_g1> <value_g2> <value_b0> <value_b1> <value_b2>\n" ;
    o << "\t\t <value_gi> : value to set to gain i, <value_bi> : value to set to bias i " ; 
    o << "\t\t set all the values of DOH\n" ;
    o << "\t init\n" ;
    o << "\t\t initalise DOH\n" ; 
    o << "\t i2c <register> <value>\n" ;
    o << "\t\t <register> : options: r0, r1, r2, r3 \n" ; 
    o << "\t\t <value> : value to write to the register" ; 
    o << "\t\t access i2c register of DOH\n\n" ;  
     
    return o.str();

  }

  //************************************WHICH
  //************************************
  else if (tokens[0]=="which"){
    
    std::ostringstream o;
    
    o << std::endl;
    if (tokens[1]=="fec")  o  << "FEC "  << std::dec << fecAddress << std::endl; 
    else if (tokens[1]=="ring")  o << "RING " << std::dec << ringAddress << std::endl;
    else if (tokens[1]=="ccu")  o  << "CCU 0x"  << std::hex << ccuAddress << std::endl; 
    else if (tokens[1]=="channel")  o  << "channel 0x"  << std::hex << channelAddress << std::endl;
    else if (tokens[1]=="addresses")  {
      o  << "Group  "  << group << std::endl;
      o  << "FEC "  << std::dec << fecAddress << std::endl; 
      o  << "RING " << std::dec << ringAddress << std::endl;
      o  << "CCU  0x"  << std::hex << ccuAddress << std::endl;
      o  << "channel 0x"  << std::hex << channelAddress << std::endl;
    }
    else if (tokens[1]=="group")  o  << "Group  "  << group << std::endl; 
    else if (tokens[1]=="sector")  o  << "Sector  "  << sector << std::endl; 
    

    else return string("what?");

 
    return o.str();

  }

  //************************************RESET
  //************************************
  else if ((tokens[0]=="reset")&&(tokens.size()==1)){

    return resetPlxFec ( fecAccess, fecAddress, ringAddress, loop, tms );
     

  } 
  //************************************RESET
  //************************************
  else if ((tokens[0]=="cratereset")&&(tokens.size()==1)){

    bool testCrateResetF = false;
    return  crateReset ( fecAccess, testCrateResetF, loop, tms );
     

  }
  //************************************SETUP
  //************************************
  else if (tokens[0]=="setup"){
    
    std::ostringstream o;

    o << setDelay25_Delay0 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, delay25deviceAddress, loop, tms, init_delay25_delay0 ) 
      << setDelay25_Delay1 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, delay25deviceAddress, loop, tms, init_delay25_delay1 )
      << setDelay25_Delay2 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, delay25deviceAddress, loop, tms, init_delay25_delay2 )
      << setDelay25_Delay3 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, delay25deviceAddress, loop, tms, init_delay25_delay3 )
      << setDelay25_Delay4 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, delay25deviceAddress, loop, tms, init_delay25_delay4 );

    o << setDelay25_Delay0 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, delay25deviceAddress, loop, tms, init_delay25_delay0 ) 
      << setDelay25_Delay1 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, delay25deviceAddress, loop, tms, init_delay25_delay1 )
      << setDelay25_Delay2 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, delay25deviceAddress, loop, tms, init_delay25_delay2 )
      << setDelay25_Delay3 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, delay25deviceAddress, loop, tms, init_delay25_delay3 )
      << setDelay25_Delay4 ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, delay25deviceAddress, loop, tms, init_delay25_delay4 ); 

    o << setPll_ClockPhase   ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, plldeviceAddress, loop, tms, init_pll_clk ) 
      << setPll_TriggerDelay ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, plldeviceAddress, loop, tms, init_pll_tr );

    o << setPll_ClockPhase   ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, plldeviceAddress, loop, tms, init_pll_clk ) 
      << setPll_TriggerDelay ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, plldeviceAddress, loop, tms, init_pll_tr );
  
    o<< setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, dohdeviceAddress, loop, tms, true, init_doh_gain0, true, init_doh_gain1, true, init_doh_gain2, true, init_doh_bias0, true, init_doh_bias1, true, init_doh_bias2);
    o<< setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, dohdeviceAddress, loop, tms, true, init_doh_gain0, true, init_doh_gain1, true, init_doh_gain2, true, init_doh_bias0, true, init_doh_bias1, true, init_doh_bias2);
    
  
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh1deviceAddress, loop, tms, true, init_aoh1a_gain0, true, init_aoh1a_gain1, true, init_aoh1a_gain2, true, init_aoh1a_bias0, true, init_aoh1a_bias1, true, init_aoh1a_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh2deviceAddress, loop, tms, true, init_aoh1b_gain0, true, init_aoh1b_gain1, true, init_aoh1b_gain2, true, init_aoh1b_bias0, true, init_aoh1b_bias1, true, init_aoh1b_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh3deviceAddress, loop, tms, true, init_aoh2a_gain0, true, init_aoh2a_gain1, true, init_aoh2a_gain2, true, init_aoh2a_bias0, true, init_aoh2a_bias1, true, init_aoh2a_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh4deviceAddress, loop, tms, true, init_aoh2b_gain0, true, init_aoh2b_gain1, true, init_aoh2b_gain2, true, init_aoh2b_bias0, true, init_aoh2b_bias1, true, init_aoh2b_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh5deviceAddress, loop, tms, true, init_aoh3a_gain0, true, init_aoh3a_gain1, true, init_aoh3a_gain2, true, init_aoh3a_bias0, true, init_aoh3a_bias1, true, init_aoh3a_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh6deviceAddress, loop, tms, true, init_aoh3b_gain0, true, init_aoh3b_gain1, true, init_aoh3b_gain2, true, init_aoh3b_bias0, true, init_aoh3b_bias1, true, init_aoh3b_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x11, poh7deviceAddress, loop, tms, true, init_aoh4a_gain0, true, init_aoh4a_gain1, true, init_aoh4a_gain2, true, init_aoh4a_bias0, true, init_aoh4a_bias1, true, init_aoh4a_bias2);
  
  
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, poh1deviceAddress, loop, tms, true, init_aoh1a_gain0, true, init_aoh1a_gain1, true, init_aoh1a_gain2, true, init_aoh1a_bias0, true, init_aoh1a_bias1, true, init_aoh1a_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, poh2deviceAddress, loop, tms, true, init_aoh1b_gain0, true, init_aoh1b_gain1, true, init_aoh1b_gain2, true, init_aoh1b_bias0, true, init_aoh1b_bias1, true, init_aoh1b_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, poh3deviceAddress, loop, tms, true, init_aoh2a_gain0, true, init_aoh2a_gain1, true, init_aoh2a_gain2, true, init_aoh2a_bias0, true, init_aoh2a_bias1, true, init_aoh2a_bias2);
      o << setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, 0x13, poh4deviceAddress, loop, tms, true, init_aoh2b_gain0, true, init_aoh2b_gain1, true, init_aoh2b_gain2, true, init_aoh2b_bias0, true, init_aoh2b_bias1, true, init_aoh2b_bias2);
  
 
      
      return o.str();
    
  }
  //************************************PRINTADDRESSMAP
  //************************************
  else if (tokens[0]=="printAddressMap"){
 
    return printAddressMap();
  
 
  }

  //************************************DCDCENABLETES
  else if (tokens[0]=="enabletest"){

    unsigned int dcdcAddress = 99;
    if ( tokens.size()>2 && tokens[1]=="dcdc" ) 
      dcdcAddress = atoi(tokens[2].c_str());
    
    if (dcdcAddress >= 0 && dcdcAddress <13) 
      return DCDCenableTest (fecAccess, fecAddress, ringAddress, ccuAddress, dcdcAddress, false) ;
    //return DCDCenableTest (fecAccess, fecAddress, ringAddress, dcdcAddress, false) ;
    else 
      return "invalid DCDC address";
 
  } 
  //************************************SCANCCU
  else if (tokens[0]=="scanccu"){
 
    //return newtestScanCCU ( fecAccess, false );
    return testScanCCU ( fecAccess, fecAddress, ringAddress, false );
  
 
  }
  //************************************
  //************************************
  else if (tokens[0]=="scanccuallfec"){
 
    return newtestScanCCU ( fecAccess, false );
      
 
  }
  //************************************SCANRINGEDEVICE
  //************************************
  else if (tokens[0]=="scanringedevice" || tokens[0]=="scanringdevice"){
  
    return testScanRingEDevice ( fecAccess, fecAddress, ringAddress );
  
  }
  //************************************SCANRINGEDEVICE
  //************************************
  else if (tokens[0]=="scanpixeldevice" ){
  
    return testScanPixelDevice ( fecAccess, fecAddress, ringAddress, loop, tms); 
  
  }
  //************************************SCANRINGEDEVICE
  //************************************
  else if (tokens[0]=="status" ){ 

    time_t now = time(0);
    char timenow[300];
    char filename[300];
    size_t s; int t;
    s = strftime( timenow, 500, "%d%b%Y_%H%M" , localtime(&now) );
    t = sprintf( filename, "./Status_%s.txt", timenow );
    
    ofstream os (filename); 

    os << testStatus ( fecAccess, false ) << endl;
    cout << testStatus ( fecAccess, false ) << endl;
    
    return "output written to file";
    
  }

  //************************************map
  //************************************
  else if (tokens[0]=="map" ){
  
    return testMap ( fecAccess, false );
  
  } 

  //************************************mapccu
  //************************************
  else if (tokens[0]=="mapccu" ){
  
    return testMapCCU ( fecAccess, false );
  
  } 

  //************************************
  //************************************
  else if (tokens[0]=="disablei2c" ){
  
    return CtrlRegE(fecAccess, fecAddress, ringAddress, ccuAddress,0x30, -1);
  
  } 
  else if (tokens[0]=="enablei2c" ){
  
    return CtrlRegE(fecAccess, fecAddress, ringAddress, ccuAddress,0x30, 0xff);
  
  } 

  //************************************PIA RESET
  //************************************
  else if (tokens[0]=="piareset" ){

    unsigned int piaResetValue = 0xFF ;
    unsigned int piaChannel    = 0x30 ;
   
    if ( tokens[1]=="all" ) {
      piaResetValue = 0xFF ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    }
    if ( tokens[1]=="roc" ) {
      piaResetValue = 0x1 ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    }
    if ( tokens[1]=="aoh" || tokens[1]=="poh" ) {
      piaResetValue = 0x2 ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    } 
    if ( tokens[1]=="doh" ) {
      piaResetValue = 0x4 ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    }
    if ( tokens[1]=="res1" ) {
      piaResetValue = 0x8 ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    }
    if ( tokens[1]=="res2" ) {
      piaResetValue = 0x10 ;
      return testPIAResetfunctions ( fecAccess, fecAddress, ringAddress, ccuAddress, piaChannel, piaResetValue, 1, 10000, loop, tms );
    }
    else {
      return string("not an option! options for piareset: [roc],[aoh],[doh],[res1],[res2]");
    }
    
  }
  //************************************REDUNDNACY
  //************************************
  else if (tokens[0]=="redundancy") { // Ring A - B
 
    bool error = false;
    unsigned int ccuAddresses[128][3] ;
    for (int ii=0; ii<128; ii++) {
      for (int jj=0; jj<3; jj++) {
	ccuAddresses[ii][jj]=0;
      }
    }
    int nbccu = 0 ;
    for ( unsigned int i=1 ; i < tokens.size() ; i++) {

      if ( tokens[i]=="fec" ) {
	ccuAddresses[nbccu][0] = 0x0 ;
	
	if ( tokens.size() > i+1 ) {
	  if ( tokens[i+1] == "A" || tokens[i+1] == "a" ) ccuAddresses[nbccu][1] = 0 ;
	  else
	    if ( tokens[i+1] == "B" || tokens[i+1] == "b" ) ccuAddresses[nbccu][1] = 1 ;
	    else
	      error = true ;
	  
	  if ( tokens[i+2] == "A" || tokens[i+2] == "a" ) ccuAddresses[nbccu][2] = 0 ;
	  else
	    if ( tokens[i+2] == "B" || tokens[i+2] == "b" ) ccuAddresses[nbccu][2] = 1 ;
	    else
	      error = true ;
	}
	else error = true ;
	
	nbccu ++ ; 
      }
      
      else if ( tokens[i]=="ccu" ) {
	
	if ( tokens.size() > i+2 ) {
	  ccuAddresses[nbccu][0] = strtol (tokens[i+1].c_str(),NULL,16) ;
	  if ( tokens[i+2] == "A" || tokens[i+2] == "a" ) ccuAddresses[nbccu][1] = 0 ;
	  else
	    if ( tokens[i+2] == "B" || tokens[i+2] == "b" ) ccuAddresses[nbccu][1] = 1 ;
	    else
	      error = true ;
	  
	  if ( tokens[i+3] == "A" || tokens[i+3] == "a" ) ccuAddresses[nbccu][2] = 0 ;
	  else
	    if ( tokens[i+3] == "B" || tokens[i+3] == "b" ) ccuAddresses[nbccu][2] = 1 ;
	    else
	      error = true ;
	}
	else error = true ;
	
	nbccu ++ ;
      }
  
    }
    if (error || nbccu==0) 
      return string("error in input");
    else {
      //printf ("%d %d %d\n", ccuAddresses[nbccu-1][0], ccuAddresses[nbccu-1][1], ccuAddresses[nbccu-1][2]);
      return testRedundancyRing ( fecAccess, fecAddress, ringAddress, ccuAddresses, nbccu) ;
    }
    
  }

  //************************************AUTOREDUNDANCY
  //************************************
  else if (tokens[0]=="autoredundancy" ){
  
    std::vector<std::string> returnvector = autoRedundancyRing ( fecAccess, fecAddress, ringAddress, loop, tms ); 
    return string ("autoredundancy finished");
  
  } 

 
  //************************************I2C
    
  //************************************
  else if (tokens[0]=="i2c"){
      
      
    if ( tokens.size()>4) { 
      // argument list including the sector (commissioning system)
      int l_fecAddress;
      unsigned int l_ringAddress, l_ccuAddress;

      string sectorName=tokens[1];
      try{
	l_fecAddress=fecAddressMap[sectorName];
	l_ringAddress=ringAddressMap[sectorName];
	l_ccuAddress=ccuAddressMap[sectorName];
	//cout << "DEBUG i2c " <<  sectorName << " " << l_fecAddress << " " << l_ringAddress << " " << l_ccuAddress << endl;
      }catch(...){
	cout << "sector " << sectorName << " not found in address table " << endl;
	return "error";
      }
      unsigned int AddressChannel = strtol (tokens[2].c_str(),NULL,16); 
      deviceAddress = strtol (tokens[3].c_str(),NULL,16); 
      if (tokens[4]=="read"){
	return getI2CDevice ( fecAccess, l_fecAddress, l_ringAddress, l_ccuAddress, AddressChannel, deviceAddress, modeType, loop, tms) ;
      }else{
	unsigned int value = strtol (tokens[4].c_str(),NULL,16);
	string result=setI2CDevice ( fecAccess, l_fecAddress, l_ringAddress, l_ccuAddress, AddressChannel, deviceAddress, modeType, loop, tms, value ) ;
	return result;
      }
     }else if ( tokens.size()>3) { 
       // argument list without group id (system 12)
      unsigned int AddressChannel = strtol (tokens[1].c_str(),NULL,16); 
      deviceAddress = strtol (tokens[2].c_str(),NULL,16); 
      unsigned int value = strtol (tokens[3].c_str(),NULL,16);
      string result=setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, AddressChannel, deviceAddress, modeType, loop, tms, value ) ;
      //return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
      return result;
    }    
    else 
      return string("wrong number of parameters! i2c needs 3 parameters");
    
  }


  //************************************display register of i2c devices
  //************************************
  else if (tokens[0]=="display" ) {

    time_t now = time(0);
    char timenow[300];
    char filename[300];
    size_t s; int t;
    s = strftime( timenow, 500, "%d%b%Y_%H%M" , localtime(&now) );
    t = sprintf( filename, "./CCUDisplay_Group%s_%s.txt", group.c_str(), timenow );
    
    ofstream os (filename);
  
    //delay25
    os << readDelay25(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, delay25deviceAddress, loop, tms) << endl;
    cout << readDelay25(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, delay25deviceAddress, loop, tms) << endl;
    //pll
    os << readPll(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, plldeviceAddress, loop, tms) << endl;
    cout << readPll(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, plldeviceAddress, loop, tms) << endl;
    //aoh1a
    os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh1deviceAddress, loop, tms) << endl;
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh1deviceAddress, loop, tms) << endl;
    //aoh1b
    os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh2deviceAddress, loop, tms) << endl;
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh2deviceAddress, loop, tms) << endl; 
    //aoh2a
    os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh3deviceAddress, loop, tms) << endl;
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh3deviceAddress, loop, tms) << endl;
    //aoh2b
    os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh4deviceAddress, loop, tms) << endl;
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh4deviceAddress, loop, tms) << endl;
    if (channelAddress==0x11){
      //aoh3a
      os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh5deviceAddress, loop, tms) << endl;
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh5deviceAddress, loop, tms) << endl;
      //aoh3b
      os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh6deviceAddress, loop, tms) << endl;
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh6deviceAddress, loop, tms) << endl;
      //aoh4a
      os << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh7deviceAddress, loop, tms) << endl;
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh7deviceAddress, loop, tms) << endl;
    }
    //doh
    os << readDoh(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, dohdeviceAddress, loop, tms) << endl;
    cout << readDoh(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, dohdeviceAddress, loop, tms) << endl;
    
    return "output written to file";
  } 
  //************************************display register of i2c devices
  //************************************
  else if (tokens[0]=="displayregister" ) {
     
    //delay25
    cout << readDelay25(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, delay25deviceAddress, loop, tms) << endl;
    //pll
    cout << readPll(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, plldeviceAddress, loop, tms) << endl;
    //aoh1a
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh1deviceAddress, loop, tms) << endl;
    //aoh1b
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh2deviceAddress, loop, tms) << endl; 
    //aoh2a
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh3deviceAddress, loop, tms) << endl;
    //aoh2b
    cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh4deviceAddress, loop, tms) << endl;
    if (channelAddress==0x11){
      //aoh3a
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh5deviceAddress, loop, tms) << endl;
      //aoh3b
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh6deviceAddress, loop, tms) << endl;
      //aoh4a
      cout << readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, poh7deviceAddress, loop, tms) << endl;
    }
    //doh
    cout << readDoh(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, dohdeviceAddress, loop, tms) << endl;
    
    return "";
  }

  //************************************DEVICE
  //************************************DELAY 25
  else if (tokens[0]=="delay25"){
    deviceAddress=delay25deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readDelay25(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    }
    //WRITE 
    //range delay   = [0,63]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="0" || tokens[2]=="d0" ) {
	return setDelay25_Delay0 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else if ( tokens[2]=="1" || tokens[2]=="d1" ) {
	return setDelay25_Delay1 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else if ( tokens[2]=="2" || tokens[2]=="d2") {
	return setDelay25_Delay2 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else if ( tokens[2]=="3" || tokens[2]=="d3" ) {
	return setDelay25_Delay3 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else if ( tokens[2]=="4" || tokens[2]=="d4" ) {
	return setDelay25_Delay4 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else 
	return string("not an option! options for delay25 set: [d0],[d1],[d2],[d3],[d4]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") || (tokens[1]=="sa")) {
      
      if (tokens.size()>6) {
	std::ostringstream o;
	o << setDelay25_Delay0 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[2].c_str()) ) 
	  << setDelay25_Delay1 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) )
	  << setDelay25_Delay2 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[4].c_str()) )
	  << setDelay25_Delay3 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[5].c_str()) )
	  << setDelay25_Delay4 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[6].c_str()) );
	return o.str();
      }
      else 
	return string("wrong number of parameters! delay25 setall needs 5 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      
      std::ostringstream o;
      o << setDelay25_Delay0 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_delay25_delay0 ) 
        << setDelay25_Delay1 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_delay25_delay1 )
        << setDelay25_Delay2 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_delay25_delay2 )
	<< setDelay25_Delay3 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_delay25_delay3 )
	<< setDelay25_Delay4 ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_delay25_delay4 );
      
      return o.str();
    }
    //RESET
    else if ((tokens[1]=="reset")) {
      
      std::ostringstream o;
      
      for (int i=0; i<5; i++) {
	o << setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+i, modeType, loop, tms, 0 ) ;
      }
      return o.str();
    }
    //RESYNCH DLL
    else if ((tokens[1]=="DLLresynch") ||(tokens[1]=="DLL")) {
      
      return ResynchDLL ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms );
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || tokens[2]=="4" || tokens[2]=="5" ||
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" || tokens[2]=="r4" || tokens[2]=="r5" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else  
	  return string("not an option! options for delay25 i2c: [r0],[r1],[r2],[r3],[r4],[r5]");
      }
      else 
	return string("wrong number of parameters! delay25 i2c needs 2 parameters");
    }
    else  
      return string("not an option! options for delay25: [read],[set],[setall],[init],[reset],[DLLresynch],[i2c]");
  }
  
  //************************************DEVICE
  //************************************PLL
  else if (tokens[0]=="pll"){
    deviceAddress=plldeviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readPll(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for clock phase   = [0,22]
    //range for trigger delay = [0,15]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="0" || tokens[2]=="clk" ) {
	return setPll_ClockPhase ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      else if ( tokens[2]=="1" || tokens[2]=="tr" ) {
	return setPll_TriggerDelay ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
      }
      
      else 
	return string("not an option! options for pll set: [clk],[tr]");
    } 
    //WRITE ALL
    else if ((tokens[1]=="setall") || (tokens[1]=="sa")) {
      
      if (tokens.size()>3) {
	std::ostringstream o;
	o << setPll_ClockPhase ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[2].c_str()) ) 
	  << setPll_TriggerDelay ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, atoi(tokens[3].c_str()) );
	  return o.str();
      }
      else 
	return string("wrong number of parameters! pll setall needs 2 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      
      std::ostringstream o;
      o << setPll_ClockPhase   ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_pll_clk ) 
        << setPll_TriggerDelay ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, init_pll_tr );
      
      return o.str();
    }
    //RESET
    else if ((tokens[1]=="reset")) {
   
      return resetPll(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" ||
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for pll i2c: [r0],[r1],[r2],[r3]");
      }    
      else 
	return string("wrong number of parameters! pll i2c needs 2 parameters");
    }
    else  
      return string("not an option! options for pll: [read],[set],[setall],[init],[reset],[i2c]");
  }
  //************************************DEVICE
  //************************************POHs
   else if( tokens[0].find("poh") != std::string::npos ){
    deviceAddress = pohAddressesMap[tokens[0]];
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readPoh(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    }
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g3" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()), false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()), false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()), false, 0);
      }
      else if ( tokens[2]=="b3" ) {
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for poh set: [g0],[g1],[g2],[g3],[b0],[b1],[b2],[b3]");
    }
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>9) 
	return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()),true, atoi (tokens[8].c_str()),true, atoi (tokens[9].c_str()));
      else
	return string("wrong number of parameters! poh setall needs 8 parameters");
    }
    else if ((tokens[1]=="init")) {
      return setPoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_poh1_gain0, true, init_poh1_gain1, true, init_poh1_gain2, true, init_poh1_gain3, true, init_poh1_bias0, true, init_poh1_bias1, true, init_poh1_bias2, true, init_poh1_bias3);
    }
    else return string("not an option! options for poh set: [g0],[g1],[g2],[g3],[b0],[b1],[b2],[b3]"); 
  }  
  //************************************DEVICE
  //************************************AOH1A
  else if (tokens[0]=="aoh1a"){
    deviceAddress=poh1deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh1a_gain0, true, init_aoh1a_gain1, true, init_aoh1a_gain2, true, init_aoh1a_bias0, true, init_aoh1a_bias1, true, init_aoh1a_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************AOH1B
  else if (tokens[0]=="aoh1b"){
    deviceAddress=poh2deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh1b_gain0, true, init_aoh1b_gain1, true, init_aoh1b_gain2, true, init_aoh1b_bias0, true, init_aoh1b_bias1, true, init_aoh1b_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************AOH2A
   else if (tokens[0]=="aoh2a"){
    deviceAddress=poh3deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh2a_gain0, true, init_aoh2a_gain1, true, init_aoh2a_gain2, true, init_aoh2a_bias0, true, init_aoh2a_bias1, true, init_aoh2a_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************AOH2B  
  else if (tokens[0]=="aoh2b"){
    deviceAddress=poh4deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh2b_gain0, true, init_aoh2b_gain1, true, init_aoh2b_gain2, true, init_aoh2b_bias0, true, init_aoh2b_bias1, true, init_aoh2b_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************AOH3A  
  else if (tokens[0]=="aoh3a"){
    deviceAddress=poh5deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh3a_gain0, true, init_aoh3a_gain1, true, init_aoh3a_gain2, true, init_aoh3a_bias0, true, init_aoh3a_bias1, true, init_aoh3a_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************AOH3B  
  else if (tokens[0]=="aoh3b"){
    deviceAddress=poh6deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh3b_gain0, true, init_aoh3b_gain1, true, init_aoh3b_gain2, true, init_aoh3b_bias0, true, init_aoh3b_bias1, true, init_aoh3b_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
 
  //************************************DEVICE
  //************************************AOH4A 
  else if (tokens[0]=="aoh4a"){
    deviceAddress=poh7deviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readLaserdriver(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for aoh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
       	return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! aoh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setLaserdriver ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_aoh4a_gain0, true, init_aoh4a_gain1, true, init_aoh4a_gain2, true, init_aoh4a_bias0, true, init_aoh4a_bias1, true, init_aoh4a_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for aoh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! aoh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for aoh: [read],[set],[setall],[init],[i2c]");
    
  }
  //************************************DEVICE
  //************************************DOH 
  else if (tokens[0]=="doh"){
    deviceAddress=dohdeviceAddress;
    //READ 
    if ((tokens[1]=="read") || (tokens[1]=="r")) {
      return readDoh(fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms);
    } 
    //WRITE
    //range for gain   = [0,1,2,3]
    //range for bias = [0,127]
    else if ((tokens[1]=="set") ||(tokens[1]=="s")) {
      if ( tokens[2]=="g0" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi(tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g1" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, true, atoi(tokens[3].c_str()) , false, 0, false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="g2" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0, false, 0);
      }
      else if ( tokens[2]=="b0" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0, false, 0);
      }
      else if ( tokens[2]=="b1" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) , false, 0);
      }
      else if ( tokens[2]=="b2" ) {
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, false, 0, false, 0, false, 0, false, 0, false, 0, true, atoi (tokens[3].c_str()) );
      }
      else return string("not an option! options for doh set: [g0],[g1],[g2],[b0],[b1],[b2]");
    }
    //WRITE ALL
    else if ((tokens[1]=="setall") ||(tokens[1]=="sa")) { 

      if ( tokens.size()>7) 
	return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, atoi (tokens[2].c_str()), true, atoi (tokens[3].c_str()), true, atoi (tokens[4].c_str()), true, atoi (tokens[5].c_str()), true, atoi (tokens[6].c_str()), true, atoi (tokens[7].c_str()) );
      else
	return string("wrong number of parameters! doh setall needs 6 parameters");
    }
    //INIT
    else if ((tokens[1]=="init")) {
      return setDoh ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress, loop, tms, true, init_doh_gain0, true, init_doh_gain1, true, init_doh_gain2, true, init_doh_bias0, true, init_doh_bias1, true, init_doh_bias2);
    }
    //I2C REGISTER
    else if ((tokens[1]=="i2c") ) { 
      if ( tokens.size()>3) {
	if ( tokens[2]=="0" || tokens[2]=="1" || tokens[2]=="2" || tokens[2]=="3" || 
	     tokens[2]=="r0" || tokens[2]=="r1" || tokens[2]=="r2" || tokens[2]=="r3" )
	  return setI2CDevice ( fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, deviceAddress+atoi(tokens[2].c_str()), modeType, loop, tms, strtol (tokens[3].c_str(),NULL,16) ) ;
	else 
	  return string("not an option! options for doh i2c: [r0],[r1],[r2],[r3]");
      }
      else 
	return string("wrong number of parameters! doh i2c needs 2 parameters");
    }
    else 
      return string("not an option! options for doh: [read],[set],[setall],[init],[i2c]");
  }
  //*********************************************
  else if ((tokens.size()==2)&&(tokens[0]=="pio")&&(tokens[1]=="read") ) { 
    return PIOrw(fecAccess, fecAddress, ringAddress, ccuAddress,0x30, -1);
  }
  else if ((tokens.size()==3)&&(tokens[0]=="pio")&&(tokens[1]=="write")) { 
    int value=0xFF;
    try{
      value= strtol (tokens[2].c_str(),NULL,0);
    }catch(...){
      return "bad value: " + tokens[2];
    }
    return PIOrw(fecAccess, fecAddress, ringAddress, ccuAddress,0x30, value);
  }
  else if ((tokens.size()==2)&&(tokens[0]=="pio")&&(tokens[1]=="init") ) { 
    string msg="";
    for(map< string, unsigned int >::iterator it= ccuAddressMap.begin();it!=ccuAddressMap.end(); it++){
      string s=it->first; // sector name
      msg+= s+":" +PIOrw(fecAccess, fecAddressMap[s], ringAddressMap[s],ccuAddressMap[s],0xFF);
    }
    return msg;
  }
  //********************************************* test i2c speed
  else if (tokens[0]=="i2cspeed" ){
 
    std::string msg = "";
    unsigned int value = 0;
    if( atoi(tokens[1].c_str()) == 100 ) value = 100;
    else if( atoi(tokens[1].c_str()) == 200 ) value = 200;
    else if( atoi(tokens[1].c_str()) == 400 ) value = 400;
    else if( atoi(tokens[1].c_str()) == 1000 ) value = 1000;
    else{
       msg = "Wrong value for the i2c speed, must be an integer and possible values are <100, 200, 400, 1000>";
       return msg;
    }
    setI2CSpeed(fecAccess,fecAddress, ringAddress, ccuAddress,channelAddress,value);
    return testScanRingEDevice (fecAccess, fecAddress, ringAddress) ;
  
  }
  //*********************************************
  else{
    return string("not an option! options: \[help] lists valid options");
  }
}

// void error(char *msg)
// {
//     perror(msg);
//     exit(1);
// }


// int openSocket(int port){
//   /* see  http://www.linuxhowtos.org/C_C++/socket.htm */ 
//      int sockfd, newsockfd;
//      socklen_t clilen;
//      //char buffer[256];

//      struct sockaddr_in serv_addr, cli_addr;
//      sockfd = socket(AF_INET, SOCK_STREAM, 0);
//      if (sockfd < 0) error("ERROR opening socket");
//      bzero((char *) &serv_addr, sizeof(serv_addr));
//      serv_addr.sin_family = AF_INET;
//      serv_addr.sin_addr.s_addr = INADDR_ANY;
//      serv_addr.sin_port = htons(port);
//      if (bind(sockfd, (struct sockaddr *) &serv_addr,
//               sizeof(serv_addr)) < 0) 
//               error("ERROR on binding");
//      listen(sockfd,5);
//      clilen = sizeof(cli_addr);
//      newsockfd = accept(sockfd, 
//                  (struct sockaddr *) &cli_addr, 
//                  &clilen);
//      if (newsockfd < 0) 
//           error("ERROR on accept");
//      cout << "accepted client connection" << endl; 
//      return newsockfd;
// }



int main(int argc, char *argv[])
{
  VMELock lock(1);
  lock.acquire();

  //Download all
  FecAccess *fecAccess  = NULL ; 
  

  unsigned int i2cSpeed = 100 ; 
  bool fack             = true  ;

  // Create the FEC Access
  int cnt; 
  
  // FEC Address and Ring Address
  unsigned int fecAddress = 0 ;
  unsigned int ringAddress = 0 ;
  unsigned int channelAddress = 0x10 ; 
  unsigned int ccuAddress = 0x6;
  long loop = 1 ;
  string sector = "-6P";
  string group = "-6PL12";

  readAddressMaps("./data/fec_ring_ccu_channel_group.txt");
  
  // Check: pci or vme mode
  if ((argc > 1) && ((strcasecmp (argv[1],"-pci") == 0))) { // If pci
    cout << "pci mode " << endl; 
    fecAddress = 0 ;
    ringAddress = 0 ;
  }
  else if ((argc > 1) && ((strcasecmp (argv[1],"-vmecaenusb") == 0))) { // If pci
    cout << "vmecaenusb mode " << endl; 
    fecAddress = 5 ;
    ringAddress = 0x7 ;
  }
  else if ((argc > 1) && ((strcasecmp (argv[1],"-vmecaenpci") == 0))) { // If pci
    cout << "vmecaenpci mode " << endl; 
    fecAddress = 9 ;
    ringAddress = 0x8 ;
  }
  else {
    cout << "Please select PCI [-pci] or VME [-vmecaenusb]/[-vmecaenpci] mode " << endl;
    return 0;
  }

  //create fec access
  try {
    fecAccess = createFecAccess ( argc, argv, &cnt, 20 ) ;  // fecslot
  }
  catch (FecExceptionHandler e) {
      
    cout << "------------ Exception ----------" << std::endl ;
    cout << e.what()  << std::endl ;
    cout << "---------------------------------" << std::endl ;
    exit (EXIT_FAILURE) ;
  }
  cout << "FecAccess created " << endl;

  fecAccess->setForceAcknowledge (fack) ;
  fecAccess->seti2cChannelSpeed (i2cSpeed) ;
  resetPlxFec ( fecAccess, fecAddress, ringAddress, loop, 0 );
  testScanCCU ( fecAccess, fecAddress, ringAddress, false );
  
  lock.release();
  
  // configure server
  //SimpleServer serv;
  MultiplexingServer serv;
  if(argc==4){
    if ((strcasecmp (argv[2],"-port") == 0)) {
      int port=atoi(argv[3]);
      cout << "port = " << port << endl;
      serv.open(port);
    }
  }

  
  while( serv.eventloop() ){
    string buffer;
  
    buffer=((SimpleCommand*) serv.getCmd())->getBuffer();

    //printf("buffer=%s\n",buffer.c_str());
    vector<string> tokens;
    tokenize(buffer,tokens,"");

    string response;

    //if(strcmp(buffer,"close")==0) break;

    if(tokens[0]=="fec"){
      if (tokens.size()>1) {
	fecAddress = atoi(tokens[1].c_str());
	cout << "FEC Address set to " << fecAddress << endl;
      }
      else 
	cout << "wrong number of parameters! fec needs 1 parameter" << endl;
    }
    else if(tokens[0]=="ring"){
      if (tokens.size()>1) {
	ringAddress = strtol (tokens[1].c_str(),NULL,16); 
	cout << "Ring Address set to " << std::dec << ringAddress << endl;
      }
      else 
	cout << "wrong number of parameters! ring needs 1 parameter" << endl;
    }
    else if(tokens[0]=="ccu"){
      if (tokens.size()>1) {
	ccuAddress = strtol (tokens[1].c_str(),NULL,16); 
	cout << "CCU Address set to 0x" << std::hex << ccuAddress << endl;
      }
      else 
	cout << "wrong number of parameters! ccu needs 1 parameter" << endl;
    }
    else if(tokens[0]=="channel"){
      if (tokens.size()>1) {
	channelAddress = strtol (tokens[1].c_str(),NULL,16); 
	cout << "Channel Address set to 0x" << std::hex << channelAddress << endl;
      } 
      else 
	cout << "wrong number of parameters! channel needs 1 parameter" << endl;
    }
    else if(tokens[0]=="loop"){
      if (tokens.size()>1) {
	loop = atoi(tokens[1].c_str());
	cout << "Loop set to " << loop << endl;
      }
      else 
	cout << "wrong number of parameters! loop needs 1 parameter" << endl;
    }
    else if(tokens[0]=="group"){
      if (tokens.size()>1) {
	
	string l_group=tokens[1];
	string l_sector=l_group.substr(0,3);
	if ((fecAddressMap[l_sector]==0) || (channelAddressMap[l_group]==0)){
	   cout << "invalid group description" << endl;
	}else{
	  group=l_group;
	  sector=l_sector;
	  fecAddress = fecAddressMap[sector]  ;
	  ringAddress = ringAddressMap[sector] ;
	  ccuAddress = ccuAddressMap[sector];
	  channelAddress = channelAddressMap[group] ; 
	  cout << "Group set to " << group << endl;
	}
      }else{
	cout << "wrong number of parameters! group needs 1 parameter" << endl;
      }
    }

    else if(tokens[0]=="sector"){
      if (tokens.size()>1) {
	string l_sector=tokens[1];
	if(fecAddressMap[l_sector]==0){
	   cout << "invalid sector description" << endl;
	}else{
	  sector=l_sector;
	  cout << "Sector set to " << sector << endl;
	  fecAddress = fecAddressMap[sector]  ;
	  ringAddress = ringAddressMap[sector] ;
	  ccuAddress = ccuAddressMap[sector];
	  channelAddress = channelAddressMap[group] ; 
	}
      }else{
	cout << "wrong number of parameters! sector needs 1 parameter" << endl;
      }
    
  

    // handle the request
    }else {
      lock.acquire();
      response=handle(tokens, sector, group, fecAccess, fecAddress, ringAddress, ccuAddress, channelAddress, loop);
      cout << response << endl;
      lock.release();
    }
  }

  lock.acquire();
  if (fecAccess != NULL) {
    try {
      delete fecAccess ;
    }
    catch (FecExceptionHandler e) {

      std::cerr << "------------ Exception ----------" << std::endl ;
      std::cerr << e.what() << " when deleting the FecAccess" << std::endl ;
      std::cerr << "---------------------------------" << std::endl ;
    }
  }
  lock.release();

  return 0;
}
