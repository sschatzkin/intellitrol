;*****************************************************************************
;*
;*   Project:        Rack Controller
;*
;*   Module:         CPURegisterTest.s
;*
;*   Revision:       REV 1.5
;*
;*   Author:         Ken Langlais
;*
;*                   @Copyright 2010  Scully Signal Company
;*
;*  Description:  CPU register test ported from Microchip Class B sample
;*                code
;*
;*   Revision History:
;*
;********************************************************************/

.equ   CPU_REGISTER_TEST_PASS,  1

.equ  CPU_REGISTER_TEST_FAIL,  0

.ifdef __PIC24H__
.include <p24Hxxxx.inc>
.endif


