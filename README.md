# The Riscy Assembler and Virtual Machine

This is a simple implementation of an assembler and a VM for the
[RiSC-16](http://www.eng.umd.edu/~blj/RiSC/) (Ridiculously Simple Computer) 
instruction set.

So far, only the most basic functionality of RiSC-16 has been implemented. 

The assembler currently handles labels, one directive (.fill), eight registers,
 and eight instructions:

| Mnemonic | Long name                    |
|----------|------------------------------|
| add      | add                          |
| addi     | add immediate                |
| nand     | NOT AND (logical expression) |
| lui      | load upper immediate         |
| sw       | store word                   |
| lw       | load word                    |
| beq      | branch if equal              |
| jalr     | jump and link register       |


### The assembler

The assembler can assemble code such as this:

```
# sum.s
# Computes sum{2*k, k=1..10}

result: .fill   0                       # Store the result here

        addi	r1, r0, 0               # r1 = 0        counter
        addi	r2, r0, 10              # r2 = 10       max
        addi	r3, r0, 0               # r3 = 0        sum
        addi	r4, r0, 0               # r4 = 0        term

loop:   add     r4, r1, r1              # term = counter * 2
        add     r3, r3, r4              # sum += term
        beq     r1, r2, end             # End loop if counter == 10
        addi    r1, r1, 1               # counter += 1
        beq     r0, r0, loop            # Unconditional branch to loop

end:    sw      r3, r0, result          # Store sum in result
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

First of all, the virtual machine is *very* simplistic; it is not hardware
descriptive, i.e. not a simulation of an actual RiSC-16 processor. The source
code can be found in [the vm directory](vm) in this repository.

What the virtual machine can do is read the "machine code" produced by the
assembler, interpret it, and print registers and memory contents.


### Usage

Clone or pull the repository, enter the directory and type "make". It will
create two new files, "asm" and "vm". To compile a source file, use the command
`./asm <input> <output>` where <input> is the source file and <output> will be
the binary. There are some examples in the *test* folder.

To run the binary, simply type `./vm <file> [options]` where <file> is the
binary and [options] can be one or more of the following:
 * **--step** – Step through the program instruction by instruction.
 * **--verbose** – Print some more information, namely which instructions were
loaded and executed.

And example usage would look like the following:

```
git clone http://github.com/AlexanderSkafte/riscy.git
cd riscy
make
./asm test/sum.s sum
./vm sum --step --verbose
```

This will compute the sum of all number from 1 to 10, where each term is
multiplied by two. The result will be stored in and printed as Data[0].

Additional debug printouts can be enabled in the file "vm/macros.h", by changing
the `DEBUG` define from 0 to a non-zero value.

As for *writing* riscy assembly programs, I will provide a documentation soon.
Full information can be found on the website linked to in the first section of
the README, but keep in mind that my program does not provide a complete
implementation of the RiSC-16 architecture.


### Motivation

Before starting this project, I had no idea whatsoever how to implement an
assembler nor a virtual machine. I'm not saying my implementations are idiomatic
or optimal; this has been nothing more than a learning project.

