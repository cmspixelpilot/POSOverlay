//PixelSLinkData.h

#ifndef PIXEL_SLINK_DATA
#define PIXEL_SLINK_DATA

#include <ostream>
#include <stdint.h>
#include <vector>
#include <iostream>

//#include <execinfo.h>
#include <stdlib.h>

namespace pixel {
    typedef unsigned int word32;
    typedef uint64_t word64;
    
    class Word32 {
    public:
	Word32(word32 value=0);
	Word32(int value);
	bool is_filler();
	bool is_error();
	int  getBit(int bit);
	void setBit(int bit, int value);
	int  getBits(int low_bit_position, int nbits);
	void setBits(int low_bit_position, int nbits, int value);
    
    word32 m_value;
    };
    
    class Word64 {
    public:
	Word64(word64 value=0);
	Word64(word32 up, word32 down);
	bool is_trailer();
	bool is_header();

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//operator word64() {return m_value;}
	word64 cast_to_word64(){return m_value;}
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	bool read(std::istream &in);
	Word32& up() {return ((Word32*)&m_value)[1];}
	Word32& down() {return ((Word32*)&m_value)[0];}
	word64 m_value;
    };
    
    class SLinkHeader {
	public:
	SLinkHeader(word64 header=0);
	SLinkHeader(word32 up, word32 down);
	static int _getBOEPosition(){return 60-32;}
	static int _getBOELength(){return 4;}
	int  getBOE() {return m_header.up().getBits(_getBOEPosition(),_getBOELength());}
	void setBOE(int value){m_header.up().setBits(_getBOEPosition(),_getBOELength(),value);}

	static int _getEvt_tyPosition(){return 56-32;}
	static int _getEvt_tyLength(){return 4;}
	int  getEvt_ty() {return m_header.up().getBits(_getEvt_tyPosition(),_getEvt_tyLength());}
	void setEvt_ty(int value) {m_header.up().setBits(_getEvt_tyPosition(),_getEvt_tyLength(),value);}
	
	static int _getLV1_idPosition(){return 32-32;}
	static int _getLV1_idLength(){return 24;}
	int  getLV1_id() {return m_header.up().getBits(_getLV1_idPosition(),_getLV1_idLength());}
	void setLV1_id(int value) {return m_header.up().setBits(_getLV1_idPosition(),_getLV1_idLength(),value);}
	
	static int _getBX_idPosition(){return 20;}
	static int _getBX_idLength(){return 12;}
	int  getBX_id() {return m_header.down().getBits(_getBX_idPosition(),_getBX_idLength());}
	void setBX_id(int value) {return m_header.down().setBits(_getBX_idPosition(),_getBX_idLength(),value);}
	
	static int _getSource_idPosition(){return 8;}
	static int _getSource_idLength(){return 12;}
	int  getSource_id() {return m_header.down().getBits(_getSource_idPosition(),_getSource_idLength());}
	void setSource_id(int value) {return m_header.down().setBits(_getSource_idPosition(),_getSource_idLength(),value);}

	static int _getFOV_Position(){return 4;}
	static int _getFOV_Length(){return 4;}
	int getFOV(){return m_header.down().getBits(_getFOV_Position(),_getFOV_Length());}
	void setFOV(int value){m_header.down().setBits(_getFOV_Position(),_getFOV_Length(),value);}

	static int _getH_Position(){return 3;}
	static int _getH_Length(){return 1;}
	int getH(){return m_header.down().getBits(_getH_Position(),_getH_Length());}
	void setH(int value){m_header.down().setBits(_getH_Position(),_getH_Length(),value);}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//const SLinkHeader& operator=(const Word64 &header);
	const SLinkHeader& assign_from(const Word64 &header);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//operator Word64();
	Word64& cast_to_Word64();
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	std::ostream& print(std::ostream& out);
    private:
	Word64 m_header;
    };
    
    class SLinkTrailer {
    public:
	SLinkTrailer(word64 trailer=0);
	SLinkTrailer(word32 up, word32 down);
	static int _getEOEPosition(){return 60-32;}
	static int _getEOELength(){return 4;}
	int  getEOE() {return m_trailer.up().getBits(_getEOEPosition(),_getEOELength());}
	void setEOE(int value){m_trailer.up().setBits(_getEOEPosition(),_getEOELength(),value);}

	static int _getEvt_lgthPosition(){return 32-32;}
	static int _getEvt_lgthLength(){return 14;}
	int  getEvt_lgth() {return m_trailer.up().getBits(_getEvt_lgthPosition(),_getEvt_lgthLength());}
	void setEvt_lgth(int value){m_trailer.up().setBits(_getEvt_lgthPosition(),_getEvt_lgthLength(),value);}

	static int _getCRCPosition(){return 16;}
	static int _getCRCLength(){return 16;}
	int  getCRC() {return m_trailer.up().getBits(_getEvt_lgthPosition(),_getEvt_lgthLength());}
	void setCRC(int value){m_trailer.up().setBits(_getEvt_lgthPosition(),_getEvt_lgthLength(),value);}

	static int _getEvt_statPosition(){return 8;}
	static int _getEvt_satLength(){return 4;}
	int  getEvt_stat() {return m_trailer.up().getBits(_getEvt_lgthPosition(),_getEvt_lgthLength());}
	void setEvt_stat(int value){m_trailer.up().setBits(_getEvt_lgthPosition(),_getEvt_lgthLength(),value);}

	static int _getTTSPosition(){return 4;}
	static int _getTTSLength(){return 4;}
	int  getTTS() {return m_trailer.down().getBits(_getTTSPosition(),_getTTSLength());}
	void setTTS(int value){m_trailer.down().setBits(_getTTSPosition(),_getTTSLength(),value);}

	static int _getTPosition(){return 3;}
	static int _getTLength(){return 1;}
	int  getT() {return m_trailer.down().getBits(_getTTSPosition(),_getTTSLength());}
	void setT(int value){m_trailer.down().setBits(_getTTSPosition(),_getTTSLength(),value);}

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//const SLinkTrailer& operator=(const Word64 &trailer);
	const SLinkTrailer& assign_from(const Word64 &trailer);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//operator Word64();
	Word64& cast_to_Word64();
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	std::ostream& print(std::ostream& out);
    private:
	Word64 m_trailer;
    };

    class SLinkError  {
    public:
        SLinkError();
	SLinkError(Word32 errorWord);

 	int  get_link_id_position(){return 26;}
	int  get_link_id_length()  {return 6;}
	int  get_link_id()    {return error_.getBits(get_link_id_position(),get_link_id_length()        );}
	void set_link_id(int link_id){error_.setBits(get_link_id_position(),get_link_id_length(),link_id);}

 	int  get_ErrType_position(){return 21;}
	int  get_ErrType_length()  {return 5;}
	int  get_ErrType()    {return error_.getBits(get_ErrType_position(),get_ErrType_length()        );}
	void set_ErrType(int link_id){error_.setBits(get_ErrType_position(),get_ErrType_length(),link_id);}

 	int  get_InvalidNROC_position(){return 11;}
	int  get_InvalidNROC_length()  {return 1;}
	int  get_InvalidNROC()    {return error_.getBits(get_ErrType_position(),get_ErrType_length()        );}
	void set_InvalidNROC(int link_id){error_.setBits(get_ErrType_position(),get_ErrType_length(),link_id);}

 	int  get_InterBits_position(){return 8;}
	int  get_InterBits_length()  {return 3;}
	int  get_InterBits()    {return error_.getBits(get_ErrType_position(),get_ErrType_length()        );}
	void set_InterBits(int link_id){error_.setBits(get_ErrType_position(),get_ErrType_length(),link_id);}

 	int  get_ErrInfo_position(){return 0;}
	int  get_ErrInfo_length()  {return 8;}
	int  get_ErrInfo()    {return error_.getBits(get_ErrInfo_position(),get_ErrInfo_length()        );}
	void set_ErrInfo(int link_id){error_.setBits(get_ErrInfo_position(),get_ErrInfo_length(),link_id);}

	std::ostream& print(std::ostream& out);

	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//const SLinkError& operator= (Word32 hit);
	const SLinkError& assign_from(Word32 hit);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	Word32 getWord32() {return cast_to_Word32();}

 	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//operator Word32();
	Word32 cast_to_Word32();
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   private:
        Word32 error_;
    };

    class SLinkHit {
    public:
	SLinkHit();
	SLinkHit(Word32 hit);
	int  get_link_id_position(){return 26;}
	int  get_link_id_length()  {return 31-26+1;}
	int  get_link_id()    {return m_hit.getBits(get_link_id_position(),get_link_id_length()        );}
	void set_link_id(int link_id){m_hit.setBits(get_link_id_position(),get_link_id_length(),link_id);}
	
	int  get_roc_id_position(){return 21;}
	int  get_roc_id_length()  {return 25-21+1;}
	int  get_roc_id()   {return m_hit.getBits(get_roc_id_position(),get_roc_id_length()       );}
	void  set_roc_id(int roc_id){m_hit.setBits(get_roc_id_position(),get_roc_id_length(),roc_id);}
	
	int  get_dcol_id_position(){return 16;}
	int  get_dcol_id_length()  {return 20-16+1;}
	int  get_dcol_id()    {return m_hit.getBits(get_dcol_id_position(),get_dcol_id_length()        );}
	void set_dcol_id(int dcol_id){m_hit.setBits(get_dcol_id_position(),get_dcol_id_length(),dcol_id);}
	
	int  get_pix_id_position(){return 8;}
	int  get_pix_id_length()  {return 15-8+1;}
	int  get_pix_id()    {return m_hit.getBits(get_pix_id_position(),get_pix_id_length()       );}
	void set_pix_id(int pix_id) {m_hit.setBits(get_pix_id_position(),get_pix_id_length(),pix_id);}
	
	int  get_adc_position(){return 0;}
	int  get_adc_length()  {return 8;}
	int  get_adc()        {return m_hit.getBits(get_adc_position(),get_adc_length()    );}
	void set_adc(int adc) {return m_hit.setBits(get_adc_position(),get_adc_length(),adc);}
	
	int get_row() {return 80 - get_pix_id()/2;}
	int get_col() {return get_dcol_id()*2+(get_pix_id()%2);}

	std::ostream& print(std::ostream& out);
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//const SLinkHit& operator= (Word32 hit);
	const SLinkHit& assign_from(Word32 hit);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	Word32 getWord32() {return cast_to_Word32();}
	
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//operator Word32();
	Word32 cast_to_Word32();
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    private:
	Word32 m_hit;
    };

    class SLinkData {
    public:
	SLinkHeader getHeader();
	SLinkTrailer getTrailer();
	std::vector<SLinkHit> getHits(){return hits_;}
	std::vector<SLinkError> getErrors(){return errors_;}

	bool load(std::istream & in);
	//bool save(std::ostream &out);
	
	std::ostream& print(std::ostream &out = std::cout);
    private:
	SLinkHeader m_header;
	SLinkTrailer m_trailer;
	std::vector<SLinkHit> hits_;
	std::vector<SLinkError> errors_;
    };
    
    //std::ostream& operator<<(std::ostream& out, SLinkData &vrd);
    //std::ostream& operator<<(std::ostream& out, SLinkHeader &header);
    //std::ostream& operator<<(std::ostream& out, SLinkTrailer &trailer);
    //std::ostream& operator<<(std::ostream& out, SLinkHit &hit);
    
    //std::ostream& operator<<(std::ostream &out, Word32 word);
    //std::ostream& operator<<(std::ostream &out, Word64 &dword);
}
#endif
