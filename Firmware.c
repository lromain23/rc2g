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
      if ( c == '\r' ) {
        sBufferFlag=1;
      }
    } else {
      putc('\a'); // Send alert. Avoid buffer overflow
    }
  }
} // }}}

#INT_RB
void RB0_INT (void) {
  int COR,CorPol;
  int COR_Pri;
  
  CorPol=(Polarity & COR0_MASK)!=0;
  COR=((input_b()&0x0F) ^ CorPol);
  if ( COR && (CORPriority[0] > CORPriority) ) {
    COR_FLAG = 1;
  }
}

#ifdef DEBUG_SBUFFER
void debug_sbuffer(void) { // {{{
  sBufferFlag=1;
  strcpy(sBuffer,"set R0G0 9\r");
} // }}}
#endif

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
  printf("DTMF Status : %u\n\r",dtmf_in);
  for(x=0;x<RegMapNum;x++) {
//    reg_index = RegMap[x].reg_name_index;
	strcpy(rname,reg_name[x]);
    regPtr=RegMap[x].reg_ptr;
    printf("[%02u] %s %u\n\r",x,rname,*regPtr);
  }
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
  delay_cycles(1);
} // }}}

int dtmf_read(int1 rs) { // {{{
  int value;
  output_bit(DTMF_RS,rs);
  delay_cycles(2);
  output_bit(DTMF_REB,0);
  value=input_d()&0x0F;
  output_bit(DTMF_REB,1);
  delay_cycles(1);
  return(value);
} // }}}

void dtmf_send_digit(int digit) { // {{{
  int x;
  dtmf_write(digit,DATA_REG);
  dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  for(x=0;x<5;x++) {
    delay_ms(100);
  }
  dtmf_write(IRQ,CONTROL_REG); // Disable tones
	
} // }}}

void init_dtmf(void) { // {{{
    dtmf_write(0,CONTROL_REG);
    dtmf_write(0,CONTROL_REG);
    //dtmf_write(8,CONTROL_REG);
//	dtmf_write(TOUT|IRQ|RSELB,CONTROL_REG); // Enable TOUT and IRQ
    dtmf_write(IRQ|RSELB,CONTROL_REG); // Enable IRQ then write to register B
    dtmf_write(BURST_OFF,CONTROL_REG);
    dtmf_read(CONTROL_REG);
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
//  setup_oscillator(OSC_8MHZ);
  clear_sBuffer();
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_RB0|INT_RB1|INT_RB2|INT_RB3);
  enable_interrupts(GLOBAL);
  output_bit(DTMF_CS ,0);
  output_bit(DTMF_WEB,1);
  output_bit(DTMF_REB,1);
  output_bit(DTMF_RS ,0);
  clearscr();
  init_variables(USE_EEPROM_VARS);
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
} // }}}

void set_var (void) { // {{{
  // This function sets the specified register if a value is specified.
  // Otherwise it displays it.
  int *pObj;
  if ( value == -1 ) {
		printf ("%s %u\n\r",argument,value);
  } else {
// DEBUG HERE!!!
    pObj=RegMap[argument].reg_ptr;
    *pObj=value;
    printf ("\n\rSetting %s(%u) to %u\n\r",argument_name,argument,value);
    dtmf_send_digit(value);
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
        printf("\n\r%s %u\n\r",argument_name,*regPtr);
        break;
      case SAVE_SETTINGS:
        store_variables();
        break;
      case RESTORE_SETTINGS:
        if ( argument == USE_EEPROM_VARS ) {
          init_src=USE_EEPROM_VARS;
        } else {
          init_src=USE_DEFAULT_VARS;
        }
        init_variables(init_src);
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
  int1 toggle;
  toggle=0;
#endif
  while(1) { // {{{
    // Process RS232 Serial Buffer Flag {{{
    // The sBufferFlag is set when a "#" or a "\r" is received.
    if ( sBufferFlag ) {
      process_sBuffer();
      clear_sBuffer();
    }
	output_bit(PTT0,toggle);
	output_bit(PTT1,toggle);
	output_bit(PTT2,toggle);
	output_bit(PTT3,toggle);
	output_bit(RX0_EN,~toggle);
	output_bit(RX1_EN,~toggle);
	output_bit(RX2_EN,~toggle);
	output_bit(RX3_EN,~toggle);
    toggle=~toggle;
	delay_ms(250);
    // Process RS232 Serial Buffer Flag }}}
  } // End of while(1) main loop
} // }}}
