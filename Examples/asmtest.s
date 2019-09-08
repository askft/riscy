

--------------------------------------------------------------------------------

mask:	.fill	0000000000111111
imm:	.fill	1010101010101010
result:	.fill	0000000000000000

	lw	r1, r0, imm
	lw	r2, r0, mask
	and	r1, r1, r2
	sw	r3, r0, result

<EQUALS>

	movi	r1, 
	lli	r1, 1111111
# ==
	lw	r1, r0, imm			# arg 1
	lw	r2, r0, mask			# arg 2
	lw	r3, r0, and			# r3 = address of AND routine
	jalr	r6, r3				# call AND routine
						# r3 == imm & 0x3f
	addi	r1, r1, r3

--------------------------------------------------------------------------------

#===============================================================================
#	MOVI
#===============================================================================

x003f:	.fill	0000000000111111

movi:
	lui	r1, rout			# r1 = &rout & 0xffc0
	lw	r2, r0, x003f			# r2 = 0x003f

	# ------------------------------------- # r3 = &rout | 0x003f
	addi	sp, sp, 1111110			# Make room for 2 on stack
	sw	r1, sp, 0000000			# Save r1
	sw	r2, sp, 0000001			# Save r2
	nand	r3, r1, r2			# ----\
	nand	r1, r1, r2			# AND  > r3 = r1 & r2
	nand	r3, r3, r1			# ----/
	lw	r1, sp, 0000000			# Restore r1
	lw	r2, sp, 0000001			# Restore r2
	addi	sp, sp, 0000010			# Remove room for 2 on stack

	addi	r1, r1, r3			# r1 |= r3 (== &rout | 0x003f)
						# OR the bottom 6 bits of &rout
						# into r1

	jalr	r6, r1				# Call rout
	beq	r0, r0, end:			# Quit program

rout:	addi	r5, r0, 0100101			# r5 = 37
	jalr	r0, r6				# Return

end:	add	r0, r0, r0			# nop

#===============================================================================


rout:	<instructions>
	<instructions>
	<instructions>

## new
	movi	r1, 
	lli	r1, 1111111
# ==
	lw	r1, r0, imm			# arg 1
	lw	r2, r0, mask			# arg 2
	lw	r3, r0, and			# r3 = address of AND routine
	jalr	r6, r3				# call AND routine
						# r3 == imm & 0x3f

--------------------------------------------------------------------------------
	Example usage
--------------------------------------------------------------------------------

################################################################################
#	and (r1, r2) -> r3
#	- r3 = r1 & r2
#	- r3 is not preserved
################################################################################

and:	addi	r7, r7, 1111110			# Stack pointer -= 2
	sw	r1, sp, 0000000			# Save r1
	sw	r1, sp, 0000001			# Save r2
	nand	r3, r1, r2			# r3 = ~(r1 & r2)
	nand	r1, r1, r2			# r4 = ~(r1 & r2)
	nand	r3, r3, r1			# r5 = ~(r3 & r4)
	lw	r1, sp, 0000000			# Restore r1
	lw	r2, sp, 0000001			# Restore r2
	addi	r7, r7, 0000011			# Stack pointer += 2

	jalr	r0, r6

################################################################################







