// I2OEventDataBlock.h

#ifndef I2O_EVENT_DATA_BLOCK
#define I2O_EVENT_DATA_BLOCK

#include "I2OPrivateMessage.h"
#include "interface/evb/i2oEVBMsgs.h"

class I2OEventDataBlock : public I2OPrivateMessage
{
public:
    I2OEventDataBlock();
    virtual ~I2OEventDataBlock();
    
    virtual void initializeDefaultValues();
    PI2O_EVENT_DATA_BLOCK_MESSAGE_FRAME getEventDataBlockMessageFrame();
    virtual int getHeaderSize();

    U32 getEventNumber();
    void setEventNumber(U32 eventNumber);

    U32 getNbBlocksInSuperFragment();
    void setNbBlocksInSuperFragment(U32 nbBlocksInSuperFragment);

    U32 getBlockNb();
    void setBlockNb(U32 blockNb);

    U32 getEventId();
    void setEventId(U32 eventId);

    U32 getBuResourceId();
    void setBuResourceId(U32 buResourceId);

    U32 getFuTransactionId();
    void setFuTransactionId(U32 fuTransactionId);

    U32 getNbSuperFragmentsInEvent();
    void setNbSuperFragmentsInEvent(U32 nbSuperFragmentsInEvent);

    U32 getSuperFragmentNb();
    void setSuperFragmentNb(U32 superFragmentNb);

    U32 getPadding();
    void setPadding(U32 padding);
};


std::ostream& operator<<(std::ostream& out, I2OEventDataBlock &event_data_block);


#endif
