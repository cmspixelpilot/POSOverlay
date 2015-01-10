// I2OBUAllocate.h

#ifndef I2O_BU_ALLOCATE_H
#define I2O_BU_ALLOCATE_H

#include "I2OEventDataBlock.h"
#include "interface/shared/i2oXFunctionCodes.h"
#include "interface/evb/i2oEVBMsgs.h"

class I2OBUAllocate : public I2OEventDataBlock
{
public:
    I2OBUAllocate();
    virtual ~I2OBUAllocate();

    PI2O_BU_ALLOCATE_MESSAGE_FRAME getBUAllocateMessageFrame();
    virtual int getHeaderSize();
    virtual void initializeDefaultValues();
    
    U32 getN();
    void setN(U32 n);

    BU_ALLOCATE getBUAllocate();
    void setBUAllocate(BU_ALLOCATE bu_allocate);
};

std::ostream& operator<<(std::ostream& out, I2OBUAllocate &tacredit);
#endif
