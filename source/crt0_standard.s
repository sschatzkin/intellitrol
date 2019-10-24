;;
;; C Run-time startup module for dsPIC30 C compiler.
;; (c) Copyright 2009 Microchip Technology, All rights reserved
;;
;; Primary version, with data initialization support.
;; The linker loads this version when the --data-init
;; option is selected.
;;
;; Standard 16-bit support, for use with devices that do not support
;; Extended Data Space
;;
;; See file crt1.s for the alternate version without
;; data initialization support.
;;
;; Entry __reset takes control at device reset and
;; performs the following:
;;
;;  1. initialize stack and stack limit register
;;  2. initialize PSV window if __const_length > 0
;;  3. process the data initialization template
;;  4. call the .user_init section, if it exists
;;  5. call the user's _main entry point
;;
;; Assigned to section .init, which may be allocated
;; at a specific address in linker scripts.
;;
;; If a local copy of this file is customized, be sure
;; to choose a file name other than crt0.s or crt1.s.
;;
        .section .init,code

        .global __resetPRI
        .global __rtn_test
        .global __KEN_CODE
        .ifdef __C30ELF
        .type   __resetPRI,@function
        .endif
__resetPRI:
        .weak  __reset
        .ifdef __C30ELF
        .type   __reset,@function
        .endif
__reset:
				mov	_LATD,w1
				mov	#0x3000,w0             ; Set the service led control bit and Truck Here bit high
				xor	w1,w0,w0
				mov	w0,_LATD

				mov	_TRISD,w1	             ; Set the service led control bit and Truck Here bit to output
				mov	~#0x3000,w0
				mov	w0,_TRISD

#ifdef __DEBUG
				clr		W0
				mov		W0, _error_address			; Save the bad address
				mov		W0, _error_good_data    		; Save the Good data pattern
				mov		W0, _error_bad_data    		; Save the failed data pattern
				bra	__rtn_test
#else
				goto _memory_test		; Perform a March memory test
#endif
__rtn_test:

;;
;; Initialize stack, PSV, and data
;;
;; registers used:  w0
;;
;; Inputs (defined by user or linker):
;;  __SP_init
;;  __SPLIM_init
;;
;; Outputs:
;;  (does not return - resets the processor)
;;
;; Calls:
;;  __psv_init1
;;  __data_init
;;  _main
;;

        .weak    __user_init, __has_user_init

        mov      #__SP_init,w15    ; initialize w15
        mov      #__SPLIM_init,w0  ;
        mov      w0,_SPLIM         ; initialize SPLIM
        nop                        ; wait 1 cycle

__KEN_CODE:
				mov	_LATD,w1
				mov	#0x2000,w0
				xor	w1,w0,w0
				mov	w0,_LATD

				rcall    __psv_init1       ; initialize PSV
        mov      #__dinit_tbloffset,w0 ; w0,w1 = template
        mov      #__dinit_tblpage,w1   ;
        rcall    __data_init_standard  ; initialize data

        mov      #__has_user_init,w0
        cp0      w0                ; user init functions?
        bra      eq,1f             ; br if not
        call     __user_init       ; else call them
				mov	_LATD,w1
				mov	#0x2000,w0
				xor	w1,w0,w0
				mov	w0,_LATD
1:
        call  _main                ; call user's main()

        .pword 0xDA4000            ; halt the simulator
        reset                      ; reset the processor


        .global __psv_init1
__psv_init1:
;;
;; Initialize PSV window if _constlen > 0
;;
;; Registers used:  w0
;;
;; Inputs (defined by linker):
;;  __const_length
;;  __const_psvpage
;;
;; Outputs:
;;  (none)
;;
	.equiv   PSV, 0x0002

        bclr     _CORCON,#PSV        ; disable PSV (default)
        mov      #__const_length,w0  ;
        cp0      w0                  ; test length of constants
        bra      z,1f                ; br if zero

        mov      #__const_psvpage,w0 ;
        mov      w0,_PSVPAG          ; PSVPAG = psvpage(constants)
        bset     _CORCON,#PSV        ; enable PSV

1:      return                       ;  and exit


.include "../source/null_signature.s"

