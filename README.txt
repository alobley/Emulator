DATE OF THIS REVISION: 01/06/2024
EMULATOR VERSION: BETA 1
ASSEMBLER VERSION: BETA 2

Required Knowledge: Basic understanding of computer architectures and assembly.

---------- ASSEMBLER DETAILS ----------
The assembler takes any file you put into it, but all outputs are "program.bin". This makes it easier to deal with in terms of the emulator. Just put
the binary file in the same directory as the emulator, make sure it has the right name, and run the emulator.

The assembler is written in Python, which makes it simpler to read and parse instructions.

The assembler is, well, an assembler. There are almost no abstractions over the binary other than ones that would be very difficult to Implement
by hand, such as jumping to other locations without labels.

The assembler supports labels and comments, and all implemented instructions defined in this file can be assembled by the assembler. 

When passing numeric values, due to how Python reads strings, only base 10 is currently supported. Entering binary or hexadecimal will result in errors.

Errors in the assembler will stop assembly, but usually the assembler tells you the line it happened on.

If you are having trouble coding and assembling a program, reference some of the .asm example files provided with this documentation. They contain example
programs.

The assembler does NOT support constants, macros, or variables, but you can reference a memory address specifically using P or LOAD/STOR.

If the assembler encounters an error and you are certain that your program has no errors, report a bug and provide your program.

$ Can be used as the current address in memory and can be an operand. Can be pushed onto the stack.
$$ Can be used as the current bank index and can be an operand. Can be pushed onto the stack.

To call a function and return, you can PUSHI $ and $$, then at the end of the function do POP, then do SOR, B, and then do POP, then JMPR, B.

IMPORTANT: The assembler parses lines by commas. The correct syntax is: [opcode], [operand], [operand] or [opcode], [operand].

---------- COMPUTER DETAILS ----------
RAM - 64.5kb. There are 256 banks of memory, with 256 bytes each.
Banks 251, 252, 253, 254, and 255 are VRAM banks. Write to them for changing what you see on the screen.
1 Core CPU

All instructions are 16 bits (two bytes) wide.

In all mathematical operations, the first numeric value is register A and the output is in register A.

SOI/SOR are still able to be used, but it is recommended to let the assembler take care of second operands.

The emulator is written in C. It has a screen output and keyboard input.

The CPU only supports direct addressing. There is only direct addressing and jumping.

In terms of hardware and software, there is just the CPU and RAM. There is no firmware, no graphics hardware, no BIOS. When programming, it's just you
and the CPU. The keyboard is memory mapped to bank 250 address 254, and VRAM is all banks from 251-255.

If the emulator encounters an error, it will provide you with a classic C error message and stop the program. First check your program for bugs, and if
you can't find any, report a bug and provide me with both the error message and your program.

---------- FUTURE GOALS ----------
- Implement a timer in the emulator for a PIT
- Add syntax highlighting for the assembly code in Visual Studio
- Name the Computer/CPU
- Name the assembly language
- Name the assembler
- Create the CPU in a circuit simulator and run an assembled program on it
- Get a real, physical version of the computer and run an assembled program on it

---------- REGISTERS ----------
---General Registers---
A - Accumulator Register. Result and first value of arithmetic operations.
B - General-Purpose Register. Can be used with the majority of instructions.
C - General-Purpose Register. Can be used with the majority of instructions.
D - General-Purpose Register. Can be used with the majority of instructions.
P - Pointer Register. Points to a memory address stored in this register. The memory bank it points to is determined by the BI register.
BI - Bank Index Register. Points to a memory bank for P to use. Default is 0, but your program will be there.
SP - Stack Pointer. Points to the stack memory bank, which is 256 bytes. Push and pop operations affect this register, but it cannot be directly manipulated.
PC - Program Counter. Points to the current memory address and memory bank that is being executed.

---Instruction Registers---
ROP - Operation register. Holds the instruction that is currently being executed.
DR1 - Data register 1. Holds the first or only operand of an instruction.
DR2 - Data register 2. Holds the second operand of an instruction.

---------- INSTRUCTION SET ----------
---Special Operations---
NOP - No operation.
SOI - Second Operand Immediate. Takes an immediate value and puts it into the second data register. One day it will be abstracted by the assembler.
SOR - Second Operand Register. Takes a register's value and puts it into the second data register. One day it will be abstracted by the assembler.
BSWCHI - Switches the memory bank you want to access by using an immediate value.
BSWCHR - Switches the memory bank you want to access by using the value of a register.

---Mathematical Operations---
ADDI - Add an immediate value.
ADDR - Add a register value.
SUBI - Subtract an immediate value.
SUBR - Subtract a register value.

---Data Manipulation---
LDI - Load an immediate value into a register
CPY - Copy the value of one register to another
MOVMI - Move an immediate value into RAM at the address pointed to by P and the bank pointed to by BI
MOVMR - Move an register value into RAM at the address pointed to by P and the bank pointed to by BI
SHL - Bit shift a register left
SHR - Bit shift a register right

---Branching Instructions---
JMPI - Jump to a given label or memory location. Memory bank is specified as the second operand.
JMPR - Jump to an address given in a register as the first operand, and a bank given as a register value as the second operand
JEI - Jump to a given label or memory location if the result of a cmp operation is equal. Memory bank is specified as the second operand.
JER - Jump to an address given in a register as the first operand, and a bank given as a register value as the second operand
JNEI - Jump to a given label or memory location if the result of a cmp operation is not equal. Memory bank is specified as the second operand.
JNER - Jump to an address given in a register as the first operand, and a bank given as a register value as the second operand if a cmp operation is not equal.
CMPI - Compare the value of a register with the value of a given immediate value. The immediate is the second operand.
CMPR - Compare the values of two given registers.

---Memory I/O Instructions---
LOADI - Load a from memory into a given register. The first operand is the register to load into, the second operand is the address.
LOADR - Load a value from memory pointed to by a register into another register. The first operand is the register to load, the second is the pointer.
STORI - Store a value from a register into a given memory address. The first operand is the register to store, the second is the address
STORR - Store a value from a register into a location in memory pointed to by a register. First operand is the register to store, the second is the pointer.
PUSHI - Push an immediate value onto the stack.
PUSHR - Push a register value onto the stack.
POP - Pop a value from the stack. The popped value will be stored in B.

---Increment/Decrement---
INCB - Increment a byte pointed to by register P.
DECB - Decrement a byte pointed to by register P.
INCR - Increment the value of a register.
DECR - Decrement the value of a register.

---Logic Instructions---
ANDI - Performs an AND operation on a register and an immediate value. Register is operand 1, immediate is operand 2.
ANDR - Performs an AND operation on two register values. Result is stored in operand 1.
ORI - Performs an OR operation on a register and an immediate value. Register is operand 1, immediate is operand 2.
ORR - Performs an OR operation on two register values. Result is stored in operand 1.
XORI - Performs an XOR operation on a register and an immediate value. Register is operand 1, immediate is operand 2.
XORR - Performs an XOR operation on two register values. Result is stored in operand 1.
NOT - Performs a NOT operation on the value of a register.
