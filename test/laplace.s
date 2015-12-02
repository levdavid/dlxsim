.global _exit
.global _open
.global _close
.global _read
.global _write
.global _printf

	.align 4
.global _fill_grid
_fill_grid:
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
	sw 8(r14),r5
	nop
	sw -12(r30),r0
L2:
	lw r3,-12(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L3
	nop
	sw -20(r30),r0
L5:
	lw r3,-20(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L6
	nop
	lw r3,-20(r30)
	addi r4,r0,#0
		;cmpsi	r3,r4
	sne	r1,r3,r4
	bnez	r1,L8
	nop
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	add r5,r0,#5000
	sh 0(r3),r5
	j L9
	nop
L8:
	lw r3,-12(r30)
	addi r4,r0,#0
		;cmpsi	r3,r4
	sne	r1,r3,r4
	bnez	r1,L10
	nop
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	add r5,r0,#3000
	sh 0(r3),r5
	j L11
	nop
L10:
	lw r3,-12(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sne	r1,r3,r4
	bnez	r1,L12
	nop
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	add r5,r0,#6000
	sh 0(r3),r5
	j L13
	nop
L12:
	lw r3,-20(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sne	r1,r3,r4
	bnez	r1,L14
	nop
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	add r5,r0,#10000
	sh 0(r3),r5
	j L15
	nop
L14:
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	sh 0(r3),r0
L15:
L13:
L11:
L9:
L7:
	lw r3,-20(r30)
	lw r3,-20(r30)
	add r3,r3,#1
	sw -20(r30),r3
	j L5
	nop
L6:
L4:
	lw r3,-12(r30)
	lw r3,-12(r30)
	add r3,r3,#1
	sw -12(r30),r3
	j L2
	nop
L3:
L1:
	;; Restore the saved registers
	lw r3,-40(r30)
	nop
	lw r4,-36(r30)
	nop
	lw r5,-32(r30)
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
LC0:
	.ascii "%hd\11\0"
LC1:
	.ascii "\12\0"
	.align 4
.global _dump_array
_dump_array:
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
	sw 8(r14),r5
	nop
	sw -20(r30),r0
L17:
	lw r3,-20(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L18
	nop
	sw -12(r30),r0
L20:
	lw r3,-12(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L21
	nop
	sub r14,r14,#8
	lhi r5,(LC0>>16)&0xffff
	addui r5,r5,(LC0&0xffff)
	sw 0(r14),r5
	lw r3,-12(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-20(r30)
	slli r4,r4,#1
	add r3,r3,r4
	lh r3,0(r3)
	sw 4(r14),r3
	jal _printf
	nop
	add r14,r14,#8
	lw r3,-12(r30)
	addi r4,r0,#127
		;cmpsi	r3,r4
	sne	r1,r3,r4
	bnez	r1,L23
	nop
	sub r14,r14,#8
	lhi r5,(LC1>>16)&0xffff
	addui r5,r5,(LC1&0xffff)
	sw 0(r14),r5
	jal _printf
	nop
	add r14,r14,#8
L23:
L22:
	lw r3,-12(r30)
	lw r3,-12(r30)
	add r3,r3,#1
	sw -12(r30),r3
	j L20
	nop
L21:
L19:
	lw r3,-20(r30)
	lw r3,-20(r30)
	add r3,r3,#1
	sw -20(r30),r3
	j L17
	nop
L18:
L16:
	;; Restore the saved registers
	lw r3,-40(r30)
	nop
	lw r4,-36(r30)
	nop
	lw r5,-32(r30)
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
	add r14,r14,#-56
	;; Save Registers 
	sw 0(r14),r3
	sw 4(r14),r4
	sw 8(r14),r5
	sw 12(r14),r6
	sw 16(r14),r7
	sw 20(r14),r8
	jal _fill_grid
	nop
	jal _dump_array
	nop
	sub r14,r14,#8
	lhi r8,(LC1>>16)&0xffff
	addui r8,r8,(LC1&0xffff)
	sw 0(r14),r8
	jal _printf
	nop
	sw -12(r30),r0
	add r14,r14,#8
L25:
	lw r3,-12(r30)
	addi r4,r0,#99
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L26
	nop
	addi r8,r0,#1
	sw -20(r30),r8
L28:
	lw r3,-20(r30)
	addi r4,r0,#126
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L29
	nop
	addi r8,r0,#1
	sw -28(r30),r8
L31:
	lw r3,-28(r30)
	addi r4,r0,#126
		;cmpsi	r3,r4
	sgt	r1,r3,r4
	bnez	r1,L32
	nop
	lw r3,-20(r30)
	lhi r4,(_GRID>>16)&0xffff
	addui r4,r4,(_GRID&0xffff)
	slli r3,r3,#8
	add r3,r4,r3
	lw r4,-28(r30)
	slli r4,r4,#1
	add r3,r3,r4
	lhi r4,(_GRID-256>>16)&0xffff
	addui r4,r4,(_GRID-256&0xffff)
	lw r5,-20(r30)
	slli r5,r5,#8
	add r4,r4,r5
	lw r5,-28(r30)
	slli r5,r5,#1
	add r4,r4,r5
	lh r4,0(r4)
	lhi r5,(_GRID+256>>16)&0xffff
	addui r5,r5,(_GRID+256&0xffff)
	lw r6,-20(r30)
	slli r6,r6,#8
	add r5,r5,r6
	lw r6,-28(r30)
	slli r6,r6,#1
	add r5,r5,r6
	lh r5,0(r5)
	add r4,r4,r5
	lw r5,-20(r30)
	lhi r6,(_GRID>>16)&0xffff
	addui r6,r6,(_GRID&0xffff)
	slli r5,r5,#8
	add r5,r6,r5
	addi r6,r0,#-2
	lw r7,-28(r30)
	slli r7,r7,#1
	add r5,r7,r5
	add r5,r5,r6
	lh r5,0(r5)
	add r4,r4,r5
	lw r5,-20(r30)
	lhi r6,(_GRID>>16)&0xffff
	addui r6,r6,(_GRID&0xffff)
	slli r5,r5,#8
	add r5,r6,r5
	addi r6,r0,#2
	lw r7,-28(r30)
	slli r7,r7,#1
	add r5,r7,r5
	add r5,r5,r6
	lh r5,0(r5)
	add r4,r4,r5
	add r4,r0,r4
	addi r5,r0,#0
		;cmpsi	r4,r5
	sge	r1,r4,r5
	bnez	r1,L34
	nop
	add r4,r4,#3
L34:
	srai r4,r4,#2
	sh 0(r3),r4
L33:
	lw r3,-28(r30)
	lw r3,-28(r30)
	add r3,r3,#1
	sw -28(r30),r3
	j L31
	nop
L32:
L30:
	lw r3,-20(r30)
	lw r3,-20(r30)
	add r3,r3,#1
	sw -20(r30),r3
	j L28
	nop
L29:
	jal _dump_array
	nop
	sub r14,r14,#8
	lhi r8,(LC1>>16)&0xffff
	addui r8,r8,(LC1&0xffff)
	sw 0(r14),r8
	jal _printf
	nop
	add r14,r14,#8
L27:
	lw r3,-12(r30)
	lw r3,-12(r30)
	add r3,r3,#1
	sw -12(r30),r3
	j L25
	nop
L26:
L24:
	;; Restore the saved registers
	lw r3,-56(r30)
	nop
	lw r4,-52(r30)
	nop
	lw r5,-48(r30)
	nop
	lw r6,-44(r30)
	nop
	lw r7,-40(r30)
	nop
	lw r8,-36(r30)
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
.global _GRID
_GRID:	.space 32768
