# sum.s
# Computes sum{2*k, k=1..10}

result:	.fill	0			# Store the result here

	addi	r1, r0, 0		# r1 = 0	( counter )
	addi	r2, r0, 10		# r2 = 10	( max     )
	addi	r3, r0, 0		# r3 = 0	( sum     )
	addi	r4, r0, 0		# r4 = 0	( term    )

loop:	add	r4, r1, r1		# term = counter * 2
	add	r3, r3, r4		# sum += term
	beq	r1, r2, end		# End loop if counter == 10
	addi	r1, r1, 1		# counter += 1
	beq	r0, r0, loop		# Unconditional branch to loop

end:	sw	r3, r0, result		# Store sum in result

