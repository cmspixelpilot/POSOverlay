// I2OPrivateMessage.cc

#include "I2OPrivateMessage.h"

I2OPrivateMessage::I2OPrivateMessage()
{

}

I2OPrivateMessage::~I2OPrivateMessage()
{

}

PI2O_PRIVATE_MESSAGE_FRAME I2OPrivateMessage::getPrivateMessageFrame()
{
    return (PI2O_PRIVATE_MESSAGE_FRAME) getBuffer();
}

void I2OPrivateMessage::initializeDefaultValues()
{
    I2OStandardMessage::initializeDefaultValues();
    I2O_TRANSACTION_CONTEXT tc;
    memset(&tc,0,sizeof(I2O_TRANSACTION_CONTEXT));
    setTransactionContext(tc);
    setXFunctionCode(0);
    setOrganizationID(XDAQ_ORGANIZATION_ID);
}

int I2OPrivateMessage::getHeaderSize()
{
    return sizeof(I2O_PRIVATE_MESSAGE_FRAME);
}


I2O_TRANSACTION_CONTEXT I2OPrivateMessage::getTransactionContext()
{
    return getPrivateMessageFrame()->TransactionContext;
}

void I2OPrivateMessage::setTransactionContext(I2O_TRANSACTION_CONTEXT TransactionContext)
{
    getPrivateMessageFrame()->TransactionContext = TransactionContext;
}

U16 I2OPrivateMessage::getXFunctionCode()
{
    return getPrivateMessageFrame()->XFunctionCode;
}

void I2OPrivateMessage::setXFunctionCode(U16 XFunctionCode)
{
    getPrivateMessageFrame()->XFunctionCode = XFunctionCode;
}
    
U16 I2OPrivateMessage::getOrganizationID()
{
    return getPrivateMessageFrame()->OrganizationID;
}

void I2OPrivateMessage::setOrganizationID(U16 OrganizationID)
{
    getPrivateMessageFrame()->OrganizationID = OrganizationID;
}

std::ostream& operator<<(std::ostream& out, I2OPrivateMessage &private_message)
{
    I2OStandardMessage &sm = private_message;
    out << sm << std::endl;
    return out << " I2OPrivateMessage: XFunctionCode " << private_message.getXFunctionCode();
}
