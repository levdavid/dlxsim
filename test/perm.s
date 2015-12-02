.global _exit
.global _open
.global _close
.global _read
.global _write
.global _printf

	.align 4
.global _Swap
_Swap:
	;; Save the old frame pointer 
	sw -4(r14),r30
	;; Save the return address 
	sw -8(r14),r31
	;; Establish new frame pointer 
	add r30,r0,r14
	;; Adjust Stack Pointer 
	add r14,r14,#-32
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	lw r3,0(r30)
	lw r5,0(r3)
	sw -12(r30),r5
	lw r3,0(r30)
	lw r4,4(r30)
	lw r5,0(r4)
	sw 0(r3),r5
	lw r3,4(r30)
	lw r5,-12(r30)
	sw 0(r3),r5
L1:
	;; Restore the saved registers
	lw r3,-32(r30)
	nop
	lw r4,-28(r30)
	nop
	lw r5,-24(r30)
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
	nop
	.align 4
.global _Initialize
_Initialize:
	;; Save the old frame pointer 
	sw -4(r14),r30
	;; Save the return address 
	sw -8(r14),r31
	;; Establish new frame pointer 
	add r30,r0,r14
	;; Adjust Stack Pointer 
	add r14,r14,#-32
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	nop
	addi r5,r0,#1
	sw -12(r30),r5
L3:
	lw r3,-12(r30)
	addi r4,r0,#7
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L4
	nop
	lw r3,-12(r30)
	lhi r4,(_permarray>>16)&0xffff
	addui r4,r4,(_permarray&0xffff)
	slli r3,r3,#2
	add r3,r4,r3
	lw r4,-12(r30)
	add r4,r4,#-1
	sw 0(r3),r4
L5:
	lw r3,-12(r30)
	lw r3,-12(r30)
	add r3,r3,#1
	sw -12(r30),r3
	j L3
	nop
L4:
L2:
	;; Restore the saved registers
	lw r3,-32(r30)
	nop
	lw r4,-28(r30)
	nop
	lw r5,-24(r30)
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
	nop
	.align 4
.global _Permute
_Permute:
	;; Save the old frame pointer 
	sw -4(r14),r30
	;; Save the return address 
	sw -8(r14),r31
	;; Establish new frame pointer 
	add r30,r0,r14
	;; Adjust Stack Pointer 
	add r14,r14,#-32
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	lhi r3,(_pctr>>16)&0xffff
	addui r3,r3,(_pctr&0xffff)
	lhi r4,(_pctr>>16)&0xffff
	addui r4,r4,(_pctr&0xffff)
	lw r4,0(r4)
	add r4,r4,#1
	sw 0(r3),r4
	lw r3,0(r30)
	addi r4,r0,#1
		;cmpsi	r3,r4
	seq	r1,r3,r4
	bnez	r1,L7
	nop
	sub r14,r14,#8
	lw r3,0(r30)
	add r3,r3,#-1
	sw 0(r14),r3
	jal _Permute
	nop
	lw r3,0(r30)
	add r3,r3,#-1
	sw -12(r30),r3
	add r14,r14,#8
L8:
	lw r3,-12(r30)
	addi r4,r0,#0
		;cmpsi	r3,r4
	sle	r1,r3,r4
	bnez	r1,L9
	nop
	sub r14,r14,#8
	lw r3,0(r30)
	slli r3,r3,#2
	lhi r5,(_permarray>>16)&0xffff
	addui r5,r5,(_permarray&0xffff)
	add r3,r3,r5
	sw 0(r14),r3
	lw r3,-12(r30)
	slli r3,r3,#2
	add r3,r3,r5
	sw 4(r14),r3
	jal _Swap
	nop
	sub r14,r14,#0
	lw r3,0(r30)
	add r3,r3,#-1
	sw 0(r14),r3
	jal _Permute
	nop
	sub r14,r14,#0
	lw r3,0(r30)
	slli r3,r3,#2
	add r3,r3,r5
	sw 0(r14),r3
	lw r3,-12(r30)
	slli r3,r3,#2
	add r3,r3,r5
	sw 4(r14),r3
	jal _Swap
	nop
	add r14,r14,#8
L10:
	lw r3,-12(r30)
	lw r3,-12(r30)
	add r3,r3,#-1
	sw -12(r30),r3
	j L8
	nop
L9:
L7:
L6:
	;; Restore the saved registers
	lw r3,-32(r30)
	nop
	lw r4,-28(r30)
	nop
	lw r5,-24(r30)
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
	nop
	.align 4
.global _Perm
_Perm:
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
	lhi r3,(_pctr>>16)&0xffff
	addui r3,r3,(_pctr&0xffff)
	sw 0(r3),r0
	jal _Initialize
	nop
	sub r14,r14,#8
	addi r4,r0,#7
	sw 0(r14),r4
	jal _Permute
	nop
	add r14,r14,#8
L11:
	;; Restore the saved registers
	lw r3,-24(r30)
	nop
	lw r4,-20(r30)
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
	nop
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
	add r14,r14,#-8
	;; Save Registers 
	jal _Perm
	nop
L12:
	;; Restore the saved registers
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
.global _pctr
_pctr:	.space 8
.global _permarray
_permarray:	.space 32
