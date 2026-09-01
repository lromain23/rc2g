// AuxIn1  --> Input from hydro out
// AuxOut2 --> Connected to 6M TX (Radio 4)
// AuxOut1 --> Connected to 2M TX (Radio 2)

#define SITE_ID_VAL  	73
#define SITE_GID_VAL 	90
#define POLARITY_DEF_VAL 0x0B 
#define ENABLE_DEFAULT 15
#define TOT_MIN 5
#define LINK_TOT 0
#define DEFAULT_GAIN 32
//#define LCD_TYPE_PI 0

#define MORSEID0	MCHAR('v')
#define MORSEID1	MCHAR('e')
#define MORSEID2	2
#define MORSEID3	MCHAR('r')
#define MORSEID4	MCHAR('e')
#define MORSEID5	MCHAR('h')

#define AUXOUTOP0 0 
#define AUXOUTARG0 0
#define AUXOUTOP1 0
#define AUXOUTARG1 0

#define AUXOUTOP2 AUX_OUT_FOLLOW_PTT
#define AUXOUTARG2 AUX_OUT_FOLLOW_PTT2 | AUX_OUT_FOLLOW_PTT3 | AUX_OUT_FOLLOW_PTT4 | AUX_OUT_FOLLOW_PTT_DELAY | AUX_OUT_FOLLOW_PTT_INVERT_OUTPUT
//#define AUXINOP0 AUXI_ENABLE
//#define AUXINARG0 AUXI_ENABLE2|AUXI_ENABLE3|AUXI_ENABLE4
// AuxIn1 used by AuxOut 1
#define AUXINOP0 AUXI_TAIL_WHEN_LO
#define AUXINARG0 MCHAR('b')
#define AUXINOP1 0
#define AUXINARG1 0
#define AUXINOP2 0
#define AUXINARG2 0

#define PO_AUX_OUT0 1
#define PO_AUX_OUT1 1
#define PO_AUX_OUT2 1


#define RX1_PTT 0x0E
#define RX2_PTT 0x0D
#define RX3_PTT 0x0B
#define RX4_PTT 0x07

#define R1Priority 4
#define R2Priority 6
#define R3Priority 6
#define R4Priority 5

