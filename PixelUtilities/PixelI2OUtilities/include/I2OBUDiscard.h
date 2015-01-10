// I2OBUDiscard.h

#ifndef I2O_BU_DISCARD_H
#define I2O_BU_DISCARD_H

#include "I2OEventDataBlock.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "interface/evb/i2oEVBMsgs.h"

class I2OBUDiscard : public I2OEventDataBlock
{
public:
    I2OBUDiscard();
    virtual ~I2OBUDiscard();

    PI2O_BU_DISCARD_MESSAGE_FRAME getBUDiscardMessageFrame();
    virtual int getHeaderSize();
    virtual void initializeDefaultValues();
    
    U32 getN();
    void setN(U32 N);

    U32 getBUResourceId();
    void setBUResourceId(U32 buResourceId);
};

std::ostream& operator<<(std::ostream& out, I2OBUDiscard &bu_discard);
#endif
