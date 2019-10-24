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

.include "..\inc\CpuTest.inc"

 .macro check_register check_reg, value
 mov #\value, \check_reg
 mov #\value, w0
 cp \check_reg, w0
 bra nz, Error
 .endm

 .macro check_sfr sfr, value
 mov #\value, w0
 mov w0, \sfr
 cp \sfr
 bra nz, Error
 .endm

 .macro check_sfr_bytewise sfr,value
 mov #\value,w0
 mov w0,\sfr
 cp.b \sfr
 bra nz,Error
 .endm


 .text


.global _CPU_RegisterTest
_CPU_RegisterTest:

   ; no need to save the values of w0->w7, the ABI does not require it
   push.d w8
   push.d w10
   push.d w12
   push   w14
   PUSH TBLPAG
   PUSH PSVPAG
   PUSH RCOUNT
   PUSH CORCON
   PUSH SR

;*********************************
; Test WREG0 && WREG1 registers - nearly identical to macro
   MOV #0xAAAA,W0
   MOV #0xAAAA,W1
   CP W0, W1
   BRA NZ, Error

   MOV #0x5555,W0
   MOV #0x5555,W1
   CP W0, W1
   mov #CPU_REGISTER_TEST_FAIL, w1
   BRA NZ, Error
;*********************************
; Test WREG2 register - use macro
   check_register w2 0xAAAA
   check_register w2 0x5555

;*********************************
; Test WREG3 register - use macro
   check_register w3 0xAAAA
   check_register w3 0x5555

;*********************************
; Test WREG4 register - use macro
   check_register w4 0xAAAA
   check_register w4 0x5555

;*********************************
; Test WREG5 register - use macro
   check_register w5 0xAAAA
   check_register w5 0x5555

;*********************************
; Test WREG6 register - use macro
   check_register w6 0xAAAA
   check_register w6 0x5555

;*********************************
; Test WREG7 register - use macro
   check_register w7 0xAAAA
   check_register w7 0x5555

;*********************************
; Test WREG8 register - use macro
   check_register w8 0xAAAA
   check_register w8 0x5555

;*********************************
; Test WREG9 register - use macro
   check_register w9 0xAAAA
   check_register w9 0x5555

;*********************************
; Test WREG10 register - use macro
   check_register w10 0xAAAA
   check_register w10 0x5555

;*********************************
; Test WREG11 register - use macro
   check_register w11 0xAAAA
   check_register w11 0x5555

;*********************************
; Test WREG12 register - use macro
   check_register w12 0xAAAA
   check_register w12 0x5555

;*********************************
; Test WREG13 register - use macro
   check_register w13 0xAAAA
   check_register w13 0x5555

;*********************************
; Test WREG14 register - use macro
   check_register w14 0xAAAA
   check_register w14 0x5555

;*********************************
; Test TBLPAG register
   check_sfr_bytewise TBLPAG 0xAA
   check_sfr_bytewise TBLPAG 0x55

;*********************************
; Test PSVPAG register
   check_sfr_bytewise PSVPAG 0xAA
   check_sfr_bytewise PSVPAG 0x55

;*********************************
; Test RCOUNT register
   check_sfr RCOUNT 0x2AAA
   check_sfr RCOUNT 0x1555


;*********************************
; Test W15
   mov #0xE0,w0
   ior SR                           ; disable interrupts while stack invalid
                                    ; re-enabled for free at POP SR
   mov w15,w2                       ; save stack pointer
   MOV #0xAAAA,w15
   MOV #0xAAAA,W0
   CP  w15,W0
   mov w2,w15
   BRA NZ,Error

   MOV #0X5554,w15
   MOV #0x5554,W0
   CP  w15,W0
   mov w2,w15                       ; restore stack pointer
   BRA NZ,Error

   MOV #CPU_REGISTER_TEST_PASS,W1

Error:

   POP SR
   POP CORCON
   POP RCOUNT
   POP PSVPAG
   POP TBLPAG
   pop w14
   pop.d w12
   pop.d w10
   pop.d w8

   mov w1,w0

   return

;.endif


.end
