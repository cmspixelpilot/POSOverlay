
#ifndef _FIFO1Decoder_h_
#define _FIFO1Decoder_h_

#include "PixelUtilities/PixelFEDDataTools/include/AddressLevels.h"
#include "CalibFormats/SiPixelObjects/interface/PixelROCName.h"
#include <vector>
#include <map>
#include <string>
#include <stdint.h>


class FIFO1Decoder
{
 public:
    
    FIFO1Decoder(uint32_t *buffer, std::vector<pos::PixelROCName> ROCStringVector, 
                 Moments &UB, Moments &B, std::map<pos::PixelROCName, 
		 AddressLevels>& ROC_AddressLevels, AddressLevels &TBM_AddressLevels);

    FIFO1Decoder(uint32_t *buffer, unsigned int ROCStringVectorSize, Moments &UB, 
		 Moments &B, AddressLevels &TBM_AddressLevels);

    // Add the individual TBM&ROC UB (d.k. 26/8/08)
    FIFO1Decoder(uint32_t *buffer, std::vector<pos::PixelROCName> ROCStringVector, 
		 Moments &UB, Moments &B, std::map<pos::PixelROCName, 
		 AddressLevels>& ROC_AddressLevels, AddressLevels &TBM_AddressLevels,
		 Moments &UB_TBM, Moments &UB_ROC);

    FIFO1Decoder(uint32_t *buffer, unsigned int ROCStringVectorSize, Moments &UB, 
		 Moments &B, AddressLevels &TBM_AddressLevels,
		 Moments &UB_TBM, Moments &UB_ROC);
    
    bool valid(){return valid_;}
    unsigned int transparentDataStart (uint32_t *buffer);
    
 private:
    bool valid_;

};

#endif
