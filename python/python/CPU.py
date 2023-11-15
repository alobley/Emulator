# File: CPU
# Author: Andrew Lobley
# Project: COMPEMU
#
# Summary:
# This file is the emulator for the CPU. Pass instructions to it to program the emulated
# computer. Theoretically speaking, this can be declared multiple times 
#
#
# Ver   Who   --Date--   Changes:
# 0.1   AND   11/15/23   Created the file and added some instructions

# Registers:
# A - Address register. Specifies the byte in RAM to start at.
# B - Length register. How many bytes to print when int 0x01 is declared

from GPU import Display

class CPU:
    def __init__(self):
        self.RAM = [0 for i in range(256)]
        self.Registers = {"A": 0, "B": 0, "C": 0, "D": 0, "E": 0, "F": 0, "G": 0, "H": 0}
        self.screen = Display(20, 20)
        
    def mov(self, dest, src):
        if dest.isnumeric():
            print("Segfault: Core Dumped")
        else:
            if src.isnumeric():
                if int(src) < 256:
                    self.Registers[dest] = int(src)
                else:
                    print("OVERFLOW")
            elif not src.isnumeric() and src in self.Registers:
                self.Registers[dest] = self.Registers[src]
            else:
                print("Error: register does not exist.")
                
    def db(self, startDest, data):
        if startDest < 256 and startDest >= 0:
            if data.isnumeric() and data < 256 and data > 0:
                self.RAM[startDest] = data
            else:
                data = [ord(char) for char in data]
                for i in range(len(data)):
                    self.RAM[i + startDest] = data[i]
        else:
            print("Memory Error: Index Out of Bounds")
            
    def int(self, interrupt):
        if interrupt == "0x01":
            start_address = self.Registers["A"]
            num_iterations = self.Registers["B"]

            printable = ''
            for i in range(num_iterations):
                if start_address + i < len(self.RAM):
                    printable += chr(self.RAM[start_address + i])
                else:
                    break

            currentRow = 0
            currentCol = 0
            self.screen.print_text(printable, currentRow, currentCol)
            currentRow += 1
            
        
    
    def PrintRAM(self):
        print(self.RAM)
        
    def PrintRegister(self, register):
        print(self.Registers[register])