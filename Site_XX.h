#define SITE_ID_VAL  	65
#define LCD_PI 1
#define SITE_GID_VAL 	90
#define POLARITY_DEF_VAL 0x0F 
#define ENABLE_DEFAULT 15
#define TOT_MIN 5
#define LINK_TOT 0
#define DEFAULT_GAIN 32
//#define LCD_TYPE_PI 0

#define MORSEID0	MCHAR('v')
#define MORSEID1	MCHAR('')
#define MORSEID2	2
#define MORSEID3	MCHAR('r')
#define MORSEID4	MCHAR('t')
#define MORSEID5	MCHAR('e')

#define AUXOUTOP0 0
#define AUXOUTARG0 0
#define AUXOUTOP1 0
#define AUXOUTARG1 0
#define AUXOUTOP2 0
#define AUXOUTARG2 0

//#define AUXINOP0 AUXI_ENABLE
//#define AUXINARG0 AUXI_ENABLE2|AUXI_ENABLE3|AUXI_ENABLE4
// AuxIn1 used by AuxOut 1
#define AUXINOP0  0
#define AUXINARG0 0
#define AUXINOP1 AUXI_TAIL_WHEN_HI
#define AUXINARG1 MCHAR('b')
#define AUXINOP2  0
#define AUXINARG2 0

#define PO_AUX_OUT0 1
#define PO_AUX_OUT1 1
#define PO_AUX_OUT2 1

#define RX1_PTT 0x0F
#define RX2_PTT 0x0F
#define RX3_PTT 0x0F
#define RX4_PTT 0x0F
