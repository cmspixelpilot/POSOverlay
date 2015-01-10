#ifndef SHAREDOBJECT_H_
#define SHAREDOBJECT_H_

//Core imports
#include <string>
#include <map>

//toolbox
#include <toolbox/mem/exception/Exception.h>
#include <toolbox/mem/Reference.h>
#include <toolbox/net/URN.h>
#include <toolbox/mem/CommittedHeapAllocator.h>
#include <toolbox/mem/HeapAllocator.h>

//pixel 
#include "SharedMemoryPoolFactory.h"

namespace pixel
{

/** \brief Allows for shared memory access in a simple way.
 * 
 * The SharedObject utilitzes xdaq's built-in memory management features to
 * allow the user to create objects that multiple xdaq applications can share.
 * 
 * You have two choices for thread safety with the SharedObject.
 * <br/>
 * #1: Use the pixel::SharedObjectOwner class to serialize access to the data.
 * <br/>
 * #2: Use your own Mutex or Semaphore to handle access
 * <br/>
 * 
 * To use this class, you simply construct shared object
 * with the name that you wish other applications to access
 * your information with.  After that, you may call getData()
 * or getWritableData() in order to get a pointer that you may
 * use to reference your data object.  If you wish, you may
 * call putObject, which basically acts like a copy constructor
 * for your object.  See the function documentation before
 * using it however!
 * 
 * 
 */
template <class T>
class SharedObject
{
public:
	SharedObject(std::string name, unsigned int size=0, std::string memPoolName = "SharedObjectPool") throw (toolbox::mem::exception::Exception);
	virtual ~SharedObject();
	
	//! See whether or not a shared object by that name exists already
	static bool exists(std::string name, std::string memPoolName = "SharedObjectPool");
	
	//! Use this function for reading data only.
	inline const T* getData() {return dynamic_cast<const T*>(object_);};
	
	//! Gets an address that may be used to edit the object.
	inline T* getWritableAddress(){return object_;}
	
	//! Release the storage
	inline void release();
	
	//! Get the name of the shared object
	inline std::string getName() {return objectName_;}
	
	//! Returns the reference for the memory
	inline toolbox::mem::Reference& getReference(){return ref_;}
	/**
	 * Assigns the object pointed to by obj to the object pointed to
	 * by this SharedObject.  
	 * <b>This function deletes whatever the pointer
	 * is pointing to when it is done!</b>  
	 * It is done this way to support the syntax putObject(new MyObject(args)).
	 * 
	 * If you don't want it to delete what that pointer is pointing to, pass
	 * false to the second argument.
	 */
	inline void putObject(T* obj, bool deleteObj = true) {*object_ = (*obj); if(deleteObj){ delete obj;}}
	
	inline unsigned int getSize() {return size_;}

private:
	static std::map<std::string, int> references_;
	
	unsigned int size_;
	std::string objectName_;
	std::string memPoolName_;
	
	T* object_;
	toolbox::mem::Reference ref_;
	bool released_;
	pixel::SharedMemoryPoolFactory* factory_;

};

/**
 * Holds the number of references for each shared object name
 */
template <class T>
std::map<std::string, int> SharedObject<T>::references_;

template <class T>
SharedObject<T>::SharedObject(std::string name, unsigned int size, std::string memPoolName) throw (toolbox::mem::exception::Exception): objectName_(name),memPoolName_(memPoolName), released_(false)
{
	//Get an instance of the factory.
	factory_ = pixel::SharedMemoryPoolFactory::getInstance();

	//Keep a count of the number of references made to that particular memory spot
	references_[memPoolName + objectName_]++;

	//allocate storage, if necessary... let the memory pool figure that out
	pixel::SharedMemoryPoolFactory* factory = pixel::SharedMemoryPoolFactory::getInstance(memPoolName);

	//Figure out the amount of memory we need to allocate

	if(size<=0)
		size = sizeof(T);

	size_ = size;

	//make the pool
	toolbox::net::URN urn("toolbox-mem-pool", memPoolName_);
	toolbox::mem::Pool* pool;

	try{
		//look to see if that name for a pool already exists
		pool = factory->findPool(urn);

	} catch(toolbox::mem::exception::MemoryPoolNotFound& e) {
		//fine... allocate another pool
		//toolbox::mem::CommittedHeapAllocator
		//* a = new toolbox::mem::CommittedHeapAllocator(size << 4);//Allocate more than necessary just in case
		toolbox::mem::HeapAllocator* a = new toolbox::mem::HeapAllocator;
		pool = factory->createPool(urn, a);
	}

	//Get the reference to the memory
	toolbox::mem::Reference* ref = 0;
	ref = factory->getNamedFrame(pool, size, name);

	//Set the size of the data
	ref->setDataSize(size);

	//Finally, point the pointer to the right spot
	object_ = (T*) ref->getDataLocation();

	ref_ = *ref;
	//object_ now points the the object (success!)
}

template <class T>
SharedObject<T>::~SharedObject()
{

	//std::cout << "Destroying shared Object " << std::endl;
	//If nothing else is referring to the object, delete it
	//references_[memPoolName_ + objectName_]--;
	//if((references_[memPoolName_ + objectName_]==0)) {
	//delete object_;
	//delete &ref_;
	//release();
	//}
}

template <class T>
void SharedObject<T>::release() {

	factory_->removeReference(objectName_);
	released_=true;


}

/**
 * Returns true if there already exists a shared object by the specified name
 * in the specified memory pool
 */
template <class T>
bool SharedObject<T>::exists(std::string name, std::string memPoolName) {

	if(!(references_.find(memPoolName + name)==references_.end())) {
		return false;
	} else {
		return true;
	}
}

}


//#include "../src/common/SharedObject.inl"
#endif /*SHAREDOBJECT_H_*/
