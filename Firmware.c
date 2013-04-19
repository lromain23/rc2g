#include "Firmware.h"

void clearscr(void) {
// Erase the screen 
  putc(ESC);
  printf("[2J");
// Move cursor back to top
  putc(ESC);
  printf("[0;0H");
}
#INT_RDA
void rs232_int (void) { // {{{
// RS232 serial buffer interrupt handler.
  char c;
  if ( kbhit() ) {
    c = getc();
    if ( c == '\b' ) {
      if ( sBufferIndex > 0 ) {
        sBufferIndex--;
        sBuffer[sBufferIndex]='\0';
        putc('\b');
	// Erase End of Line <ESC>[K
	putc(ESC);
	printf("[K");
      } else {
        putc('\a'); // Send alert. Cannot backspace further
      }
    } else if (sBufferIndex < sizeof(sBuffer)) {
      putc(c); // echo the character
      sBuffer[sBufferIndex++] = c;
      if ( c == '\r' || c=='+' || c=='-') {
        sBufferFlag=1;
      }
    } else {
      putc('\a'); // Send alert. Avoid buffer overflow
    }
  }
} // }}}

#INT_RB
void RB0_INT (void) { // {{{
  int COR_Pri;
  int value;

  COR_IN=(input_b()^ Polarity)&0x0F;
  if ( COR_IN ) {
    COR_FLAG = 1;
  }
  if ( input_b() & DTMF_INT_MASK ) {
    DTMF_FLAG = 1;
    value=input_d()&0x0F;
    *DTMF_ptr->Strobe=1;
    *DTMF_ptr->Key=value;
    DTMF_ptr++;
  }
} // }}}

void update_ptt(int cor) { // {{{
}// }}}

void process_cor (void) { // {{{
  int cor_priority_tmp;
  int cor_ptr;

  cor_priority_tmp = CORPriority;
  cor_ptr = 0;

  // Check for RX0 {{{
  if ( COR_IN & COR0_MASK ) {
    if ( RXPriority[0] > cor_priority_tmp ) {
      cor_priority_tmp = RXPriority[0];
      cor_ptr=0;
    }
  }
  // }}}
  // Check for RX1 {{{
  if ( COR_IN & COR1_MASK ) {
    if ( RXPriority[1] > cor_priority_tmp ) {
      cor_priority_tmp = RXPriority[1];
      cor_ptr=1;
    }
  }
  // }}}
  // Check for RX2 {{{
  if ( COR_IN & COR2_MASK ) {
    if ( RXPriority[2] > cor_priority_tmp ) {
      cor_priority_tmp = RXPriority[2];
      cor_ptr=2;
    }
  }
  // }}}
  // Check for RX3 {{{
  if ( COR_IN & COR3_MASK ) {
    if ( RXPriority[3] > cor_priority_tmp ) {
      cor_priority_tmp = RXPriority[3];
      cor_ptr=3;
    }
  }
  // }}}

//  update_ptt(cor);

  COR_FLAG = 0;
} // }}}

#ifdef DEBUG_SBUFFER
void debug_sbuffer(void) { // {{{
//    const char tmp[]="set R0G0 9\r";
const char tmp[]="restore pol 1\r";
  sBufferFlag=1;
  strcpy(sBuffer,tmp);
} // }}}
#endif

void clear_dtmf_array(void) { // {{{
  int x;

  for(x=0;x<sizeof(DTMF_ARRAY);x++) {
    DTMF_ARRAY[x]=(sDTMF)0;
  }
  DTMF_ptr=&DTMF_ARRAY[0];
} // }}}

void header (void) { // {{{
  putc(ESC);
  printf("[47;34m\n\rRadio Repeater Controller - ");
  putc(ESC);
  printf("[47;31mVE2LRS");
  putc(ESC);
  printf("[47;34m (C) 2013\n\n\r");
  putc(ESC);
  printf("[40;37m");
} // }}}

void help (void) { // {{{
  unsigned int x;
  unsigned int * regPtr;
  int dtmf_in;
  char rname[REG_NAME_SIZE];
  clearscr();
  header();
  dtmf_in=dtmf_read(CONTROL_REG);
  for(x=0;x<RegMapNum;x++) {
//    reg_index = RegMap[x].reg_name_index;
	strcpy(rname,reg_name[x]);
    regPtr=RegMap[x].reg_ptr;
    printf("[%02u] %s %u\n\r",x,rname,*regPtr);
  }
  printf("\n\rDTMF Status : %u\n\r",dtmf_in);
  printf("\n\rCOMMAND> ");
} // }}}

void clear_sBuffer(void) { // {{{
// This function initializes the RS232 serial
// buffer, index and flag.
  unsigned x,char_num;
  char_num = sizeof(sBuffer)/sizeof(char);
  for (x=0;x<char_num;x++) {
    sBuffer[x]='\0';
  }
  sBufferIndex=0;
  sBufferFlag=0;  
  argument=-1;
  argument_name[0]='\0';
  command=0;
} // }}}

void dtmf_write(int data,int1 rs) { // {{{
  int1 dbit;
// Write Data Bits {{{
  set_tris_d(0x00);
  dbit=((data&0x0F)&0x01)!=0;
  output_bit(DTMF_D0,dbit);
  dbit=((data&0x0F)&0x02)!=0;
  output_bit(DTMF_D1,dbit);
  dbit=((data&0x0F)&0x04)!=0;
  output_bit(DTMF_D2,dbit);
  dbit=((data&0x0F)&0x08)!=0;
  output_bit(DTMF_D3,dbit);
// }}}
  output_bit(DTMF_RS,rs);
  delay_cycles(2);
  output_bit(DTMF_WEB,0);
  delay_cycles(2);
  output_bit(DTMF_WEB,1);  
  //output_bit(DTMF_RS,DATA_REG);
  delay_cycles(2);
  set_tris_d(0x0F);
} // }}}

int dtmf_read(int1 rs) { // {{{
  int value;
  set_tris_d(0x0F);
  output_bit(DTMF_RS,rs);
  delay_cycles(2);
  output_bit(DTMF_REB,0);
  delay_cycles(1);
  value=input_d()&0x0F;
  output_bit(DTMF_REB,1);
  //output_bit(DTMF_RS,DATA_REG);  
  delay_cycles(2);
  return(value);
} // }}}

void init_dtmf(void) { // {{{
    output_bit(DTMF_REB,1);
  	output_bit(DTMF_WEB,1);
  	output_bit(DTMF_RS ,DATA_REG);
    dtmf_write(0,CONTROL_REG);
    dtmf_write(0,CONTROL_REG);
    //dtmf_write(8,CONTROL_REG);
//	dtmf_write(TOUT|IRQ|RSELB,CONTROL_REG); // Enable TOUT and IRQ
    dtmf_write(IRQ|RSELB,CONTROL_REG); // Enable IRQ then write to register B
    dtmf_write(BURST_OFF,CONTROL_REG);
    dtmf_read(CONTROL_REG);
} // }}}

void dtmf_send_digit(int digit) { // {{{
  int x;
  //init_dtmf();
  dtmf_write(digit,DATA_REG);
  //dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  output_bit(PTT3,1);
  for(x=0;x<5;x++) {
    delay_ms(100);
  }
  dtmf_write(IRQ,CONTROL_REG); // Disable tones
  output_bit(PTT3,0);
	
} // }}}

void update_checksum (int *cksum,int value) { // {{{
  const int seed = 0x09;
  int tmp;

  tmp=*cksum;
  if ( tmp > 127 ) {
    tmp++;
  }
  tmp = ((tmp << 1)^seed)+value;
  *cksum=tmp;
} // }}}

int _init_variables (int1 source) { // {{{
  int x;
  int *regPtr;
  int cksum;
  int eeprom_index;
  int default_value;
  int retVal;

  cksum=1;
  eeprom_index=0;
  retVal = 1;
  if ( source == USE_EEPROM_VARS ) {
    printf("Initializing RAM variables from EEPROM\n\r");
  } else {
    printf("Initializing RAM variables with Default values\n\r");
  }
  for(x=0;x<RegMapNum;x++) {
    regPtr=RegMap[x].reg_ptr;
    if ( source == USE_EEPROM_VARS && RegMap[x].non_volatile ) {
	  *regPtr=read_eeprom(eeprom_index);
	  update_checksum(&cksum,*regPtr);    
      eeprom_index++;
    } else {
      default_value=(int8)RegMap[x].default_value;
      *regPtr=default_value;
    }
  }
  if ( source == USE_EEPROM_VARS ) {
    if ( read_eeprom(eeprom_index) != cksum ) {
       retVal = 0 ; // Error : Checksum does not match whenn initializing variables from EEPROM
    }
  }
  return (retVal);
} // }}}

void store_variables(void) { // {{{
// Save RAM variables in EEPROM
  int x;
  int eeprom_index;
  int *regPtr;
  int cksum;
  int8 value;

  cksum=1;

  eeprom_index=0;
  for(x=0;x<RegMapNum;x++) {
    regPtr=RegMap[x].reg_ptr;
    if ( RegMap[x].non_volatile ) {
     value=*regPtr;
     write_eeprom(eeprom_index,value);
     update_checksum(&cksum,value);
     eeprom_index++;
    }
  }
  write_eeprom(eeprom_index,cksum);
  printf("Saving RAM configuration in EEPROM.\n\r");
} // }}}

void init_variables (int1 source) { // {{{
    // Attempt initialization from EEPROM and verify checksum.
    // If checksum does not match, use default variables.
    if ( !_init_variables(source) ) {
      printf("Checksum missmatch. Restoring default values.\n\r");
        _init_variables(USE_DEFAULT_VARS);
		store_variables();
    }
} // }}}

void initialize (void) { // {{{
// This function performs all initializations upon
// power-up
  clear_sBuffer();
  LastRegisterIndexValid=0;
  LastRegisterIndex=0;
  set_tris_b(0xFF);
  set_tris_d(0x00);
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_RB0|INT_RB1|INT_RB2|INT_RB3);
  enable_interrupts(GLOBAL);
  output_bit(DTMF_CS ,0);
  output_bit(DTMF_WEB,1);
  output_bit(DTMF_REB,1);
  output_bit(DTMF_RS ,0);
  clearscr();
  init_variables(USE_EEPROM_VARS);
  init_dtmf();
  clear_dtmf_array();
  header();
} // }}}

void tokenize_sBuffer() { // {{{
  char match_tok[8],match_val[4];
  char verb[8];
  char smatch_reg[6];
  char *sptr;

  // Get verb {{{
  strcpy(match_tok," ;\r");
  sptr=strtok(sBuffer,match_tok);
  if (sptr!=0) {;
    strcpy(verb,sptr);
  }  
  // }}}  
  // Get argument {{{
  sptr=strtok(0,match_tok);
  if (sptr!=0) {;
    strcpy(argument_name,sptr);
  }  
  // }}}
  // Get value {{{
  sptr=strtok(0,match_tok);
  if (sptr!=0) {;
    strcpy(match_val,sptr);
		value=(int8)strtoul(match_val,NULL,10);
  } else {
    value=-1;
  }  
  // }}}
	// Check for "SET" command {{{
  strcpy(smatch_reg,"set");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    if ( value == -1 ) {
      command=GET_REG;
    } else {
      command=SET_REG;
    }
  } // }}}
	// Check for "SAVE" command {{{
  strcpy(smatch_reg,"SAVE");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
      command=SAVE_SETTINGS;
  } // }}}
	// Check for "RESTORE" command {{{
  strcpy(smatch_reg,"RESTORE");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
      command=RESTORE_SETTINGS;
  } // }}}
  // Check for "HELP" command {{{
  strcpy(smatch_reg,"help");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=HELP;
  } // }}}
  // Check for "+ (INCR)" command {{{
  strcpy(smatch_reg,"+");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=INCREMENT_REG;
  } // }}}
  // Check for "- (DECR)" command {{{
  strcpy(smatch_reg,"-");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=DECREMENT_REG;
  } // }}}
} // }}}

void set_var (void) { // {{{
  // This function sets the specified register if a value is specified.
  // Otherwise it displays it.
  int *pObj;
  if ( value == -1 ) {
		printf ("%s %u\n\r",argument,value);
  } else {
    pObj=RegMap[argument].reg_ptr;
    *pObj=value;
    LastRegisterIndex = argument;
    LastRegisterIndexValid=1;
    printf ("\n\rSetting %s(%u) to %u\n\r",argument_name,argument,value);
    dtmf_send_digit(value);
  }
} // }}}

void increment(void) { // {{{
  int *pObj;
  int value;
  char argname[REG_NAME_SIZE];
  if ( LastRegisterIndexValid > 0 ) {
    pObj=RegMap[LastRegisterIndex].reg_ptr;
    value=(*pObj+1);
    *pObj=value;
    strcpy(argname,reg_name[LastRegisterIndex]);
    printf ("\n\rIncrementing %s(%u) = %u\n\r",argname,LastRegisterIndex,*pObj);
  }
} // }}}
void decrement(void) { // {{{
  int *pObj;
  int value;
  char argname[REG_NAME_SIZE];
  if ( LastRegisterIndexValid > 0 ) {
    pObj=RegMap[LastRegisterIndex].reg_ptr;
    value=*pObj-1;
    *pObj=value;
    strcpy(argname,reg_name[LastRegisterIndex]);
    printf ("\n\rDecrementing %s(%u) = %u\n\r",argname,LastRegisterIndex,*pObj);
  }
} // }}}

void romstrcpy(char *dest,rom char *src) { // {{{
  int c=0;
  while(c<REG_NAME_SIZE) {
    dest[c]=src[c];
	c++;
  }
} // }}}

void process_sBuffer(void) { // {{{
  unsigned int x;
  int1 init_src;
  char rname[REG_NAME_SIZE];
  unsigned int* regPtr;
  tokenize_sBuffer();

  // Find matching reg_id
  argument=-1;
  for(x=0;x<RegMapNum;x++) {
	  strcpy(rname,reg_name[x]);
    if(stricmp(argument_name,rname)==0) {
	    argument=x;
  	}
  }
  if ( command==SET_REG && argument==-1) {
    printf ("\n\rError : Invalid argument %s\n\r",argument_name);
  } else {
    switch(command) {
      case SET_REG:
        set_var();
        break;
      case GET_REG:
        regPtr=RegMap[argument].reg_ptr;
        LastRegisterIndex = argument;
        LastRegisterIndexValid=1;
        printf("\n\r%s %u\n\r",argument_name,*regPtr);
        break;
      case SAVE_SETTINGS:
        store_variables();
        break;
      case RESTORE_SETTINGS:
        if ( value == USE_EEPROM_VARS ) {
          init_src=USE_EEPROM_VARS;
        } else {
          init_src=USE_DEFAULT_VARS;
        }
        init_variables(init_src);
        break;
      case INCREMENT_REG:
        increment();
        break;
      case DECREMENT_REG:
        decrement();
        break;
      case HELP:
        help();
        break;
    }
  }
  sBufferFlag=0;
} // }}}

void main (void) { // {{{
  initialize();

#ifdef DEBUG_SBUFFER
    debug_sbuffer();
#endif
  while(1) { // {{{
    // Process RS232 Serial Buffer Flag {{{
    // The sBufferFlag is set when a "#" or a "\r" is received.
    if ( sBufferFlag ) {
      process_sBuffer();
      clear_sBuffer();
    }
    // Process RS232 Serial Buffer Flag }}}
	if ( COR_FLAG ) {
      process_cor();
	}
  } // End of while(1) main loop
} // }}}
