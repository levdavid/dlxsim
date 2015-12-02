.global _exit
.global _open
.global _close
.global _read
.global _write
.global _printf

	.align 8
LC0:
	.double 2.00000000000000000000
	.align 4
.global _average
_average:
	;; Save the old frame pointer 
	sw -4(r14),r30
	;; Save the return address 
	sw -8(r14),r31
	;; Establish new frame pointer 
	add r30,r0,r14
	;; Adjust Stack Pointer 
	add r14,r14,#-56
	;; Save Registers 
	sw 0(r14),r3
	sd 4(r14),f4
	sd 12(r14),f6
	sf 20(r14),f8
	lf f4,0(r30)
	lf f5,4(r30)
	addf f4,f4,f5
	cvtf2d f4,f4
	lhi r3,(LC0>>16)&0xffff
	addui r3,r3,(LC0&0xffff)
	ld f6,0(r3)
	nop
	divd f4,f4,f6
	cvtd2f f8,f4
	movfp2i r1, f8
	j L1
	;; nop
L1:
	;; Restore the saved registers
	lw r3,-32(r30)
	nop
	ld f4,-28(r30)
	nop
	ld f6,-20(r30)
	nop
	lf f8,-12(r30)
	nop
	;; Restore return address
	lw r31,-8(r30)
	nop
	;; Restore stack pointer
	add r14,r0,r30
	;; Restore frame pointer
	lw r30,-4(r30)
	nop
	;; Return
	jr r31
	;;nop
	.align 4
LC1:
	.float 3.500000000000
	.align 4
LC2:
	.float 5.000000000000
LC3:
	.ascii "Average is %f\12\0"
	.align 4
.global _main
_main:
	;; Initialize Stack Pointer
	add r14,r0,r0
	lhi r14, ((memSize-4)>>16)&0xffff
	addui r14, r14, ((memSize-4)&0xffff)
	;; Save the old frame pointer 
	sw -4(r14),r30
	;; Save the return address 
	sw -8(r14),r31
	;; Establish new frame pointer 
	add r30,r0,r14
	;; Adjust Stack Pointer 
	add r14,r14,#-40
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sd 8(r14),f4
	sub r14,r14,#8
	lhi r3,(LC1>>16)&0xffff
	addui r3,r3,(LC1&0xffff)
	lw r4,0(r3)
	sw 0(r14),r4
	lhi r3,(LC2>>16)&0xffff
	addui r3,r3,(LC2&0xffff)
	lw r4,0(r3)
	sw 4(r14),r4
	jal _average
	;; nop
	sw -12(r30),r1
	sub r14,r14,#8
	lhi r4,(LC3>>16)&0xffff
	addui r4,r4,(LC3&0xffff)
	sw 0(r14),r4
	lf f4,-12(r30)
	cvtf2d f4,f4
	sd 4(r14),f4
	jal _printf
	;;nop
	add r14,r14,#16
L2:
	;; Restore the saved registers
	lw r3,-32(r30)
	nop
	lw r4,-28(r30)
	nop
	ld f4,-24(r30)
	nop
	;; Restore return address
	lw r31,-8(r30)
	nop
	;; Restore stack pointer
	add r14,r0,r30
	;; Restore frame pointer
	lw r30,-4(r30)
	nop
	;; HALT
	jal _exit
	;; nop ;; here

_exit:
	trap #0
	jr r31
	;;nop
_open:
	trap #1
	jr r31
	;;nop
_close:
	trap #2
	jr r31
	;;nop
_read:
	trap #3
	jr r31
;;	nop
_write:
	trap #4
	jr r31
	;;nop
_printf:
	trap #5
	jr r31
	;;nop
