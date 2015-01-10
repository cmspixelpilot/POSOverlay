// I2OReceiver.cc

#include "I2OReceiver.h"

template<class T>
I2OReceiver<T>::I2OReceiver(toolbox::mem::Reference *ref, int bauto_release)
{
    I2OBase::setReference(ref, bauto_release);
}

template<class T>
I2OReceiver<T>::~I2OReceiver()
{

}

template<class T>
int I2OReceiver<T>::next()
{
    toolbox::mem::Reference *ref = I2OBase::getReference();
    toolbox::mem::Reference *next;
    if (ref)
    {
	next = ref->getNextReference();
	if (next)
	{
	    ref->setNextReference(0);
	    I2OBase::setReference(next);
	    return 1;
	}
	else 
	    return 0;
    }
    else
	return 0;
}

// this method is only temporary (will be moved later)
template<class T>
int I2OReceiver<T>::getTrailingBufferSize()
{
  return 4*(int)I2OStandardMessage::getMessageSize()-I2OBase::getHeaderSize();
}

template<class T>
std::ostream& operator<<(std::ostream &out, I2OReceiver<T> &receiver)
{
    out << "I2O*Receiver: " << std::endl;
    T &t = receiver;
    return out << t;
}




//template I2OReceiver<I2OBase>;
template class I2OReceiver<I2OStandardMessage>;
template class I2OReceiver<I2OPrivateMessage>;
template class I2OReceiver<I2OEventDataBlock>;
template class I2OReceiver<I2OTACredit>;
template class I2OReceiver<I2OBUAllocate>;
template class I2OReceiver<I2OBUDiscard>;

template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OBase> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OStandardMessage> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OPrivateMessage> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OEventDataBlock> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OTACredit> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OBUAllocate> &receiver);
template std::ostream& operator<<(std::ostream &out, I2OReceiver<I2OBUDiscard> &receiver);


