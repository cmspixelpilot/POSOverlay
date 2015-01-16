#include <iostream>
#include <time.h>
#include "VMEDevice.hh"
#include "VMEAddressTable.hh"
#include "VMEAddressTableASCIIReader.hh"
#include "CAENLinuxBusAdapter.hh"

using namespace std;

int main(int argc, char** argv) {
  try {
    unsigned long fedBase = 0;
  
    if( (argc != 2) ) {cout<<" Usage FecStatus <baseaddress or slotnumber>"<<endl; return 0;}
    sscanf(argv[1], "%lx", &fedBase);

    if (fedBase < 0x100) {
      fedBase = fedBase/16*10 + fedBase % 16;
      cout << "interpeting " << fedBase << " as slot number\n";
      fedBase = fedBase << 27;
    }
    cout<<"If this fails, keep in mind the translation (bit shift) from slot to base: slot 6 // base 0x30000000"<<endl; 
    cout<<"for the base address you entered 0x"<<hex<<fedBase<<dec<<" I think your FEC is in slot "<<(fedBase>>27)<<endl<<endl;

    HAL::CAENLinuxBusAdapter busAdapter(HAL::CAENLinuxBusAdapter::V2718, 0, 0, HAL::CAENLinuxBusAdapter::A3818);
    HAL::VMEAddressTableASCIIReader addressTableReader("PFECAddressMap.dat");
    HAL::VMEAddressTable addressTable("Test address table", addressTableReader);
    HAL::VMEDevice PixFECCard(addressTable, busAdapter, fedBase);
  
    uint32_t data = 0x80000000;
    PixFECCard.read("STATUS", &data);
    cout<<endl<<"FEC Status!! Hurray Status!!!: 0x"<<hex<<data<<dec<<endl;

    if((data & 0x2) && !(data & 0x4)){
      cout<<endl<<"*************************************"<<endl;
      cout<<"Even better, the QPLL is locked!!!!!!"<<endl;
      cout<<"*************************************"<<endl<<endl;

    }

  } catch ( HAL::HardwareAccessException& e ) {
    cout << "*** Exception occurred : " << endl;
    cout << e.what() << endl;
  } catch ( exception e ) {
    cout << "*** Unknown exception occurred" << endl;
  }

  return 0;
}
