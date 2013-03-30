Changes since the last manual:


The #USE DELAY now allows the options USB_LOW and USB_FULL to set up
the clocks for USB 1.x or USB 2.x.


The #USE RS232 now allows the incomming and outgoing data to be routed
through an ICD-U64 (while not in debugging mode).  To use:
      #USE RS232(ICD)
The CCSLOAD program will open a terminal window when you run the program.
The PGD pin is used for TX on the PIC and PGC is used for RX.

A new ticks library has been implemented, #use ticks().  See the help file 
for documentation and ex_ticks.c for an example of it's usage.

PSV support for 16-bit PICs has been added:
   #device PSV=16
When used, all 'rom' and 'const' keywords are fit into a PSV page of the 
program memory, and the contents of the 'rom and 'const' structure/array 
are made PSV friendly.  'const' or 'rom' pointers can be assigned to RAM 
pointers and still allow access to the data in the program memory.
Example:
   #device PSV=16
   const char constString[] = "Hello";
   const char *constPtr = constString; //constPtr points to program memory in PSV
   char *ramPtr = constString;   //ramPtr points to program memory in PSV
   printf("%s", constPtr);
   printf("%s", ramPtr);

The INTR_NESTING option for enable/disable interrupts has been removed.  The
new way to enable nesting is to use the following since too many problems occur
when changing the nesting state at run time:
                #device  NESTED_INTERRUPTS

A new option has been added to the command line to allow a source line to be
injected at the start of the source file.  For example:
        CCSC +FM myfile.c sourceline="#include <16F887.h>"

Interrupts have been defined in the device header file to also define
the edge on interrupts that allow it.  This saves a separate call to set_int_edge().
For example:
      enable_interrupts( INT_EXT_H2L );

A new built in function INPUT_CHANGE_x() may be used to find out what port
pins have changed since the last call to INPUT_CHANGE_x().  This may be used
in a interrupt like the INT_RB to find out what pins changed state.

#export not accepts a filetype of C to export interface definitions for
another program to link to this program in the same PIC memory.  #build
also has new options to support this interface.  See API.ZIP for a full
example.

#USE RS232 now accpets CALL_PUTC=, CALL_GETC= and CALL_KBHIT= to use user
defined functions for these functions.

The new preprocessor function DEFINEDINC(id) returns a C status
of an ID as follows:
   0 - not known
   1 - typedef or enum
   2 - struct of union type
   3 - typemod qualifier
   4 - function prototype
   5 - defined function
   6 - compiler built in function
   7 - local variable
   8 - global variable

#PIN_SELECT has been added to PCH for parts with PPS.  Use it like this:
        #PIN_SELECT U1TX=PIN_B0

The names of the ICD pair select fuses have changed to
ICSP1,ICSP2 and ICSP3 to match the pin numbers.  The old
fuses are still accepted by the compiler but will not be documented.
The following may also be used instead of the fuses:
      #device  ICD=2



Older versions of the compiler did not handle pointers to ROM consistantly.
The following documents the current syntax:
   Declaration                 Result
   -----------                 ----------------------------------------
   char id;                    id is stored in RAM
   char rom id;                id is stored in ROM
   rom char id;                id is stored in ROM
   rom char * id;              id is stored in RAM, is a pointer to ROM
   rom char rom * id;          id is stored in RAM, is a pointer to ROM
   char rom * id;              id is stored in RAM, is a pointer to ROM
   char * rom id;              id is stored in ROM, is a pointer to RAM
   rom char * rom id;          id is stored in ROM, is a pointer to ROM

There is one exception to the above rules in that a function parametter is never
stored in ROM.  At present the compiler may not issue an error for that syntax but
rather save the id in RAM.

Setup_ccpx() when used with timer 3 now assigns timer3 in the call to setup_timer_3()
not in the call to setup_ccp().

A new option has been added to #rom to pack 7 bit characters into the PCM 14 bit word.
For example:
        #ROM CHAR 0x1000={"THIS STRING TAKES 22 INSTRUCTION LOCATIONS"}

Some keywords have been added to make selecting the hardware pins for a UART or I2C easier.
For the UART the #USE RS232 will accept UART1 or UART2 to select the hardware UART.
For I2C the #USE I2C will accept I2C1 or I2C2 to select the hardware SPI.

The #WORD directive was added and it works like #BYTE except
two bytes are allocated for created variables.

A new built in function ADC_DONE() has been added.  This function
returns FALSE if the A/D converter is currently doing a conversion.

Users that have old code with expresions of the form:
            *(&data + i)
need to change them to:
            *((int8 *)(&data) + i) 
A compiler change was made to be ANSI compliant. 


Use OPTIONS > EDITOR OPTIONS > DISPLAY > CLASSIC MENUS
to replace the ribbons with traditional menus in the IDE.

#ROM has a new option to put a checksum value in ROM.  A value will be put into
this location such that the sum of all ROM locations equals 0x1248.
    #rom getenv("Program_memory")-1 = CHECKSUM

The PORT_x_PULLUPS function allows for a second parametter for devices that have
a pulldown option.  The default value is 0.  Example:
         port_b_pullups( 0xF0, 0x0F );  // Bits 4-7 are pullup and Bits 0-3 are pulldown

#USE I2C has a new option MASK=value where value is a mask that can be applied to
the hardware I2C address match.  This is only available on some chips (like 18FxxK20).

A new function SET_SLOW_SLEW_x(value) is available for chips with a slew select option.
For example:
           set_slow_slew_b(TRUE);


The CCS compilers traditionaly used the C keyword CONST to locate data in program memory.
ANSI uses this keyword to specify a read only data item.  The version 4 compiler has a
new method of storing data in program memory for chips that allow program memory to be read.
These three storage modes may be set directly using one of the following qualifiers:
        ROMI  Traditional CCS method to store in ROM optimized for access from indexed arrays
	ROM   New method to save data in ROM optimized for when the user needs pointers to the data
        _READONLY  Marks the data item as read only, defaults to RAM (like ANSI wants)

By default the CONST keyword is the same as ROMI.  The CONST behavior may be changed with
any of the following:
          #device  CONST=READ_ONLY
          #device  CONST=ROM
          #device  CONST=ROMI

Examples:
         char rom commands[] = {"put|get|status|shutdown"};
         int32 add32( long _readonly * a; long _readonly * b );
         char const hex[16] = "0123456789ABCDEF";
         char rom * list[] = {"red","green","blue"};  






-------------------------------------------------------------------------------------------
#device compilation mode differences:

Default is CCS4:
   Pointer size is set to *=16 if the part has RAM over 0FF.
     
Mode ANSI:
   Default data type is SIGNED all other modes default is UNSIGNED
   Compilation is case sensitive, all other modes are case insensitive
   Pointer size is set to *=16 if the part has RAM over 0FF.

Mode CCS2 and CCS3:
   var16 = NegConst8 is compiled as: var16 = NegConst8 & 0xff (no sign extension)
   Pointer size is set to *=8 for PCM and PCH and *=5 for PCB.
   The overload keyword is required.
   The ROM keyword is _ROM for these versions.
   The WDT/NOWDT/NOLVP fuses are not automatically set by the compiler for these versions.

Mode CCS2:
     onebit = eightbits is compiled as onebit = (eightbits != 0)    
     all other modes compile as: onebit = (eightbits & 1)    

     The default #device ADC= is set to the resolution of the part, all other
     modes default to 8.

