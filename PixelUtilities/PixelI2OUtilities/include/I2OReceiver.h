// I2OReceiver.h

#ifndef I2O_RECEIVER_H
#define I2O_RECEIVER_H

#include "i2o/shared/i2omsg.h"
#include "toolbox/mem/Reference.h"

#include "I2OBase.h"
#include "I2OStandardMessage.h"
#include "I2OPrivateMessage.h"
#include "I2OEventDataBlock.h"
#include "I2OBUAllocate.h"
#include "I2OBUDiscard.h"
#include "I2OTACredit.h"

template<class T> 
class I2OReceiver : public T
{
public:
    I2OReceiver(toolbox::mem::Reference *ref, int bauto_release = 1);
    int next();
    virtual ~I2OReceiver();

// this method is only temporary (will be moved later)
    int getTrailingBufferSize();
};

template<class T>
std::ostream& operator<<(std::ostream &out, I2OReceiver<T> &receiver);


typedef I2OReceiver<I2OBase> I2OBaseReceiver;
typedef I2OReceiver<I2OStandardMessage> I2OStandardMessageReceiver;
typedef I2OReceiver<I2OPrivateMessage> I2OPrivateMessageReceiver;
typedef I2OReceiver<I2OEventDataBlock> I2OEventDataBlockReceiver;
typedef I2OReceiver<I2OBUAllocate> I2OBUAllocateReceiver;
typedef I2OReceiver<I2OBUDiscard> I2OBUDiscardReceiver;
typedef I2OReceiver<I2OTACredit> I2OTACreditReceiver;
#endif
