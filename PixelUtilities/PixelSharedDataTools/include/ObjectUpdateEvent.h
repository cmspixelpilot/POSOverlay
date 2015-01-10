#ifndef OBJECTUPDATEEVENT_H_
#define OBJECTUPDATEEVENT_H_

#include "SharedObject.h"

namespace pixel
{

/*
 * \class ObjectUpdateEvent
 * \brief The event given to ObjectUpdateListener s
 * 
 * The ObjectUpdateEvent is the event given to ObjectUpdateListeners
 * when a SharedObjectOwner calls fireObjectUpdatedEvent().  The
 * ObjectUpdateListeners may then use this event to get the
 * associated SharedObject and read the data from it.
 * <BR>
 * 
 * 
 */
template <class T>
class ObjectUpdateEvent
{
public:
	inline ObjectUpdateEvent(pixel::SharedObject<T>* object,void* originator) : originator_(originator), object_(object){};
	inline virtual ~ObjectUpdateEvent(){};

	//! Gets a read-only version of the data that was updated
	inline virtual const T* getData() const {return dynamic_cast<const T*>(object_->getData());}
	
	//! Gets a writable version of the data that was updated
	inline virtual T* getWritableData() const {return object_->getWritableAddress();}
	
	//! Gets the SharedObject that is associated with the data that was updated
	inline virtual pixel::SharedObject<T>* getSharedObject() const {return object_;}
private:
	void* originator_;//! The source of the event
	pixel::SharedObject<T>* object_;//! The shared object that was updated
};

}

#endif /*OBJECTUPDATEEVENT_H_*/
