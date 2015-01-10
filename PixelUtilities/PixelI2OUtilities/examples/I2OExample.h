// I2OExample.h

#ifndef I2O_EXAMPLE_H
#define I2O_EXAMPLE_H

#include "xdaq/WebApplication.h"
#include "I2OReceiver.h"
#include "i2o.h"
#include "i2o/Method.h"
#include "toolbox/mem/Reference.h"
#include "Task.h"

#include "I2OReceiver.h"
#include "I2OSender.h"

class I2OExample : public xdaq::WebApplication , public Task
{
public:
    XDAQ_INSTANTIATOR();

    I2OExample(xdaq::ApplicationStub *s) throw(xdaq::exception::Exception);
    ~I2OExample();

    int svc();

//    void defaultWebPage(xgi::Input *in, xgi::Output *out) throw (xgi::exception::Exception);

    void callback(toolbox::mem::Reference *ref) throw(i2o::exception::Exception);
};


#endif
