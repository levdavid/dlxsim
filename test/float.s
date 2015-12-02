.global _exit
.global _open
.global _close
.global _read
.global _write
.global _printf

	.align 8
LC0:
	.double 2.29999999999999982236
	.align 8
LC1:
	.double 3.50000000000000000000
LC2:
	.ascii "f1 = %f f2 = %f\12\0"
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
	add r14,r14,#-72
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	sd 12(r14),f4
	sd 20(r14),f6
	lhi r3,(LC0>>16)&0xffff
	addui r3,r3,(LC0&0xffff)
	lw r4,0(r3)
	nop
	lw r5,4(r3)
	nop
	sw -16(r30),r4
	sw -12(r30),r5
	lhi r3,(LC1>>16)&0xffff
	addui r3,r3,(LC1&0xffff)
	lw r4,0(r3)
	nop
	lw r5,4(r3)
	nop
	sw -24(r30),r4
	sw -20(r30),r5
	ld f4,-16(r30)
	nop
	ld f6,-24(r30)
	nop
	addd f4,f4,f6
	sd -16(r30),f4
	sub r14,r14,#24
	lhi r4,(LC2>>16)&0xffff
	addui r4,r4,(LC2&0xffff)
	sw 0(r14),r4
	lw r4,-16(r30)
	nop
	lw r5,-12(r30)
	nop
	sw 4(r14),r4
	sw 8(r14),r5
	lw r4,-24(r30)
	nop
	lw r5,-20(r30)
	nop
	sw 12(r14),r4
	sw 16(r14),r5
	jal _printf
	nop
	add r14,r14,#24
L1:
	;; Restore the saved registers
	lw r3,-56(r30)
	nop
	lw r4,-52(r30)
	nop
	lw r5,-48(r30)
	nop
	ld f4,-44(r30)
	nop
	ld f6,-36(r30)
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
	nop

_exit:
	trap #0
	jr r31
	nop
_open:
	trap #1
	jr r31
	nop
_close:
	trap #2
	jr r31
	nop
_read:
	trap #3
	jr r31
	nop
_write:
	trap #4
	jr r31
	nop
_printf:
	trap #5
	jr r31
	nop
