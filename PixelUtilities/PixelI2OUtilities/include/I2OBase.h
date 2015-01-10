// I2OBase.h

#ifndef I2O_BASE_H
#define I2O_BASE_H

#include "i2o/shared/i2omsg.h"
#include "toolbox/mem/Reference.h"

class I2OBase
{
public:
    I2OBase();
    virtual ~I2OBase();

    void* getBuffer();
    void* getTrailingBuffer();

    virtual void initializeDefaultValues();

    /*
     * Size in BYTES!!!!!
     *
     */
    virtual int getHeaderSize();

protected:
    void empty();
    void  setBuffer(void* buffer);
    void  setReference(toolbox::mem::Reference *ref, int bauto_release = 1);
    toolbox::mem::Reference *getReference();
private:
    toolbox::mem::Reference *m_ref;
    void* m_buffer;
    int m_bauto_release;
};

std::ostream& operator<<(std::ostream& out, I2OBase &base);


#endif
