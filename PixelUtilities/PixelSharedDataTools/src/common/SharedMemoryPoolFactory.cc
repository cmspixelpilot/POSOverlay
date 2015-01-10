#include "SharedMemoryPoolFactory.h"

//debug
#include <iostream>

//toolbox
#include <toolbox/mem/Reference.h>
#include <toolbox/mem/Pool.h>
#include <toolbox/mem/exception/Exception.h>

namespace pixel
{

//static data members
toolbox::mem::MemoryPoolFactory* SharedMemoryPoolFactory::factory_ = toolbox::mem::getMemoryPoolFactory();
pixel::SharedMemoryPoolFactory* SharedMemoryPoolFactory::FACTORY = new SharedMemoryPoolFactory;
std::map<std::string, pixel::SharedMemoryPoolFactory> SharedMemoryPoolFactory::factoryNames_;

SharedMemoryPoolFactory::SharedMemoryPoolFactory()
{
	
}

/**
 * 
 * The destructor calls release on all references that have ever touched the object.
 */
SharedMemoryPoolFactory::~SharedMemoryPoolFactory()
{
	//std::map<std::string, toolbox::mem::Reference*>::iterator it;
	
	//for(it = names_.begin(); it!=names_.end();it++) {
	//	it->second->release();
	//}
}

toolbox::mem::Reference* SharedMemoryPoolFactory::getNamedFrame(toolbox::mem::Pool* pool, size_t size, std::string name) throw (toolbox::mem::exception::Exception){
	
	if(!(names_.find(name)==names_.end())) {
		return this->names_[name];
//		return this->names_[name]->duplicate(); //Matt: Removed duplicate method... that's what I believe caused the memory problems

	} else {
		//std::cout << "Getting a reference to a frame" <<std:: endl;
		
		toolbox::mem::Reference* ref = 0;
		ref = factory_->getFrame(pool, size);
		
		names_[name] = ref;
		
		return ref;
	}
}

toolbox::mem::Pool* SharedMemoryPoolFactory::findPool (toolbox::net::URN &urn) throw (toolbox::mem::exception::MemoryPoolNotFound) {
	return factory_->findPool(urn);
}

toolbox::mem::Pool* SharedMemoryPoolFactory::createPool (const std::string &name, toolbox::mem::Allocator *allocator) throw (toolbox::mem::exception::DuplicateMemoryPool, toolbox::mem::exception::AllocatorNotFound, toolbox::mem::exception::FailedCreation) {
	return factory_->createPool(name,allocator);
}

toolbox::mem::Pool* SharedMemoryPoolFactory::createPool (toolbox::net::URN &urn, toolbox::mem::Allocator *allocator) throw (toolbox::mem::exception::DuplicateMemoryPool, toolbox::mem::exception::AllocatorNotFound, toolbox::mem::exception::FailedCreation) {
	return factory_->createPool(urn,allocator);
}

toolbox::mem::Reference* SharedMemoryPoolFactory::getFrame(toolbox::mem::Pool *pool, size_t size) throw (toolbox::mem::exception::Exception) {
	return factory_->getFrame(pool,size);
}

/**
 * Returns the global instance of the singleton SharedMemoryPoolFactory.
 */
pixel::SharedMemoryPoolFactory* SharedMemoryPoolFactory::getInstance() {
	return FACTORY;
}

/**
 * This function gets a named instance of a SharedMemoryPoolFactory.  Every time that
 * you use this function with the same string argument, you are guaranteed that you get
 * the same instance of a SharedMemoryPoolFactory, whether or not you are in different xdaq
 * Applications.
 */
pixel::SharedMemoryPoolFactory* SharedMemoryPoolFactory::getInstance(std::string uniqueName) {
	
	return &(factoryNames_[uniqueName]);
}


}

