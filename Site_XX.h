#define SITE_ID_VAL  	53
#define SITE_GID_VAL 	90
#define POLARITY_DEF_VAL 0x0F 
#define ENABLE_DEFAULT 14
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

#define AUXOUTOP0 0
#define AUXOUTARG0 0

// Fan Repeteur
#define AUXOUTOP1 AUX_OUT_FOLLOW_PTT
#define AUXOUTARG1 AUX_OUT_FOLLOW_PTT1 | AUX_OUT_FOLLOW_PTT_DELAY

// Fan Lien
#define AUXOUTOP2 AUX_OUT_FOLLOW_PTT
#define AUXOUTARG2 AUX_OUT_FOLLOW_PTT2 | AUX_OUT_FOLLOW_PTT_DELAY

//#define AUXINOP0 AUXI_ENABLE
//#define AUXINARG0 AUXI_ENABLE2|AUXI_ENABLE3|AUXI_ENABLE4
// AuxIn1 used by AuxOut 1
#define AUXINOP0  0
#define AUXINARG0 0

// Tail panne d'hydro
#define AUXINOP1 AUXI_TAIL_WHEN_LO
#define AUXINARG1 MCHAR('b')

// Non-utilise.
#define AUXINOP2  0
#define AUXINARG2 0

#define PO_AUX_OUT0 0
#define PO_AUX_OUT1 0
#define PO_AUX_OUT2 0

//
// Radio1 : Repeteur
// Radio2 : Lien prioritaire
// Radio3 : Lien backup
#define RX1_PTT 0x03
#define RX2_PTT 0x01
#define RX3_PTT 0x01
#define RX4_PTT 0x0F

#define R1Priority 8
#define R2Priority 6
#define R3Priority 4
#define R4Priority 2
