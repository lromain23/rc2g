#include "Firmware.h"

#INT_RDA
void rs232_int (void) { // {{{
// RS232 serial buffer interrupt handler.
  char c;
  if ( kbhit() && (sBufferIndex < sizeof(sBuffer))) {
    c = getc();
	sBuffer[sBufferIndex++] = c;
	if ( c == '\r' || c == '#' ) {
      sBufferFlag=1;
    }
  }
} // }}}

#ifdef DEBUG_SBUFFER
void debug_sbuffer(void) { // {{{
  sBufferFlag=1;
  sBuffer="set POLARITY 15\r";
} // }}}
#endif

void clear_sBuffer(void) { // {{{
// This function initializes the RS232 serial
// buffer, index and flag.
  int x,char_num;
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
} // }}}

void set_var (void) { // {{{
  // This function sets the specified register if a value is specified.
  // Otherwise it displays it.
  if ( value == -1 ) {
		printf ("%s %d",argument,value);
  } else {
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
  int reg_id,x;
  char str[REG_NAME_SIZE];
  char rname[REG_NAME_SIZE];
  tokenize_sBuffer();

  // Find matching reg_id
  for(x=0;x<RegMapNum;x++) {
    romstrcpy(&rname[0],&RegMap[x].reg_name[0]);
    //read_program_memory(rom_ptr,rname,5);
    if(stricmp(argument_name,rname)==0) {
	  argument=x;
	}
  }
  if (command==SET_REG) {
    set_var();
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
  } // End of while(1) main loop
} // }}}
