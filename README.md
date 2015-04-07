# The Riscy Assembler and Virtual Machine

This is a simple albeit functioning implementation of an assembler and a VM for
the [RiSC-16](http://www.eng.umd.edu/~blj/RiSC/) instruction set.


### The assembler

The assembler can assemble code such as this:

```
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
```

into "machine code" such as this:

```
0x0001
0x0000
0x000a
0x2400
0x280a
0x2c00
0x3000
0x1081
0x0d84
0xc502
0x2481
0xc07b
0x8c01
```


### The interpreter / virtual machine

The virtual machine can read the "machine code" produced by the assembler,
interpret it, and print registers and memory contents.


### Motivation

Before starting this project, I had no idea whatsoever how to implement an
assembler nor a virtual machine. I'm not saying my implementations are idiomatic
nor optimal, but this has been nothing more than a learning project.

