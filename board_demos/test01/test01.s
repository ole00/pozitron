# -------------------------------
# Simple test of assembler
# -------------------------------

.syntax unified
.cpu cortex-m3
.fpu softvfp

# -------------------------------
.global test_asm
.type   test_asm, %function
# -------------------------------
# (unsigned int a, unsigned int b, unsigned int c)

test_asm:
@ r0 = a
@ r1 = b
@ r2 = c
@ result in r0

	add r0, r1
	add r0, r2
	bx lr 

