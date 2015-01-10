// I2OTACredit.h

#ifndef I2O_TA_CREDIT_H
#define I2O_TA_CREDIT_H

#include "I2OEventDataBlock.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "interface/evb/i2oEVBMsgs.h"

class I2OTACredit : public I2OEventDataBlock
{
public:
    I2OTACredit();
    virtual ~I2OTACredit();

    PI2O_TA_CREDIT_MESSAGE_FRAME getTACreditMessageFrame();
    virtual int getHeaderSize();
    virtual void initializeDefaultValues();
    
    U32 getNbCredits();
    void setNbCredits(U32 NbCredits);
};

std::ostream& operator<<(std::ostream& out, I2OTACredit &tacredit);
#endif
