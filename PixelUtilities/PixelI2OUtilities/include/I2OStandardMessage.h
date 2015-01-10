// I2OStandardMessage.h

#ifndef I2O_STANDARD_MESSAGE_H
#define I2O_STANDARD_MESSAGE_H

#include "I2OBase.h"

class I2OStandardMessage : public I2OBase
{
public:
    I2OStandardMessage();
    virtual ~I2OStandardMessage();

    PI2O_MESSAGE_FRAME getStandardMessageFrame();
    virtual int getHeaderSize();
    virtual void initializeDefaultValues();
    
    U8    getVersionOffset();
    void  setVersionOffset(U8 VersionOffset);

    U8    getMsgFlags();
    void  setMsgFlags(U8 MessageFlags);

    U16   getMessageSize();
    void  setMessageSize(U16 MessageSize);

    BF    getTargetAddress();
    void  setTargetAddress(BF TargetAddress);

    BF    getInitiatorAddress();
    void  setInitiatorAddress(BF InitiatorAddress);

    BF    getFunction();
    void  setFunction(BF Function);

    I2O_INITIATOR_CONTEXT getInitiatorContext();
    void                  setInitiatorConext(I2O_INITIATOR_CONTEXT InitiatorContext);

};

std::ostream& operator<<(std::ostream& out, I2OStandardMessage &standard_message);
#endif
