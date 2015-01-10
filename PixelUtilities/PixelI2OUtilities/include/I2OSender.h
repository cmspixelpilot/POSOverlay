// I2OSender.h

#ifndef I2O_SENDER_H
#define I2O_SENDER_H

#include "I2OBase.h"
#include "I2OStandardMessage.h"
#include "I2OPrivateMessage.h"
#include "I2OEventDataBlock.h"
#include "I2OBUAllocate.h"
#include "I2OBUDiscard.h"
#include "I2OTACredit.h"

#include "xdaq/Application.h"
#include "toolbox/mem/MemoryPoolFactory.h"
#include "toolbox/mem/CommittedHeapAllocator.h"
#include "i2o/utils/AddressMap.h"

template<class T>
class I2OSender : public T
{
public:
    I2OSender(xdaq::Application *app,
	      int trailing_siz,
	      toolbox::mem::Pool *memory_pool,
	      int ballocate = 1);
    I2OSender(xdaq::Application *app, int trailing_size = 0, int memory_pool_size = 0, int ballocate = 1);
    virtual ~I2OSender();

    static toolbox::mem::Pool *CreateDefaultMemoryPool(xdaq::Application *app, int trailing_size, int nblocks = 100);

    void setDestinationDescriptor(xdaq::ApplicationDescriptor *destination_descriptor);
	      
    void allocate(int trailing_buffer_size = 0);

    void send();
private:
    static std::string getDefaultMemoryPoolName();
    xdaq::Application *m_app;
    xdaq::ApplicationDescriptor *m_destination_descriptor;
    toolbox::mem::Pool *m_memory_pool;
};

std::string I2OSenderGetDefaultMemoryPoolName(xdaq::Application *app);
toolbox::mem::Pool *I2OSenderCreateDefaultMemoryPool(xdaq::Application *app, 
					      int memory_pool_size);

typedef I2OSender<I2OPrivateMessage> I2OPrivateMessageSender;
typedef I2OSender<I2OEventDataBlock> I2OEventDataBlockSender;
typedef I2OSender<I2OBUAllocate> I2OBUAllocateSender;
typedef I2OSender<I2OBUDiscard> I2OBUDiscardSender;
typedef I2OSender<I2OTACredit> I2OTACreditSender;
#endif
