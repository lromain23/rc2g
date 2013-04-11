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
unsigned int sBufferIndex;
unsigned int1 sBufferFlag;
// }}}

#define DEBUG_SBUFFER

// Commands
#define SET_REG 2
#define GET_REG 7
#define ADM_CMD 9
#define HELP    3


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

#define REG_NAME_SIZE 6
unsigned int command,argument,value;
char argument_name[REG_NAME_SIZE];

// cMorseChar {{{
// Word is read from right to left (LSB to MSB)
unsigned int rom cMorseChar[] = {
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

typedef struct sRegMap_t { 
//	int      reg_name_index;
	int *	 reg_ptr;
	int	 default_value;
};

unsigned int COR_IN;
unsigned int COR_FLAG;
unsigned int Polarity;
unsigned int RX_GAIN[4][4];
unsigned int AuxIn[3],AuxOut[3];
unsigned int CORPriority[4];
unsigned int RX_PTT[4];

#define COR0 PIN_B0
#define COR1 PIN_B1
#define COR2 PIN_B2
#define COR3 PIN_B3
#define DTMF_INT PIN_B4

#define PTT0 PIN_A4
#define PTT1 PIN_A5
#define PTT2 PIN_E0
#define PTT3 PIN_E1

#define RX0_EN PIN_A0 
#define RX1_EN PIN_A1
#define RX2_EN PIN_A2
#define RX3_EN PIN_A3

#define COR0_MASK 0x01
#define COR1_MASK 0x02
#define COR2_MASK 0x04
#define COR3_MASK 0x08
#byte IOCBF = 0x396 

//rom char COR_IN_NAME[]="COR_IN";
//rom char POL_NAME[]="POLARITY";
//rom char COR0_GAIN_NAME[]="C0GAIN";

//rom char * rom strPtr=COR_IN_NAME;

#define DEFAULT_GAIN 32

const char reg_name[][REG_NAME_SIZE]={
	{"POL"},	// 0
	{"R0G0"},	// 1
	{"R0G1"},	// 2
	{"R0G2"},	// 3
	{"R0G3"},	// 4
	{"R1G0"},	// 5
	{"R1G1"},	// 6
	{"R1G2"},	// 7
	{"R1G3"},	// 8
	{"R2G0"},	// 9
	{"R2G1"},	// 10
	{"R2G2"},	// 11
	{"R2G3"},	// 12
	{"R3G0"},	// 13
	{"R3G1"},	// 14
	{"R3G2"},	// 15
	{"R3G3"},	// 16
	{"XI0"},	// 17
	{"XI1"},	// 18
	{"XI2"},	// 19
	{"XO0"},	// 20
	{"XO1"},	// 21
	{"XO2"},	// 22
	{"C0P"},	// 23
	{"C1P"},	// 24
	{"C2P"},	// 25
	{"C3P"},	// 26
	{"R0PTT"},	    // 27
	{"R1PTT"},	    // 28
	{"R2PTT"},	    // 29
	{"R3PTT"},	    // 30

};


//struct sRegMap_t const RegMap[]={
//	{0, &Polarity   ,15},
//	{1, &RX_GAIN[0][0] ,DEFAULT_GAIN},
//	{2, &RX_GAIN[0][1] ,DEFAULT_GAIN},
//	{3, &RX_GAIN[0][2] ,DEFAULT_GAIN},
//	{4, &RX_GAIN[0][3] ,DEFAULT_GAIN},
//	{5, &RX_GAIN[1][0] ,DEFAULT_GAIN},
//	{6, &RX_GAIN[1][1] ,DEFAULT_GAIN},
//	{7, &RX_GAIN[1][2] ,DEFAULT_GAIN},
//	{8, &RX_GAIN[1][3] ,DEFAULT_GAIN},
//	{9, &RX_GAIN[2][0] ,DEFAULT_GAIN},
//	{10,&RX_GAIN[2][1] ,DEFAULT_GAIN},
//	{11,&RX_GAIN[2][2],DEFAULT_GAIN},
//	{12,&RX_GAIN[2][3],DEFAULT_GAIN},
//	{13,&RX_GAIN[3][0],DEFAULT_GAIN},
//	{14,&RX_GAIN[3][1],DEFAULT_GAIN},
//	{15,&RX_GAIN[3][2],DEFAULT_GAIN},
//	{16,&RX_GAIN[3][3],DEFAULT_GAIN},
//	{17,&AuxIn[0]   ,0},
//	{18,&AuxIn[1]   ,0},
//	{19,&AuxIn[2]   ,0},
//	{20,&AuxOut[0]  ,1},
//	{21,&AuxOut[1]  ,1},
//	{22,&AuxOut[2]  ,1},
//	{23,&CORPriority[0] ,0},
//	{24,&CORPriority[1] ,5},
//	{25,&CORPriority[2] ,5},
//	{26,&CORPriority[3] ,3}
//	{26,&RX_PTT[0] ,0x0E},
//	{26,&RX_PTT[1] ,0x0D},
//	{26,&RX_PTT[2] ,0x0B},
//	{26,&RX_PTT[3] ,0x07},
//};
struct sRegMap_t const RegMap[]={
	{&Polarity      ,15},
	{&RX_GAIN[0][0] ,DEFAULT_GAIN},
	{&RX_GAIN[0][1] ,DEFAULT_GAIN},
	{&RX_GAIN[0][2] ,DEFAULT_GAIN},
	{&RX_GAIN[0][3] ,DEFAULT_GAIN},
	{&RX_GAIN[1][0] ,DEFAULT_GAIN},
	{&RX_GAIN[1][1] ,DEFAULT_GAIN},
	{&RX_GAIN[1][2] ,DEFAULT_GAIN},
	{&RX_GAIN[1][3] ,DEFAULT_GAIN},
	{&RX_GAIN[2][0] ,DEFAULT_GAIN},
	{&RX_GAIN[2][1] ,DEFAULT_GAIN},
	{&RX_GAIN[2][2],DEFAULT_GAIN},
	{&RX_GAIN[2][3],DEFAULT_GAIN},
	{&RX_GAIN[3][0],DEFAULT_GAIN},
	{&RX_GAIN[3][1],DEFAULT_GAIN},
	{&RX_GAIN[3][2],DEFAULT_GAIN},
	{&RX_GAIN[3][3],DEFAULT_GAIN},
	{&AuxIn[0]     ,0},
	{&AuxIn[1]     ,0},
	{&AuxIn[2]     ,0},
	{&AuxOut[0]    ,1},
	{&AuxOut[1]    ,1},
	{&AuxOut[2]    ,1},
	{&CORPriority[0] ,1},
	{&CORPriority[1] ,5},
	{&CORPriority[2] ,5},
	{&CORPriority[3] ,3},
	{&RX_PTT[0]    ,0x0E},
	{&RX_PTT[1]    ,0x0D},
	{&RX_PTT[2]    ,0x0B},
	{&RX_PTT[3]    ,0x07},
};
 	
unsigned int const RegMapNum=sizeof(RegMap)/sizeof(struct sRegMap_t);
