// I2OSender.cc

#include "I2OSender.h"
   
template<class T>
I2OSender<T>::I2OSender(xdaq::Application *app,
			int trailing_size,
			toolbox::mem::Pool *memory_pool,
			int ballocate)
{
    m_app = app;
    m_memory_pool = memory_pool;
    if (ballocate)
	allocate(trailing_size);
}

template<class T>
I2OSender<T>::I2OSender(xdaq::Application *app, int trailing_size, int memory_pool_nblocks, int ballocate)
{
    try {
	m_app = app;
	toolbox::net::URN urn("toolbox-mem-pool", I2OSenderGetDefaultMemoryPoolName(app));
	try {
	    m_memory_pool = toolbox::mem::getMemoryPoolFactory()->findPool(urn);
	}
	catch (toolbox::mem::exception::MemoryPoolNotFound &e)
	{
	    if (memory_pool_nblocks)
		m_memory_pool = CreateDefaultMemoryPool(m_app,trailing_size, memory_pool_nblocks);
	    else
		m_memory_pool = CreateDefaultMemoryPool(m_app, trailing_size);
	}
	if (ballocate)
	    allocate(trailing_size);
    }
    catch (xcept::Exception &e)
    {
	std::cout << "error " << e.what() << std::endl;
	throw;
    }
}

template<class T>
I2OSender<T>::~I2OSender()
{
    // ??
}

template<class T>
void I2OSender<T>::setDestinationDescriptor(xdaq::ApplicationDescriptor *destination_descriptor)
{
    try {
	m_destination_descriptor = destination_descriptor;
	this->setTargetAddress(i2o::utils::getAddressMap()->getTid(m_destination_descriptor));
    }
    catch (xcept::Exception &e)
    {
	std::cout << "error " << e.what() << std::endl;
	throw;
    }
}

template<class T>
void I2OSender<T>::allocate(int trailing_buffer_size)
{
    try {
	if (m_memory_pool)
	{
	    int size = I2OBase::getHeaderSize()+trailing_buffer_size;
	    toolbox::mem::Reference *ref = 
		toolbox::mem::getMemoryPoolFactory()->getFrame(m_memory_pool,size);
	    I2OBase::setReference(ref,0);
	    I2OBase::initializeDefaultValues();
	    if (size % 4 != 0)
	    {
		XCEPT_RAISE(xcept::Exception, " size must be a multiple of 4 bytes  ");
	    }
	    I2OStandardMessage::setMessageSize(size >> 2);
	    ref->setDataSize(size);
	    if (((size >> 2) << 2) != size)
	    {
		std::cout << size << " " <<(size>>2) 
			  << std::endl << *this<<std::endl;
		std::cout << trailing_buffer_size<<std::endl;
	    }
	    this->setInitiatorAddress(i2o::utils::getAddressMap()->getTid(m_app->getApplicationDescriptor()));
	}
	else
	{
	    ; // ?? 
	}
    }
    catch (xcept::Exception &e)
    {
	std::cout << "error " << e.what()<<std::endl;
	throw;
    }
}

template<class T>
void I2OSender<T>::send()
{
    try {
	m_app->getApplicationContext()->postFrame(I2OBase::getReference(),
						  m_app->getApplicationDescriptor(),
						  m_destination_descriptor);
    }
    catch (xcept::Exception &e)
    {
	std::cout << "error " << e.what() << std::endl;
	throw;
    }
}

template<class T>
toolbox::mem::Pool *I2OSender<T>::CreateDefaultMemoryPool(xdaq::Application *app, int trailing_size, int nblocks)
{
    T t;
    return I2OSenderCreateDefaultMemoryPool(app, (trailing_size+t.getHeaderSize()) * nblocks);
}

std::string I2OSenderGetDefaultMemoryPoolName(xdaq::Application *app)
{
    std::ostringstream str;
    str << "memory pool for xdaq::Application = 0x" << app ;
    return str.str();    
}

toolbox::mem::Pool *I2OSenderCreateDefaultMemoryPool(xdaq::Application *app, int memory_pool_size)
{
    try {
    toolbox::mem::CommittedHeapAllocator* a =
	new toolbox::mem::CommittedHeapAllocator(memory_pool_size);
    toolbox::net::URN urn("toolbox-mem-pool", I2OSenderGetDefaultMemoryPoolName(app));
    return toolbox::mem::getMemoryPoolFactory()->createPool(urn, a);
    }
    catch (xcept::Exception &e)
    {
	std::cout << "error " << e.what() << std::endl;
	throw;
    }
}

template class I2OSender<I2OPrivateMessage>;
template class I2OSender<I2OEventDataBlock>;
template class I2OSender<I2OBUAllocate>;
template class I2OSender<I2OBUDiscard>;
template class I2OSender<I2OTACredit>;
