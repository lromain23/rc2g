#include <16F1937.h>

#fuses INTRC_IO
#fuses NOPROTECT
#fuses BROWNOUT
#fuses NOMCLR
#fuses NOCPD
#fuses WDT // WDT controlled by sw
#fuses NOPUT
#fuses NOFCMEN
#fuses NOIESO
#fuses NODEBUG
#case

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#define MCHAR(c) c-'a'+10

#define TAIL_CHAR 0

#use delay(internal=8M,restart_wdt)
#use I2C (master,force_hw,I2C1)
#use RS232 (BAUD=9600,UART1,RESTART_WDT)
#use fast_io (b)
#use fast_io (c)
#use fast_io (d)
#use fast_io (e)

//function headers
void send_tail(void);
void morse(char);
void dit(void);
void dah(void);
void prompt(void);
void increment(int);
void pot_values_to_lcd(void);
void init_variables(int1 src);
void status(void);
void set_var(void);
void tokenize_sBuffer(void);
void store_variables(void);
void clear_dtmf_array(void);
void dtmf_send_digit(int);
void romstrcpy(char *,rom char *);
void update_aux_in(void);

// Variables accessed using linear addressing {{{
unsigned int RX_GAIN[4][4];
unsigned int AuxIn[3],AuxInSW[3];
unsigned int AuxOut[3];
unsigned int RXPriority[4];
unsigned int RX_PTT[4];
unsigned int Morse[6];
unsigned int AuxOutOp[3],AuxOutArg[3];
unsigned int AuxInOp[3],AuxInArg[3];
unsigned int COR_IN;
unsigned int Enable,Enable_Mask;
unsigned int Polarity;
unsigned int SiteID;
unsigned int Tail;
unsigned int TOT_Min;
unsigned int COR_EMUL;
unsigned int MorseDitLength;
unsigned int TailChar;
// Variables accessed using linear addressing }}}

// COR variables {{{
unsigned int CurrentCorMask;
unsigned int CurrentCorIndex;
unsigned int CurrentCorPriority;
// }}}

// RS232 variables / buffers {{{
char sBuffer[16];
unsigned int sBufferIndex;
unsigned int1 sBufferFlag;
// }}}

//#define DEBUG_SBUFFER
#define ESC 0x1B

// Commands
#define SET_REG 2
#define GET_REG 3
#define SAVE_SETTINGS 4
#define RESTORE_SETTINGS 5
#define INCREMENT_REG 6
#define DECREMENT_REG 7
#define STATUS    8
#define REBOOT    9
#define DTMF_SEND 10
#define MORSE_SEND 11
#define I2C_SEND 12

// Auxiliary Output Operators
#define AUX_OUT_IDLE 0
#define AUX_OUT_FOLLOW_COR 0x01
#define AUX_OUT_FOLLOW_AUX_IN 0x02
// Follow COR args:
#define AUX_OUT_FOLLOW_COR0 0x01
#define AUX_OUT_FOLLOW_COR1 0x02
#define AUX_OUT_FOLLOW_COR2 0x04
#define AUX_OUT_FOLLOW_COR3 0x08
#define AUX_OUT_FOLLOW_COR_INVERT0 0x10
#define AUX_OUT_FOLLOW_COR_INVERT1 0x20
#define AUX_OUT_FOLLOW_COR_INVERT2 0x40
#define AUX_OUT_FOLLOW_COR_INVERT3 0x80
// Follow AUX_IN args:
#define AUX_OUT_FOLLOW_AUX_IN0 0x01
#define AUX_OUT_FOLLOW_AUX_IN0_INV 0x11
#define AUX_OUT_FOLLOW_AUX_IN1 0x02
#define AUX_OUT_FOLLOW_AUX_IN1_INV 0x22
#define AUX_OUT_FOLLOW_AUX_IN2 0x04
#define AUX_OUT_FOLLOW_AUX_IN2_INV 0x44

// Auxiliary Input Operators
// Set mask bits to 1 to make Enable bit controllable by AuxIn
// The AUXI_ENABLE# words can be OR'ed to include other enable
// signals controlled by the same AuxIn.
// All signals that are not included as the argument will be 
// gated off when the Aux Input is low.
#define AUXI_ENABLE 0x01
#define AUXI_ENABLE1 0x01
#define AUXI_ENABLE2 0x02
#define AUXI_ENABLE3 0x04
#define AUXI_ENABLE4 0x08

#define AUXI_TAIL_WHEN_LO 0x02
#define AUXI_TAIL_WHEN_HI 0x03
#define AUXI_TAIL_CHAR MCHAR('s')

#define AUXI_EMULATE_COR 0x04
#define AUXI_EMULATE_COR_ACTIVE_LO 0x10
#define AUXI_EMULATE_COR0 0x01
#define AUXI_EMULATE_COR1 0x02
#define AUXI_EMULATE_COR2 0x04
#define AUXI_EMULATE_COR3 0x08


// Digital TrimPot
//
#define TRIMPOT_READ_CMD  0x51
#define TRIMPOT_WRITE_CMD 0x50
#define LCD_I2C_ADD 0x60
#define LCD_LINE1 0x60
#define LCD_LINE2 0x62
#define LCD_LINE3 0x64
#define LCD_LINE4 0x66
unsigned int CurrentTrimPot;
unsigned long rtcc_cnt;
unsigned long aux_timer;
#define AUX_TIMER_1S 31
#define AUX_TIMER_500ms 16 
#define AUX_TIMER_400ms 13 
#define AUX_TIMER_300ms 10 
#define AUX_TIMER_200ms 6
#define AUX_TIMER_100ms 3
#define AUX_TIMER_60ms 2
#define AUX_TIMER_30ms 1
const int MorseLen[4]={AUX_TIMER_60ms,AUX_TIMER_100ms,AUX_TIMER_200ms,AUX_TIMER_300ms};

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

#define DTMF_ARRAY_SIZE 10
sDTMF DTMF_ARRAY[DTMF_ARRAY_SIZE];
sDTMF *DTMF_ptr;

typedef struct {
		int RX0 : 1;
		int RX1 : 1;
		int RX2 : 1;
		int RX3 : 1;
} sCOR;

#define REG_NAME_SIZE 6
unsigned int command,argument,value;
#LOCATE command=0x070
char argument_name[REG_NAME_SIZE];
char LCD_str[21];
// RegisterPointer is set by the get_var command.
// It points to the last register that was accessed.
// It is used by the INCR or DECR commands
unsigned int  LastRegisterIndex;
unsigned int  LastRegisterIndexValid;

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
	int	 non_volatile : 1;
};

int dtmf_read(int1 rs);
void dtmf_write(int data,int1 rs);

int1       COR_FLAG;
int1       SECOND_FLAG;
int1       MINUTE_FLAG;
int1       THIRTY_MIN_FLAG;
int1       COR_DROP_FLAG;
int1       AUX_IN_FLAG;
int        SecondCounter,MinuteCounter;
unsigned long TOT_SecondCounter;
int1	     DTMF_FLAG;
int1	     DTMF_IN_FLAG;
int1	     CLEAR_DTMF_FLAG;
int1       PROMPT_FLAG;

// Source is used by init_variables
// EEPROM -- Initializes variables using values stored in EEPROM
// DEFAULT -- Initializes variables using values in ROM
#define PTT_TIMEOUT_SECS 60
#define USE_EEPROM_VARS 1
#define USE_DEFAULT_VARS 0
#define EEPROM   1
#define RAM      0

#define DTMF_D0  PIN_D0
#define DTMF_D1  PIN_D1
#define DTMF_D2  PIN_D2
#define DTMF_D3  PIN_D3
#define DTMF_REB PIN_D4 // PH2 (12)
#define DTMF_WEB PIN_D5 // RWb (9)
#define DTMF_RS  PIN_D6 // RS0 (11)
#define DTMF_CS  PIN_D7 // CSB (10)
#define DTMF_INT PIN_B4
#define CONTROL_REG 1
#define DATA_REG 0
#define DTMF_IRQ         0x01
#define DTMF_TX_BUF_EMPTY 0x02
#define DTMF_BUFFER_FULL 0x04
// Register A bits
#define TOUT     0x01
#define IRQ		 0x04
#define RSELB	 0x08
// Register B bits
#define BURST    0x00
#define BURST_OFF 0x01
#define DUAL_TONE 0x00
#define SINGLE_TONE 0x04
#define ST_ROW   0x00
#define ST_COL   0x08
#define LCD_ENABLE

#define AUX_IN0  PIN_B6
#define AUX_IN1  PIN_B7
#define AUX_IN2  PIN_C0
#define AUX_OUT0 PIN_C1
#define AUX_OUT1 PIN_C5
#define AUX_OUT2 PIN_E2


#define COR0 PIN_B0
#define COR1 PIN_B1
#define COR2 PIN_B2
#define COR3 PIN_B3

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
#define DTMF_INT_MASK 0x10
#byte WPUB  = 0x20D
#byte IOCBF = 0x396 

//rom char COR_IN_NAME[]="COR_IN";
//rom char POL_NAME[]="POLARITY";
//rom char COR0_GAIN_NAME[]="C0GAIN";

//rom char * rom strPtr=COR_IN_NAME;

#ifndef GAIN
  #define DEFAULT_GAIN 32
#endif
#ifndef POLARITY_DEF_VAL
  #define POLARITY_DEF_VAL 0x0F
#endif
#ifndef ENABLE_DEFAULT
  #define ENABLE_DEFAULT 0x0F
#endif
const char RX_PIN[4]={RX0_EN,RX1_EN,RX2_EN,RX3_EN};
const char PTT_PIN[4]={PTT0,PTT1,PTT2,PTT3};
const int AUX_OUT_PIN[3]={AUX_OUT0,AUX_OUT1,AUX_OUT2};
const int AUX_IN_PIN[3] ={AUX_IN0 ,AUX_IN1 ,AUX_IN2};

char const reg_name[][REG_NAME_SIZE]={
	{"EN"},	  // 0
	{"POL"},	  // 1
	{"R1G1"},	  // 2
	{"R1G2"},	  // 3
	{"R1G3"},	  // 4
	{"R1G4"},	  // 5
	{"R2G1"},	  // 6
	{"R2G2"},	  // 7
	{"R2G3"},	  // 8
	{"R2G4"},	  // 9
	{"R3G1"},	  // 10
	{"R3G2"},	  // 11
	{"R3G3"},	  // 12
	{"R3G4"},	  // 13
	{"R4G1"},	  // 14
	{"R4G2"},	  // 15
	{"R4G3"},	  // 16
	{"R4G4"},	  // 17
	{"XI1"},	  // 18
	{"XI2"},	  // 19
	{"XI3"},	  // 20
	{"XO1"},	  // 21
	{"XO2"},	  // 22
	{"XO3"},	  // 23
	{"R1P"},	  // 24
	{"R2P"},	  // 25
	{"R3P"},	  // 26
	{"R4P"},	  // 27
	{"R1PTT"},  // 28
	{"R2PTT"},  // 29
	{"R3PTT"},  // 30
	{"R4PTT"},  // 31
    {"SID"},  // 32
    {"MRSL"}, // 33
    {"MRS1"}, // 34
    {"MRS2"}, // 35
    {"MRS3"}, // 36
    {"MRS4"}, // 37
    {"MRS5"}, // 38
    {"MRS6"}, // 39
    {"XOO1"}, // 40
    {"XOO2"}, // 41
    {"XOO3"}, // 42
    {"XOA1"}, // 43
    {"XOA2"}, // 44
    {"XOA3"}, // 45
    {"XIO1"}, // 46
    {"XIO2"}, // 47
    {"XIO3"}, // 48
    {"XIA1"}, // 49
    {"XIA2"}, // 50 
    {"XIA3"}, // 51
    {"TAIL"}, // 52
    {"TOT"},  // 53
    {"COR"},  // 54
    {"CPOT"}  // 55
};

#include "SITE_XX.h"

struct sRegMap_t const RegMap[]={
	{&Enable        ,ENABLE_DEFAULT  ,EEPROM},
	{&Polarity      ,POLARITY_DEF_VAL,EEPROM},
	{&RX_GAIN[0][0] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[0][1] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[0][2] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[0][3] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[1][0] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[1][1] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[1][2] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[1][3] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[2][0] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[2][1] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[2][2] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[2][3] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[3][0] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[3][1] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[3][2] ,DEFAULT_GAIN, EEPROM},
	{&RX_GAIN[3][3] ,DEFAULT_GAIN, EEPROM},
	{&AuxIn[0]      ,0           , EEPROM},
	{&AuxIn[1]      ,0           , EEPROM},
	{&AuxIn[2]      ,0           , EEPROM},
	{&AuxOut[0]     ,0           , EEPROM},
	{&AuxOut[1]     ,0           , EEPROM},
	{&AuxOut[2]     ,0           , EEPROM},
	{&RXPriority[0] ,1           , EEPROM},
	{&RXPriority[1] ,3           , EEPROM},
	{&RXPriority[2] ,3           , EEPROM},
	{&RXPriority[3] ,2           , EEPROM},
	{&RX_PTT[0]     ,0x0E        , EEPROM},
	{&RX_PTT[1]     ,0x0D        , EEPROM},
	{&RX_PTT[2]     ,0x0B        , EEPROM},
	{&RX_PTT[3]     ,0x07        , EEPROM},
	{&SiteID        ,SITE_ID_VAL , EEPROM},
	{&MorseDitLength ,1          , EEPROM},
  {&Morse[0]      ,MCHAR('v')  , EEPROM},
  {&Morse[1]      ,MCHAR('e')  , EEPROM},
  {&Morse[2]      ,2           , EEPROM},
  {&Morse[3]      ,MCHAR('r')  , EEPROM},
  {&Morse[4]      ,MCHAR('e')  , EEPROM},
  {&Morse[5]      ,MCHAR('h')  , EEPROM},
	{&AuxOutOp[0]   ,AUXOUTOP0   , EEPROM},
	{&AuxOutOp[1]   ,AUXOUTOP1   , EEPROM},
	{&AuxOutOp[2]   ,AUXOUTOP2   , EEPROM},
	{&AuxOutArg[0]  ,AUXOUTARG0  , EEPROM},
	{&AuxOutArg[1]  ,AUXOUTARG1  , EEPROM},
	{&AuxOutArg[2]  ,AUXOUTARG2  , EEPROM},
	{&AuxInOp[0]    ,AUXINOP0    , EEPROM},
	{&AuxInOp[1]    ,AUXINOP1    , EEPROM},
	{&AuxInOp[2]    ,AUXINOP2    , EEPROM},
	{&AuxInArg[0]   ,AUXINARG0   , EEPROM},
	{&AuxInArg[1]   ,AUXINARG1   , EEPROM},
	{&AuxInArg[2]   ,AUXINARG2   , EEPROM},
  {&Tail          ,TAIL_CHAR   , EEPROM},
  {&TOT_Min       ,TOT_MIN     , EEPROM},
	{&COR_EMUL      ,0x00        , RAM},
	{&CurrentTrimPot,0x00        , RAM},
};


// AuxInOp/AuxInArg are not initialized correctly.
// Debug using RS232.
 	
unsigned int const RegMapNum=sizeof(RegMap)/sizeof(struct sRegMap_t);
