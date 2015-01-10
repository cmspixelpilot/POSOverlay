// I2OPrivateMessage.h

#ifndef I2O_PRIVATE_MESSAGE_H
#define I2O_PRIVATE_MESSAGE_H

#include "I2OStandardMessage.h"
#include "i2o/Method.h"

class I2OPrivateMessage : public I2OStandardMessage
{
public:
    I2OPrivateMessage();
    virtual ~I2OPrivateMessage();

    virtual void initializeDefaultValues();

    PI2O_PRIVATE_MESSAGE_FRAME getPrivateMessageFrame();
    virtual int getHeaderSize();


    I2O_TRANSACTION_CONTEXT getTransactionContext();
    void setTransactionContext(I2O_TRANSACTION_CONTEXT TransactionContext);

    U16 getXFunctionCode();
    void setXFunctionCode(U16 XFunctionCode);
    
    U16 getOrganizationID();
    void setOrganizationID(U16 OrganizationID);
};

std::ostream& operator<<(std::ostream& out, I2OPrivateMessage &private_message);

#endif
