
var:	.fill	0b0000000000111111
&rout:	.fill	rout

	addi	r1, r0, 1
	addi	r2, r0, 2

	lw	r3, r0, &rout
	jalr	r6, r3

	addi	r4, r0, 3
	beq	r0, r0, end

rout:	addi	r5, r0, 37
	jalr	r0, r6

end:	add	r0, r0, r0
