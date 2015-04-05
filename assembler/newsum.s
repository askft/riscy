# sum.s
# Computes sum{2*k, k=1..10}

result:	.fill	0b0000000000000000	# Store the result here
bigneg:	.fill	0b1000000000000000	# test of big negative number
smlneg:	.fill	0b1111111111111100	# test of small negative number

	addi	r1, r0, 0000000		# r1 = 0	( counter )
	addi	r2, r0, 0001010		# r2 = 10	( max     )
	addi	r3, r0, 0000000		# r3 = 0	( sum     )
	addi	r4, r0, 0000000		# r4 = 0	( term    )
	addi	r5, r0, 1100000		# r5 = some negative number, test

loop:	add	r4, r1, r1		# term = counter * 2
	add	r3, r3, r4		# sum += term
	beq	r1, r2, end		# End loop if counter == 10
	addi	r1, r1, 0000001		# counter += 1
	beq	r0, r0, loop		# Unconditional branch to loop

end:	sw	r3, r0, result		# Store sum in result
