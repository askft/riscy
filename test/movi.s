#===============================================================================
#	MOVI
#===============================================================================

x003f:	.fill	0x003f				# mask lowest 6 bits

movi:	lui	r1, rout			# r1 = &rout & 0xffc0
	lw	r2, r0, x003f			# r2 = 0x003f

	# ---------------------	r3 = &rout | 0x003f
	addi	r7, r7, -2			# Make room for 2 on stack
	sw	r1, r7, 0			# Save r1
	sw	r2, r7, 1			# Save r2
	nand	r3, r1, r2			# ----\
	nand	r1, r1, r2			# AND  > r3 = r1 & r2
	nand	r3, r3, r1			# ----/
	lw	r1, r7, 0			# Restore r1
	lw	r2, r7, 1			# Restore r2
	addi	r7, r7, 2			# Remove room for 2 on stack

	add	r1, r1, r3			# r1 |= r3 (== &rout | 0x003f)
						# OR the bottom 6 bits of &rout
						# into r1

	jalr	r6, r1				# Call rout
	beq	r0, r0, end			# Quit program

rout:	addi	r5, r0, 37			# r5 = 37
	jalr	r0, r6				# Return

end:	add	r0, r0, r0			# nop

#===============================================================================
