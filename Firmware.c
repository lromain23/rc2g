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
    DTMF_INTERRUPT_FLAG = 1;
    clear_interrupt(INT_RB4_H2L);
  }
  if(interrupt_active(INT_RB6|INT_RB7)) {
    AUX_IN_FLAG=1;
    clear_interrupt(INT_RB6|INT_RB7);
  }
} // }}}
#INT_TIMER0
void int_rtcc(void) { // {{{
  if ( rtcc_cnt ) {
    rtcc_cnt--;
  } else {
    COR_IN=(input_b() ^ Polarity)&0x0F;
    COR_FLAG=1;
    SECOND_FLAG=1;
    AUX_IN_FLAG=1;
    rtcc_cnt=30;
  }
  if (aux_timer ) {
    aux_timer--;
  }
} // }}}
int1 warn_no_lcd = 1;

void lcd_strobe(char data) { // {{{
  i2c_write(data | E);
  i2c_write(data & ~E);
} // }}}

void lcd_write(char rs, char data) { // {{{
  char lcd_word_a,lcd_word_b;
  lcd_word_a = LCD_WRITE | BT | (data&0xF0);
  lcd_word_b = LCD_WRITE | BT | ((data<<4)&0xF0);
  if ( rs ) {
    lcd_word_a |= RS;
    lcd_word_b |= RS;
  } 
  lcd_strobe(lcd_word_a);
  lcd_strobe(lcd_word_b);
} // }}}

void lcd_send(char line,char * s) { // {{{
  int lcd_cmd;
  int1 ack;

#ifdef LCD_ENABLE
  i2c_start();
  #ifdef LCD_TYPE_PI
  ack=i2c_write(LCD_I2C_ADD << 1);
//  ack=i2c_write(line&0x03);
// Change line
  switch(line&0x03) {
    case 0x00: lcd_write(0,CHANGE_LINE);break;
    case 0x01: lcd_write(0,CHANGE_LINE+0x40);break;
    case 0x02: lcd_write(0,CHANGE_LINE+0x14);break;
    case 0x03: lcd_write(0,CHANGE_LINE+0x54);break;
  }
  restart_wdt();
  #else
  lcd_cmd = LCD_I2C_ADD | ((line<<1) & 0x0e);
  ack=i2c_write(lcd_cmd);
  #endif
  if ( ack!=0 ) {
    if ( warn_no_lcd ) {
      printf("\n\rI2C : No ACK from LCD : %u",ack);
      warn_no_lcd = 0;
    }
  } else {
    warn_no_lcd = 1;
  }
  while(*s) {
#ifdef LCD_TYPE_PI
    if ( *s != '\r' && *s != '\n') {
      lcd_write(1,*s);
    }
    s++;
#else
   i2c_write(*s++);
#endif
  }
#ifndef LCD_TYPE_PI
  i2c_write(0); // EOL
#endif
  i2c_stop();
#endif
} // }}}
#ifdef BUTTON_STATES
void status_led(void) { // {{{
  char cnt_val;
  STATUS_LED=0;
  cnt_val = rtcc_cnt>>3;
  if ( button_state!=0 ) {
    if ( (rtcc_cnt & 0x04) ) {
      STATUS_LED = (cnt_val < (CurrentTrimPot+1)); 
    }
  } 
  output_bit(STATUS_LED_PIN,STATUS_LED);
} // }}}
#endif
void execute_command(void) { // {{{
  unsigned int* regPtr;
  int1 init_src;
  int lcd_cmd;
  rom char * cPtr;
  char rname[REG_NAME_SIZE];

  if ( command ) {
    printf("\n\rProcessing Command:");
    printf("\n\r  SiteID  : %u",SiteID);
    printf("\n\r  Command : %u",command);
    printf("\n\r  Argument: %u",argument);
    printf("\n\r  Value   : %u",value);
  }

  switch(command) {
    case SET_REG:
      set_var();
      break;
    case GET_REG:
      regPtr=RegMap[argument].reg_ptr;
      LastRegisterIndex = argument;
      LastRegisterIndexValid=1;
      cPtr = &reg_name + ((unsigned long)argument * REG_NAME_SIZE);
      romstrcpy(rname,cPtr);
      sprintf(LCD_str,"[%02u] %s %u\n\r",argument,rname,*regPtr);
      printf("\n\r%s",LCD_str);
      lcd_send(2,LCD_str);
      prompt();
      break;
    case SAVE_SETTINGS:
      if ( in_admin_mode() ) {
        store_variables();
      }
      break;
    case RESTORE_SETTINGS:
      if ( value == USE_EEPROM_VARS ) {
        init_src=USE_EEPROM_VARS;
      } else {
        init_src=USE_DEFAULT_VARS;
      }
      if ( in_admin_mode() ) {
        init_variables(init_src);
      }
      break;
    case INCREMENT_REG:
      CurrentTrimPot = argument & 0x03;
      increment(value);
      PROMPT_FLAG=1;
      break;
    case DECREMENT_REG:
      CurrentTrimPot = argument & 0x03;
      increment(-1*value);
      PROMPT_FLAG=1;
      break;
    case STATUS:
      status();
      break;
    case ADMIN:
      switch(argument) {
        case REBOOT:
          if ( in_admin_mode() ) {
            reset_cpu();
          }
          break;
        case ENTER_ADMIN:
          set_admin_mode(1);
          break;
        case SEND_MORSE_ID:
          send_morse_id();
        break;
        default:
          set_admin_mode(0);
          break;
      }
      break;
    case I2C_SEND:
      // I2C special commands
      // 0 - 0b100X - Restart LCD
      // 1 - 0b101X - Clear LCD
      // 2 - 0b110X - Init LCD + Welcome screen
      // 3 - 0b111X - Not implemented
#ifdef LCD_TYPE_PI
      switch(value) {
        case 0 : init_lcd();break;  
        case 1 : lcd_write(0,0x01);
      }
#else
      lcd_cmd=4+(value&0x03);
      sprintf(LCD_str,"I2C(%d) CMD : %d",LCD_I2C_ADD,lcd_cmd);
      printf("\n\r%s",LCD_str);
      lcd_send(lcd_cmd,LCD_str);
#endif
      break;
    case MORSE_SEND:
      morse(value);
      break;
  }
} // }}}
void process_sBuffer(void) { // {{{
  unsigned long x;
  rom char * cPtr;
  char rname[REG_NAME_SIZE];

  lcd_send(2,sBuffer);
  argument = -1;
  // tokenize_sBuffer may set argument for some commands
  tokenize_sBuffer();

  // Find matching register or argument
  for(x=0;x<RegMapNum;x++) {
    cPtr = &reg_name + (x * REG_NAME_SIZE);
    romstrcpy(rname,cPtr);
    if(my_stricmp(argument_name,rname)==0) {
      argument=x;
    }
  }
  // Match "EEPROM" or "RAM" for restore/save functions
  if ( argument == -1 ) {
    // save/restore <eeprom>
    // Convert "argument_name" to value when sending dtmf via RS232 using:
    // d <digit> 
    value=str_to_decimal(argument_name);
    strcpy(rname,"eeprom");
    if(my_stricmp(argument_name,rname)==0) {
      value=USE_EEPROM_VARS;
    }
    // save/restore <default>
    strcpy(rname,"default");
    if(my_stricmp(argument_name,rname)==0) {
      value=USE_DEFAULT_VARS;
    }
  }
  if ( command == INCREMENT_REG || command == DECREMENT_REG ) {
    value = 1;
    argument = CurrentTrimPot;
  }
  rs232_mode = 1;
  execute_command();
  rs232_mode = 0;
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
  int1 ack;
  tx_value=pot << 6;
  tx_value=tx_value + (value & 0x3F);
  i2c_start();
  ack=i2c_write(TRIMPOT_WRITE_CMD);
  if ( ack != 0) {
    printf("\n\rI2C : No ACK : %u\n",ack);
  } else {
    i2c_write(tx_value);
    i2c_stop();  
    printf("\n\rPot(%u)<=%u",pot,value);
  }
} // }}}
void morse (int c) { // {{{
  int mc;
  int x;
  int1 do_delay;

  mc = cMorseChar[c]; 
  
  PROMPT_FLAG=1;
  for(x=0;x<4;x++) {
    do_delay=1;
    switch(mc & 0xc0) { // Check two MSB's
      case(0x40):
        dit();
        break;
      case(0x80):
        dah();
        break;
      default:
        do_delay=0;
        break;
    }
    mc = mc << 2; // Shift two MSB's out and continue with next ones
    if ( do_delay ) {
      aux_timer=MorseLen[(MorseDitLength&0x03)];
      while(aux_timer) {
        delay_cycles(1);
      }
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
} // }}}
void update_ptt(int cor) { // {{{
  char x,pot;
  int pot_val;
  int mask;
  int ptt;
  char COR_s[5]={'0','0','0','0',0};
  char PTT_s[5]={'0','0','0','0',0};
  int1 rx_bit,ptt_bit;

  CurrentCorIndex=cor;

  if ( cor ) {
    ptt=RX_PTT[cor-1] & (Enable & Enable_Mask);
  } else {
    ptt=0;
    if ( COR_DROP_FLAG ) {
      COR_DROP_FLAG=0;
      if ( ConfirmChar || TailChar ) {
        send_tail();
      }
    }
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
    if(ptt_bit) {
      PTT_s[x]='1';
    }
    mask=mask<<1;
  } 
  if(!cor) {
    CurrentCorPriority=0;
  } else {
    CurrentCorPriority=RXPriority[cor-1];
  // Update TrimPots
    for(pot=0;pot<4;pot++){
      pot_val=RX_GAIN[cor-1][pot];
      set_trimpot(pot,pot_val);
    }
    PROMPT_FLAG=1;
  }
  if(cor>0) {
    COR_s[cor-1]='1';
  }
  sprintf(LCD_str,"COR:%s PTT:%s",COR_s,PTT_s);
  lcd_send(1,LCD_str); // COR/PTT on line 1
  delay_ms(50);
  pot_values_to_lcd();
}// }}}
int ValidKey(int index) { // {{{
  int strobe;
  if(index>=0 && (index <= DTMF_ARRAY_SIZE)) {
    if(DTMF_ARRAY[index].Strobe && (DTMF_ARRAY[index].Key != dp)) {
      strobe=1;
     } else {
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
  unsigned site_id;
  unsigned digit;
  // Structure:
  // [SID1][SID0][CMD1][CMD0][ARG1][ARG0][Valx][Valy][Valz]
  //   0     1     2     3     4     5     6     7     8
  // Commands:
  // 01 : Link commands
  // 02 : Set register value
  // 03 : get register value
  // 04 : Save settings to EEPROM
  // 05 : Restore settings from EEPROM
  // 06 : Increment Current Pot
  // 07 : Decrement Current Pot
  // 08 : Status
  // 09 : AdminSettings 
  //    :    Args : 0 - Normal mode
  //    :           1 - Enter Admin mode
  //    :           2 - Reboot
  // 10 : -- UNUSED --
  // 11 : Morse send
  // 12 : Send to I2C
  // 
  // Ex: (# = 12)
  // Enter Admin mode       : 52 09 01 #
  // Reboot                 : 52 09 02 #
  // Send Morse ID          : 52 09 03 #
  // Set XO3(22) to 0       : 52 02 22 0 #
  // Set XO3(22) to 1       : 52 02 22 1 #
  // Change to pot 4        : 52 02 55 3 #
  // Increment POT 01 by 3  : 52 06 01 3 #
  // Decrement POT 03 by 4  : 52 07 03 4 #
  // -- User Functions --
  // Disable AuxOut0        : 51 02 21 0 # 
  // Disable AuxOut0 (!Arg) : 51 02 21 # 
  // Enable  AuxOut1        : 51 02 22 1 #
  command=0;
  value=0;
  if ( ValidKeyRange(0,3)) {
    site_id = DTMF_ARRAY[0].Key *10 + DTMF_ARRAY[1].Key;
    command = DTMF_ARRAY[2].Key * 10 + DTMF_ARRAY[3].Key;
    if ( ValidKeyRange(4,5) ) {
      // Admin mode {{{
      argument = DTMF_ARRAY[4].Key * 10 + DTMF_ARRAY[5].Key;
      digit=6;
      while(ValidKey(digit)) {
       value = value * 10 + DTMF_ARRAY[digit].Key;
       digit++;
      }
      // Admin mode }}}
    } else {
      // User function {{{
      // Only 4 digits were entered. 
      // Use 'command' value as user function.
      switch(command) {
        // Commands 10 (disable link) and 11 (enable link)
        case(10):
          argument = 0;
          value = 0x0E;
  	      break;
        case(11):
          argument = 0;
          value = 0x0F;
   		    break;
      }
      // Override command
      command=SET_REG;
      // User function }}}
    }
    // Commands that don't need arguments but need a value:
    switch(command) {
      case(SAVE_SETTINGS):
      case(RESTORE_SETTINGS): 
        value = argument;
        break;
    }
    if ( site_id == SiteID ) {
      execute_command();
    }
  }
  // Allow 'b', 'c' and 'd' to turn 'down' and 'up' potentiometer {{{
  // 'b' --> Next pot (DTMF 14)
  // 'c' --> pot down (DTMF 15)
  // 'd' --> pot up   (DTMF 13)
  command=DTMF_ARRAY[0].Key;
  if ( AdminMode && ValidKeyRange(0,0) ) {
    restart_wdt();
    switch(command) {
      case(db): // d 14 d 12
        CurrentTrimPot=(CurrentTrimPot+1)&0x03;
        pot_values_to_lcd();
        break;
      case(dc): // d 15 d 12 
        increment(-1);
        break;
      case(d0): // d 10 d 12
        // DTMF (D) is mapped to value 10 (d0)
        increment(1);
        break;
    }
    in_admin_mode();
  }
  // Allow 'b', 'c' and 'd' to turn 'down' and 'up' potentiometer }}}
  CLEAR_DTMF_FLAG=1;
} // }}}
void process_cor (void) { // {{{
  int cor_mask;
  int rx_priority;
  int cor_in;
  int do_update_ptt;
  int cor_index;
  int x;

  cor_mask=1;
  do_update_ptt=0;
  // Allow emulated COR[4] for DTMF control (No audio feed-thru)
  cor_in = COR_IN | (COR_EMUL&0x1F);
  // Different COR was waiting for the active one to fall.
  if ( CurrentCorPriority && !(cor_in&CurrentCorMask) ) {
    CurrentCorPriority=0;
    CurrentCorMask=0;
    do_update_ptt=1;
    if ( (cor_in & Enable & Enable_Mask) == 0x00) {
      COR_DROP_FLAG=1;
      if ( Tail ) {
        // Tail register has priority over any aux input char
        TailChar=Tail;
      }
    }
  }
  cor_index=0;
  for(x=0;x<4;x++) {
    if ( cor_in & cor_mask ) {
      if ( (Enable & Enable_Mask) & cor_mask ) {
        rx_priority=RXPriority[x];
      } else {
        // Radio is not enabled. Only listen for DTMF (if no other radio is enabled).
        rx_priority = 1; // Least priority while still active.
      }
      // New COR is being captured.
      // Initialize TOT timer
      if ( rx_priority > CurrentCorPriority ) {
        if ( ! CurrentCorPriority ) {
          CurrentCorPriority = rx_priority;
        }
        cor_index=x+1;
        do_update_ptt=1;
        TOT_SecondCounter= 60 * TOT_Min;
        // COR_IN_EFFECTIVE points to the one that is selected
        COR_IN_EFFECTIVE=cor_mask;
      }
    }
    cor_mask = cor_mask << 1;
  }
  if ( do_update_ptt ) {
    update_ptt(cor_index);
    PROMPT_FLAG=1;
  }
  // Clear the DTMF array when all CORs fall
  if ( !cor_in ) {
    // --> Don't clear the DTMF if the Aux Input is emulating a COR
    CLEAR_DTMF_FLAG=1;
    COR_IN_EFFECTIVE=0;
  }
  // Refresh Link Time-out timer when COR is received.
  // Any COR value refreshes teh link TOT timer.
  if ( Link_TOT != 0 && (cor_in)!=0 ) {
    LinkDurationTimer = Link_TOT;
  }
} // }}}
void clear_dtmf_array(void) { // {{{
  int x;

  for(x=0;x<sizeof(DTMF_ARRAY);x++) {
    DTMF_ARRAY[x]=(sDTMF)0;
  }
  DTMF_ptr=&DTMF_ARRAY[0];
} // }}}
void header (void) { // {{{
//  putc(ESC);
//  printf("[47;34m\n\rRadio Repeater Controller - ");
//  putc(ESC);
//  printf("[47;31mVE2LRS");
//  putc(ESC);
//  printf("[47;34m (C) 2013\n\n\r");
//  putc(ESC);
//  printf("[40;37m");
} // }}}
void status (void) { // {{{
  unsigned long x;
  char y;
  rom char * cPtr;
  unsigned int * regPtr;
  int dtmf_in;
  char aux_in;
  char rname[REG_NAME_SIZE];
  clearscr();
  header();
  dtmf_in=dtmf_read(CONTROL_REG);
  aux_in = 0;
  for(x=0;x<RegMapNum;x++) {
// Bug when X = 0x2B (6). cPtr wraps back to 0x017C!!!
// reg_name = 0x017A.
// cPtr is assigned to 0x017C
    cPtr = &reg_name + (x*REG_NAME_SIZE);
    romstrcpy(rname,cPtr);
  regPtr=RegMap[x].reg_ptr;
    printf("[%02Lu] %s %u\t",x,rname,*regPtr);
    if ( x %4 == 3 ) {
      putc('\n');
      putc('\r');
    }
    restart_wdt();
  }
  for(y=0;y<3;y++) {
    if(AuxInSW[y]==1) {
      aux_in += 2<<y;
    }
  }
  printf("\n\n\rCOR:%u (Emul:%u); AuxIn:%u",COR_IN,COR_EMUL,aux_in);
  printf("\n\rDTMF Status : %u\n\r",dtmf_in);
  pot_values_to_lcd();
  PROMPT_FLAG=1;
} // }}}
void pot_values_to_lcd (void) { // {{{
  char x;
  int8 pot_val;
  int1 ack,ack_in;
  unsigned c[4]={' ',' ',' ',' '};
  unsigned pval[4]={0,0,0,0};
  delay_ms(40);
  i2c_start();
  ack_in=i2c_write(TRIMPOT_READ_CMD);
  for(x=0;x<4;x++) {
    if(x==3) {
      ack=0;
    } else {
      ack=1;
    }
    pot_val=i2c_read(ack);
    pot_val=pot_val&0x3F;
    pval[x]=pot_val;
    if ( (0x03 & CurrentTrimPot) == x ) {
      c[x] = '*';
    }
  }
  i2c_stop();
  delay_ms(50);
  if ( ack_in!=0 ) {
    printf("\n\rI2C Error : No ACK from TRIMPOTS : %u",ack);
  }
  // 0x7e character is right arrow
  // 0xc7 on LCD displays with standard characters
  sprintf(LCD_str,"POT:%c%d %c%d %c%d %c%d",c[0],pval[0],c[1],pval[1],c[2],pval[2],c[3],pval[3]);
  lcd_send(0,LCD_str); // COR/PTT on line 0
  printf("\n\r%s",LCD_str);

} // }}}
void prompt(void) { // {{{
  if ( AdminMode ) {
    printf("\n\n\rADMIN> ");
  } else {
    printf("\n\n\rCOMMAND> ");
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
//  dtmf_write(TOUT|IRQ|RSELB,CONTROL_REG); // Enable TOUT and IRQ
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
  restart_wdt();
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
  restart_wdt();
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
void print_dtmf_info(void) { // {{{
  unsigned int x;
  char dtmf;
  char tmp[5];
  strcpy(LCD_str,"DTMF:");
  printf("\n\rDTMF=");
  for(x=0;x<DTMF_ARRAY_SIZE;x++) {
    if(DTMF_ARRAY[x].Strobe) {
      dtmf=(int)DTMF_ARRAY[x].Key;
      sprintf(tmp,"%d ",dtmf);
      strcat(LCD_str,tmp);
      printf(" %u",dtmf);
    }
  restart_wdt();
  }
  printf("\n\r");
  PROMPT_FLAG=1;
  lcd_send(2,LCD_str); // Send DTMF on line 3
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
    printf("\n\rInit RAM <= EEPROM");
  } else {
    printf("\n\rInit RAM <= HW Defaults");
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
       retVal = 0 ; // Error : Checksum does not match when initializing variables from EEPROM
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
  COR_IN=0;
  COR_DROP_FLAG=0;
  DTMF_IN_FLAG=0;
  DTMF_INTERRUPT_FLAG=0;
  LastRegisterIndexValid=0;
  LastRegisterIndex=0;
  CurrentCorMask=0;
  CurrentCorPriority=0;
  CurrentCorIndex=0;
  CurrentTrimPot=0;
  set_tris_b(0xFF);
  set_tris_d(0x00);
  // 0b11111000
  set_tris_e(0xF8);
  enable_interrupts(INT_RDA);
  enable_interrupts(INT_RB0|INT_RB1|INT_RB2|INT_RB3|INT_RB6|INT_RB7);
  enable_interrupts(INT_RB4_H2L);
  enable_interrupts(GLOBAL);
  output_bit(DTMF_CS ,0);
  output_bit(DTMF_WEB,1);
  output_bit(DTMF_REB,1);
  output_bit(DTMF_RS ,0);
  //clearscr();
  init_variables(USE_EEPROM_VARS);
  init_dtmf();
  CLEAR_DTMF_FLAG=1;
  Enable_Mask = 0x0F;
  // Port B Pullups {{{
  // AuxIn pins : B6, B7, C0
  // Port_x_pullups requires a bit value corresponding to each
  // bit.
  // AuxIn 1:0: Pins B6&B7
  // COR 3:0: Pins B3:B0
  // DTMF interrupt : PIN_B4 (No pull-up required)
  // PIN_B5 : Adjust trmipot. Nu pull-up required
  // port_b_pullups(0b11000000 | (Polarity & 0x0F));
  WPUB = 0b11000000 | ( Polarity & 0x0F);
  // Set WPUEN (bar) bit on OPTION_REG
  // Master Weak pull-up enable
  WPUEN = 0;
  // }}}
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
  // Pin A7 --> ENTER button
  set_tris_a(0b10000000);
  init_trimpot();
  // Initialize RTC
  rtcc_cnt=30;
  setup_timer_0(T0_INTERNAL|T0_DIV_256);
  enable_interrupts(INT_TIMER0);
  update_ptt(0);
  MinuteCounter=MIN_COUNTER;
  SecondCounter=SEC_COUNTER;
  THIRTY_MIN_FLAG=0;
  MINUTE_FLAG=0;
  PROMPT_FLAG=1;
  TailChar=Tail;
  ConfirmChar=0;
  //AuxOut[0] = PO_AUX_OUT0;
  //AuxOut[1] = PO_AUX_OUT1;
  //AuxOut[2] = PO_AUX_OUT2;
  AuxInSW[0] = 0;
  AuxInSW[1] = 0;
  AuxInSW[2] = 0;
  AUX_IN_FLAG = 0;
  COR_IN_EFFECTIVE=0;
  set_admin_mode(0);
  rs232_mode=0;
  button_state=0;
#ifdef LCD_TYPE_PI
  init_lcd();
#endif
  setup_adc(ADC_CLOCK_INTERNAL);
  setup_adc_ports(ADJ_POT|VSS_VDD);
  set_adc_channel(13);

//  setup_adc_ports(ADJ_POT);
//  set_adc_channel(13);
//  printf("\n\rInitialization complete");
} // }}}
void tokenize_sBuffer() { // {{{
  char verb[8];
  int1 do_get_var=0;
  char *sptr;
  char match_tok[8],match_val[4],smatch_reg[8];

  // Get verb {{{
  strcpy(match_tok," ,;\r");
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
    value = str_to_decimal(match_val);
  } else {
    value = 0;
    do_get_var = 1;
  }  
  // }}}
  // Check for "SET" command {{{
  strcpy(smatch_reg,"set");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    if ( do_get_var ) {
      command=GET_REG;
    } else {
      command=SET_REG;
    }
  } // }}}
  // Check for "SAVE" command {{{
  strcpy(smatch_reg,"SAVE");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
      command=SAVE_SETTINGS;
  } // }}}
  // Check for "RESTORE" command {{{
  strcpy(smatch_reg,"RESTORE");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
      command=RESTORE_SETTINGS;
  } // }}}
  // Check for "STATUS" command {{{
  strcpy(smatch_reg,"status");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=STATUS;
  } // }}}
  // Check for "reboot" command {{{
  strcpy(smatch_reg,"reboot");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=ADMIN;
    argument=REBOOT;
  } // }}}
  // Check for "dtmf" command {{{
  strcpy(smatch_reg,"d");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    //command=DTMF_SEND;
    command=0;
    value = str_to_decimal(argument_name);
    if ( value == d0 ) {
      value = dd;
    } else if (value == dd) {
      value = d0;
    }
    dtmf_send_digit(value&0x0F);
  } // }}}
  // Check for "i2c" command {{{
  strcpy(smatch_reg,"i2c");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=I2C_SEND;
  } // }}}
  // Check for "morse" command {{{
  // morse [0..35] --> send 0-9 or a-z in morse
  // morse[36]     --> Silence
  // morse <value> 37 --> send site ID
  strcpy(smatch_reg,"morse");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    value = str_to_decimal(argument_name);
    if ( value < MORSE_CHAR_ARRAY_LENGTH ) {
      argument = 0;
      command  = MORSE_SEND;
    } else {
      command  = ADMIN;
      argument = SEND_MORSE_ID;
    }
  } // }}}
  // Check for "+ (INCR)" command {{{
  strcpy(smatch_reg,"+");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=INCREMENT_REG;
  } // }}}
  // Check for "- (DECR)" command {{{
  strcpy(smatch_reg,"-");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=DECREMENT_REG;
  } // }}}
  // Check for "n (Next CPOT)" command {{{
  strcpy(smatch_reg,"n");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    command=SET_REG;
    value = (CurrentTrimPot + 1)&0x03;
    strcpy(argument_name,"CPOT");
  } // }}}
  // AdminMode toggle Check for "admin" command {{{
  strcpy(smatch_reg,"admin");  
  if ( my_stricmp(smatch_reg,verb) == 0 ) {
    AdminMode = ~AdminMode;
    set_admin_mode(AdminMode);
    PROMPT_FLAG = 1;
  } // }}}
} // }}}
void set_var (void) { // {{{
  // This function sets the specified register if a value is specified.
  // Otherwise it displays it.
  int *pObj;
  int lVar;
  if ( value == -1 ) {
    printf ("\n\r%s %u",argument,value);
  } else {
    pObj=RegMap[argument].reg_ptr;
    // Consider allowing some registers to be updated outside AdminMode.
    // Example : AuxOut registers, Enable
    if ( in_admin_mode() || (RegMap[argument].usage==PUBLIC) ) {
      *pObj=value;
    }
    lVar = *pObj;
    LastRegisterIndex = argument;
    LastRegisterIndexValid=1;
    printf ("\n\rSetting %s(%u) to %u",argument_name,argument,lVar);
    if ( (pObj >= &RX_GAIN[0][0] && pObj <= &RX_GAIN[3][3]) || pObj == &CurrentTrimPot ) {
      increment(0); // Increment is done in this function. Only update trim pot.
    }
    PROMPT_FLAG=1;
  }
} // }}}
void increment(int incr) { // {{{
  int8 *pot_ptr;
  int8 value;
  char CPotPtr;
  CPotPtr=CurrentTrimPot & 0x03;
  if ( CurrentCorIndex ) {
    pot_ptr=&RX_GAIN[CurrentCorIndex-1][CPotPtr];
    value = *pot_ptr;
    // Do not exceed 63 during increment or 0 during decrement
    *pot_ptr = value + incr;
    if ( in_admin_mode() ) {
      set_trimpot(CPotPtr,*pot_ptr);
    }
  }
  pot_values_to_lcd();
} // }}}
void romstrcpy(char *dest,rom char *src) { // {{{
  int c=0;
  while(c<REG_NAME_SIZE) {
    dest[c]=src[c];
  c++;
  }
} // }}}
void ExecAuxOutOp(int op,int arg,int ID) { // {{{
  char larg,uarg; // Lower and upper nibbles
  larg = arg & 0x0F;
  uarg = (arg & 0xF0) >> 4;
  switch(op) {
    case AUX_OUT_FOLLOW_COR: 
      // Invert AuxIn value if argument 1 is set
      // Check what is the effective COR_IN. Many COR_INs can be applied but 
      // Only one is really effective and used to drive PTTs
      AuxOut[ID] = ((COR_IN_EFFECTIVE ^ uarg) & larg) != 0;
    break;
    case AUX_OUT_FOLLOW_AUX_IN:
      // Lower argument (larg) enables the comparison (bitwise enable)
      // Upper argument (uarg) inverts the output (bitwise invert selection)
      AuxOut[ID] = ((AuxInSW & larg) ^ uarg)!=0;
    break;
  }
} // }}}
char str_to_decimal(char *str) { // {{{
  // Convert string to unsigned integer
  int x=0;
  char value=0;
  while(str[x]!=0 && str[x] >= '0' && str[x] <= '9') {
    value = (value * 10) + (str[x]-'0');
    x++;
  }
  return(value);
} // }}}
void ExecAuxInOp(int op,int arg,int ID) { // {{{
  int1 in_bit;
  int1 tmp_bit;
  in_bit = AuxInSW[ID]!=0;
  char larg,uarg; // Lower and upper nibbles
  // Include arg[4] for COR4 emulation
  larg = arg & 0x1F;
  uarg = (arg & 0xF0) >> 4;
  switch(op) {
// Must add a method to reset the Enable_Mask to 0x0F when
// the operator is not AUXI_ENABLE
    case AUXI_ENABLE: 
      if (in_bit) { // Enable
        Enable_Mask &= arg;
      } else { // AuxIn is not enabled
        Enable_Mask |= (~arg & 0x0F);
      }
      break;
    case AUXI_TAIL_WHEN_HI:
      if ( in_bit ) {
        COR_DROP_FLAG=1;
        TailChar=arg;
      } else {
        TailChar=0;
      }
    break;
    case AUXI_TAIL_WHEN_LO:
      if ( in_bit==0 ) {
        COR_DROP_FLAG=1;
        TailChar=arg;
      } else {
        TailChar=0;
      }
    break;
    case AUXI_EMULATE_COR:
      if ( (arg & AUXI_EMULATE_COR_ACTIVE_LO) != 0 ) {
        tmp_bit = ~in_bit;
      } else {
        tmp_bit = in_bit;
      }
      if ( tmp_bit ) {
        COR_EMUL |= larg;
      } else {
        COR_EMUL &= ~larg;
      }
    break;
  }
} // }}}
void update_aux_in(void) { // {{{
  int x;
  for(x=0;x<3;x++) {
    // AuxIn is enabled via RS232 only for test/emulation purpose
    AuxInSW[x] = (input(AUX_IN_PIN[x])!=0) || (AuxIn[x]!=0);
  }
} // }}}
void update_aux_out(void) { // {{{
  char x;
  char AuxOp;
  char AuxArg;
  char AuxIn_s[4]={'0','0','0',0};
  char AuxOut_s[4]={'0','0','0',0};
  char ADM[]=" ADMIN";
  int1 out_bit;

  for(x=0;x<3;x++) {
    AuxOp = AuxOutOp[x];
    AuxArg = AuxOutArg[x];
    ExecAuxOutOp(AuxOp,AuxArg,x); // This updates AuxOut global reg.
    out_bit = (AuxOut[x])==0;
    output_bit(AUX_OUT_PIN[x],out_bit);
    if(out_bit==0) {
      AuxOut_s[x]='1';
    }
    // Execute aux inputs {{{
    AuxOp = AuxInOp[x];
    AuxArg = AuxInArg[x];
    if(AuxInSW[x]==1) {
      AuxIn_s[x]='1';
    }
    ExecAuxInOp(AuxOp,AuxArg,x);
    // }}}
  }
  sprintf(LCD_str,"I:%s O:%s",AuxIn_s,AuxOut_s);
  if ( AdminMode ) {
    strcat(LCD_str,ADM);
  }
  lcd_send(3,LCD_str);
} // }}}
void send_morse_id (void) { // {{{
  int x;
  int mchar;
  // Send morse as if it was received from COR(1) -- Link radio
  update_ptt(1); 
  delay_ms(1000);
  for(x=0;x<6;x++) {
    mchar=Morse[x];
    morse(mchar);
    // Delay 3 "dits" between letters
    aux_timer= 3 * MorseLen[(MorseDitLength&0x03)];
    while(aux_timer) {
      delay_cycles(1);
    }
  }
  delay_ms(1000);
  COR_FLAG=1;
} // }}}
void main (void) { // {{{
  initialize();
#ignore_warnings 203
  while(1) { // {{{
#ignore_warnings none
    restart_wdt();
  process_buttons();
    // Process RS232 Serial Buffer Flag {{{
    // The sBufferFlag is set when a "\r" or "+" or "-" is received.
    if ( sBufferFlag ) {
      process_sBuffer();
      clear_sBuffer();
      sBufferFlag=0;
    }
    // Process RS232 Serial Buffer Flag }}} 
  restart_wdt();
    if ( AUX_IN_FLAG ) {
      update_aux_in();
      AUX_IN_FLAG=0;
    }
    do_delay_counters();
    restart_wdt();
    if ( COR_FLAG ) {
      process_cor();
      // Call update_aux_out to instantly update AuxOut 
      // values when one of them is following a COR.
      update_aux_out(); 
      COR_FLAG=0;
      restart_wdt();
    }
    if ( DTMF_INTERRUPT_FLAG ) {
      // Extract data from DTMF device
      process_dtmf_interrupt();
      DTMF_INTERRUPT_FLAG=0;
    }
    if ( DTMF_IN_FLAG ) {
      print_dtmf_info();
      DTMF_IN_FLAG=0;
      restart_wdt();
    }
    if ( DTMF_FLAG ) {
      process_dtmf();
      DTMF_FLAG=0;
    restart_wdt();
    }
    if ( CLEAR_DTMF_FLAG ) {
      clear_dtmf_array();
      CLEAR_DTMF_FLAG=0;
    }
    if ( PROMPT_FLAG ) {
      prompt();
      PROMPT_FLAG=0;
    restart_wdt();
    }
  } // End of while(1) main loop }}}
} // }}}
// send_tail {{{
void send_tail(void) {
  restart_wdt();
  delay_ms(1000);
  if ( ConfirmChar!=0 ) {
    morse(ConfirmChar);
    ConfirmChar=0;
    restart_wdt();
    delay_ms(500);
  }
  if (TailChar != 0) {
    morse(TailChar);
    TailChar=0;
    restart_wdt();
    delay_ms(500);
  }
  restart_wdt();
}
// send_tail }}}
int1 in_admin_mode(void) { // {{{
  // Refresh timer
  if (AdminMode) {
    admin_timer = ADMIN_TIMEOUT;
  }
  return(AdminMode||rs232_mode);
} // }}}
void set_admin_mode(int1 enable) { // {{{
  AdminMode = (enable!=0);
  if (AdminMode) {
    // Enter Admin mode
    ConfirmChar = MCHAR('a');
    admin_timer = ADMIN_TIMEOUT;
  } else {
    // Exit / out of admin mode
    ConfirmChar = MCHAR('o');
  } 
} // }}}
// string matchnig function with case insensitive match.
int1 my_stricmp(char *s1,char *s2) { // {{{
  unsigned int x=0;
  const char AMASK=0xDF;
  while((AMASK&s1[x])==(AMASK&s2[x])) {
    if(s1[x]==0) {
      return 0;
    }
    x++;
  }
  // Strings don't match. Return 1.
  return 1;
} // }}}
// Fetches data from MC8888 device upon interrupt
void process_dtmf_interrupt(void) { // {{{
  int value,dtmf_status;
  dtmf_status = dtmf_read(CONTROL_REG);
  if ( dtmf_status & DTMF_BUFFER_FULL) {
    value=dtmf_read(DATA_REG);
    DTMF_IN_FLAG=1;
    if ( value == dd ) {
      value=d0;
    } else if ( value == d0 ) {
      value=dd;
    }
    if ( value == ds ) {
      CLEAR_DTMF_FLAG=1;
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
  }
} // }}}
void init_lcd(void) { // {{{
#define INIT 0x18
#define MODE_SET 0x1C
#define ENABLE 0x04
  i2c_start();
  i2c_write(LCD_I2C_ADD<<1);
  // Function set
  // Initialize in 8-bit mode first for 3 clock cycles
  lcd_strobe(0x30);lcd_strobe(0x30);lcd_strobe(0x30);
  // Init 4-bit interface mode
  lcd_strobe(0x20);
  lcd_write(0,0x2C);
  // Display On/Off, Cursor off
  lcd_write(0,0x0C);
  // Display clear
  lcd_write(0,0x01);
  // Entry mode set
  lcd_write(0,0x04);
  i2c_stop();
} // }}}
void do_delay_counters(void) {
  // Second Flag {{{
  if ( SECOND_FLAG ) {
    update_aux_out();
    // Time Out PTT {{{
    if ( TOT_SecondCounter || TOT_Min == 0) {
      TOT_SecondCounter--;
    } else if ( COR_IN != 0x00 ) {
      update_ptt(0);
      printf("\n\r# PTT Timeout!\n");
      PROMPT_FLAG=1;
    }
    // }}}
    // Admin mode timeout {{{
    if ( admin_timer ) {
      admin_timer--;
    } else {
      // Exit admin mode.
      if ( AdminMode ) {
        set_admin_mode(0);
      }
    }
    // }}}
    restart_wdt();
    if ( SecondCounter ) {
      SecondCounter--;
    } else {
      SecondCounter=SEC_COUNTER;
      MINUTE_FLAG = 1;
    }
    SECOND_FLAG=0;
  }
  // Second Flag }}}
  // Minute flag {{{
  if ( MINUTE_FLAG ) {
    if ( MinuteCounter ) {
      MinuteCounter--;
    } else {
      THIRTY_MIN_FLAG=1;
      MinuteCounter = MIN_COUNTER;
    }
    MINUTE_FLAG = 0;
    // Link timeout timer {{{
    if ( Link_TOT != 0 ) {
      if ( LinkDurationTimer ) {
        LinkDurationTimer--;
      } else {
        // Disable Link
        printf("\n\r# Link Timeout!\n");
        Enable&=0xFE;
      }
    }
    // Link timeout timer }}}
  }
  // Minute flag }}}
  if ( THIRTY_MIN_FLAG ) { // {{{
    if ( (TXSiteID&0x03) !=0 ) {
      // Transmit Site ID every 30 mins when:
      // TXSiteID = <EnableMask[3:0]>,{x,x,M,E}
      // E = Transmit every 30 mins
      // M = Transmit only if EnableMask is off
      if ( (TXSiteID & 0x01)!=0 || ((TXSiteID & 0x02)!=0 && (((TXSiteID >> 4) & 0x0F) & Enable)==0) ) {
        send_morse_id();
      }
    }
    THIRTY_MIN_FLAG=0;
  } // }}}
}
void process_buttons(void) { // {{{
#ifdef BUTTON_STATES
  char enter_b,select_b;
  unsigned _cor_in;
  unsigned int pot_value;
  char CPotPtr;
  CPotPtr=CurrentTrimPot & 0x03;
  // Process Enter / select buttons {{{
  _cor_in = (COR_IN | COR_EMUL ) & 0x0F;
  if ( input(ENTER_BUTTON)==0 ) {
    ENTER_PRESSED = (enter_b == DEBOUNCE_COUNT);
    if ( enter_b < DEBOUNCE_COUNT+ 1 ) {
      enter_b++; 
    }
  } else {
    enter_b = 0;
    ENTER_PRESSED = 0;
  }
  if ( input(SELECT_BUTTON)==0 ) {
    SELECT_PRESSED = (select_b == DEBOUNCE_COUNT);
    if ( select_b < DEBOUNCE_COUNT + 1 ) {
      select_b++;
    }
  } else {
    select_b = 0;
    SELECT_PRESSED = 0;
  }
  // Define Button States
  // IDLE + ENTER --> TRIM
  // TRIM + ENTER --> Exit
  // TRIM + SELECT --> NextPot
  switch (button_state) {
    case BUTTON_IDLE:  
      if ( ENTER_PRESSED == 1 ) {
        button_state=CALIB;
      }
    break;
    case CALIB:
      adj_value_a = read_adc() >> 2;
      adj_value_b = adj_value_a;
      button_state=TRIM;
      pot_values_to_lcd();
      break;
    case TRIM:
       if ( _cor_in != 0 ) {
         adj_value_a = read_adc() >> 2;
         pot_value = 63-adj_value_a;
         if ( adj_value_a != adj_value_b ) {
           rs232_mode = 1;
           set_trimpot(CurrentTrimPot, pot_value);
           pot_values_to_lcd();
	   RX_GAIN[CurrentCorIndex-1][CPotPtr]=pot_value;
           rs232_mode = 0;
         }
         adj_value_b = adj_value_a;
	     }
       if ( SELECT_PRESSED == 1 ) {
         CurrentTrimPot = (CurrentTrimPot + 1 ) & 0x03;
         pot_values_to_lcd();
       }
       if ( ENTER_PRESSED == 1 ) {
         // Hold SELECT and press ENTER to store settings in EEPROM
         if ( input(SELECT_BUTTON)==0 ) {
           store_variables();
         }
         button_state = BUTTON_IDLE;
	     } 
       status_led();
    break;
    default:
  		button_state = BUTTON_IDLE;
    break;
  }
  restart_wdt();
  // }}}
#endif
} // }}}
