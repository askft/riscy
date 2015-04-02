# test.s
	# Indented comment
# Comment with trailing spaces       
	# Indented comment with two trailing tabs		

# Label on its own
var:	.fill	1010101010101010	# Just a variable

arr:	.fill	0000000000000000	# An array (which is just a sequence of
	.fill	0000000000000001	# variables, of course)
	.fill	0000000000000010
	.fill	0000000000000011
	.fill	0000000000000100

loop:
	add	r1, r4, r4
	addi	r4, r0, 0111111
	nand	r2, r4, r4		# commented line w/ trailing spaces   
	sw	r1, r3, 1100110		# same but with a trailing tab	
	lw	r1, r3, 1100101
# Label on the same line as an instruction
asd:	lui	r5, 1010101010
	beq	r1, r2, 0000000#comment directly after instruction
	jalr	r6, r0
	beq	r1, r1, loop

