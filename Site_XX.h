#define SITE_ID_VAL  	62
#define SITE_GID_VAL 	90
#define POLARITY_DEF_VAL 0x0F 
#define ENABLE_DEFAULT 15
#define TOT_MIN 5
#define LINK_TOT 0
#define DEFAULT_GAIN 32
//#define LCD_TYPE_PI 1

#define MORSEID0	MCHAR('v')
#define MORSEID1	MCHAR('e')
#define MORSEID2	2
#define MORSEID3	MCHAR('r')
#define MORSEID4	MCHAR('e')
#define MORSEID5	MCHAR('h')

// Controle de Fan
#define AUXOUTOP0 AUX_OUT_FOLLOW_COR
#define AUXOUTARG0 AUX_OUT_FOLLOW_COR_INVERT_OUTPUT | AUX_OUT_FOLLOW_COR_OFF_DELAY | AUX_OUT_FOLLOW_COR1 | AUX_OUT_FOLLOW_COR2 | AUX_OUT_FOLLOW_COR3 | AUX_OUT_FOLLOW_COR4

// Sortie relais 12V pour power-cycle
#define AUXOUTOP1 0
#define AUXOUTARG1 0

// Non-utilise
#define AUXOUTOP2 0
#define AUXOUTARG2 0

// Entree COR auxiliaire
#define AUXINOP0  0
#define AUXINARG0 0

// Tail panne d'hydro
#define AUXINOP1 AUXI_TAIL_WHEN_LO
#define AUXINARG1 MCHAR('b')

// Non-utilise.
#define AUXINOP2  0
#define AUXINARG2 0

#define PO_AUX_OUT0 0
#define PO_AUX_OUT1 1
#define PO_AUX_OUT2 1

#define RX1_PTT 0x0F
#define RX2_PTT 0x0F
#define RX3_PTT 0x0F
#define RX4_PTT 0x0F
