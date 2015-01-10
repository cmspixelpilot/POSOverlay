#ifndef SHAREDMEMORYPOOLFACTORY_H_
#define SHAREDMEMORYPOOLFACTORY_H_
//superclass
#include <toolbox/mem/MemoryPoolFactory.h>

//core
#include <map>
#include <string>

//toolbox
#include <toolbox/mem/Reference.h>
#include <toolbox/mem/Pool.h>

namespace pixel
{

/**
 * \class SharedMemoryPoolFactory
 * \brief An extension of the toolbox::mem::MemoryPoolFactory of xdaq
 * 
 * The SharedMemoryPoolFactory provides mechanisms to generate 
 * named spots in memory that are accessible globally to an application.
 * <BR>
 * You use the SharedMemoryPoolFactory as follows:
 * \code
  //get a reference to a factory
  pixel::SharedMemoryPoolFactory* factory = pixel::SharedMemoryPoolFactory::getInstance();
  
  //make the pool
  toolbox::net::URN urn("toolbox-mem-pool", "MyPool");
  toolbox::mem::Pool* pool;
  //Get a reference to the memory
  toolbox::mem::CommittedHeapAllocator
 		* a = new toolbox::mem::CommittedHeapAllocator(size);//size=number of bytes to allocate
  pool = factory->createPool(urn,a);
  
  toolbox::mem::Reference* memoryReference = factory->getNamedFrame(pool, size, "MyObject");
  \endcode
   
   After you have done that, you can get a pointer to the memory anywhere
   else in the same xdaq process by doing the following:
   
   \code
  //get a reference to a factory
  pixel::SharedMemoryPoolFactory* factory = pixel::SharedMemoryPoolFactory::getInstance();
  
  //make the pool
  toolbox::net::URN urn("toolbox-mem-pool", "MyPool");
  toolbox::mem::Pool* pool;
  pool = factory->findPool(urn);
  
  toolbox::mem::Reference* memoryReference = factory->getNamedFrame(pool, size, "MyObject");
 * \endcode
 * 
 * All memory allocated by the same SharedMemoryFactory may be shared this way.  If you wish
 * to create a new SharedMemoryPoolFactory, you simply call getInstance() with a string
 * argument, ie, 
 * 
 * \code
	pixel::SharedMemoryPoolFactory* factory = pixel::SharedMemoryPoolFactory::getInstance("MyFactory");
   \endcode
   
 * instead of the void argument overload of get instance.  Then, to get a pointer
 * to this same factory in other code, you simply do the same thing.
 */
class SharedMemoryPoolFactory
{
public:
	static pixel::SharedMemoryPoolFactory* FACTORY;
	SharedMemoryPoolFactory();
	virtual ~SharedMemoryPoolFactory();

	static pixel::SharedMemoryPoolFactory* getInstance();
	static pixel::SharedMemoryPoolFactory* getInstance(std::string uniqueName);

	toolbox::mem::Reference* getNamedFrame(toolbox::mem::Pool*, size_t size, std::string name ) throw (toolbox::mem::exception::Exception);

	toolbox::mem::Pool* findPool (toolbox::net::URN &urn) throw (toolbox::mem::exception::MemoryPoolNotFound);
	toolbox::mem::Pool* createPool (toolbox::net::URN &urn, toolbox::mem::Allocator *allocator) throw (toolbox::mem::exception::DuplicateMemoryPool, toolbox::mem::exception::AllocatorNotFound, toolbox::mem::exception::FailedCreation);
	toolbox::mem::Pool* createPool (const std::string &name, toolbox::mem::Allocator *allocator) throw (toolbox::mem::exception::DuplicateMemoryPool, toolbox::mem::exception::AllocatorNotFound, toolbox::mem::exception::FailedCreation);
	toolbox::mem::Reference* getFrame(toolbox::mem::Pool *pool, size_t size) throw (toolbox::mem::exception::Exception);
	inline void removeReference(std::string name) {names_[name]->release();names_.erase(name);}
private:
	static toolbox::mem::MemoryPoolFactory* factory_;
	
	static std::map<std::string, pixel::SharedMemoryPoolFactory> factoryNames_;
	std::map<std::string, toolbox::mem::Reference*> names_;
	
};

}



#endif /*SHAREDMEMORYPOOLFACTORY_H_*/
