// I2OTACredit.cc

#include "I2OTACredit.h"

I2OTACredit::I2OTACredit()
{

}

I2OTACredit::~I2OTACredit()
{

}

void I2OTACredit::initializeDefaultValues()
{
    I2OEventDataBlock::initializeDefaultValues();
    setNbCredits(0);
}

PI2O_TA_CREDIT_MESSAGE_FRAME I2OTACredit::getTACreditMessageFrame()
{
    return (PI2O_TA_CREDIT_MESSAGE_FRAME) getBuffer();
}

int I2OTACredit::getHeaderSize()
{
    return sizeof(I2O_TA_CREDIT_MESSAGE_FRAME);
}

U32 I2OTACredit::getNbCredits()
{
    return getTACreditMessageFrame()->nbCredits;
}

void I2OTACredit::setNbCredits(U32 NbCredits)
{
    getTACreditMessageFrame()->nbCredits = NbCredits;
}


std::ostream& operator<<(std::ostream& out, I2OTACredit &tacredit)
{
    I2OEventDataBlock &edb = tacredit;
    out << edb << std::endl;
    out << " I2OTACredit: " << std::endl;
    return out << "  nbCredits " <<tacredit.getNbCredits();
}
