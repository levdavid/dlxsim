.global _exit
.global _open
.global _close
.global _read
.global _write
.global _printf
.global _a

	.align 4
_a:
	.word 46

LC0:
	.ascii "Value of a is %d\12\0"
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
	add r14,r14,#-24
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	lhi r3,(_a>>16)&0xffff
	addui r3,r3,(_a&0xffff)
	lhi r4,(_a>>16)&0xffff
	addui r4,r4,(_a&0xffff)
	lw r4,0(r4)
	add r4,r4,#1
	sw 0(r3),r4
	sub r14,r14,#8
	lhi r5,(LC0>>16)&0xffff
	addui r5,r5,(LC0&0xffff)
	sw 0(r14),r5
	lhi r3,(_a>>16)&0xffff
	addui r3,r3,(_a&0xffff)
	lw r5,0(r3)
	sw 4(r14),r5
	jal _printf
	nop
	add r14,r14,#8
L1:
	;; Restore the saved registers
	lw r3,-24(r30)
	nop
	lw r4,-20(r30)
	nop
	lw r5,-16(r30)
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
