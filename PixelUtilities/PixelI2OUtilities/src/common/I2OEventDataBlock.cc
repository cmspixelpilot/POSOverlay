// I2OEventDataBlock.cc

#include "I2OEventDataBlock.h"

I2OEventDataBlock::I2OEventDataBlock()
{

}

I2OEventDataBlock::~I2OEventDataBlock()
{

}

void I2OEventDataBlock::initializeDefaultValues()
{
    I2OPrivateMessage::initializeDefaultValues();
    setEventNumber(0);
    setNbBlocksInSuperFragment(1);
    setBlockNb(0);
    setBuResourceId(0);
    setFuTransactionId(0);
    setSuperFragmentNb(1);
    setPadding(0);
}

PI2O_EVENT_DATA_BLOCK_MESSAGE_FRAME I2OEventDataBlock::getEventDataBlockMessageFrame()
{
    return (PI2O_EVENT_DATA_BLOCK_MESSAGE_FRAME) getBuffer();
}

int I2OEventDataBlock::getHeaderSize()
{
    return sizeof(I2O_EVENT_DATA_BLOCK_MESSAGE_FRAME);
}



U32 I2OEventDataBlock::getEventNumber()
{
    return getEventDataBlockMessageFrame()->eventNumber;
}

void I2OEventDataBlock::setEventNumber(U32 eventNumber)
{
    getEventDataBlockMessageFrame()->eventNumber = eventNumber;
}

U32 I2OEventDataBlock::getNbBlocksInSuperFragment()
{
    return getEventDataBlockMessageFrame()->nbBlocksInSuperFragment;
}

void I2OEventDataBlock::I2OEventDataBlock::setNbBlocksInSuperFragment(U32 nbBlocksInSuperFragment)
{
    getEventDataBlockMessageFrame()->nbBlocksInSuperFragment = nbBlocksInSuperFragment;
}

U32 I2OEventDataBlock::getBlockNb()
{
    return getEventDataBlockMessageFrame()->blockNb;
}

void I2OEventDataBlock::setBlockNb(U32 blockNb)
{
    getEventDataBlockMessageFrame()->blockNb = blockNb;
}

U32 I2OEventDataBlock::getEventId()
{
    return getEventDataBlockMessageFrame()->eventId;
}

void I2OEventDataBlock::setEventId(U32 eventId)
{
    getEventDataBlockMessageFrame()->eventId = eventId;
}

U32 I2OEventDataBlock::getBuResourceId()
{
    return getEventDataBlockMessageFrame()->buResourceId;
}

void I2OEventDataBlock::setBuResourceId(U32 buResourceId)
{
    getEventDataBlockMessageFrame()->buResourceId = buResourceId;
}

U32 I2OEventDataBlock::getFuTransactionId()
{
    return getEventDataBlockMessageFrame()->fuTransactionId;
}

void I2OEventDataBlock::setFuTransactionId(U32 fuTransactionId)
{
    getEventDataBlockMessageFrame()->fuTransactionId = fuTransactionId;
}

U32 I2OEventDataBlock::getNbSuperFragmentsInEvent()
{
    return getEventDataBlockMessageFrame()->nbSuperFragmentsInEvent;
}

void I2OEventDataBlock::setNbSuperFragmentsInEvent(U32 nbSuperFragmentsInEvent)
{
    getEventDataBlockMessageFrame()->nbSuperFragmentsInEvent = nbSuperFragmentsInEvent;
}

U32 I2OEventDataBlock::getSuperFragmentNb()
{
    return getEventDataBlockMessageFrame()->superFragmentNb;
}

void I2OEventDataBlock::setSuperFragmentNb(U32 superFragmentNb)
{
    getEventDataBlockMessageFrame()->superFragmentNb = superFragmentNb;
}

U32 I2OEventDataBlock::getPadding()
{
  assert(0);
  //padding removed from struct... don't know how to fix! (ryd)
  //  return getEventDataBlockMessageFrame()->padding;
  return 0;
}

void I2OEventDataBlock::setPadding(U32 padding)
{
  assert(0);
  //padding removed from struct... don't know how to fix! (ryd)
  //   getEventDataBlockMessageFrame()->padding = padding

}

std::ostream& operator<<(std::ostream& out, I2OEventDataBlock &event_data_block)
{
    I2OPrivateMessage &pm = event_data_block;
    out << pm << std::endl;
    out << " I2OEventDataBlock: " <<std::endl;
    return out << "  event_number " <<event_data_block.getEventNumber();
}
