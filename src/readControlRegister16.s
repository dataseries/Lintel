;;; Two changes have been made to this file.
;;; First, the .LEVEL 2.0 directive has been added.
;;; Second, the stw has been changed to a std so that all 64 bits are stored.
;;; 
;;; This code only works on PA2.0 as PA1.1 doesn't have 64 bits, and
;;; moreover, the interval timer is on 32 bits :(
;;; 
;;; original code was (gcc only):
;;; void readControlRegister(unsigned long long *ret) {
;;;   asm volatile ("MFCTL %%cr16,%0" : "=r" (x));
;;;   *ret = x;
;;; }

	.LEVEL 2.0
	.SPACE $PRIVATE$
	.SUBSPA $DATA$,QUAD=1,ALIGN=8,ACCESS=31
	.SUBSPA $BSS$,QUAD=1,ALIGN=8,ACCESS=31,ZERO,SORT=82
	.SPACE $TEXT$
	.SUBSPA $LIT$,QUAD=0,ALIGN=8,ACCESS=44
	.SUBSPA $CODE$,QUAD=0,ALIGN=8,ACCESS=44,CODE_ONLY
	.IMPORT $global$,DATA
	.IMPORT $$dyncall,MILLICODE
; gcc_compiled.:
	.SPACE $TEXT$
	.SUBSPA $CODE$

	.align 4
	.EXPORT readControlRegister16,ENTRY,PRIV_LEV=3,ARGW0=GR
readControlRegister16
	.PROC
	.CALLINFO FRAME=0,NO_CALLS
	.ENTRY
	MFCTL %cr16,%r20
	std %r20,0(%r26)
	bv,n %r0(%r2)
	.EXIT
	.PROCEND
