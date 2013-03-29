//////// Standard Header file for the PIC16F1937 device ////////////////
#device PIC16F1937
#nolist
//////// Program memory: 8192x14  Data RAM: 496  Stack: 16
//////// I/O: 36   Analog Pins: 14
//////// Data EEPROM: 256
//////// C Scratch area: 77   ID Location: 8000
//////// Fuses: LP,XT,HS,RC,INTRC_IO,ECL,ECM,ECH,NOWDT,WDT_SW,WDT_NOSL,WDT
//////// Fuses: PUT,NOPUT,NOMCLR,MCLR,PROTECT,NOPROTECT,CPD,NOCPD,NOBROWNOUT
//////// Fuses: BROWNOUT_SW,BROWNOUT_NOSL,BROWNOUT,CLKOUT,NOCLKOUT,NOIESO
//////// Fuses: IESO,NOFCMEN,FCMEN,WRT,WRT_1000,WRT_200,NOWRT,VCAP_A0
//////// Fuses: VCAP_A5,VCAP_A6,NOVCAP,PLL_SW,PLL,NOSTVREN,STVREN,BORV25
//////// Fuses: BORV19,DEBUG,NODEBUG,NOLVP,LVP
//////// 
////////////////////////////////////////////////////////////////// I/O
// Discrete I/O Functions: SET_TRIS_x(), OUTPUT_x(), INPUT_x(),
//                         PORT_x_PULLUPS(), INPUT(),
//                         OUTPUT_LOW(), OUTPUT_HIGH(),
//                         OUTPUT_FLOAT(), OUTPUT_BIT()
// Constants used to identify pins in the above are:

#define PIN_A0  96
#define PIN_A1  97
#define PIN_A2  98
#define PIN_A3  99
#define PIN_A4  100
#define PIN_A5  101
#define PIN_A6  102
#define PIN_A7  103

#define PIN_B0  104
#define PIN_B1  105
#define PIN_B2  106
#define PIN_B3  107
#define PIN_B4  108
#define PIN_B5  109
#define PIN_B6  110
#define PIN_B7  111

#define PIN_C0  112
#define PIN_C1  113
#define PIN_C2  114
#define PIN_C3  115
#define PIN_C4  116
#define PIN_C5  117
#define PIN_C6  118
#define PIN_C7  119

#define PIN_D0  120
#define PIN_D1  121
#define PIN_D2  122
#define PIN_D3  123
#define PIN_D4  124
#define PIN_D5  125
#define PIN_D6  126
#define PIN_D7  127

#define PIN_E0  128
#define PIN_E1  129
#define PIN_E2  130
#define PIN_E3  131

////////////////////////////////////////////////////////////////// Useful defines
#define FALSE 0
#define TRUE 1

#define BYTE int8
#define BOOLEAN int1

#define getc getch
#define fgetc getch
#define getchar getch
#define putc putchar
#define fputc putchar
#define fgets gets
#define fputs puts

////////////////////////////////////////////////////////////////// Control
// Control Functions:  RESET_CPU(), SLEEP(), RESTART_CAUSE()
// Constants returned from RESTART_CAUSE() are:
#define NORMAL_POWER_UP      0x3C
#define BROWNOUT_RESTART     0x3E
#define MCLR_FROM_SLEEP      0x27
#define WDT_TIMEOUT          0x1F
#define WDT_FROM_SLEEP       0x0F
#define INTERRUPT_FROM_SLEEP 0x2F
#define MCLR_FROM_RUN        0x37
#define RESET_INSTRUCTION    0x3B
#define STACK_OVERFLOW       0xBF
#define STACK_UNDERFLOW      0x7F

////////////////////////////////////////////////////////////////// Timer 0
// Timer 0 (AKA RTCC)Functions: SETUP_COUNTERS() or SETUP_TIMER_0(),
//                              SET_TIMER0() or SET_RTCC(),
//                              GET_TIMER0() or GET_RTCC()
// Constants used for SETUP_TIMER_0() are:
#define T0_INTERNAL   0
#define T0_EXT_L_TO_H 32
#define T0_EXT_H_TO_L 48

#define T0_DIV_1      8
#define T0_DIV_2      0
#define T0_DIV_4      1
#define T0_DIV_8      2
#define T0_DIV_16     3
#define T0_DIV_32     4
#define T0_DIV_64     5
#define T0_DIV_128    6
#define T0_DIV_256    7


#define T0_8_BIT      0     

#define RTCC_INTERNAL   0      // The following are provided for compatibility
#define RTCC_EXT_L_TO_H 32     // with older compiler versions
#define RTCC_EXT_H_TO_L 48
#define RTCC_DIV_1      8
#define RTCC_DIV_2      0
#define RTCC_DIV_4      1
#define RTCC_DIV_8      2
#define RTCC_DIV_16     3
#define RTCC_DIV_32     4
#define RTCC_DIV_64     5
#define RTCC_DIV_128    6
#define RTCC_DIV_256    7
#define RTCC_8_BIT      0     

// Constants used for SETUP_COUNTERS() are the above
// constants for the 1st param and the following for
// the 2nd param:

////////////////////////////////////////////////////////////////// WDT
// Watch Dog Timer Functions: SETUP_WDT(), RESTART_WDT()
// WDT base is 1ms
//
#define WDT_ON      0x1000
#define WDT_OFF     0x2000

#define WDT_1MS     0x100
#define WDT_2MS     0x101
#define WDT_4MS     0x102
#define WDT_8MS     0x103
#define WDT_16MS    0x104
#define WDT_32MS    0x105
#define WDT_64MS    0x106
#define WDT_128MS   0x107
#define WDT_256MS   0x108
#define WDT_512MS   0x109
#define WDT_1S      0x10A
#define WDT_2S      0x10B
#define WDT_4S      0x10C
#define WDT_8S      0x10D
#define WDT_16S     0x10E
#define WDT_32S     0x10F
#define WDT_64S     0x110
#define WDT_128S    0x111
#define WDT_256S    0x112

////////////////////////////////////////////////////////////////// Timer 1
// Timer 1 Functions: SETUP_TIMER_1, GET_TIMER1, SET_TIMER1
// Constants used for SETUP_TIMER_1() are:
//      (or (via |) together constants from each group)
#define T1_DISABLED         0
#define T1_CAPSENSE         0xC5
#define T1_EXTERNAL         0x85
#define T1_EXTERNAL_SYNC    0x81
#define T1_FOSC             0x45
#define T1_INTERNAL         0x05

#define T1_ENABLE_T1OSC	    0x08

#define T1_DIV_BY_1         0x00
#define T1_DIV_BY_2         0x10
#define T1_DIV_BY_4         0x20
#define T1_DIV_BY_8         0x30

#define T1_GATE           0x8000
#define T1_GATE_INVERTED  0xC000
#define T1_GATE_TOGGLE    0xA000
#define T1_GATE_SINGLE    0x9000
#define T1_GATE_TIMER0    0x8100
#define T1_GATE_COMP1     0x8200 
#define T1_GATE_COMP2     0x8300  

#define T1_GATE_B5          0
#define T1_GATE_C4          0x10000

////////////////////////////////////////////////////////////////// Timer 2
// Timer 2 Functions: SETUP_TIMER_2, GET_TIMER2, SET_TIMER2
// Constants used for SETUP_TIMER_2() are:
#define T2_DISABLED         0
#define T2_DIV_BY_1         4
#define T2_DIV_BY_4         5
#define T2_DIV_BY_16        6
#define T2_DIV_BY_64        7 

////////////////////////////////////////////////////////////////// Timer 4
// Timer 4 Functions: SETUP_TIMER_4, GET_TIMER4, SET_TIMER4
// Constants used for SETUP_TIMER_4() are:
#define T4_DISABLED         0
#define T4_DIV_BY_1         4
#define T4_DIV_BY_4         5
#define T4_DIV_BY_16        6
#define T4_DIV_BY_64        7 

////////////////////////////////////////////////////////////////// Timer 6
// Timer 6 Functions: SETUP_TIMER_6, GET_TIMER5, SET_TIMER5
// Constants used for SETUP_TIMER_6() are:
//      (or (via |) together constants from each group)
#define T6_DISABLED         0
#define T6_DIV_BY_1         4
#define T6_DIV_BY_4         5
#define T6_DIV_BY_16        6
#define T6_DIV_BY_64        7 

////////////////////////////////////////////////////////////////// CCP
// CCP Functions: SETUP_CCPx, SET_PWMx_DUTY
// CCP Variables: CCP_x, CCP_x_LOW, CCP_x_HIGH
// Constants used for SETUP_CCPx() are:
#define CCP_OFF                         0
#define CCP_CAPTURE_FE                  4
#define CCP_CAPTURE_RE                  5
#define CCP_CAPTURE_DIV_4               6
#define CCP_CAPTURE_DIV_16              7
#define CCP_COMPARE_SET_ON_MATCH        8
#define CCP_COMPARE_CLR_ON_MATCH        9
#define CCP_COMPARE_INT                 0xA
#define CCP_COMPARE_RESET_TIMER         0xB
#define CCP_PWM                         0xC
#define CCP_PWM_PLUS_1                  0x1c  
#define CCP_PWM_PLUS_2                  0x2c
#define CCP_PWM_PLUS_3                  0x3c
#word   CCP_1       =                   getenv("SFR:CCPR1L")
#byte   CCP_1_LOW   =                   getenv("SFR:CCPR1L")
#byte   CCP_1_HIGH  =                   getenv("SFR:CCPR1H")
// The following should be used with the ECCP unit only (or these in)
#define CCP_PWM_H_H                     0x0c
#define CCP_PWM_H_L                     0x0d
#define CCP_PWM_L_H                     0x0e
#define CCP_PWM_L_L                     0x0f

#define CCP_PWM_FULL_BRIDGE             0x40
#define CCP_PWM_FULL_BRIDGE_REV         0xC0
#define CCP_PWM_HALF_BRIDGE             0x80

#define CCP_SHUTDOWN_ON_COMP1           0x100000
#define CCP_SHUTDOWN_ON_COMP2           0x200000
#define CCP_SHUTDOWN_ON_COMP            0x300000
#define CCP_SHUTDOWN_ON_INT0            0x400000
#define CCP_SHUTDOWN_ON_COMP1_INT0      0x500000
#define CCP_SHUTDOWN_ON_COMP2_INT0      0x600000
#define CCP_SHUTDOWN_ON_COMP_INT0       0x700000

#define CCP_SHUTDOWN_AC_L               0x000000
#define CCP_SHUTDOWN_AC_H               0x040000
#define CCP_SHUTDOWN_AC_F               0x080000

#define CCP_SHUTDOWN_BD_L               0x000000
#define CCP_SHUTDOWN_BD_H               0x010000
#define CCP_SHUTDOWN_BD_F               0x020000

#define CCP_SHUTDOWN_RESTART            0x80000000  

#define CCP_PULSE_STEERING_A            0x01000000
#define CCP_PULSE_STEERING_B            0x02000000
#define CCP_PULSE_STEERING_C            0x04000000
#define CCP_PULSE_STEERING_D            0x08000000
#define CCP_PULSE_STEERING_SYNC         0x10000000


#define CCP_TIMER2            0x0000
#define CCP_TIMER4            0x1000
#define CCP_TIMER6            0x2000

#define CCP2_C1             0x000
#define CCP2_B3             0x100
#define CCP_P2B_C0          0x000
#define CCP_P2B_D2          0x200  
#define CCP3_E0             0x000  
#define CCP3_B5             0x100

#word   CCP_2       =                   getenv("SFR:CCPR2L")
#byte   CCP_2_LOW   =                   getenv("SFR:CCPR2L")
#byte   CCP_2_HIGH  =                   getenv("SFR:CCPR2H")
#word   CCP_3       =                   getenv("SFR:CCPR3L")
#byte   CCP_3_LOW   =                   getenv("SFR:CCPR3L")
#byte   CCP_3_HIGH  =                   getenv("SFR:CCPR3H")
#word   CCP_4       =                   getenv("SFR:CCPR4L")
#byte   CCP_4_LOW   =                   getenv("SFR:CCPR4L")
#byte   CCP_4_HIGH  =                   getenv("SFR:CCPR4H")
#word   CCP_5       =                   getenv("SFR:CCPR5L")
#byte   CCP_5_LOW   =                   getenv("SFR:CCPR5L")
#byte   CCP_5_HIGH  =                   getenv("SFR:CCPR5H")
////////////////////////////////////////////////////////////////// SPI
// SPI Functions: SETUP_SPI, SPI_WRITE, SPI_READ, SPI_DATA_IN
// Constants used in SETUP_SPI() are:
#define SPI_DISABLED             0x00
#define SPI_MASTER               0x20
#define SPI_SLAVE                0x24
#define SPI_SCK_IDLE_HIGH        0x10
#define SPI_SCK_IDLE_LOW         0x00
#define SPI_CLK_DIV_4            0x00
#define SPI_CLK_DIV_16           0x01
#define SPI_CLK_DIV_64           0x02
#define SPI_CLK_T2               0x03
#define SPI_SS_DISABLED          0x01

#define SPI_SS_A5             0x000
#define SPI_SS_A0             0x10000

#define SPI_XMIT_L_TO_H          0x4000
#define SPI_XMIT_H_TO_L          0x0000

#define SPI_SAMPLE_AT_MIDDLE     0x0000
#define SPI_SAMPLE_AT_END        0x8000

//The following are provided for compatibility
#define SPI_L_TO_H       SPI_SCK_IDLE_LOW
#define SPI_H_TO_L       SPI_SCK_IDLE_HIGH

////////////////////////////////////////////////////////////////// UART
// Constants used in setup_uart() are:
// FALSE - Turn UART off
// TRUE  - Turn UART on
#define UART_ADDRESS           2
#define UART_DATA              4
////////////////////////////////////////////////////////////////// COMP
// Comparator Variables: C1OUT, C2OUT
// Constants used in setup_comparator() are:
//
#define NC_NC_NC_NC    0x00

//Pick one constant for COMP1
#define CP1_A0_A3      0x8000
#define CP1_A1_A3      0x8001
#define CP1_B3_A3      0x8002  
#define CP1_B1_A3      0x8003  
#define CP1_A0_DAC     0x8010
#define CP1_A1_DAC     0x8011
#define CP1_B3_DAC     0x8012  
#define CP1_B1_DAC     0x8013  
#define CP1_A0_FVR     0x8020
#define CP1_A1_FVR     0x8021
#define CP1_B3_FVR     0x8022  
#define CP1_B1_FVR     0x8023  
#define CP1_A0_VSS     0x8030  
#define CP1_A1_VSS     0x8031  
#define CP1_B3_VSS     0x8032  
#define CP1_B1_VSS     0x8033  
//Optionally OR with any of the following
#define CP1_INT_H2L    0x0040
#define CP1_INT_L2H    0x0080
#define CP1_OUT_ON_A4  0x2000  
#define CP1_INVERT     0x1000
#define CP1_FAST       0x0400
#define CP1_HYST       0x0200
#define CP1_SYNC       0x0100

//Pick one constant for COMP2
#define CP2_A0_A2      0x80000000
#define CP2_A1_A2      0x80010000
#define CP2_B3_A2      0x80020000  
#define CP2_B1_A2      0x80030000  
#define CP2_A0_DAC     0x80100000
#define CP2_A1_DAC     0x80110000
#define CP2_B3_DAC     0x80120000  
#define CP2_B1_DAC     0x80130000  
#define CP2_A0_FVR     0x80200000
#define CP2_A1_FVR     0x80210000
#define CP2_B3_FVR     0x80220000  
#define CP2_B1_FVR     0x80230000  
#define CP2_A0_VSS     0x80300000
#define CP2_A1_VSS     0x80310000
#define CP2_B3_VSS     0x80320000  
#define CP2_B1_VSS     0x80330000  
//Optionally OR with any of the following
#define CP2_INT_H2L    0x00400000
#define CP2_INT_L2H    0x00800000
#define CP2_OUT_ON_A5  0x20000000  
#define CP2_OUT_ON_A0  0x60000000  
#define CP2_INVERT     0x10000000
#define CP2_FAST       0x04000000
#define CP2_HYST       0x02000000
#define CP2_SYNC       0x01000000

#bit C1OUT = getenv("SFR:CM1CON0").6
#bit C2OUT = getenv("SFR:CM2CON0").6

////////////////////////////////////////////////////////////////// VREF
// Constants used in setup_vref() are:
//
#define VREF_OFF                        0
#define VREF_ON                         0x80
// The following may be OR'ed in with the above using |
#define VREF_ADC_OFF                    0
#define VREF_ADC_1v024                  0x01
#define VREF_ADC_2v048                  0x02
#define VREF_ADC_4v096                  0x03
// The following may be OR'ed in with the above using |
#define VREF_COMP_DAC_OFF               0
#define VREF_COMP_DAC_1v024             0x04
#define VREF_COMP_DAC_2v048             0x08
#define VREF_COMP_DAC_4v096             0x0C
// The following may be OR'ed in with the above using |
#define TEMPERATURE_INDICATOR_DISABLED  0
#define TEMPERATURE_INDICATOR_ENABLED   0x20
#define TEMPERATURE_RANGE_LOW           0
#define TEMPERATURE_RANGE_HIGH          0x10

////////////////////////////////////////////////////////////////// VREF
// Constants used in setup_low_volt_detect() are:
//
#define LVD_45 0x17
#define LVD_42 0x16
#define LVD_40 0x15
#define LVD_23 0x14
#define LVD_22 0x13
#define LVD_21 0x12
#define LVD_20 0x11
#define LVD_19 0x10


////////////////////////////////////////////////////////////////// DAC
// Digital to Analog Functions: SETUP_DAC(), DAC_WRITE()
// Constants used in SETUP_DAC() are:
#define DAC_OFF  0
#define DAC_VSS_VDD   0x80
#define DAC_VREF_VDD  0x81
#define DAC_VSS_VREF  0x84
#define DAC_VREF_VREF 0x85
#define DAC_VSS_FVR   0x88
#define DAC_FVR_VREF  0x89
// The following may be OR'ed in with the above using |
#define DAC_OUTPUT    0x20
#define DAC_LVP_POS   0x40
#define DAC_LVP_NEG   0

////////////////////////////////////////////////////////////////// LCD
// LCD Functions: SETUP_LCD, LCD_SYMBOL, LCD_LOAD, LCD_CONTRAST
// Constants used in setup_lcd() are:

// LCD Multiplex Mode (first param)
#define LCD_STATIC         0x0000000080
#define LCD_MUX12          0x0000000081
#define LCD_MUX13          0x0000000082
#define LCD_MUX14          0x0000000083
#define LCD_DISABLED       0x0000000000

//      Any of the following may be or'ed in with the first parameter
// LCD Sleep Mode
#define LCD_HALT_IDLE      0x0000000040
#define LCD_CONTINUE_IDLE  0x0000000000   //default

// LCD Clock Source
#define LCD_FOSC           0x0000000000   //default
#define LCD_TIMER1         0x0000000004
#define LCD_LFINTOSC       0x0000000008

// LCD Waveform Type
#define LCD_TYPE_B         0x0000008000
#define LCD_TYPE_A         0x0000000000  //default

// LCD Bias Mode (MUX12 & MUX13 Only)
#define LCD_BIAS12         0x0000004000
#define LCD_BIAS13         0x0000000000  //default

// LCD Internal Reference Enable
#define LCD_REF_ENABLED    0x0000800000
#define LCD_REF_DISABLED   0x0000000000  //default

// LCD Internal Reference Source
#define LCD_REF_VDD        0x0000000000  //default
#define LCD_REF_FVR        0x0000400000

// LCD Internal Reference Ladder Idle
#define LCD_FVR_DISABLED   0x0000200000
#define LCD_FVR_IGNORE     0x0000000000 //default

// LCD Voltage Pin Enable
#define LCD_VOLTAGE3       0x0000080000
#define LCD_VOLTAGE2       0x0000040000
#define LCD_VOLTAGE1       0x0000020000
#define LCD_NO_VOLTAGE     0x0000000000 //default

// LCD Ladder A Controls
#define LCD_A_NO_POWER     0x0000000000 //default
#define LCD_A_LOW_POWER    0x0040000000
#define LCD_A_MED_POWER    0x0080000000
#define LCD_A_HIGH_POWER   0x00C0000000

// LCD Ladder B Controls
#define LCD_B_NO_POWER     0x0000000000 //default
#define LCD_B_LOW_POWER    0x0010000000
#define LCD_B_MED_POWER    0x0020000000
#define LCD_B_HIGH_POWER   0x0030000000

// LCD Ladder A Time Interval
#define LCD_B_ONLY         0x0000000000  //default
#define LCD_A_CLOCK1       0x0001000000
#define LCD_A_CLOCK2       0x0002000000
#define LCD_A_CLOCK3       0x0003000000
#define LCD_A_CLOCK4       0x0004000000
#define LCD_A_CLOCK5       0x0005000000
#define LCD_A_CLOCK6       0x0006000000
#define LCD_A_CLOCK7       0x0007000000

// Constants used in lcd_symbol() are:
#define COM0 0
#define COM1 24
#define COM2 48
#define COM3 72


////////////////////////////////////////////////////////////////// INTERNAL RC
// Constants used in setup_oscillator() are:
// First param:
#define OSC_31KHZ   0
#define OSC_31250   (2*8)
#define OSC_62KHZ   (4*8)
#define OSC_125KHZ  (5*8)
#define OSC_250KHZ  (6*8)
#define OSC_500KHZ  (7*8)
#define OSC_1MHZ    (11*8)
#define OSC_2MHZ    (12*8)
#define OSC_4MHZ    (13*8)
#define OSC_8MHZ    (14*8)
#define OSC_16MHZ   (15*8)
// The following may be OR'ed in with the above using |
#define OSC_TIMER1  1
#define OSC_INTRC   2
#define OSC_NORMAL  0
// The following may be OR'ed in with the above using |
#define OSC_PLL_ON  0x80
#define OSC_PLL_OFF 0


////////////////////////////////////////////////////////////////// ADC
// ADC Functions: SETUP_ADC(), SETUP_ADC_PORTS() (aka SETUP_PORT_A),
//                SET_ADC_CHANNEL(), READ_ADC()
// Constants used for SETUP_ADC() are:
#define ADC_OFF                0              // ADC Off
#define ADC_CLOCK_DIV_2    0x100
#define ADC_CLOCK_DIV_4     0x40
#define ADC_CLOCK_DIV_8     0x10
#define ADC_CLOCK_DIV_32    0x20
#define ADC_CLOCK_DIV_16    0x50
#define ADC_CLOCK_DIV_64    0x60
#define ADC_CLOCK_INTERNAL  0x30              // Internal 2-6us

// Constants used in SETUP_ADC_PORTS() are:
// First argument:
// OR together desired pins
#define sAN0          0x01000000    //| A0
#define sAN1          0x02000000    //| A1
#define sAN2          0x04000000    //| A2
#define sAN3          0x08000000    //| A3
#define sAN4          0x20000000    //| A5
#define sAN5          0x00010000    //| E0 
#define sAN6          0x00020000    //| E1 
#define sAN7          0x00040000    //| E2 
#define sAN8          0x00000400    //| B2
#define sAN9          0x00000800    //| B3
#define sAN10         0x00000200    //| B1
#define sAN11         0x00001000    //| B4
#define sAN12         0x00000100    //| B0
#define sAN13         0x00002000    //| B5
#define NO_ANALOGS             0    // None
#define ALL_ANALOG    0x2F073F00    // A0 A1 A2 A3 A4 E0 E1 E2 B0 B1 B2 B3 B4 B5 

// Optional Second argument:
#define VSS_VDD             0x00    //| Range 0-Vdd
#define VSS_VREF            0x02    //| Range 0-VrefH
#define VSS_FVR             0x03    //| Range 0-Fixed Voltage Reference
#define VREF_VDD            0x04    //| Range VrefL-Vdd 
#define VREF_VREF           0x06    //| Range VrefL-VrefH 
#define VREF_FVR            0x07    //| Range VrefL-Fixed Voltage Reference 

// Constants used in SET_ADC_CHANNEL() are:
// either use the channel number or one of the following
#define TEMPERATURE_INDICATOR     0x1D
#define DAC_CHANNEL               0x1E
#define FVR_CHANNEL               0x1F

// Constants used in READ_ADC() are:
#define ADC_START_AND_READ     7   // This is the default if nothing is specified
#define ADC_START_ONLY         1
#define ADC_READ_ONLY          6

////////////////////////////////////////////////////////////////// INT
// Interrupt Functions: ENABLE_INTERRUPTS(), DISABLE_INTERRUPTS(),
//                      CLEAR_INTERRUPT(), INTERRUPT_ACTIVE(),
//                      EXT_INT_EDGE()
//
// Constants used in EXT_INT_EDGE() are:
#define L_TO_H              0x40
#define H_TO_L                 0
// Constants used in ENABLE/DISABLE_INTERRUPTS() are:
#define GLOBAL                    0x0BC0
#define PERIPH                    0x0B40
#define INT_RB                    0x31FF0B08
#define INT_EXT_L2H               0x50000B10
#define INT_EXT_H2L               0x60000B10
#define INT_EXT                   0x300B10
#define INT_TIMER0                0x300B20
#define INT_TIMER1                0x309101
#define INT_TIMER2                0x309102
#define INT_CCP1                  0x309104
#define INT_SSP                   0x309108
#define INT_TBE                   0x309110
#define INT_RDA                   0x309120
#define INT_AD                    0x309140
#define INT_TIMER1_GATE            0x309180
#define INT_LCD                   0x309204
#define INT_BUSCOL                0x309208
#define INT_EEPROM                0x309210
#define INT_COMP                  0x309220
#define INT_COMP2                 0x309240
#define INT_OSC_FAIL              0x309280
#define INT_TIMER4                0x309302
#define INT_TIMER6                0x309308
#define INT_CCP3                  0x309310
#define INT_CCP4                  0x309320
#define INT_CCP5                  0x309340
#define INT_CCP2                  0x309201
#define INT_RB0                   0x31010B08
#define INT_RB0_L2H               0x11010B08
#define INT_RB0_H2L               0x21010B08
#define INT_RB1                   0x31020B08
#define INT_RB1_L2H               0x11020B08
#define INT_RB1_H2L               0x21020B08
#define INT_RB2                   0x31040B08
#define INT_RB2_L2H               0x11040B08
#define INT_RB2_H2L               0x21040B08
#define INT_RB3                   0x31080B08
#define INT_RB3_L2H               0x11080B08
#define INT_RB3_H2L               0x21080B08
#define INT_RB4                   0x31100B08
#define INT_RB4_L2H               0x11100B08
#define INT_RB4_H2L               0x21100B08
#define INT_RB5                   0x31200B08
#define INT_RB5_L2H               0x11200B08
#define INT_RB5_H2L               0x21200B08
#define INT_RB6                   0x31400B08
#define INT_RB6_L2H               0x11400B08
#define INT_RB6_H2L               0x21400B08
#define INT_RB7                   0x31800B08
#define INT_RB7_L2H               0x11800B08
#define INT_RB7_H2L               0x21800B08

#list
