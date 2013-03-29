#include <16LF1937.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "SITE_XX.h"

#fuses INTRC_IO
#fuses NOPROTECT
#fuses BROWNOUT
#fuses NOMCLR
#fuses NOCPD
#fuses NOWDT // WDT controlled by sw
#fuses NOPUT
#fuses NOFCMEN
#fuses NOIESO
#case

#use delay(internal=8M,restart_wdt)
#use I2C (master,force_hw,scl=PIN_C3,sda=PIN_C4)
#use RS232 (BAUD=9600,UART2)

// RS232 variables / buffers {{{
char sBuffer[16];
int8 sBufferIndex;
int1 sBufferFlag;
// }}}

#define DEBUG_SBUFFER

// Commands
#define SET_REG 2
#define GET_REG 7
#define ADM_CMD 0

// DTMF character -- MT8888 maps {{{
#define d1 0x01
#define d2 0x02
#define d3 0x03
#define d4 0x04
#define d5 0x05
#define d6 0x06
#define d7 0x07
#define d8 0x08
#define d9 0x09
#define d0 0x0a
#define ds 0x0b
#define dp 0x0c
#define da 0x0d
#define db 0x0e
#define dc 0x0f
#define dd 0x00
// }}}

// sDTMF character structure 
//
typedef struct { 
			int Key 	: 4;
			int Strobe	: 1;
			int Last	: 1;
			int UNUSED	: 2;
} sDTMF;

typedef struct {
		int RX0 : 1;
		int RX1 : 1;
		int RX2 : 1;
		int RX3 : 1;
} sCOR;

int8 command,argument,value;
char argument_name[8];

// cMorseChar {{{
// Word is read from right to left (LSB to MSB)
int8 rom cMorseChar[] = {
	0b10101010, // 0 (dah dah dah dah dah)	0
	0b01101010, // 1 (dit dah dah dah dah)	1
	0b01011010, // 2 (dit dit dah dah dah)	2
	0b01010110, // 3 (dit dit dit dah dah)	3
	0b01010101, // 4 (dit dit dit dit dah)	4
	0b01010101, // 5 (dit dit dit dit dit)	5
	0b10010101, // 6 (dah dit dit dit dit)	6
	0b10100101, // 7 (dah dah dit dit dit)	7
	0b10101001, // 8 (dah dah dah dit dit)	8
	0b10101010, // 9 (dah dah dah dah dit)	9
	0b01100000, // a (dit dah)		10
	0b10010101, // b (dah dit dit dit)	11
	0b10011001, // c (dah dit dah dit)	12
	0b10010100, // d (dah dit dit)		13
	0b01000000, // e (dit)			14
	0b01011001, // f (dit dit dah dit)	15
	0b10100100, // g (dah dah dit) 		16
	0b01010101, // h (dit dit dit dit)	17
	0b01010000, // i (dit dit)		18
	0b01101010, // j (dit dah dah dah)	19
	0b10011000, // k (dah dit dah)		20
	0b01100101, // l (dit dah dit dit)	21
	0b10100000, // m (dah dah)		22
	0b10010000, // n (dah dit)		23
	0b10101000, // o (dah dah dah)		24
	0b01101001, // p (dit dah dah dit)	25
	0b10100110, // q (dah dah dit dah)	26
	0b01100100, // r (dit dah dit)		27
	0b01010100, // s (dit dit dit)		28
	0b10000000, // t (dah)			29
	0b01011000, // u (dit dit dah)		30
	0b01010110, // v (dit dit dit dah)	31
	0b01101000, // w (dit dah dah)		32
	0b10010110, // x (dah dit dit dah)	33
	0b10011010, // y (dah dit dah dah)	34
	0b10100101 // z (dah dah dit dit)	35
}; // }}}

#define REG_NAME_SIZE 9
typedef struct sRegMap_t { 
	int	 *reg_ptr;
	char reg_name[REG_NAME_SIZE];
	int	  reg_size;
} sRegMap;

int8 COR_IN;
int8 Polarity;
int8 COR0_GAIN;

//rom char COR_IN_NAME[]="COR_IN";
//rom char POL_NAME[]="POLARITY";
//rom char COR0_GAIN_NAME[]="C0GAIN";

//rom char * rom strPtr=COR_IN_NAME;

rom sRegMap RegMap[]={
    {&COR0_GAIN,{"C0GAIN"},1},
	{&Polarity,{"POLARITY"},1},
	{&COR_IN,{"COR_IN"},1}
};
 	
int RegMapNum=sizeof(RegMap)/sizeof(sRegMap);
