# The assembler for my custom CPU!
# Takes an assembly file and translates each line to machine code. Output is always program.bin, but input can be any file.
#
# Abstractions:
# SOI/SOR - There is an instruction for second operands, but the assembler abstracts it.
# Labels - There are labels that you can set which can be jumped to instead of specific memory addresses.
#          Note: if you want to jump to a specific memory address you can, so I should tell you that instructions with 0 or 1 operand are 2 bytes while
#          instructions with 2 operands are 4 bytes. That's because each instruction is 16 bits.
# Register vs. Immediate operations - likely only a few of them and in later versions, but for almost every instruction there is both a register and an
#                                     immediate version. For example, lda and cpy will be separate, but addi and addr will be the same.
#
# To Add:
# - .equ values
# - Checks for overflow errors
# - (Maybe) call/ret
#
# What I won't add:
# - Variables (use P)

import struct

# Set a dictionary to assign each instruction to its mnemonic. All instructions will be converted to uppercase. There are 37 unique instructions.
instructions = {"NOP": 0b00000000, "SOI": 0b10000000, "SOR": 0b10000001, "BSWCHI": 0b00100110, "BSWCHR": 0b00100111, "ADDI": 0b00000010, "ADDR": 0b00000011, 
                "SUBI": 0b00000100, "GETP": 0b00101001, "SUBR": 0b00000101, "LDI": 0b00000110, "CPY": 0b00000111, "MOVMI": 0b00001000, "MOVMR": 0b00001001, 
                "SHL": 0b10001011, "SHR": 0b10001101, "JMPI": 0b00001010, "JMPR": 0b00001011, "JEI": 0b00001100, "JER": 0b00001101, "JNEI": 0b00001110, 
                "JNER": 0b00001111, "CMPI": 0b00010000, "CMPR": 0b00010001, "LOADI": 0b00010010, "LOADR": 0b00010011, "STORI": 0b00010100, 
                "STORR": 0b00010101, "PUSHI": 0b00010110,  "PUSHR": 0b00010111, "POP": 0b00011001, "INCB": 0b00011000, "DECB": 0b00011010, 
                "INCR": 0b00011011, "DECR": 0b00011101, "ANDI": 0b00011110, "ANDR": 0b00011111, "ORI": 0b00100000, "ORR": 0b00100001, "XORI": 0b00100010, 
                "XORR": 0b00100011, "NOT": 0b00100101}

# Set the register IDs (will likely be put as a decimal value in the output text file)
registerIDs = {"A": 0x01, "B": 0x02, "C": 0x03, "D": 0x04, "BI": 0x05, "P": 0x06, "S": 0x07}

labels = {}

# Set the global variables
currentLine = 0
assembledCode = []
inputString = ""
bankOffset = 0
memoryOffset = 0
originalChar = ""

# This function gets the locations of all the labels in the code.
def GetLabels(lines):
    global memoryOffset
    global bankOffset
    for line in lines:
        if line and line[0] == "_" and line[-1] == ":":
            # As long as your line starts with _ and ends with :, it will become a label you can jump to.

            # Remove the : from the line so that it can be jumped to in a similar way to x86 and so that the assembler won't 
            # identify it as a new label when jumping.
            labels[line[:-1]] = [bankOffset, memoryOffset]
        if ";" in line:
            # If there is a comment on the line, remove and ignore it.
            line = line.split(";")[0].strip()
        if line.strip() and not line[0] == "_" and not line[-1] == ":":
            # If the line exists and is not a label, increase the memory offset
            if len(line.split(',')) == 3:
                # If there are two operands, increase the memory offset by 4
                memoryOffset += 4
            elif len(line.split(',')) <= 2:
                # Otherwise, increase it by 2
                memoryOffset += 2
            else:
                # If there are more than two operands, return an error.
                print("Error: too many operands on line: " + currentLine + ". Cannot assemble.")
                exit()
    memoryOffset = 0

# This function assembles the given opcode and operand and turns them into binary code
def AssembleInstruction(instruction, operand):
    global labels
    global currentLine
    global bankOffset
    global memoryOffset

    # Clear leading and trailing whitespace around the instruction and make it uppercase in case a label was detected
    instruction = instruction.upper().strip()

    if instruction in instructions:
        # If the instruction is in the instructions dictionary, add it.
        if memoryOffset >= 255:
            # If we have reached the end of a memory bank, go to the next one
            bankOffset += 1
            memoryOffset = 0


        if str(operand).isnumeric() and int(operand) < 256:
            # If the operand is a number, make it an integer and add it along with its instruction to two bytes
            memoryOffset += 2
            return struct.pack('BB', instructions[instruction], int(operand))
        
        elif "_" in operand:
            # If the operand is a label
            memoryOffset += 2
            # Does the label exist in the labels dictionary?
            try:
                # If so, add it and its opcode to two new bytes as well as its bank offset.
                assembledCode.append(AssembleInstruction("SOI", labels[operand][0]))
                return struct.pack('BB', instructions[instruction], int(labels[operand][1]))
            except KeyError:
                # If not, tell the user and stop assembly.
                print("Label error: There is no label by the name: " + str(operand) + ". Cannot assemble.")
                print("Error is on line: " + str(currentLine))
                print("Did you mistype it?")
                exit()
        elif ((operand[0] == "'" and operand[-1] == "'") or (operand[0] == '''"''' and operand[-1] == '''"''')) and len(operand) < 4:
            # If there is a character as the operand, turn it into its integer ASCII value and pass it as an integer value. Only supports lowercase.
            # Only supports one character. If you try more, it will result in an error.
            operand = ord(operand[1].lower())
            return struct.pack('BB', instructions[instruction], operand)

        # Is the operand a register?
        elif operand in registerIDs:
            memoryOffset += 2
            # If so, assign it to its ID and pack it as a byte with its operand.
            return struct.pack('BB', instructions[instruction], registerIDs[operand])
        
        # Is the operand the current memory offset?
        elif operand == "$":
            memoryOffset += 2
            # If so, pack it along with its operand.
            output = struct.pack('BB', instructions[instruction], memoryOffset - 2)    # Not sure if it should be subtracted by 2 or not, but I think so
            return output
        
        # Is the operand the current bank index?
        elif operand == "$$":
            # If so, pack it along with its operand
            memoryOffset += 2
            return struct.pack('BB', instructions[instruction], bankOffset)
        
        else:
            # If the operand is not valid, stop assembly and inform the user.
            print("Error. Invalid operand on line: " + str(currentLine) + ". Cannot assemble.")
            exit()
    elif instruction != "":
        # If the instruction is invalid, stop assembly and inform the user. Whitespace is ignored.
        print("ERROR: Unknown instruction on line: " + str(currentLine) + ". Cannot assemble.")
        print("Maybe you missed a typo?")
        exit()

# This function takes an instruction and separates it into its opcode and operand
def ParseInstruction(line):
    global memoryOffset
    global bankOffset
    global currentLine
    currentLine += 1

    # Is there a comment on the line?
    if ';' in line:
        # If so, remove it.
        line = line.split(";")[0].strip()

    # If the line exists, starts with _, and ends with :, make it a label. Although this is done in another function, if it is removed, the program
    # stops working. If it ain't broke, don't fix it.
    if line and line[0] == "_" and line[-1] == ":":
        # As long as your line starts with _ and ends with :, it will become a label you can jump to.

        # Remove the : from the line so that it can be jumped to in a similar way to x86 and so that the assembler won't 
        # identify it as a new label when jumping.
        labels[line[:-1]] = [bankOffset, memoryOffset]
    elif line:
        # If the line is not a label or a comment and it exists, process it
        operand = 0                         # Default operand value is 0 because all instructions are 16 bits wide, even with no operands.

        # Split the line based on commas and make it all uppercase as long as there is no label call. This allows for case sensitive labels.
        if not "_" in line:
            tokens = line.strip().split(',')
        else:
            tokens = line.strip().split(',')

        # Get the opcode, which is the first token, make it uppercase, and remove whitespace, then assign it to the opcode variable
        opcode = tokens[0].strip().upper()

        # If there is an operand with the opcode, which will be the second token, remove whitespace, then assign it to the operand variable.
        if len(tokens) > 1:
            operand = tokens[1].strip()

        # Abstract away SOI/SOR
        if len(tokens) > 2 and len(tokens) < 4:
            # If there are two operands and the second one is an immediate value, add the second operand instruction before the original instruction
            # gets executed.
            for i in range(len(tokens)):
                tokens[i] = tokens[i].strip().upper()
            if tokens[2].strip() in registerIDs:
                secondOpcode = "SOR"
            else:
                secondOpcode = "SOI"
            secondOperand = tokens[2]
            assembledCode.append(AssembleInstruction(secondOpcode, secondOperand))
        # Assemble the code and add it to the array of assembled code
        assembledCode.append(AssembleInstruction(opcode, operand))

# Get the name of the asm source file
fileName = input("Enter the name of your asm file: ")

# Open the asm source file
with open(fileName, 'r') as AssemblyFile:
    # Read the whole file and assign it to a string
    inputString = AssemblyFile.read()

# Get the lines of the .asm file. Newline means new instruction.
lines = inputString.split("\n")

# Get the names of all the labels
GetLabels(lines)

# Debug output to make sure that the labels always line up
#print(labels)

for line in lines:
    # Parse the instructions of each line
    ParseInstruction(line)

# Debug output to make sure that the labels always line up
#print(labels)

# Remove any NoneTypes from the assembled code.
assembledCode = [item for item in assembledCode if item is not None]

# Tell the user how many bytes were assembled if there were no errors.
print(str(memoryOffset) + " bytes assembled.")

# Write the assembled binary to program.bin
with open('program.bin', 'wb') as binary_file:
    binary_file.write(b''.join(assembledCode))


# This grew very fast. The first functional version had ~40 lines of code.
