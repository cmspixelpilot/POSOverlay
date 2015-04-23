/************************************************************************/
/*									*/
/*	Module:			jbistub_vme.c				*/
/*									*/
/*									*/
/*	Description:	VME Jam STAPL ByteCode Player main source file  */
/*	                based on jbistub.c v2.2             		*/
/*		(Copyright (C) Altera Corporation 1997-2001)	    	*/
/*		and the VME port of jbistub.c v1.1                      */
/*              							*/
/*  Autor:			Ing. Christian Irmler, Hephy 		*/
/*									*/
/*									*/
/*	Revisions:	W.Johns / H. Steininger June 2007		*/
/*          Modified for buffer transfers (multi-) and 			*/
/*              targeted read/write of Jtag address 			*/
/*	      (setup functions occur on board)         			*/
/************************************************************************/
//For CAENVME
#ifndef LINUX
#define LINUX
#endif




/* VME - SUPPORT */
#include "CAENVMElib.h"  // CAEN library prototypes
#include <iostream>
#include <string>
#include <sys/stat.h>
using namespace std;


#define PORT EMBEDDED
/* VME - SUPPORT end*/


#ifndef NO_ALTERA_STDIO
#define NO_ALTERA_STDIO
#endif

#if ( _MSC_VER >= 800 )
#pragma warning(disable:4115)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#pragma warning(disable:4514)
#endif

#include "jbiport.h"

typedef int BOOL;
typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned int DWORD;
#define TRUE 1
#define FALSE 0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <sys/time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "jbiexprt.h"


/************************************************************************
*
*	Global variables
*/

	char *action = NULL;

/* VME - SUPPORT */

bool DBugg=false;
// toms global variables
unsigned int VME_write_address;
unsigned int VME_read_address;  
unsigned int VME_base_address;  
char *filename = NULL;
int          BHandle;
// prototypes
void printIO (char *StatusString);
int playermain(int argc,char **argv);


void write(char channeldata)
{
 // Define VME parameters
  CVDataWidth dw = cvD16 ; // data width (see CAENVMEtypes.h )
  CVAddressModifier am = cvA32_U_DATA;
  CVErrorCodes ret = CAENVME_WriteCycle(BHandle,VME_write_address,&channeldata,am,dw);
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform VME Read");}
		if(DBugg)cout<<"write "<<((int)channeldata)<<endl;
}

unsigned int readcount=0;

void read001(char *channeldata)
{//TDI = 0, TMS = 0
  unsigned int readbyte;

  CVDataWidth dw = cvD16 ; // data width (see CAENVMEtypes.h )
  CVAddressModifier am = cvA32_U_DATA;
  VME_read_address=0xa00020+VME_base_address;
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,VME_read_address,&readbyte,am,dw);
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform VME Read");}
//cout<<"read 001 0x"<<readbyte<<endl;

  *channeldata=readbyte;
}
void read011(char *channeldata)
{//TDI = 0, TMS = 1
  unsigned int readbyte;

  CVDataWidth dw = cvD16 ; // data width (see CAENVMEtypes.h )
  CVAddressModifier am = cvA32_U_DATA;
  VME_read_address=0xa00024+VME_base_address;
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,VME_read_address,&readbyte,am,dw);
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform VME Read");}
//cout<<"read 011 0x"<<readbyte<<endl;


  *channeldata=readbyte;
}
void read101(char *channeldata)
{//TDI = 1, TMS = 0
  unsigned int readbyte;

  CVDataWidth dw = cvD16 ; // data width (see CAENVMEtypes.h )
  CVAddressModifier am = cvA32_U_DATA;
  VME_read_address=0xa00028+VME_base_address;
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,VME_read_address,&readbyte,am,dw);
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform VME Read");}

//cout<<"read 101 0x"<<readbyte<<endl;

  *channeldata=readbyte;
}
void read111(char *channeldata)
{//TDI = 1, TMS = 1
  unsigned int readbyte;

  CVDataWidth dw = cvD16 ; // data width (see CAENVMEtypes.h )
  CVAddressModifier am = cvA32_U_DATA;
  VME_read_address=0xa0002c+VME_base_address;
  CVErrorCodes ret = CAENVME_ReadCycle(BHandle,VME_read_address,&readbyte,am,dw);
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform VME Read");}

//cout<<"read 111 0x"<<readbyte<<endl;

  *channeldata=readbyte;
}
/* VME - SUPPORT end*/


/* file buffer for Jam STAPL ByteCode input file */
unsigned char *file_buffer = NULL;
long file_pointer = 0L;
long file_length = 0L;

/* delay count for one millisecond delay */
long one_ms_delay = 0L;

/* serial port interface available on all platforms */
BOOL jtag_hardware_initialized = FALSE;
char *serial_port_name = NULL;
BOOL specified_com_port = FALSE;
int com_port = -1;
void initialize_jtag_hardware(void);
void close_jtag_hardware(void);

/* function prototypes to allow forward reference */
extern void delay_loop(long count);

/*
*	This structure stores information about each available vector signal
*/
struct VECTOR_LIST_STRUCT
{
	char *signal_name;
	int  hardware_bit;
	int  vector_index;
};

struct VECTOR_LIST_STRUCT vector_list[] =
{
	/* add a record here for each vector signal */
	{ "", 0, -1 }
};

#define VECTOR_SIGNAL_COUNT ((int)(sizeof(vector_list)/sizeof(vector_list[0])))

BOOL verbose = FALSE;

/************************************************************************
*
*	Customized interface functions for Jam STAPL ByteCode Player I/O:
*
*	jbi_jtag_io()
*	jbi_message()
*	jbi_delay()
*/

int jbi_jtag_io(int tms, int tdi, int read_tdo)
{
	int tdo = 0;
	int read_type=0;
    /* VME - SUPPORT */
	char tomsdataout =0;
	char tomsdatain =0;
    /* VME - SUPPORT end*/

if(tms)read_type+=1;
if(tdi)read_type+=2;

//cout<<"TMS= "<<hex<<tms<<" TDI= "<<hex<<tdi<<" TD0= "<<hex<<read_tdo<<" read type= "<<read_type<<endl;
	if (!jtag_hardware_initialized)
	{
		initialize_jtag_hardware();
		jtag_hardware_initialized = TRUE;
	}
    
    /* customized code for VME - SUPPORT*/
    tdo = 0;
	tomsdataout = 0;
	tomsdatain = 0;
		
	tomsdataout = ((tdi?0x40:0)|(tms?0x02:0));
		
	if (read_tdo)
	{

	switch(read_type)
	{
	case 0:
	 read001(&tomsdatain);if(DBugg){cout<<readcount<<" read001: "<<((int)tomsdatain)<<endl;readcount++;}
	 break;
	case 1:
	 read011(&tomsdatain);if(DBugg){cout<<readcount<<" read011: "<<((int)tomsdatain)<<endl;readcount++;}
	 break;
	case 2:
	 read101(&tomsdatain);if(DBugg){cout<<readcount<<" read101: "<<((int)tomsdatain)<<endl;readcount++;}
	 break;
	case 3:
	 read111(&tomsdatain);if(DBugg){cout<<readcount<<" read111: "<<((int)tomsdatain)<<endl;readcount++;}
	 break;
	 }
         tdo=(tomsdatain&0x80)?1:0; 	  //no inversion of TDO in VME case
	} else {//write
	//write(tomsdataout);
	write(tomsdataout);
        }

	return (tdo);
}

void jbi_jtag_io_multiwrite(int* tmsbuf, int* tdibuf, int cycles)
{

if(DBugg){ cout<<"multiwrite"<<endl;}   
    // customized code for VME - SUPPORT
 CVDataWidth dw[cycles];   // data width (see CAENVMEtypes.h )
 CVAddressModifier am[cycles]; 
 CVErrorCodes errs[cycles];
 unsigned int addrses[cycles];
 unsigned int tomsdataout[cycles]; 		

for(int i=0;i<cycles;i++){
dw[i]=cvD16;
am[i]=cvA32_U_DATA;
addrses[i]=VME_write_address;
tomsdataout[i] = 0;
tomsdataout[i] = ((tdibuf[i]?0x40:0)|(tmsbuf[i]?0x02:0));
if(DBugg){cout<<i<<" "<<tomsdataout[i]<<endl;}
}
if(DBugg){cout<<"multiwrite end"<<endl;string str; usleep(10000);}//cin>>str;}
 CVErrorCodes ret = CAENVME_MultiWrite(BHandle,addrses,tomsdataout,cycles,am,dw,errs);		
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform Multiwrite!");}


	return;
}
void jbi_jtag_io_multiread(int* tmsbuf, int* tdibuf,int* read_tdobuf,unsigned int* tdobuf, int cycles)
{

 CVDataWidth dw[cycles];   // data width (see CAENVMEtypes.h )
 CVAddressModifier am[cycles]; 
 CVErrorCodes errs[cycles];
 unsigned int addrses[cycles];
int read_type;
for(int i=0;i<cycles;i++){
dw[i]=cvD16;
am[i]=cvA32_U_DATA;
read_type=0;
if(tmsbuf[i])read_type+=1;
if(tdibuf[i])read_type+=2;
	switch(read_type)
	{
	case 0:
	 addrses[i]=0xa00020+VME_base_address;
	 break;
	case 1:
	addrses[i]=0xa00024+VME_base_address;
	 break;
	case 2:
	 addrses[i]=0xa00028+VME_base_address;
	 break;
	case 3:
	 addrses[i]=0xa0002c+VME_base_address;
	 break;
	 }
}
 CVErrorCodes ret = CAENVME_MultiWrite(BHandle,addrses,tdobuf,cycles,am,dw,errs);		
    if(ret != cvSuccess) {  // Error
    printf ("Unable to Perform Multiwrite!");}

for(int i=0;i<cycles;i++){ tdobuf[i]=(tdobuf[i]&0x80)?1:0;}//no inversion of TDO in VME case

	return;
}

void jbi_message(char *message_text)
{
	puts(message_text);
	fflush(stdout);
}

void jbi_export_integer(char *key, long value)
{
	if (verbose)
	{
		printf("Export: key = \"%s\", value = %ld\n", key, value);
		fflush(stdout);
	}
}

#define HEX_LINE_CHARS 72
#define HEX_LINE_BITS (HEX_LINE_CHARS * 4)

char conv_to_hex(unsigned int value)
{
	char c;

	if (value > 9)
	{
		c = (char) (value + ('A' - 10));
	}
	else
	{
		c = (char) (value + '0');
	}

	return (c);
}

void jbi_export_boolean_array(char *key, unsigned char *data, long count)
{
	char string[HEX_LINE_CHARS + 1];
	long i, offset;
	unsigned int size, line, lines, linebits, value, j, k;

	if (verbose)
	{
		if (count > HEX_LINE_BITS)
		{
			printf("Export: key = \"%s\", %ld bits, value = HEX\n", key, count);
			lines = (count + (HEX_LINE_BITS - 1)) / HEX_LINE_BITS;

			for (line = 0; line < lines; ++line)
			{
				if (line < (lines - 1))
				{
					linebits = HEX_LINE_BITS;
					size = HEX_LINE_CHARS;
					offset = count - ((line + 1) * HEX_LINE_BITS);
				}
				else
				{
					linebits = count - ((lines - 1) * HEX_LINE_BITS);
					size = (linebits + 3) / 4;
					offset = 0L;
				}

				string[size] = '\0';
				j = size - 1;
				value = 0;

				for (k = 0; k < linebits; ++k)
				{
					i = k + offset;
					if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
					if ((i & 3) == 3)
					{
						string[j] = conv_to_hex(value);
						value = 0;
						--j;
					}
				}
				if ((k & 3) > 0) string[j] = conv_to_hex(value);

				printf("%s\n", string);
			}

			fflush(stdout);
		}
		else
		{
			size = (count + 3) / 4;
			string[size] = '\0';
			j = size - 1;
			value = 0;

			for (i = 0; i < count; ++i)
			{
				if (data[i >> 3] & (1 << (i & 7))) value |= (1 << (i & 3));
				if ((i & 3) == 3)
				{
					string[j] = conv_to_hex(value);
					value = 0;
					--j;
				}
			}
			if ((i & 3) > 0) string[j] = conv_to_hex(value);

			printf("Export: key = \"%s\", %ld bits, value = HEX %s\n",
				key, count, string);
			fflush(stdout);
		}
	}
}

void jbi_delay(long microseconds)
{

	delay_loop(microseconds *
		((one_ms_delay / 1000L) + ((one_ms_delay % 1000L) ? 1 : 0)));
}

int jbi_vector_map
(
	int signal_count,
	char **signals
)
{
	int signal, vector, ch_index, diff;
	int matched_count = 0;
	char l, r;

	for (vector = 0; (vector < VECTOR_SIGNAL_COUNT); ++vector)
	{
		vector_list[vector].vector_index = -1;
	}

	for (signal = 0; signal < signal_count; ++signal)
	{
		diff = 1;
		for (vector = 0; (diff != 0) && (vector < VECTOR_SIGNAL_COUNT);
			++vector)
		{
			if (vector_list[vector].vector_index == -1)
			{
				ch_index = 0;
				do
				{
					l = signals[signal][ch_index];
					r = vector_list[vector].signal_name[ch_index];
					diff = (((l >= 'a') && (l <= 'z')) ? (l - ('a' - 'A')) : l)
						- (((r >= 'a') && (r <= 'z')) ? (r - ('a' - 'A')) : r);
					++ch_index;
				}
				while ((diff == 0) && (l != '\0') && (r != '\0'));

				if (diff == 0)
				{
					vector_list[vector].vector_index = signal;
					++matched_count;
				}
			}
		}
	}

	return (matched_count);
}

int jbi_vector_io
(
	int signal_count,
	long *dir_vect,
	long *data_vect,
	long *capture_vect
)
{
	int signal, vector, bit;
	int matched_count = 0;
	int data = 0;
	int mask = 0;
	int dir = 0;
	int i = 0;
	int result = 0;
	char ch_data = 0;

	if (!jtag_hardware_initialized)
	{
		initialize_jtag_hardware();
		jtag_hardware_initialized = TRUE;
	}

	/*
	*	Collect information about output signals
	*/
	for (vector = 0; vector < VECTOR_SIGNAL_COUNT; ++vector)
	{
		signal = vector_list[vector].vector_index;

		if ((signal >= 0) && (signal < signal_count))
		{
			bit = (1 << vector_list[vector].hardware_bit);

			mask |= bit;
			if (data_vect[signal >> 5] & (1L << (signal & 0x1f))) data |= bit;
			if (dir_vect[signal >> 5] & (1L << (signal & 0x1f))) dir |= bit;

			++matched_count;
		}
	}

	/*
	*	Write outputs to hardware interface, if any
	*/
	if (dir != 0)
	{
	
	}

	/*
	*	Read the input signals and save information in capture_vect[]
	*/
	if ((dir != mask) && (capture_vect != NULL))
	{
		if (specified_com_port)
		{
			ch_data = 0x7e;
			//write(com_port, &ch_data, 1);
			for (i = 0; (i < 100) && (result != 1); ++i)
			{
				//result = read(com_port, &ch_data, 1);
			}
			if (result == 1)
			{
				data = ((ch_data << 7) & 0x80) | ((ch_data << 3) & 0x10);
			}
			else
			{
				
				fprintf(stderr, "Error:  BitBlaster not responding\n");
			}
		}
		else
		{

		}

		for (vector = 0; vector < VECTOR_SIGNAL_COUNT; ++vector)
		{
			signal = vector_list[vector].vector_index;

			if ((signal >= 0) && (signal < signal_count))
			{
				bit = (1 << vector_list[vector].hardware_bit);

				if ((dir & bit) == 0)	/* if it is an input signal... */
				{
					if (data & bit)
					{
						capture_vect[signal >> 5] |= (1L << (signal & 0x1f));
					}
					else
					{
						capture_vect[signal >> 5] &= ~(unsigned int)
							(1L << (signal & 0x1f));
					}
				}
			}
		}
	}

	return (matched_count);
}

void *jbi_malloc(unsigned int size)
{
	return (malloc(size));
}

void jbi_free(void *ptr)
{
	free(ptr);
}

/************************************************************************
*
*	get_tick_count() -- Get system tick count in milliseconds
*
*	for DOS, use BIOS function _bios_timeofday()
*	for WINDOWS use GetTickCount() function
*	for UNIX use clock() system function
*/
DWORD get_tick_count(void)
{
	DWORD tick_count = 0L;


	/* assume clock() function returns microseconds */
	tick_count = (DWORD) (clock() / 1000L);


	return (tick_count);
}

#define DELAY_SAMPLES 10
#define DELAY_CHECK_LOOPS 100000000

void calibrate_delay(void)
{

	one_ms_delay = 0L;

	/* This is system-dependent!  Updates the number for target system */

struct timeval tv;
struct timezone tz;
struct timeval tv1;
struct timezone tz1;

unsigned int diff=0;

for(int i=0;i<DELAY_SAMPLES;i++){

long counts = DELAY_CHECK_LOOPS;

gettimeofday(&tv, &tz);
delay_loop(counts);
gettimeofday(&tv1, &tz1);
if(DBugg){
cout<<tv1.tv_sec<<" "<<tv.tv_sec<<endl;
cout<<tv1.tv_usec<<" "<<tv.tv_usec<<endl;
}
unsigned int diff1=tv1.tv_sec - tv.tv_sec;
unsigned int diff2=tv1.tv_usec - tv.tv_usec;


diff+=diff1*1000000+diff2;

if(DBugg)cout<<(diff1*1000000+diff2)<<endl;
}
one_ms_delay =int(10000.0*(100000000.0/(1.0*diff)));

//uncomment to use your own delay
//one_ms_delay = 50000L;

cout<<"one ms # loops ="<<one_ms_delay<<endl;
cout<<" you may need to uncomment here and use a minimum number if there are failures. Suggested min is 620000"<<endl;
}

char *error_text[] =
{
/* JBIC_SUCCESS            0 */ "success",
/* JBIC_OUT_OF_MEMORY      1 */ "out of memory",
/* JBIC_IO_ERROR           2 */ "file access error",
/* JAMC_SYNTAX_ERROR       3 */ "syntax error",
/* JBIC_UNEXPECTED_END     4 */ "unexpected end of file",
/* JBIC_UNDEFINED_SYMBOL   5 */ "undefined symbol",
/* JAMC_REDEFINED_SYMBOL   6 */ "redefined symbol",
/* JBIC_INTEGER_OVERFLOW   7 */ "integer overflow",
/* JBIC_DIVIDE_BY_ZERO     8 */ "divide by zero",
/* JBIC_CRC_ERROR          9 */ "CRC mismatch",
/* JBIC_INTERNAL_ERROR    10 */ "internal error",
/* JBIC_BOUNDS_ERROR      11 */ "bounds error",
/* JAMC_TYPE_MISMATCH     12 */ "type mismatch",
/* JAMC_ASSIGN_TO_CONST   13 */ "assignment to constant",
/* JAMC_NEXT_UNEXPECTED   14 */ "NEXT unexpected",
/* JAMC_POP_UNEXPECTED    15 */ "POP unexpected",
/* JAMC_RETURN_UNEXPECTED 16 */ "RETURN unexpected",
/* JAMC_ILLEGAL_SYMBOL    17 */ "illegal symbol name",
/* JBIC_VECTOR_MAP_FAILED 18 */ "vector signal name not found",
/* JBIC_USER_ABORT        19 */ "execution cancelled",
/* JBIC_STACK_OVERFLOW    20 */ "stack overflow",
/* JBIC_ILLEGAL_OPCODE    21 */ "illegal instruction code",
/* JAMC_PHASE_ERROR       22 */ "phase error",
/* JAMC_SCOPE_ERROR       23 */ "scope error",
/* JBIC_ACTION_NOT_FOUND  24 */ "action not found",
};

#define MAX_ERROR_CODE (int)((sizeof(error_text)/sizeof(error_text[0]))+1)

/************************************************************************/

/* VME - SUPPORT */
int main (int argc, char *argv[])
{
 	char reply;
	char comm[256];
	
	//Cls();
	printf("welcome to VMERA stdio version\n");
	

if( (argc != 6) )
    {
    printf("Usage: fed_jam1.exe <ACTION> <BASEADDRESS> <FPGA GROUP> <DEVICE> <FILENAME> \n");
    printf("<ACTION> may be PROGRAM or VERIFY\n");
    printf("<BASEADDRESS> should be hex e.g. 0x1C000000\n");
    printf("<FPGA GROUP> should be FRONT or CENTER\n");
     printf("<DEVICE> is which fiber in the VME opto card (old PCI card = 0) \n");

    exit(1);
    }
else 
    {
    if( strcmp((char*)argv[1], "VERIFY") == 0 )
    {action="VERIFY";}
    else if( strcmp((char*)argv[1], "PROGRAM") == 0 )
    {action="PROGRAM";}else{
            {
            printf("Usage: fed_jam1.exe <ACTION> <BASEADDRESS> <FPGA GROUP> <FILENAME>\n");
            printf("<ACTION> can be PROGRAM or VERIFY\n");
            exit(1);
            }
          }
	CVBoardTypes  VMEBoard = cvV2718;  // define interface type
  int         Link = 0;  // define device & link
  int         Device = 0;
 
sscanf((char*)argv[4], "%d", &Device);


//init the CAEN VME

    if (CAENVME_Init(VMEBoard, Device, Link, &BHandle) != cvSuccess)
    {
  	  printf ("Unable to Initialize CAEN library... exiting");
	  return -1;	/* Initialisation failure */
    }
    else
    {
      printf("CAEN initialized. lets go to work... \n\n");
    }

	  
   filename=(char*)argv[5];	  
   sprintf(comm,"ls -l %s",filename);
   printf("\n=====================================================================\n");
   system(comm);
   printf("=====================================================================\n\n");

  unsigned int number;
  sscanf((char*)argv[2], "%x", &number);
  printf("BaseAddress 0x%x\n",number);
VME_base_address=number;
     if( strcmp((char*)argv[3], "FRONT") == 0 )
    {	 		    VME_write_address=VME_base_address+0xa00018;
	 		    VME_read_address=VME_base_address+0xa00018;
    }
    else if( strcmp((char*)argv[3], "CENTER") == 0 )
    {			    VME_write_address=VME_base_address+0xa0001c;
	 		    VME_read_address=VME_base_address+0xa0001c;
    }else{
            {
            printf("Usage: fed_jam1.exe <ACTION> <BASEADDRESS> <FPGA GROUP> <FILENAME>\n");
            printf("<FPGA GROUP> can be FRONT or CENTER\n");
            exit(1);
            }
          }

    }

	 		    playermain(0,0);
	 
	
	
	 CAENVME_End(BHandle);//CloseVXIlibrary();
	printf("CAEN closed. good bye... \n");
	return 0;
}
/* VME - SUPPORT end*/


int playermain(int argc, char **argv)
{
	BOOL help = FALSE;
	BOOL error = FALSE;
	//char *filename = NULL;	 is global now
	long offset = 0L;
	long error_address = 0L;
	JBI_RETURN_TYPE crc_result = JBIC_SUCCESS;
	JBI_RETURN_TYPE exec_result = JBIC_SUCCESS;
	unsigned short expected_crc = 0;
	unsigned short actual_crc = 0;
	char key[33] = {0};
	char value[257] = {0};
	int exit_status = 0;
	int arg = 0;
	int exit_code = 0;
	int format_version = 0;
	time_t start_time = 0;
	time_t end_time = 0;
	int time_delta = 0;
	char *workspace = NULL;
	int init_count = 0;
	FILE *fp = NULL;
//	char *action = NULL;
	char *init_list[10];
	struct stat sbuf;
	long workspace_size = 0;
	char *exit_string = NULL;
	int reset_jtag = 1;
	int execute_program = 1;
	int action_count = 0;
	int procedure_count = 0;
	int index = 0;
	char *action_name = NULL;
	char *description = NULL;
	JBI_PROCINFO *procedure_list = NULL;
	JBI_PROCINFO *procptr = NULL;
	
	verbose = FALSE;

	init_list[0] = NULL;

    // stdio version : here come the default values
    // filename (global) now specified in main      
    //action = "PROGRAM";
    //action = "VERIFY";
    verbose = TRUE;

	/* print out the version string and copyright message */
	
    printf("%s", "VME Byte-Code Player 2.0 alpha based on Jam STAPL Version 2.2 starts playing...\n\n");
      
	for (arg = 1; arg < argc; arg++)
	{
		if ((argv[arg][0] == '-') || (argv[arg][0] == '/'))

		{
			switch(toupper(argv[arg][1]))
			{
			case 'A':				/* set action name */
				if (action == NULL)
				{
					action = &argv[arg][2];
				}
				else
				{
					error = TRUE;
				}
				break;


			case 'D':				/* initialization list */
				if (argv[arg][2] == '"')
				{
					init_list[init_count] = &argv[arg][3];
				}
				else
				{
					init_list[init_count] = &argv[arg][2];
				}
				init_list[++init_count] = NULL;
				break;


			case 'R':		/* don't reset the JTAG chain after use */
				reset_jtag = 0;
				break;

			case 'S':				/* set serial port address */
				serial_port_name = &argv[arg][2];
				specified_com_port = TRUE;
				break;

			case 'M':				/* set memory size */
				if (sscanf(&argv[arg][2], "%ld", &workspace_size) != 1)
					error = TRUE;
				if (workspace_size == 0) error = TRUE;
				break;

			case 'H':				/* help */
				help = TRUE;
				break;

			case 'V':				/* verbose */
				verbose = TRUE;
				break;

			case 'I':				/* show info only, do not execute */
				verbose = TRUE;
				execute_program = 0;
				break;

			default:
				error = TRUE;
				break;
			}
		}
		else
		{
			/* it's a filename */
			if (filename == NULL)
			{
				filename = argv[arg];
			}
			else
			{
				/* error -- we already found a filename */
				error = TRUE;
			}
		}

		if (error)
		{
			//fprintf(stderr, "Illegal argument: \"%s\"\n", argv[arg]);
			fprintf(stderr, "Illegal argument\n");
			help = TRUE;
			error = FALSE;
		}
	}


	if (help || (filename == NULL))
	{
		fprintf(stderr, "Usage:  jbi [options] <filename>\n");
		fprintf(stderr, "\nAvailable options:\n");
		fprintf(stderr, "    -h          : show help message\n");
		fprintf(stderr, "    -v          : show verbose messages\n");
		fprintf(stderr, "    -i          : show file info only - does not execute any action\n");
		fprintf(stderr, "    -a<action>  : specify an action name (Jam STAPL)\n");
		fprintf(stderr, "    -d<var=val> : initialize variable to specified value (Jam 1.1)\n");
		fprintf(stderr, "    -d<proc=1>  : enable optional procedure (Jam STAPL)\n");
		fprintf(stderr, "    -d<proc=0>  : disable recommended procedure (Jam STAPL)\n");
		fprintf(stderr, "    -s<port>    : serial port name (for BitBlaster)\n");
		fprintf(stderr, "    -r          : don't reset JTAG TAP after use\n");
		exit_status = 1;
	}
	else if ((workspace_size > 0) &&
		((workspace = (char *) jbi_malloc((size_t) workspace_size)) == NULL))
	{
		//fprintf(stderr, "Error: can't allocate memory (%d Kbytes)\n",
		//	(int) (workspace_size / 1024L));
		fprintf(stderr, "Error: can't allocate memory\n");
		exit_status = 1;
	}
	//else if (access(filename, 0) != 0)
	//{
	//	fprintf(stderr, "Error: can't access file \"%s\"\n", filename);
	//	exit_status = 1;
	//}
	else
	{
		/* get length of file */
		if (stat(filename, &sbuf) == 0) file_length = sbuf.st_size;
		//GetFileInfo(filename, &file_length);

		if ((fp = fopen(filename, "rb")) == NULL)
		{
			fprintf(stderr, "Error: can't open file \"%s\"\n", filename);
			exit_status = 1;
		}
		else
		{
			/*
			*	Read entire file into a buffer
			*/
			file_buffer = (unsigned char *) jbi_malloc((size_t) file_length);

			if (file_buffer == NULL)
			{
				fprintf(stderr, "Error: can't allocate memory (%d Kbytes)\n",
					(int) (file_length / 1024L));
				exit_status = 1;
			}
			else
			{
				if (fread(file_buffer, 1, (size_t) file_length, fp) !=
					(size_t) file_length)
				{
					fprintf(stderr, "Error reading file \"%s\"\n", filename);
					exit_status = 1;
				}
			}

			fclose(fp);
		}

		if (exit_status == 0)
		{
			/*
			*	Calibrate the delay loop function
			*/
			calibrate_delay();

			/*
			*	Check CRC
			*/
			crc_result = jbi_check_crc(file_buffer, file_length,
				&expected_crc, &actual_crc);

			if (verbose || (crc_result == JBIC_CRC_ERROR))
			{
				switch (crc_result)
				{
				case JBIC_SUCCESS:
					printf("CRC matched: CRC value = %04X\n", actual_crc);
					break;

				case JBIC_CRC_ERROR:
					printf("CRC mismatch: expected %04X, actual %04X\n",
						expected_crc, actual_crc);
					break;

				case JBIC_UNEXPECTED_END:
					printf("Expected CRC not found, actual CRC value = %04X\n",
						actual_crc);
					break;

				case JBIC_IO_ERROR:
					printf("Error: File format is not recognized.\n");
					exit(1);
					break;

				default:
					printf("CRC function returned error code %d\n", crc_result);
					break;
				}
			}

			if (verbose)
			{
				/*
				*	Display file format version
				*/
				jbi_get_file_info(file_buffer, file_length,
					&format_version, &action_count, &procedure_count);

				printf("File format is %s ByteCode format\n",
					(format_version == 2) ? "Jam STAPL" : "pre-standardized Jam 1.1");

				/*
				*	Dump out NOTE fields
				*/
				while (jbi_get_note(file_buffer, file_length,
					&offset, key, value, 256) == 0)
				{
					printf("NOTE \"%s\" = \"%s\"\n", key, value);
				}

				/*
				*	Dump the action table
				*/
				
				if ((format_version == 2) && (action_count > 0))
				{
					printf("\nActions available in this file:\n");

					for (index = 0; index < action_count; ++index)
					{
						jbi_get_action_info(file_buffer, file_length,
							index, &action_name, &description, &procedure_list);

						if (description == NULL)
						{
							printf("%s\n", action_name);
						}
						else
						{
							printf("%s \"%s\"\n", action_name, description);
						}


						procptr = procedure_list;
						while (procptr != NULL)
						{
							if (procptr->attributes != 0)
							{
								printf("    %s (%s)\n", procptr->name,
									(procptr->attributes == 1) ?
									"optional" : "recommended");
							}

							procedure_list = procptr->next;
							jbi_free(procptr);
							procptr = procedure_list;
						}
					}

					/* add a blank line before execution messages */
					if (execute_program) printf("\n");
				}
			}

			if (execute_program)
			{
				/*
				*	Execute the Jam STAPL ByteCode program
				*/
				time(&start_time);
				exec_result = jbi_execute(file_buffer, file_length, workspace,
					workspace_size, action, init_list, reset_jtag,
					&error_address, &exit_code, &format_version);
				time(&end_time);

				if (exec_result == JBIC_SUCCESS)
				{
					if (format_version == 2)
					{
						switch (exit_code)
						{
						case  0: exit_string = "Success"; break;
						case  1: exit_string = "Checking chain failure"; break;
						case  2: exit_string = "Reading IDCODE failure"; break;
						case  3: exit_string = "Reading USERCODE failure"; break;
						case  4: exit_string = "Reading UESCODE failure"; break;
						case  5: exit_string = "Entering ISP failure"; break;
						case  6: exit_string = "Unrecognized device"; break;
						case  7: exit_string = "Device revision is not supported"; break;
						case  8: exit_string = "Erase failure"; break;
						case  9: exit_string = "Device is not blank"; break;
						case 10: exit_string = "Device programming failure"; break;
						case 11: exit_string = "Device verify failure"; break;
						case 12: exit_string = "Read failure"; break;
						case 13: exit_string = "Calculating checksum failure"; break;
						case 14: exit_string = "Setting security bit failure"; break;
						case 15: exit_string = "Querying security bit failure"; break;
						case 16: exit_string = "Exiting ISP failure"; break;
						case 17: exit_string = "Performing system test failure"; break;
						default: exit_string = "Unknown exit code"; break;
						}
					}
					else
					{
						switch (exit_code)
						{
						case 0: exit_string = "Success"; break;
						case 1: exit_string = "Illegal initialization values"; break;
						case 2: exit_string = "Unrecognized device"; break;
						case 3: exit_string = "Device revision is not supported"; break;
						case 4: exit_string = "Device programming failure"; break;
						case 5: exit_string = "Device is not blank"; break;
						case 6: exit_string = "Device verify failure"; break;
						case 7: exit_string = "SRAM configuration failure"; break;
						default: exit_string = "Unknown exit code"; break;
						}
					}

					printf("Exit code = %d... %s\n", exit_code, exit_string);
				}
				else if ((format_version == 2) &&
					(exec_result == JBIC_ACTION_NOT_FOUND))
				{
					if ((action == NULL) || (*action == '\0'))
					{
						printf("Error: no action specified for Jam STAPL file.\nProgram terminated.\n");
					}
					else
					{
						printf("Error: action \"%s\" is not supported for this Jam STAPL file.\nProgram terminated.\n", action);
					}
				}
				else if (exec_result < MAX_ERROR_CODE)
				{
					printf("Error at address %ld: %s.\nProgram terminated.\n",
						error_address, error_text[exec_result]);
				}
				else
				{
					printf("Unknown error code %d\n", exec_result);
				}

				/*
				*	Print out elapsed time
				*/
				if (verbose)
				{
					time_delta = (int) (end_time - start_time);
					printf("Elapsed time = %02u:%02u:%02u\n",
						time_delta / 3600,			/* hours */
						(time_delta % 3600) / 60,	/* minutes */
						time_delta % 60);			/* seconds */
				}
			}
		}
	}

	if (jtag_hardware_initialized) close_jtag_hardware();

	if (workspace != NULL) jbi_free(workspace);
	if (file_buffer != NULL) jbi_free(file_buffer);

	return (exit_status);
}

void initialize_jtag_hardware()
{
}

void close_jtag_hardware()
{
}

//#if !defined (DEBUG)
//#pragma optimize ("ceglt", off)
//#endif

void delay_loop(long count)
{
	while (count != 0L) {count--;if((count%1000000000==0)&&count>100)cout<<"ha!"<<endl;}
}
