// I2OBUDiscard.cc'

#include "I2OBUDiscard.h"

I2OBUDiscard::I2OBUDiscard()
{

}

I2OBUDiscard::~I2OBUDiscard()
{

}

void I2OBUDiscard::initializeDefaultValues()
{
    I2OEventDataBlock::initializeDefaultValues();
    setXFunctionCode(I2O_BU_DISCARD);
    setN(1);
    setBUResourceId(0);

}

PI2O_BU_DISCARD_MESSAGE_FRAME I2OBUDiscard::getBUDiscardMessageFrame()
{
    return (PI2O_BU_DISCARD_MESSAGE_FRAME) getBuffer();
}

int I2OBUDiscard::getHeaderSize()
{
    return sizeof(I2O_BU_DISCARD_MESSAGE_FRAME);
}

U32 I2OBUDiscard::getN()
{
    return getBUDiscardMessageFrame()->n;
}

void I2OBUDiscard::setN(U32 n)
{
    getBUDiscardMessageFrame()->n = n;
}

U32 I2OBUDiscard::getBUResourceId()
{
    // only one BUResourceID for now...
    return getBUDiscardMessageFrame()->buResourceId[0];
}

void I2OBUDiscard::setBUResourceId(U32 buResourceId)
{
    getBUDiscardMessageFrame()->buResourceId[0] = buResourceId;
}

std::ostream& operator<<(std::ostream& out, I2OBUDiscard &bu_discard)
{
    I2OEventDataBlock &edb = bu_discard;
    out << edb << std::endl;
    return out << " I2OBUCredit " ;
}
