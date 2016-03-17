

#include <iostream>
#include <string>

#include <typeinfo>
#include <math.h>

/*

User function is 

* decode_dump( std::string buf )

 */



void print_words( const std::string & words );

int inquire( const std::string & refWord , 
	     const std::string & buf , 
	     int readpoint ); 

int ThreeBitCodeGroup( const std::string code ); 

enum inquire_result {
  match = 0 , 
  not_match = 1 ,
  insufficient_data = 2 
};

int PulseHight( const std::string bits , bool * illData ); 

void decode_dump( std::string buf ){

  std::cout << "---- Input words -------" << std::endl ; 
  {
    unsigned long n = buf.find_first_of("0") ;
    if( n == std::string::npos ){
      std::cout <<"No meaningful words found." << std::endl;
      return;
    }
    print_words( buf.substr( n ) );
  }
  std::cout << "---------------------------" << std::endl ; 

  const unsigned int nWords = buf.size();

 
  enum analysis_state{

    SEARCH_TBMHEADER = 1 , 
    SEARCH_ROCHEADER = 2 , // this state also search TBM trailer. 
    SEARCH_ROCDATA   = 3,
    END = 0  
  };

  const std::string WORDS_TBMHEADER         ("011111111100") ;
  const std::string WORDS_TBMTRAILER        ("011111111110yyyyyyyyyyxxxxxx") ;
  const std::string WORDS_ROCHEADER         ("0111111110xx") ;
  const std::string WORDS_BROKEN_ROCHEADER  ("xxxxxxxxxxxx") ;
  const std::string WORDS_ROCDATA           ("ccccccrrrrrrrrrpppp0pppp") ;

  const int N_WORDS_TBM_EVNENTNUMBER = 8 ; 
  const int N_WORDS_TBM_DATA  = 8 ; 

  analysis_state state = SEARCH_TBMHEADER ; 

  unsigned int i = 0 ;  
  int i_roc = 0 ;
  for(  ; i < nWords ; ){ 

    if( state == SEARCH_TBMHEADER && buf[i] != '0' ){
      i++ ; 
      continue ; 
    }

    if( state == SEARCH_TBMHEADER ){

      int result = inquire( WORDS_TBMHEADER , buf ,  i );
      if ( result == insufficient_data ){ 
	break ;
      } 
      if ( result == match ){ 
	std::cout << buf.substr( i , WORDS_TBMHEADER.size() ) << " : TBM_Header"  << std::endl ; 
	i += WORDS_TBMHEADER.size() ; 
	
	// also output TBM event number and data .
	std::cout << buf.substr( i , N_WORDS_TBM_EVNENTNUMBER ) << " : Event Number " << std::endl ; 
	i += N_WORDS_TBM_EVNENTNUMBER ;  
	std::cout << buf.substr( i , N_WORDS_TBM_DATA ) << " : DataID " << std::endl ; 
	i += N_WORDS_TBM_DATA ; 

	state = SEARCH_ROCHEADER ; 
	continue ;  
      } 
      if( result == not_match ){
	i ++ ; 
	continue ;
      }
      
    }else if( state == SEARCH_ROCHEADER ){
      
      int result_roc        = inquire( WORDS_ROCHEADER  , buf ,  i );
      int result_tbmtrailer = inquire( WORDS_TBMTRAILER , buf ,  i );
      if ( result_roc        == insufficient_data &&
	   result_tbmtrailer == insufficient_data ){ 
	break ;
      } 

      // 
      if ( result_roc == match ){ 
	std::cout << buf.substr( i , WORDS_ROCHEADER.size() ) << " : ROC#"<< ++ i_roc <<" Header."  << std::endl ; 
	i += WORDS_ROCHEADER.size() ; 
	state = SEARCH_ROCDATA ; 
	continue ;
      }
      if ( result_tbmtrailer == match ){ 
	std::cout << buf.substr( i , 12 ) <<       " : TBM Trailer"  << std::endl ; 
	std::cout << buf.substr( i+12 , 10 ) <<     "   : TBM Trailer - info" << std::endl ; 
	{
	  int j = 0 ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : NoTokenPass" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Reset TBM" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Reset Roc" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Sync Error" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Sync Trigger" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Clear Trig Cntr" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Cal Trigger" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Stack Full" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Auto Reset Sent" << std::endl ; 
	  std::cout << "    * " << buf.substr( i+12 + (j++) , 1 ) << " : Pkam Reset Sent" << std::endl ; 
	}
	std::cout << buf.substr( i+12+10 ,  6 ) << "   : TBM Trailer - 6bit stack counter" << std::endl ; 

	i += WORDS_TBMTRAILER.size() ; 

	state = END ; 
	break ; 
      }

      // Arriving here means
      //    that the word, which is just after the TBM header,
      //      was not either ROC header or TBM trailer. 
      //
      // [tentative, common treatment : We assume such data(12-bit) is 
      //   ROC header but broken somehow.
      //
      std::cout << buf.substr( i , WORDS_ROCHEADER.size() ) << " : ROC#"<< ++ i_roc <<" Header, due to bit flip?"  << std::endl ; 
      i += WORDS_ROCHEADER.size() ; 
      state = SEARCH_ROCDATA ; 
      continue ;

    }else if( state == SEARCH_ROCDATA ){
      
      int result = inquire( WORDS_ROCDATA  , buf ,  i );

      // Extra check -- Address should be in Code-Group
      int col_0 , col_1 , raw_0 , raw_1 ,  raw_2 ;
      
      if( result == match ){ 
	col_0 = ThreeBitCodeGroup(   buf.substr( i+ 0  , 3 ) );
	col_1 = ThreeBitCodeGroup(   buf.substr( i+ 3  , 3 ) );
	raw_0 = ThreeBitCodeGroup(   buf.substr( i+ 6  , 3 ) );
	raw_1 = ThreeBitCodeGroup(   buf.substr( i+ 9  , 3 ) );
	raw_2 = ThreeBitCodeGroup(   buf.substr( i+12  , 3 ) );

	if( col_0 < 0 || col_1 < 0 || raw_0 < 0 || raw_1 < 0 || raw_2 < 0  ){
	  result = not_match ; 
	}else{

	  std::cout << " +-- " << std::endl ;
	  std::cout << " | " <<buf.substr( i+0  , 6 ) << " : Col Addr (" << 6*col_0 + col_1 <<")"<< std::endl ; 
	  std::cout << " | " <<buf.substr( i+6  , 9 ) << " : Row Addr (" << 6*6*raw_0 + 6*raw_1 + raw_2 <<")"<< std::endl ; 
	  {
	    bool isBadPHbit = true ; 
	    std::string PHbits = buf.substr( i+15 , 9 ) ;
	    std::cout << " | " << PHbits << " : PulseHight bit " << std::endl;

	    const long col = 6*col_0 + + col_1 ;
	    const long row = 6*6*raw_0 + 6*raw_1 + raw_2 ;
	    const bool isRowVal_odd = row % 2 == 1 ;
	    const long col_HumanReadable = col * 2 + (isRowVal_odd ? 1 : 0 ) ;
	    const long row_HumanReadable = 80 - ( row - ( isRowVal_odd ? 1 : 0 ) ) / 2 ;
	    std::cout << " | [col,row] = [" << col_HumanReadable <<","<<row_HumanReadable <<"] has PH = "
		      << PulseHight( PHbits , & isBadPHbit )<< std::endl ; 
	    if( isBadPHbit ){
	      std::cout << " | [CATION] BAD PH BITS(hint : 5th Bit must be always zero)." << std::endl ; 
	    }
	  }
	  std::cout << " +-- " << std::endl ;
	  
	  i += WORDS_ROCDATA.size() ; 
	  continue; 

	} // end-else ;
      } // end-if result == "match".
	
      if( result == not_match ||
	  result == insufficient_data ){ 

	state = SEARCH_ROCHEADER ; 

	// i++ ; 
	//  \__ Here, increment is not needed.

	continue; 
      }

    }

    std::cout <<"BUG : unexpected input. Need to fix program." << std::endl; 
    return  ; 
    //
    // -- do not do anything here...
    //
  } // end  for-loop.


  // Dump leftover...
  std::cout << "- - - end decoding - - - -  " << std::endl ; 
  if( state != END ){
    std::cout << "Data structure of input seems not perfect." << std::endl ; 
    if( state == SEARCH_TBMHEADER ){
      std::cout << "Decode could not find TBM header." << std::endl ; 
    }
    if( state == SEARCH_ROCHEADER ){
      std::cout << "Decode had problem during looking for ROC header and TBM trailer." << std::endl ; 
    }
    if( state == SEARCH_ROCDATA  ){
      std::cout << "Decode had problem during looking for ROC data." << std::endl ; 
    }

  }else{
    std::cout << "Data structure of input seems fine. Decode finished finiding TBM trailer."<< std::endl ; 
  }

  //Debugging info. Commented LC
  std::cout <<" - - - Dump leftover - - - " << std::endl ;
  print_words( buf.substr( i ) );

}



//  Invarid input -> Return -1.
int ThreeBitCodeGroup( const std::string code ){ 

  if( code == "000" ) return 0  ;
  if( code == "001" ) return 1  ;
  if( code == "010" ) return 2  ;
  if( code == "011" ) return 3  ;
  if( code == "100" ) return 4  ;
  if( code == "101" ) return 5  ;

  return -1 ; 
  
}




int inquire( const std::string & refWord , 
	     const std::string & buf , 
	     int readpoint ){

  if( readpoint + refWord.size() > buf.size() ){
    return insufficient_data ; 
  }
  
  for(unsigned int i = 0 ; i < refWord.size() ; i ++ ){
    if( refWord[i] == '0' || refWord[i] == '1' ){

      if( refWord[i] !=  buf[ readpoint + i ] ) return not_match ; 
      
    }

  } // end for-loop

  return match ; 

}

void print_words( const std::string & words ){

  for( unsigned int i=0; i<words.size() ; i++ ){
    std::cout << words[i] ;
    if( (i+1)%4 == 0 ){
      std::cout <<" ";
    }
    if( (i+1)%16 == 0 ){
      std::cout <<"\n";
    }
  }
  std::cout <<"\n";
}

int PulseHight( const std::string bits , bool * illData ){

  (* illData ) = 
    ( bits.substr( 4 , 1 ) != "0" )  // 5th bit should be zero
    ||
    ( bits.length() != 9 ) // this should be 9-bit data.
    ;

  std::string words = bits.substr( 0, 4 ) + bits.substr( 5, 4 );

  int num = 0 ; 
  const unsigned int NBITS = words.length() ;
  for( unsigned int i = 0 ; i < NBITS ; i ++ ){
    if( words.substr( i, 1 ) == "1" ){
      num += pow( 2, NBITS -1 - i );
    }
  }

  return num ; 
}
