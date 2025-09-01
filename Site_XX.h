//
// Configuration
// Radio1 - Local
// Radio2 - Lien Principal
// Radio3 - Lien Auxiliaire
// Radio4 - 

// AuxIn1 - Hydro (Hydro=1, Panne=0)
// AuxIn2 -
// AuxIn3 - 
#define SITE_ID_VAL  	53
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

#define AUXOUTOP0 0
#define AUXOUTARG0 0

// Fan Lien
#define AUXOUTOP1 AUX_OUT_FOLLOW_PTT
#define AUXOUTARG1 AUX_OUT_FOLLOW_PTT2 | AUX_OUT_FOLLOW_PTT_DELAY | AUX_OUT_FOLLOW_PTT_INVERT_OUTPUT

// Fan Repeteur
#define AUXOUTOP2 AUX_OUT_FOLLOW_PTT
#define AUXOUTARG2 AUX_OUT_FOLLOW_PTT1 | AUX_OUT_FOLLOW_PTT_DELAY | AUX_OUT_FOLLOW_PTT_INVERT_OUTPUT

//#define AUXINOP0 AUXI_ENABLE
//#define AUXINARG0 AUXI_ENABLE2|AUXI_ENABLE3|AUXI_ENABLE4
// Tail panne d'hydro
#define AUXINOP0  AUXI_TAIL_WHEN_LO
#define AUXINARG0 MCHAR('b')

// Non utilise
#define AUXINOP1  0
#define AUXINARG1 0

// Non-utilise.
#define AUXINOP2  0
#define AUXINARG2 0

#define PO_AUX_OUT0 1
#define PO_AUX_OUT1 0
#define PO_AUX_OUT2 0

//
// Radio1 : Repeteur
// Radio2 : Lien prioritaire
// Radio3 : Lien backup
// First Enter Admin Mode:
// *53 09 01 # (Use 00 argument to exit admin mode)
#define RX1_PTT 0x02
// Activation Lien vers local: *5302291#
#define RX2_PTT 0x00
// Activation LienAux vers local: *5302301#
#define RX3_PTT 0x00
#define RX4_PTT 0x00

#define R1Priority 8
#define R2Priority 6
#define R3Priority 4
#define R4Priority 2
