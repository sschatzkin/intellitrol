;*****************************************************************************
; *
; *   Project:        Rack Controller
; *
; *   Module:         memory.s
; *
; *   Revision:       REV 1.0
; *
; *   Author:         Ken Langlais
; *
; *                   @Copyright 2010  Scully Signal Company
; *
; *  Description:    Assembly language routines used by the Intellitrol SPI Loader
; *
; *   Revision History:
; *
; *    Rev 1.0 K Langlais  02/17/2010 - Initial
; * Registers used:
; * W2 - address of the location under test
; * W3 - last memory address
; * W4 - contains the address of the test table data
; * W5 - test data
; ****************************************************************************/

.include "p24Hxxxx.inc"

	; dsPIC registers
	.equ CORCONL, CORCON
	.equ PSV,2
	.section .const,psv
	.align	2

	.global memory_test_table
memory_test_table:
	.word	0x00FF
	.word	0x0F0F
	.word	0x3333
	.word	0x5555
	.word	0

	.section	.text,code
	.align	2
	.global _memory_test

_memory_test:

				; Clear out error packet
				clr		W0
				mov		W0, _error_address			; Save the bad address
				mov		W0, _error_good_data    		; Save the Good data pattern
				mov		W0, _error_bad_data    		; Save the failed data pattern
				; W2 Contains the beginning of memory under test

				clr		W1							; Loop count - There are 5 test patterns
				mov	#psvpage(memory_test_table), w0
				mov 	w0, PSVPAG
				; enable Program Space Visibility
				bset.b CORCONL, #PSV
				; make a pointer to 'hello'
				mov 	#psvoffset(memory_test_table), w4 ; W4 contains the test table pointer

tst_loop:
				mov 	[W4++], W5					; The test current test data is in W5
				mov		#0x800, W2					; Start of memory	W2 is the address of the location under test
				mov		#0x47FE, W3  				; W3 contains the last memory address

				; Write test data through memory
bkgnd:
				mov   W5, [W2++]					; Write test data into memory
				cp 		W2, W3
				bra		le, bkgnd					; Leave loop when end of memory has been reached

;				nop
;				nop
;				nop

				; March up memory with current data pattern
				mov		#0x800, W2					; Start of memory	W2 is the address of the location under test
				mov		#0x47FE, W3   				; W3 contains the last memory address
forward:
				mov 	[W2], W6    						; fetch contents of location under test
				cp		W5, W6	  							; Compare good data with read data
				bra	 	nz, mem_err   					; Branch if subtraction is not zero (error)
				com		W5, [W2++]    					; Write complement data into the test location and bump the address
				cp		W2, W3
				bra		le, forward							; Leave loop when end of memory has been reached

;				nop
;				nop
;				nop

				; March down memory with complemented data
				mov		#0x800, W3						; Start of memory	W2 is the address of the location under test
				mov		#0x47FE, W2   					; W3 contains the last memory address
				com		W5, W5
back:
				mov 	[W2], W6    						; fetch contents of location under test
				cp		W5, W6	  						; Compare good data with read data
				bra	 	nz, mem_err  					; Branch if subtraction is not zero (error)
				com		W5, [W2--]   				; Write it into the test location and bump the address
				cp		W2, W3
				bra		ge, back							; Leave loop when end of memory has been reached

;				nop
;				nop
;				nop
				inc		W1, W1
				cp		W1, #5								; When used all test patterns leave
				bra		lt, tst_loop
				goto	__rtn_test

mem_err:
				mov		W2, _error_address			; Save the bad address
				mov		W5, _error_good_data    ; Save the Good data pattern
				mov		W6, _error_bad_data     ; Save the failed data pattern

				goto	__rtn_test

;***************************************************************
.end
