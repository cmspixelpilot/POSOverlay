// I2OBUAllocate.cc

#include "I2OBUAllocate.h"

I2OBUAllocate::I2OBUAllocate()
{

}

I2OBUAllocate::~I2OBUAllocate()
{

}

void I2OBUAllocate::initializeDefaultValues()
{
    I2OEventDataBlock::initializeDefaultValues();
    setXFunctionCode(I2O_BU_ALLOCATE);
    setN(1);
    BU_ALLOCATE b;
    memset(&b, 0, sizeof(b));
    setBUAllocate(b);
}

PI2O_BU_ALLOCATE_MESSAGE_FRAME I2OBUAllocate::getBUAllocateMessageFrame()
{
    return (PI2O_BU_ALLOCATE_MESSAGE_FRAME) getBuffer();
}

int I2OBUAllocate::getHeaderSize()
{
    return sizeof(I2O_BU_ALLOCATE_MESSAGE_FRAME);
}

U32 I2OBUAllocate::getN()
{
    return getBUAllocateMessageFrame()->n;
}

void I2OBUAllocate::setN(U32 n)
{
    getBUAllocateMessageFrame()->n = n;
}

BU_ALLOCATE I2OBUAllocate::getBUAllocate()
{
    // only dealing with one BU_ALLOCATE for now....
    return getBUAllocateMessageFrame()->allocate[0];
}

void I2OBUAllocate::setBUAllocate(BU_ALLOCATE bu_allocate)
{
    // only dealing with one BU_ALLOCATE for now....
    getBUAllocateMessageFrame()->allocate[0] = bu_allocate;
}

std::ostream& operator<<(std::ostream& out, I2OBUAllocate &bu_allocate)
{
    I2OEventDataBlock &edb = bu_allocate;
    out << edb << std::endl;
    return out << " I2OBUAllocate ";
}
