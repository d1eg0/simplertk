.extern _sptemp
.text
	.global __T1Interrupt 
__T1Interrupt: 
	;bset  LATB,#10		; debug timing
	;; save context
	
	push    _SPLIM
	push    _SR
	
	push.d  W0
	push.d  W2
	push.d  W4
	push.d  W6
	push.d  W8
	push.d  W10
	push.d  W12
	push    W14
	
	
	
	push    _RCOUNT
	push    _TBLPAG
	push    _CORCON
	push    _PSVPAG
		
	disi    #2
	disi    #0x3FFF                                 ; disable interrupts
	
	;; services timer 1 interrupts 
	mov W15,_sptemp
	call _scheduler
	bclr  IFS0,#3        ; clear the interrupt flag
	mov #127,W1
	cp w0,w1
	

	bra Z,	__noswctx
	call _dispatch
	mov _sptemp,W15
	
__noswctx:	
	bclr  IFS0,#3        ; clear the interrupt flag

	;; load context
	pop     _PSVPAG
	pop     _CORCON
	pop     _TBLPAG
	pop     _RCOUNT
	
	pop     W14
	pop.d   W12
	pop.d   W10
	pop.d   W8
	pop.d   W6
	pop.d   W4
	pop.d   W2
	pop.d   W0
	
	pop     _SR
	pop     _SPLIM

	disi    #2
	disi    #2
	
	;bclr  LATB,#10			; debug timing
	
	retfie                  ;  and return from interrupt 
.end
