/*****************************************************************************
 *
 *   Project:        Rack Controller
 *
 *   Module:         common.h
 *
 *   Revision:       REV 1.0
 *
 *   Author:         Ken Langlais
 *
 *                   @Copyright 2008  Scully Signal Company
 *
 *   Description:    Main program enumerated list for Rack controller main
 *                   microprocessor PIC24HJ256GP210
 *
 *   Revision History:
 *
 *****************************************************************************/
#ifndef COMMON_H
#define COMMON_H

#ifndef __PIC24HJ256GP210__
#define __PIC24HJ256GP210__
#endif

#define FCY     20000000
#define SYSCLK  8000000

#include <p24HJ256GP210.h>
#include <libpic30.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "hardass.h"
#include "stdsym.h"
#include "enum.h"
#include "stsbits.h"
#include "ledlite.h"  /* symbols */
#include "comdat.h"
#include "modbus.h"
#include "proto.h"

#define PRESCALE_256    3
#define PRESCALE_64     2
#define PRESCALE_8      1
#define NO_PRESCALE     0
#define TIMER2_PERIOD   20000             // 1ms
#define TIMER3_PERIOD   20000             // 1ms
#define TIMER4_PERIOD   30000             // 1.5ms
#define TIMER5_PERIOD   1000              // 500us

/* variables used in raw ADC data to hex */
#define SERIAL_BUFFER_SIZE 100   /*  FOR UART ISR BUFFER  */

typedef struct {
  unsigned char reg;
  unsigned char data;
} EXP_DEF;

#endif      /* end of COMMON_H */

