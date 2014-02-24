#include "Firmware.h"

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
  int value,dtmf_status;
  int LAST_COR_IN;

  //if(interrupt_active(INT_RB0|INT_RB1|INT_RB2|INT_RB3)) {
  if(IOCBF&0x0F) { // Check for interrupts on RB[3:0] only
    LAST_COR_IN=COR_IN;
    COR_IN=(input_b() ^ Polarity)&0x0F;
    if ( LAST_COR_IN != COR_IN ) {
      COR_FLAG = 1;
    }
    clear_interrupt(INT_RB0|INT_RB1|INT_RB2|INT_RB3);
  }
  if ( interrupt_active(INT_RB4_H2L) ) {
    // Read DTMF data first. Then mark it as valid if the DTMF_BUFFER_FULL flag is set.
    dtmf_status = dtmf_read(CONTROL_REG);
    if ( dtmf_status & DTMF_BUFFER_FULL) {
      value=dtmf_read(DATA_REG);
      DTMF_IN_FLAG=1;
      if ( value == dd ) {
        value=d0;
      } else if ( value == d0 ) {
        value=dd;
      }
      // Check for '#'
      if ( value == dp ) {
        DTMF_FLAG = 1;
        DTMF_ptr->Last=1;
      } else {
	      if ( DTMF_ptr <= &DTMF_ARRAY[DTMF_ARRAY_SIZE-1] ) {
          DTMF_ptr->Key=value;
  	      DTMF_ptr->Strobe=1;
          DTMF_ptr++;
        }
      }
      clear_interrupt(INT_RB4_H2L);
    }
  }
} // }}}

#INT_TIMER0
void int_rtcc(void) { // {{{
  if ( rtcc_cnt ) {
    rtcc_cnt--;
  } else {
    COR_FLAG=1;
    SECOND_FLAG=1;
    rtcc_cnt=30;
  }
  if (aux_timer ) {
    aux_timer--;
  }
} // }}}

void lcd_send(int line,char * s) { // {{{
  int lcd_cmd;

  lcd_cmd = LCD_I2C_ADD | ((line<<1) & 0x06);
  i2c_start();
  i2c_write(lcd_cmd);
  while(*s) {
    i2c_write(*s++);
  }
  i2c_write(0); // EOL
  i2c_stop();
} // }}}

void execute_command(void) { // {{{
  unsigned int* regPtr;
  int1 init_src;
  switch(command) {
    case SET_REG:
      set_var();
      break;
    case GET_REG:
      regPtr=RegMap[argument].reg_ptr;
      LastRegisterIndex = argument;
      LastRegisterIndexValid=1;
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
      increment(1);
      PROMPT_FLAG=1;
      break;
    case DECREMENT_REG:
      increment(-1);
      PROMPT_FLAG=1;
      break;
    case STATUS:
      status();
      break;
    case REBOOT:
      reset_cpu();
      break;
    case DTMF_SEND:
	    if ( value == d0 ) {
    		value=dd;
    	} else if (value == dd) {
    		value = d0;
  	  }
      dtmf_send_digit(value&0x0F);
      break;
    case I2C_SEND:
      printf("\n\rSending %d on I2C(%d)",value,LCD_I2C_ADD);
      i2c_start();
        i2c_write(LCD_I2C_ADD);
        i2c_write(value); // Send Value
        i2c_write(0); // End of transmission
      i2c_stop();
      break;
    case MORSE_SEND:
      morse(value);
      break;
  }
} // }}}
void process_sBuffer(void) { // {{{
  unsigned int x;
  char rname[REG_NAME_SIZE];

  tokenize_sBuffer();

  argument=-1;
  // Find matching reg_id
  for(x=0;x<RegMapNum;x++) {
	  strcpy(rname,reg_name[x]);
    if(stricmp(argument_name,rname)==0) {
	    argument=x;
  	}
  }
  // Match "EEPROM" or "RAM" for restore/save functions
  if ( argument == -1 ) {
    // save/restore <eeprom>
		value=(int8)strtoul(argument_name,NULL,10);
    strcpy(rname,"eeprom");
    if(stricmp(argument_name,rname)==0) {
      value=USE_EEPROM_VARS;
    }
    // save/restore <default>
    strcpy(rname,"default");
    if(stricmp(argument_name,rname)==0) {
      value=USE_DEFAULT_VARS;
    }
  }
  execute_command();
} // }}}
void clearscr(void) { // {{{
// Erase the screen 
  putc(ESC);
  printf("[2J");
// Move cursor back to top
  putc(ESC);
  printf("[0;0H");
} // }}}

void set_trimpot(pot,value) { // {{{
  int tx_value;
  tx_value=pot << 6;
  tx_value=tx_value + (value & 0x3F);
  i2c_start();
  i2c_write(TRIMPOT_WRITE_CMD);
  i2c_write(tx_value);
  i2c_stop();  
  printf("\n\rSetting Pot(%u) to %u",pot,value);

} // }}}

void morse (int c) { // {{{
	int mc;
	int x;

	mc = cMorseChar[c]; 
	
  printf("\n\rSending morse : %u",c);
  PROMPT_FLAG=1;
	for(x=0;x<4;x++) {
		switch(mc & 0xc0) { // Check two MSB's
			case(0x40):
				dit();
				break;
			case(0x80):
				dah();
				break;
			default:
				break;
		}
		mc = mc << 2; // Shift two MSB's out and continue with next ones
    aux_timer=MorseLen[(MorseDitLength&0x03)];
    while(aux_timer) {
      delay_cycles(1);
    }
	}
	if ( c < 10 ) { // Digits --> add the 5th dit or dah.
		if ( c < 5 ) {
			dah();
		}
		else {
			dit();
		}
	}
  aux_timer=3*MorseLen[(MorseDitLength&0x03)];
  while(aux_timer) {
    delay_cycles(1);
  }
} // }}}

void update_ptt(int cor) { // {{{
  int x,pot;
  int pot_val;
  int mask;
  int ptt;
  unsigned pval[4]={0,0,0,0};
  int1 rx_bit,ptt_bit;

  CurrentCorIndex=cor;

  if ( cor ) {
    ptt=RX_PTT[cor-1] & (Enable&Enable_Mask);
  } else {
    ptt=0;
  }


  mask=1;
  for(x=0;x<4;x++) {
    if ( !cor ) {
      rx_bit=0;
      ptt_bit=0;
    } else {
      if ( cor == (x+1) ) {
        rx_bit=1;
        CurrentCorMask=mask;
      } else {
        rx_bit=0;
      }
      ptt_bit=(ptt&mask)!=0;
	}
    output_bit(RX_PIN[x],rx_bit);
    output_bit(PTT_PIN[x],ptt_bit);
    mask=mask<<1;
  } 
  if(!cor) {
    CurrentCorPriority=0;
  } else {
    CurrentCorPriority=RXPriority[cor-1];
	// Update TrimPots
    for(pot=0;pot<4;pot++){
      pot_val=RX_GAIN[cor-1][pot];
      pval[pot]=pot_val;
	    set_trimpot(pot,pot_val);
  	}
    PROMPT_FLAG=1;
  }
  sprintf(LCD_str,"COR: %x PTT:0x%x",cor,ptt);
  lcd_send(1,LCD_str); // COR/PTT on line 1
  delay_ms(50);
  sprintf(LCD_str,"POT:%d %d %d %d",pval[0],pval[1],pval[2],pval[3]);
  lcd_send(0,LCD_str); // COR/PTT on line 0
}// }}}

int ValidKey(int index) { // {{{
  int strobe;
  if(index>=0 && (index <= DTMF_ARRAY_SIZE)) {
    if(DTMF_ARRAY[index].Strobe && (DTMF_ARRAY[index].Key != dp)) {
		strobe=1;
	}else {
		strobe = 0;
	} 
  } else {
    strobe=0;
  }
  return(strobe);
} // }}}

int ValidKeyRange(unsigned int a,unsigned int b) { // {{{
  int key;
  int x;
  int1 valid;

  if(b>=a && (a < DTMF_ARRAY_SIZE)) {
    valid=1;
    for(x=a;x<=b;x++) {
      key=(int)DTMF_ARRAY[x].Key;
      if(! DTMF_ARRAY[x].Strobe ) {
        valid=0;
      }
     if(key==dp) {
        valid=0;
      }
    }
  } else {
    valid=0;
  }
  return(valid);
} // }}}

void process_dtmf(void) { // {{{
  int site_id;
  int digit;
  // Structure:
  // [SID1][SID0][CMD1][CMD0][ARG1][ARG0][Valx][Valy][Valz]
  //   0     1     2     3     4     5     6     7     8
  value = 0;
  command=0;
  if ( ValidKeyRange(0,5)) {
    site_id = DTMF_ARRAY[0].Key *10 + DTMF_ARRAY[1].Key;
    command = DTMF_ARRAY[2].Key * 10 + DTMF_ARRAY[3].Key;
    argument = DTMF_ARRAY[4].Key * 10 + DTMF_ARRAY[5].Key;
    digit=6;
    while(ValidKey(digit)) {
     value = value * 10 + DTMF_ARRAY[digit].Key;
     digit++;
    }
    // Commands that don't need arguments but need a value:
    switch(command) {
      case(DTMF_SEND):
      case(SAVE_SETTINGS):
      case(RESTORE_SETTINGS): 
         value = argument;
         break;
    }
    printf("\n\rProcessing DTMF sequence:");
    printf("\n\r  SiteID  : %u",site_id);
    printf("\n\r  Command : %u",command);
    printf("\n\r  Argument: %u",argument);
    printf("\n\r  Value   : %u",value);

    if ( site_id == SiteID ) {
      execute_command();
    }
  }
  CLEAR_DTMF_FLAG=1;
} // }}}

void process_cor (void) { // {{{
  int cor_priority_tmp;
  int cor_mask,cor_index;
  int rx_priority;
  int cor_in;
  int do_update_ptt;
  int x;

  cor_priority_tmp = 0;
  cor_mask=1;
  do_update_ptt=0;
  cor_in = COR_IN | (COR_EMUL&0x0F);
  if ( CurrentCorPriority && !(cor_in&CurrentCorMask) ) {
    CurrentCorPriority=0;
    CurrentCorMask=0;
    do_update_ptt=1;
  }
  cor_index=0;
  for(x=0;x<4;x++) {
    if ( cor_in & cor_mask ) {
      if ( (Enable&Enable_Mask) & cor_mask ) {
        rx_priority=RXPriority[x];
      } else {
        // Radio is not enabled. Only listen for DTMF (if no other radio is enabled).
        rx_priority = 1; // Least priority while still active.
      }
      if ( rx_priority > CurrentCorPriority ) {
        cor_priority_tmp = rx_priority;
        cor_index=x+1;
        do_update_ptt=1;
      }
    }
    cor_mask = cor_mask << 1;
  }
  if ( do_update_ptt ) {
    printf("\n\r# COR Ports = %u; SW Emulated COR : %u",COR_IN,COR_EMUL);
    update_ptt(cor_index);
    PROMPT_FLAG=1;
  }
  // Clear the DTMF array when all CORs fall
  if ( !cor_in ) {
    CLEAR_DTMF_FLAG=1;
  }
} // }}}

#ifdef DEBUG_SBUFFER
void debug_sbuffer(void) { // {{{
//    const char tmp[]="set R0G0 9\r";
const char tmp[]="set pol 0\r";
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

void status (void) { // {{{
  int x;
  int nak;
  int8 pot_val;
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
  printf("\n\n\rCOR : %u (Emulated : %u)",COR_IN,COR_EMUL);
  printf("\n\rDTMF Status : %u\n\r",dtmf_in);
  printf("\n\rTRIMPOTS : ");

  i2c_start();
  i2c_write(TRIMPOT_READ_CMD);
  for(x=0;x<4;x++) {
      if(x==3) {
	    nak=0;
	  } else {
	    nak=1;
	  }
      pot_val=i2c_read(nak);
	  pot_val=pot_val&0x3F;
      printf(" Pot(%d)=%d",x,pot_val);
  }
  i2c_stop();
  PROMPT_FLAG=1;
} // }}}

void prompt(void) { // {{{
  printf("\n\n\rCOMMAND> ");
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

int dtmf_read(int rs) { // {{{
  int value;
  set_tris_d(0x0F);
  output_bit(DTMF_RS,rs);
  delay_cycles(1);
  output_bit(DTMF_REB,0);
  delay_cycles(1);
  value=input_d();
  value&=0x0F;
  output_bit(DTMF_REB,1);
  //output_bit(DTMF_RS,DATA_REG);  
  delay_cycles(1);
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
  dtmf_write(digit,DATA_REG);
  dtmf_write(IRQ|RSELB,CONTROL_REG); // Switch to DTMF mode
  dtmf_write(BURST_OFF|DUAL_TONE,CONTROL_REG); // Enable DTMF
  dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  aux_timer=AUX_TIMER_500ms;
  while(aux_timer) {
    delay_cycles(1);
  }
  dtmf_write(IRQ,CONTROL_REG); // Disable tones
} // }}}

void dit (void) { // {{{
  dtmf_write(1,DATA_REG);
  dtmf_write(IRQ|RSELB,CONTROL_REG); // Switch to DTMF mode
  dtmf_write(BURST_OFF|SINGLE_TONE,CONTROL_REG); // Enable DTMF
  dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  aux_timer=MorseLen[(MorseDitLength&0x03)];
  while(aux_timer) {
    delay_cycles(1);
  }
  dtmf_write(IRQ,CONTROL_REG); // Disable tones
} // }}}
void dah (void) { // {{{
  dtmf_write(1,DATA_REG);
  dtmf_write(IRQ|RSELB,CONTROL_REG); // Switch to DTMF mode
  dtmf_write(BURST_OFF|SINGLE_TONE,CONTROL_REG); // Enable DTMF
  dtmf_write(TOUT|IRQ,CONTROL_REG); // Enable Tones
  aux_timer=3*MorseLen[(MorseDitLength&0x03)];
  while(aux_timer) {
    delay_cycles(1);
  }
  dtmf_write(IRQ,CONTROL_REG); // Disable tones
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
  int eeprom_val;

  cksum=1;
  eeprom_index=0;
  retVal = 1;
  if ( source == USE_EEPROM_VARS ) {
    printf("\n\rInitializing RAM variables from EEPROM");
  } else {
    printf("\n\rInitializing RAM variables with firmware default values");
  }
  for(x=0;x<RegMapNum;x++) {
    regPtr=RegMap[x].reg_ptr;
    if ( source == USE_EEPROM_VARS && RegMap[x].non_volatile ) {
	  eeprom_val=read_eeprom(eeprom_index);
	  *regPtr=eeprom_val;
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
     if ( read_eeprom(eeprom_index) != value ) {
       write_eeprom(eeprom_index,value);
     }
     update_checksum(&cksum,value);
     eeprom_index++;
    }
  }
  write_eeprom(eeprom_index,cksum);
  printf("\n\rSaving RAM configuration in EEPROM.");
} // }}}

void init_variables (int1 source) { // {{{
    // Attempt initialization from EEPROM and verify checksum.
    // If checksum does not match, use default variables.
    if ( !_init_variables(source) ) {
      printf("\n\r  Checksum mismatch. Restoring default values.");
        _init_variables(USE_DEFAULT_VARS);
		store_variables();
    }
} // }}}

void init_trimpot(void) {//{{{
  set_trimpot(0,0);
  set_trimpot(1,0);
  set_trimpot(2,0);
  set_trimpot(3,0);
} // }}}

void initialize (void) { // {{{
// This function performs all initializations upon
// power-up
  clear_sBuffer();
  setup_comparator(NC_NC_NC_NC); 
  setup_wdt(WDT_2S);
  WPUB=0x00;
  COR_IN=0;
  LastRegisterIndexValid=0;
  LastRegisterIndex=0;
  CurrentCorMask=0;
  CurrentCorPriority=0;
  CurrentCorIndex=0;
  CurrentTrimPot=0;
  set_tris_b(0xFF);
  set_tris_d(0x00);
  set_tris_e(0xF8);
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_RB0|INT_RB1|INT_RB2|INT_RB3);
  enable_interrupts(INT_RB4_H2L);
  enable_interrupts(GLOBAL);
  output_bit(DTMF_CS ,0);
  output_bit(DTMF_WEB,1);
  output_bit(DTMF_REB,1);
  output_bit(DTMF_RS ,0);
  clearscr();
  init_variables(USE_EEPROM_VARS);
  init_dtmf();
  CLEAR_DTMF_FLAG=1;
  header();
  // C7 : UART RX
  // C6 : UART TX
  // C5 : Aux1 Out
  // C4 : I2C SDA
  // C3 : I2C SCL
  // C2 : PWM Out
  // C1 : Aux0 Out
  // C0 : Aux2 In
  // TRIS_C = 0x5D;
  set_tris_c(0b10011101);
  init_trimpot();
  // Initialize RTC
  rtcc_cnt=60;
  setup_timer_0(T0_INTERNAL|T0_DIV_256);
  enable_interrupts(INT_TIMER0);
  update_ptt(0);
  printf("\n\rInitialization complete");
  MinuteCounter=30;
  SecondCounter=60;
  THIRTY_MIN_FLAG=0;
  MINUTE_FLAG=0;
  PROMPT_FLAG=1;
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
  // Check for "STATUS" command {{{
  strcpy(smatch_reg,"status");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=STATUS;
  } // }}}
  // Check for "reboot" command {{{
  strcpy(smatch_reg,"reboot");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=REBOOT;
  } // }}}
  // Check for "dtmf" command {{{
  strcpy(smatch_reg,"dtmf");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=DTMF_SEND;
  } // }}}
  // Check for "i2c" command {{{
  strcpy(smatch_reg,"i2c");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=I2C_SEND;
  } // }}}
  // Check for "morse" command {{{
  strcpy(smatch_reg,"morse");  
  if ( stricmp(smatch_reg,verb) == 0 ) {
    command=MORSE_SEND;
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
		printf ("\n\r%s %u",argument,value);
  } else {
    pObj=RegMap[argument].reg_ptr;
    *pObj=value;
    LastRegisterIndex = argument;
    LastRegisterIndexValid=1;
    printf ("\n\rSetting %s(%u) to %u",argument_name,argument,value);
    if ( pObj >= &RX_GAIN[0][0] && pObj <= &RX_GAIN[3][3] ) {
      increment(0); // Increment is done in this function. Only update trim pot.
    }
    PROMPT_FLAG=1;
  }
} // }}}

void increment(int incr) { // {{{
  int *pot_ptr;
  int value;
  if ( CurrentCorIndex ) {
    pot_ptr=&RX_GAIN[CurrentCorIndex-1][CurrentTrimPot];
    value = *pot_ptr;
    *pot_ptr = value + incr;
    set_trimpot(CurrentTrimPot,*pot_ptr);
//    printf ("\n\rRX_GAIN [Radio%u] [Pot%u] = %u\n\r",CurrentCorIndex,CurrentTrimPot,*pot_ptr);
  }
} // }}}
void romstrcpy(char *dest,rom char *src) { // {{{
  int c=0;
  while(c<REG_NAME_SIZE) {
    dest[c]=src[c];
	c++;
  }
} // }}}
 
void ExecAuxOutOp(int op,int arg,int ID) { // {{{
  switch(op) {
    case AUXO_FOLLOW_COR: 
      AuxOut[ID] = (COR_IN & arg) != 0;
    break;
  }
} // }}}

void ExecAuxInOp(int op,int arg,int ID) { // {{{
  int1 in_bit;
  in_bit = input(AUX_IN_PIN[ID]);
  switch(op) {
    case AUXI_ENABLE: 
      if (in_bit) { // Enable
        Enable_Mask = arg;
      } else { // Disable bit
        Enable_Mask = 0x0F;
      }
      break;
  }
} // }}}

void update_aux_out(void) { // {{{
  int x;
  int AuxOp;
  int AuxArg;
  int1 out_bit;

  for(x=0;x<3;x++) {
    AuxOp = AuxOutOp[x];
    AuxArg = AuxOutArg[x];
    ExecAuxOutOp(AuxOp,AuxArg,x); // This updates AuxOut global reg.
    out_bit = (AuxOut[x])==0;
    output_bit(AUX_OUT_PIN[x],out_bit);
    // Execute aux inputs {{{
    AuxOp = AuxInOp[x];
    AuxArg = AuxInArg[x];
    ExecAuxInOp(AuxOp,AuxArg,x);
    // }}}
  }
} // }}}

void send_morse_id (void) { // {{{
  int x;
  int mchar;
  for(x=0;x<6;x++) {
    mchar=Morse[x];
    morse(mchar);
    // Delay 7 "dits" between letters
    aux_timer=7*MorseLen[(MorseDitLength&0x03)];
    while(aux_timer) {
      delay_cycles(1);
    }
  }
} // }}}

void main (void) { // {{{
  int x,dtmf;
  initialize();

#ifdef DEBUG_SBUFFER
    //debug_sbuffer();
    //COR_EMUL=1;
#endif
  while(1) { // {{{
    char tmp[5];
	restart_wdt();
    // Process RS232 Serial Buffer Flag {{{
    // The sBufferFlag is set when a "#" or a "\r" is received.
    if ( sBufferFlag ) {
      process_sBuffer();
      clear_sBuffer();
      sBufferFlag=0;
    }
    // Process RS232 Serial Buffer Flag }}} 
    if ( SECOND_FLAG ) {
      update_aux_out();
      if ( SecondCounter ) {
        SecondCounter--;
      } else {
        SecondCounter=60;
        if ( MinuteCounter ) {
          MinuteCounter--;
        } else {
          THIRTY_MIN_FLAG=1;
          MinuteCounter = 30;
        }
      }
      SECOND_FLAG=0;
    }
    if ( THIRTY_MIN_FLAG ) {
      send_morse_id();
      THIRTY_MIN_FLAG=0;
    }
  	if ( COR_FLAG ) {
      process_cor();
      COR_FLAG=0;
   	}
    if ( DTMF_IN_FLAG ) {
      strcpy(LCD_str,"DTMF:");
      printf("\n\rDTMF=");
      for(x=0;x<DTMF_ARRAY_SIZE;x++) {
        if(DTMF_ARRAY[x].Strobe) {
          dtmf=(int)DTMF_ARRAY[x].Key;
          sprintf(tmp,"%d ",dtmf);
          strcat(LCD_str,tmp);
          printf(" %u",dtmf);
        }
      }
      printf("\n\r");
      DTMF_IN_FLAG=0;
      PROMPT_FLAG=1;
      lcd_send(2,LCD_str); // Send DTMF on line 3
    }
    if ( DTMF_FLAG ) {
      process_dtmf();
      DTMF_FLAG=0;
    }
    if ( CLEAR_DTMF_FLAG ) {
      clear_dtmf_array();
      CLEAR_DTMF_FLAG=0;
    }
    if ( PROMPT_FLAG ) {
      prompt();
      PROMPT_FLAG=0;
    }
  } // End of while(1) main loop
} // }}}
