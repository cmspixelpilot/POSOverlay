// I2OBase.cc

#include "I2OBase.h"
#include <iostream>
I2OBase::I2OBase()
{
    m_ref = 0;
    m_buffer = 0;
    m_bauto_release = 0;
}

I2OBase::~I2OBase()
{
    empty();
}

void I2OBase::empty()
{
    if (m_bauto_release)
    {
	if (m_ref)
	{
	    m_ref->release();
	}
	else
	    ; // error?

    }
    m_ref = 0;
}

void* I2OBase::getBuffer()
{
    return m_buffer;
}

void* I2OBase::getTrailingBuffer()
{
    return (void*)((intptr_t)getBuffer() + getHeaderSize());
}

void I2OBase::initializeDefaultValues()
{
    // no buffer so nothing to do
}

int I2OBase::getHeaderSize()
{
    return 0;
}
void I2OBase::setBuffer(void *buffer)
{
    m_buffer = buffer;
}

void I2OBase::setReference(toolbox::mem::Reference *ref, int bauto_release)
{
    if (m_ref)
	empty();
    m_ref = ref;
    m_bauto_release = bauto_release;
    m_buffer = ref->getDataLocation();
}

toolbox::mem::Reference *I2OBase::getReference()
{
    return m_ref;
}

std::ostream& operator<<(std::ostream &out, I2OBase &base)
{
    return out << "I2OBase";
}


