// I2OStandardMessage.cc

#include "I2OStandardMessage.h"

I2OStandardMessage::I2OStandardMessage()
{

}

I2OStandardMessage::~I2OStandardMessage()
{

}

void I2OStandardMessage::initializeDefaultValues()
{
    I2OBase::initializeDefaultValues();
    setVersionOffset(0);
    setMsgFlags(0);
    setTargetAddress(0);
    setInitiatorAddress(0);
    setMessageSize(getHeaderSize() >> 2);
    setFunction(I2O_PRIVATE_MESSAGE);
}

PI2O_MESSAGE_FRAME I2OStandardMessage::getStandardMessageFrame()
{
    return (PI2O_MESSAGE_FRAME) getBuffer();
}

int I2OStandardMessage::getHeaderSize()
{
    return sizeof(I2O_MESSAGE_FRAME);
}

U8 I2OStandardMessage::getVersionOffset()
{
    return getStandardMessageFrame()->VersionOffset;
}

void I2OStandardMessage::setVersionOffset(U8 VersionOffset)
{
    getStandardMessageFrame()->VersionOffset = VersionOffset;
}

U8 I2OStandardMessage::getMsgFlags()
{
    return getStandardMessageFrame()->MsgFlags;
}

void I2OStandardMessage::setMsgFlags(U8 MsgFlags)
{
    getStandardMessageFrame()->MsgFlags = MsgFlags;
}

U16 I2OStandardMessage::getMessageSize()
{
    return getStandardMessageFrame()->MessageSize;
}

void I2OStandardMessage::setMessageSize(U16 MessageSize)
{
    getStandardMessageFrame()->MessageSize = MessageSize;
}

BF I2OStandardMessage::getTargetAddress()
{
    return getStandardMessageFrame()->TargetAddress;
}

void I2OStandardMessage::setTargetAddress(BF TargetAddress)
{
    getStandardMessageFrame()->TargetAddress = TargetAddress;
}

BF I2OStandardMessage::getInitiatorAddress()
{
    return getStandardMessageFrame()->InitiatorAddress;
}

void I2OStandardMessage::setInitiatorAddress(BF InitiatorAddress)
{
    getStandardMessageFrame()->InitiatorAddress = InitiatorAddress;
}

BF I2OStandardMessage::getFunction()
{
    return getStandardMessageFrame()->Function;
}

void I2OStandardMessage::setFunction(BF Function)
{
    getStandardMessageFrame()->Function = Function;
}

std::ostream& operator<<(std::ostream& out, I2OStandardMessage &standard_message)
{
    I2OBase &b = standard_message;
    out << b << std::endl;
    out << " I2OStandardMessage:" <<std::endl;
    out << "  message size is (32 bit words) " << standard_message.getMessageSize() << std::endl;
    out << "  initiator address (fid) " << standard_message.getInitiatorAddress() << std::endl;
    return out << "  target address (tid) " << standard_message.getTargetAddress();
}
