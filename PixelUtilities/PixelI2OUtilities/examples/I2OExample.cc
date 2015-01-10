// I2OExample.cc

#include "I2OExample.h"

I2OExample::I2OExample(xdaq::ApplicationStub *s)
    throw(xdaq::exception::Exception)
    : xdaq::WebApplication(s), Task("")
{
    // two instances from example.xml
    // send an i2o message from instance from tid 100 to tid 101
    if (i2o::utils::getAddressMap()->getTid(getApplicationDescriptor())==101)
    {
	i2o::bind(this,&I2OExample::callback, 1,  XDAQ_ORGANIZATION_ID);
    }
    else    
	activate();
}

I2OExample::~I2OExample()
{

}

int I2OExample::svc()
{
    int event_number = 1;
    while (1)
    {

	sleep(2);
	char buffer[100];
	sprintf(buffer," the current event is %d", event_number);
	int len = strlen(buffer)+1;
	if (len % 4)
	{
	    len += 4 - (len % 4);
	}
	
	I2OEventDataBlockSender s(this, len);
	
	s.setDestinationDescriptor(getApplicationContext()->
				   getApplicationGroup()->
				   getApplicationDescriptor("I2OExample",1));
	
	s.setXFunctionCode(1); // match call to bind
	s.setEventNumber(event_number++);
	strcpy((char*)s.getTrailingBuffer(),buffer);
	s.send();
    }
}

void I2OExample::callback(toolbox::mem::Reference *ref) throw(i2o::exception::Exception)
{
    cout << endl;
    cout << "Entering I2OExample::callback." <<endl;

    I2OEventDataBlockReceiver r(ref);
    cout << r << endl;


    cout << " The Trailing Buffer contains: " << (char*) r.getTrailingBuffer() << endl;
    cout << "Leaving I2OExample::callback." << endl;
}

XDAQ_INSTANTIATOR_IMPL(I2OExample)
