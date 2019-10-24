/****************************************************************************
 *
 *   File:              L_spimpol.h
 *
 *   Revision:       REV 1.5
 *
 *   Author:         Ken Langlais; edits by Dave Paquette
 *
 *                   @Copyright 2009, 2014  Scully Signal Company
 *
 *       Description:
 *           CONSTANTS, CODES AND ADDRESSES FOR ALL HARDWARE REGISTERS
 *           Prototype definitions for Loader specific functions.
 *
 * Revision History:
 *   Rev      Date        Who   Description of Change Made
 *  -------  --------  ----    --------------------------------------------
 *  1.5.28  06/13/14  DHP  Included file spimpol.h and deleted everything that was duplicated.
 *
****************************************************************************/
#include "spi_mpol.h"
#ifndef L_SPIMPOL_H
#define L_SPIMPOL_H

#define  L_mSPIMPolGet() SPIBUF

void __attribute__((__section__(".LoaderSection"))) L_SPIMPolInit();

unsigned __attribute__((__section__(".LoaderSection"))) L_SPIMPolPut(unsigned Data);

unsigned __attribute__((__section__(".LoaderSection")))  L_SPIMPolIsTransmitOver(void);

#endif                    /* L_SPIMPOL_H */
