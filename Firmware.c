#include "Firmware.h"

#INT_RDA
void rs232_int (void) { // {{{
// RS232 serial buffer interrupt handler.
  char c;
  if ( kbhit() && (sBufferIndex < sizeof(sBuffer))) {
    c = getc();
	sBuffer[sBufferIndex++] = c;
	if ( c == '\r' ) {
      sBufferFlag=1;
    }
  }
} // }}}

#ifdef DEBUG_SBUFFER
void debug_sbuffer(void) { // {{{
  sBufferFlag=1;
  sBuffer="set R0_GAIN0 9\r";
} // }}}
#endif

void help (void) { // {{{
  int x;
  int * regPtr;
  char rname[REG_NAME_SIZE];
  for(x=0;x<RegMapNum;x++) {
//    reg_index = RegMap[x].reg_name_index;
	strcpy(rname,reg_name[x]);
    regPtr=RegMap[x].reg_ptr;
    printf("%10s %d",rname,*regPtr);
  }
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
} // }}}

void initialize (void) { // {{{
// This function performs all initializations upon
// power-up
  clear_sBuffer();
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
		printf ("%s %d",argument,value);
  } else {
// DEBUG HERE!!!
    pObj=(int *)RegMap[argument].reg_ptr;
    *pObj=value;
    printf ("Setting %s to %d",argument,value);
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
  char rname[REG_NAME_SIZE];
  unsigned int* regPtr;
  tokenize_sBuffer();

  // Find matching reg_id
  argument=-1;
  for(x=0;x<RegMapNum;x++) {
//    romstrcpy(&rname[0],&RegMap[x].reg_name[0]);
//    reg_index = RegMap[x].reg_name_index;
	strcpy(rname,reg_name[x]);
//    romstrcpy(&rname[0],&reg_name[x]);
    if(stricmp(argument_name,rname)==0) {
	  argument=x;
	}
  }
  if ( argument==-1) {
    printf ("Error : Invalid argument %s",argument_name);
  } else {
    if (command==SET_REG) {
      set_var();
    } else if (command==GET_REG) {
      regPtr=RegMap[argument].reg_ptr;
      printf("%s %d",argument_name,*regPtr);
	}
  }
  sBufferFlag=0;
} // }}}

void main (void) { // {{{
  initialize();
  unsigned int gain;
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
	gain = RX_GAIN[1][1];
	gain = RX_GAIN[1][2];
    // Process RS232 Serial Buffer Flag }}}
  } // End of while(1) main loop
} // }}}
