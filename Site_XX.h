// AuxIn1  --> Input from hydro out
// AuxOut2 --> Connected to 6M TX (Radio 4)
// AuxOut1 --> Connected to 2M TX (Radio 2)

#define SITE_ID_VAL  	53
#define SITE_GID_VAL 	90
#define POLARITY_DEF_VAL 0x0F 
#define ENABLE_DEFAULT 15

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
#define AUXOUTOP2 0
#define AUXOUTARG2 0

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



