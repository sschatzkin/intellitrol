/*******************************************************************************
* Project:     Scully Signal Company (MicroChip based) Intellitrol
* Author:      Dave Paquette
* Filename:    lint.c
* Description: Prototypes for compiler builtins to satisfy lint
* History:     Created November 2009
*******************************************************************************/
#define       __PIC24HJ256GP210__
#define       __C30__       

int           __builtin_return_address (const int level);
unsigned int  __builtin_tblpage(const void *p);
unsigned int  __builtin_tblrdl(unsigned int offset);
void          __builtin_tblwtl(unsigned int offset, unsigned int data);
void          __builtin_write_NVM(void);
void          __builtin_write_OSCCONH(unsigned char value);
void          __builtin_write_OSCCONL(unsigned char value);

unsigned long __builtin_tbladdress(const void *p);
unsigned int  __builtin_tbloffset(const void *p);
void          __builtin_nop(void);
