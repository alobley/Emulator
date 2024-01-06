/* Program: Emulator
 * Developer: Me
 * Language: C
 * 
 * Description: This is an emulator for a simple CPU that uses a custom architecture and instruction set.
 * 
 * TODO:
 * - Implement the ability to execute the rest of the instructions (done)
 * - Create the ability to import a binary or text file (done)
 * - Create an assembler to make programming easier (functional but not complete)
 * - Add IN/OUT instructions for communicating with external hardware
 * - Memory map some of the RAM to a visual library's window for visual output (done)
 * - Add keyboard I/O operations
 * - Add some simple assembly programs for reference
 * - Create Pong (final goal)
 * 
 * Change Log:
 * --DATE--   WHO   Description of Change
 * 01/02/23   AND    Wrote the initial program. Added the instruction set, registers, RAM, and the byte variable. Implemented some instruction execution
 *                   functions and the function that calls execution functions.
 * 
 * 01/03/23   AND    Added the rest of the execution functions, the ability to load programs into RAM and execute them, started the assembler, added
 *                   the ability to execute the rest of the functions and added the ability to read and execute imported binary files.
 * 
 * 01/04/23   AND    Created the README file which contains extensive documentation and some example assembly programs. Greatly updated the assembler, added
 *                   some new instructions to replace old ones, and made jumps work properly.
 *                   Added and memory mapped an SDL-based virtual screen which does function properly. The user can write 16x16 squares to the screen
 *                   with 8-bit colors based on the location in RAM. VRAM is the last 1kb or so of RAM.
 */

/* 
 * 
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>       // I believe SDL has a keyboard module I can use. Will be helpful.
#include <time.h>           // Not implemented yet. Will be the PIT, speed will eventually be set.

// Reminder: stdbool boolean values are 1 and 0, very helpful in this context.

/* Hex Table
 * 0000 = 0x0       1000 = 0x8
 * 0001 = 0x1       1001 = 0x9
 * 0010 = 0x2       1010 = 0xA
 * 0011 = 0x3       1011 = 0xB
 * 0100 = 0x4       1100 = 0xC
 * 0101 = 0x5       1101 = 0xD
 * 0110 = 0x6       1110 = 0xE
 * 0111 = 0x7       1111 = 0xF
*/

#pragma region Computer

#pragma region Variables
// Get the total size of the Computer's RAM. This equates to ~65.5kb.
#define BANK_SIZE 0xFF              // 256 (0xFF) bytes per bank
#define NUM_BANKS 0xFF              // 256 (0xFF) banks

// For easier understanding, define byte and word instead of using their C identifiers.
typedef unsigned char byte;
typedef unsigned short word;

// Memory bank variable, which holds an array of 256 bytes called address.
typedef struct {
    byte address[BANK_SIZE];
} MemoryBank;

MemoryBank RAM[NUM_BANKS];          // An array of 256 memory bank variables

bool JMPFunction = false;           // This flag is activated when a JMP instruction is called. This allows the program to jump to and read from the correct
                                    // spot.

#pragma endregion Variables

#pragma region Graphics
// Define the screen parameters
#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 496

// Set up the window and renderer pointers
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Initialize SDL and create window and renderer
int initSDL() {
    window = SDL_CreateWindow("8-bit CPU Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    // Setup renderer
    renderer =  SDL_CreateRenderer( window, -1, SDL_RENDERER_ACCELERATED);

    // Set render color to black ( background will be rendered in this color )
    SDL_SetRenderDrawColor( renderer, 0, 0, 0, 0 );

    return 0;
}

// This function gets an 8-bit RGB color value based on an 8-bit input
Uint8 extractBits(Uint8 value, int position) {
    return (value >> (position * 2)) & 0b11;
}

// Update the texture with the data from the array. Supports 8-bit colors.
void UpdateTexture(int x, int y, byte color) {
    // Set render color to black (background will be rendered in this color)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    Uint8 red = 0;
    Uint8 green = 0;
    Uint8 blue = 0;

    if(color != 0){
        // Extract 2 bits for Red, Green, and Blue channels
        red = extractBits(color, 0);
        green = extractBits(color, 1);
        blue = extractBits(color, 2);
    }

    // Create a rect at the specified position with a size of 16x16
    SDL_Rect r;
    r.x = x;
    r.y = y;
    r.w = 16;
    r.h = 16;

    // Set render color based on the color parameter
    SDL_SetRenderDrawColor(renderer, red * 85, green * 85, blue * 85, SDL_ALPHA_OPAQUE);

    // Render rect
    SDL_RenderFillRect(renderer, &r);
}

// Draw something to the screen based on the value in the computer's RAM
void DrawToScreen(){
    // Set the X and Y values to draw to
    int x = 0;
    int y = 0;

    for(int i = 0; i < 4; i++){
        // Iterate four times
        for(int j = 0; j < 256; j++){
            // Iterate 256 times. That makes 4 whole banks of memory to be written to the screen.

            // Write to the screen starting from bank 251 address 0
            UpdateTexture(x, y, RAM[251+i].address[j]);
            if(x > 512 - 16){
                // If the X value has reached the right side of the screen, increment y by 16 and reset X to 0
                x = 0;
                y += 16;
            }else{
                // Otherwise, increment X by 16
                x += 16;
            }
        }
    }
    // Render everything that was drawn to the screen
    SDL_RenderPresent(renderer);
}

// Clean up and close SDL
void closeSDL() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
#pragma endregion graphics

#pragma region CPU
#pragma region instructions
// I = immediate, R = register

// Special Instructions
byte NOP = 0b00000000;          // No operation. No operands.
byte SOI = 0b10000000;          // Second Operand Immediate
byte SOR = 0b10000001;          // Second Operand Register
byte BSWCHI = 0b00100110;       // Perform a bank switch to access memory in another bank. Does not affect PC.
byte BSWCHR = 0b00100111;

// Arithmetic Instructions (all output in register A)
byte ADDI = 0b00000010;         // Add immediate value to A
byte ADDR = 0b00000011;         // Add register value to A
byte SUBI = 0b00000100;         // Subtract immediate value from A
byte SUBR = 0b00000101;         // Subtract register value from A

// Register Instructions
byte LDI = 0b00000110;          // Load immediate value into a register
byte CPY = 0b00000111;          // Copy one register value to another register. If the first operand is P, it is a memory location.
byte MOVMI = 0b00001000;        // Move an immediate value to the address pointed to by P
byte MOVMR = 0b00001001;        // Move a register's value to an address pointed to by P
byte GETP = 0b00101001;         // Get the value from the location P points to and store it in a given register.
byte SHL = 0b10001011;          // Bit shift left
byte SHR = 0b10001101;          // Bit shift right

// Branching Instructions
byte JMPI = 0b00001010;         // Jump to an immediate address
byte JMPR = 0b00001011;         // Jump to the address in two registers
byte JEI = 0b00001100;          // Jump if equal to an immediate address
byte JER = 0b00001101;          // Jump if equal to the address in two registers
byte JNEI = 0b00001110;         // Jump if not equal to an immediate address
byte JNER = 0b00001111;         // Jump if not equal to an address in two registers
byte CMPI = 0b00010000;         // Compare a register with an immediate value
byte CMPR = 0b00010001;         // Compare a register with another register

// Memory I/O Instructions
byte LOADI = 0b00010010;        // Load from an immediate address into a register
byte LOADR = 0b00010011;        // Load from a pointer in another register into a register
byte STORI = 0b00010100;        // Store from a given register into an immediate address
byte STORR = 0b00010101;        // Store from a given register to a pointer given by another register
byte PUSHI = 0b00010110;        // Push an immediate value onto the stack
byte PUSHR = 0b00010111;        // Push a register's value onto the stack
byte POP = 0b00011001;          // Pop the item at the top of the stack

// Increment/Decrement Instructions
byte INCB = 0b00011000;         // Increment byte - increments the value that P points to
byte DECB = 0b00011010;         // Decrement byte - decrements the value P points to
byte INCR = 0b00011011;         // Increment register - increments the value in a given register
byte DECR = 0b00011101;         // Decrement register - decrements the value in a given register

// Logic Instructions
byte ANDI = 0b00011110;         // AND immediate - perform an AND instruction with a register and a given immediate value
byte ANDR = 0b00011111;         // AND register - perform an AND instruction with the values of two registers
byte ORI = 0b00100000;          // OR immediate - perform an OR instruction with a register and a given immediate value
byte ORR = 0b00100001;          // OR register - perform an OR instruction with the values of two registers
byte XORI = 0b00100010;         // XOR immediate - perform an XOR instruction with a register and a given immediate value
byte XORR = 0b00100011;         // XOR register - perform an XOR instruction with the values of two registers
byte NOT = 0b00100101;          // NOT - perform a bitwise NOT operation on the value in a register
#pragma endregion instructions

#pragma region registers/memory
byte A = 0x00;                  // Accumulator register
byte B = 0x00;                  // General-purpose register
byte C = 0x00;                  // General-purpose register
byte D = 0x00;                  // General-purpose register

byte P = 0x00;                  // Pointer register

byte PC[] = {0x00, 0x00};       // Program counter. Upper 8 bits are the memory bank, lower 8 bits are the address.

byte ROP = 0x00;                // Opcode register. Holds the 7-bit opcodes.
byte DR1 = 0x00;                // Value register 1. Holds the first operand or the only operand, depending on the instruction
byte DR2 = 0x00;                // Value register 2. If there are two operands, this register will hold the second one.

// Flags register. Flags in order are negative, carry, equal, and overflow.
bool F[] = {0, 0, 0, 0};

byte BI = 0x00;                 // Bank register
byte S = 0x00;                  // Stack pointer

byte stack[0xFF];               // Stack memory bank
#pragma endregion registers/memory

#pragma region pointers
enum Flags { NEGATIVE, CARRY, EQUAL, OVERFLOW };    // For easier access to the flags

// Register IDs
byte RA = 0x01; // Register A
byte RB = 0x02; // Register B
byte RC = 0x03; // Register C
byte RD = 0x04; // Register D
byte BNK = 0x05;// Bank Register
byte PTR = 0x06;// Pointer Register
byte SP = 0x07; // Stack Pointer

// Register Pointers
byte* A_ptr = &A;
byte* B_ptr = &B;
byte* C_ptr = &C;
byte* D_ptr = &D;
byte* BI_ptr = &BI;
byte* P_ptr = &P;
byte* S_ptr = &S;
#pragma endregion pointers

#pragma region methods
byte* GetRegister(byte code){
    byte* registers[] = { NULL, A_ptr, B_ptr, C_ptr, D_ptr, BI_ptr, P_ptr, S_ptr };
    if(code >= 0 && code <= 8){
        return registers[code];
    }else{
        return NULL;
    }
}

void PrintRegisters(){
    // Print the values of all registers to the terminal. Change %x to %i for decimal.
    printf("A: 0x%02x\n", A);
    printf("B: 0x%02x\n", B);
    printf("C: 0x%02x\n", C);
    printf("D: 0x%02x\n", D);
    printf("P: 0x%02x\n", P);
    printf("\nROP: 0x%02x\n", ROP);
    printf("DR1: 0x%02x\n", DR1);
    printf("DR2: 0x%02x\n", DR2);
    printf("\nPCH: 0x%02x\n", PC[0]);
    printf("PCL: 0x%02x\n", PC[1]);
}

void PrintRAMDebug(int programLength){
    printf("\nProgram Length (In Bytes): %i\n", programLength);
    printf("\nRAM:\n");
    for(int i =0; i < programLength; i++){
        // For each memory address taken up by the program, print the value at that address. Right now it only does bank 1, but starter programs
        // likely won't be bigger than that.
        printf("0x%02x\n", RAM[0].address[i]);
    }
}
#pragma endregion methods

#pragma region Instruction Functions
// These are described in the instructions region. Their names match the names of the instruction as closely as possible.

void BankSwitchImmediate(){
    // One operand. Execute before changing P if you are trying to access a different memory bank.
    BI = DR1;
    return;
}
void BankSwitchRegister(){
    // One operand. If you want to change it to the value in a register, you can.
    byte* registerPointer = GetRegister(DR1);
    BI = (*registerPointer);
    return;
}
void AddImmediate(){
    // One operand
    A = A + DR1;
    return;
}
void AddRegister(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    A = A + (*registerPointer);
    return;
}
void SubImmediate(){
    // One operand
    A = A - DR1;
    return;
}
void SubRegister(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    A = A - (*registerPointer);
    return;
}
void LoadImmediate(){
    // DR1 = register, DR2 = value
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = DR2;
    return;
}
void Copy(){
    // Destination = DR1, source = DR2
    byte* dest = GetRegister(DR1);
    byte* src = GetRegister(DR2);
    (*dest) = *src;
    return;
}
void WriteImmediateToP(){
    // One operand
    RAM[BI].address[P] = DR1;
    return;
}
void WriteRegisterToP(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    RAM[BI].address[P] = (*registerPointer);
    return;
}
void GetFromP(){
    // One operand, which is a register
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = RAM[BI].address[P];
}
void ShiftLeft(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = (*registerPointer) << 1;
    return;
}
void ShiftRight(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = (*registerPointer) >> 1;
    return;
}
void JumpImmediate(){
    // Where PC is what points to the address where the current instruction is being executed
    // P and BI are independent of the program's location in execution

    // DR1 = address, DR2 = bank
    PC[1] = DR1;
    PC[0] = DR2;
    JMPFunction = true;
    return;
}
void JumpRegister(){
    // DR1 = address register, DR2 = bank register
    byte* addressRegister = GetRegister(DR1);
    byte* bankRegister = GetRegister(DR2);
    PC[1] = (*addressRegister);
    PC[2] = (*bankRegister);
    JMPFunction = true;
    return;
}
void JumpEqualImmediate(){
    // DR1 = address, DR2 = bank
    if(F[EQUAL] == true){
        // If the equal flag is true
        PC[1] = DR1;
        PC[0] = DR2;
        JMPFunction = true;
    }
    return;
}
void JumpEqualRegister(){
    // DR1 = address register, DR2 = bank register
    if(F[EQUAL] == true){
        // If the equal flag is true
        byte* addressRegister = GetRegister(DR1);
        byte* bankRegister = GetRegister(DR2);
        PC[1] = (*addressRegister);
        PC[0] = (*bankRegister);
        JMPFunction = true;
    }
    return;
}
void JumpNotEqualImmediate(){
    // DR1 = address, DR2 = bank
    if(F[EQUAL] == false){
        // If the equal flag is not true
        PC[1] = DR1;
        PC[0] = DR2;
        JMPFunction = true;
    }
    return;
}
void JumpNotEqualRegister(){
    // DR1 = address register, DR2 = bank register
    if(F[EQUAL] == false){
        // If the equal flag is not true
        byte* addressRegister = GetRegister(DR1);
        byte* bankRegister = GetRegister(DR2);
        PC[1] = (*addressRegister);
        PC[0] = (*bankRegister);
        JMPFunction = true;
    }
    return;
}
void CompareImmediate(){
    // DR1 = register, DR2 = value to compare
    byte* registerPointer = GetRegister(DR1);
    if((*registerPointer) == DR2){
        // If the two numbers are equal, set the equal flag
        F[EQUAL] = true;
    }else{
        // Otherwise, clear the equal flag
        F[EQUAL] = false;
    }
    return;
}
void CompareRegister(){
    // Data register use does not matter
    byte* firstRegister = GetRegister(DR1);
    byte* secondRegister = GetRegister(DR2);
    if((*firstRegister) == (*secondRegister)){
        // If the two numbers are equal, set the equal flag
        F[2] = 1;
    }else{
        // Otherwise, clear the equal flag
        F[2] = 0;
    }
    return;
}
void ReadImmediate(){
    // DR1 = register, DR2 = address
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = RAM[BI].address[DR2];
    return;
}
void ReadRegister(){
    // DR1 = load register, DR2 = address register
    byte* load = GetRegister(DR1);
    byte* address = GetRegister(DR2);
    (*load) = RAM[BI].address[(*address)];
    return;
}
void StoreImmediate(){
    // DR1 = register, DR2 = address
    byte* registerPointer = GetRegister(DR1);
    RAM[BI].address[DR2] = (*registerPointer);
    return;
}
void StoreRegister(){
    // DR1 = store register, DR2 = address register
    byte* store = GetRegister(DR1);
    byte* address = GetRegister(DR2);
    RAM[BI].address[(*address)] = (*store);
    return;
}
void PushImmediate(){
    // Put the value in DR1 onto the stack
    stack[S] = DR1;
    S++;
    return;
}
void PushRegister(){
    // Put the value of a register onto the stack
    byte* registerPointer = GetRegister(DR1);
    stack[S] = (*registerPointer);
    (*registerPointer) = 0;
    S++;
    return;
}
void Pop(){
    // Take the top value off of the stack and store it in register B.
    S--;
    B = stack[S];
    stack[S] = 0;
    return;
}
void IncrementByte(){
    // No operands
    RAM[BI].address[P]++;
    return;
}
void DecrementByte(){
    // No operands
    RAM[BI].address[P]--;
    return;
}
void Increment(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    *registerPointer += 1;

    return;
}
void Decrement(){
    // One operand
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = (*registerPointer) - 1;
    return;
}
void AndImmediate(){
    // DR1 = value, DR2 = register
    byte* registerPointer = GetRegister(DR2);
    (*registerPointer) = DR1 & (*registerPointer);
    return;
}
void AndRegister(){
    // Source register = DR1, operand register = DR2
    byte* firstRegister = GetRegister(DR1);
    byte* secondRegister = GetRegister(DR2);
    (*firstRegister) = (*firstRegister) & (*secondRegister);
    return;
}
void OrImmediate(){
    // DR1 = value, DR2 = register
    byte* registerPointer = GetRegister(DR2);
    (*registerPointer) = (*registerPointer) | DR1;
    return;
}
void OrRegister(){
    // Source register = DR1, operand register = DR2
    byte* firstRegister = GetRegister(DR1);
    byte* secondRegister = GetRegister(DR2);
    (*firstRegister) = (*firstRegister) | (*secondRegister);
    return;
}
void XorImmediate(){
    // DR1 = value, DR2 = register
    byte* registerPointer = GetRegister(DR2);
    (*registerPointer) = (*registerPointer) ^ DR1;
    return;
}
void XorRegister(){
    // Source register = DR1, operand register = DR2
    byte* firstRegister = GetRegister(DR1);
    byte* secondRegister = GetRegister(DR2);
    (*firstRegister) = (*firstRegister) ^ (*secondRegister);
    return;
}
void Not(){
    // Performs a bitwise not operation on the value of a register
    byte* registerPointer = GetRegister(DR1);
    (*registerPointer) = ~(*registerPointer);
    return;
}
#pragma endregion Instruction Functions

#pragma region Execution
// Execute an instruction
void ExecuteInstruction(byte opcode, byte operand){
    // Assign the data and operation registers to the opcode/operand values
    ROP = opcode;                   // Put the opcode into the operation register
    if(ROP == SOI || ROP == SOR){
        DR2 = operand;              // If the instruction is Second Opcode, put the operand into data register 2
        return;
    }else{
        DR1 = operand;              // Otherwise, put the operand into data register 1
    }

    // Find the opcode of the instruction and execute the matching function

    // NOP
    if(ROP == NOP){
        // Clear data registers
        DR1 = 0;
        DR2 = 0;
        return;
    }
    if(ROP == BSWCHI){
        BankSwitchImmediate();
        return;
    }
    if(ROP == BSWCHR){
        BankSwitchRegister();
        return;
    }
    // Addition
    if(ROP == ADDI){
        AddImmediate();
        return;
    }
    if(ROP == ADDR){
        AddRegister();
        return;
    }
    // Subtraction
    if(ROP == SUBI){
        SubImmediate();
        return;
    }
    if(ROP == SUBR){
        SubRegister();
        return;
    }
    // Move values into registers
    if(ROP == LDI){
        LoadImmediate();
        return;
    }
    if(ROP == CPY){
        Copy();
        return;
    }
    // Send to P
    if(ROP == MOVMI){
        WriteImmediateToP();
        return;
    }
    if(ROP == MOVMR){
        WriteRegisterToP();
        return;
    }
    // Get from P
    if(ROP == GETP){
        GetFromP();
        return;
    }
    // Bit shifting
    if(ROP == SHL){
        ShiftLeft();
        return;
    }
    if(ROP == SHR){
        ShiftRight();
        return;
    }
    // Branching
    if(ROP == JMPI){
        JumpImmediate();
        return;
    }
    if(ROP == JMPR){
        JumpRegister();
        return;
    }
    if(ROP == JEI){
        JumpEqualImmediate();
        return;
    }
    if(ROP == JER){
        JumpEqualRegister();
        return;
    }
    if(ROP == JNEI){
        JumpNotEqualImmediate();
        return;
    }
    if(ROP == JNER){
        JumpNotEqualRegister();
        return;
    }
    if(ROP == CMPI){
        CompareImmediate();
        return;
    }
    if(ROP == CMPR){
        CompareRegister();
        return;
    }
    // Read/Write
    if(ROP == LOADI){
        ReadImmediate();
        return;
    }
    if(ROP == LOADR){
        ReadRegister();
        return;
    }
    if(ROP == STORI){
        StoreImmediate();
        return;
    }
    if(ROP == STORR){
        StoreRegister();
        return;
    }
    if(ROP == PUSHI){
        PushImmediate();
        return;
    }
    if(ROP == PUSHR){
        PushRegister();
        return;
    }
    if(ROP == POP){
        Pop();
        return;
    }
    // Increment/Decrement
    if(ROP == INCB){
        IncrementByte();
        return;
    }
    if(ROP == DECB){
        DecrementByte();
        return;
    }
    if(ROP == INCR){
        Increment();
        return;
    }
    if(ROP == DECR){
        Decrement();
        return;
    }
    // Logic
    if(ROP == ANDI){
        AndImmediate();
        return;
    }
    if(ROP == ANDR){
        AndRegister();
        return;
    }
    if(ROP == ORI){
        OrImmediate();
        return;
    }
    if(ROP == ORR){
        OrRegister();
        return;
    }
    if(ROP == XORI){
        XorImmediate();
        return;
    }
    if(ROP == XORR){
        XorRegister();
        return;
    }
    if(ROP == NOT){
        Not();
        return;
    }
}
#pragma endregion Execution

#pragma region Keyboard
byte buffer = 0x00;
#pragma endregion Keyboard

#pragma region Run
int quit = 0;
SDL_Event e;
// Load a program from a given disk (which is an array of instructions) into memory
void LoadProgram(byte disk[], int arrayLen){
    for(int i = 0; i < arrayLen; i++){
        if(PC[1] < 256){
            // If we haven't reached the end of the current memory bank, write to the next byte of RAM
            RAM[PC[0]].address[PC[1]] = disk[i-(256*PC[0])];
        }else{
            PC[0]++;
            PC[1] = 0;
            RAM[PC[0]].address[PC[1]] = disk[i-(256*PC[0])];
        }
        PC[1]++;
    }
    // Reset the program counter
    PC[0] = 0;
    PC[1] = 0;
}

// Execute a program in memory
void ExecuteProgram(int programLength){
    // Reset the program counter
    PC[0] = 0;
    PC[1] = 0;

    // For each instruction in the given program length, execute it.
    while((PC[0] << 8) | PC[1] <= programLength){
    //for(int i = 0; i < programLength*2; i++){
        if(RAM[PC[0]].address[PC[1]] < 255){
            // If we have not reached address 255, which is the end of a memory bank, continute executing
            ExecuteInstruction(RAM[PC[0]].address[PC[1]], RAM[PC[0]].address[PC[1]+1]);
        }else{
            // If we have reached address 254 or higher, go to the next memory bank and start writing.
            PC[0]++;
            PC[1] = 0;
            ExecuteInstruction(RAM[PC[0]].address[PC[1]], RAM[PC[0]].address[PC[1]+1]);
        }
        if(JMPFunction == true){
            JMPFunction = false;
        }else{
            PC[1] += 2;
        }

        while (SDL_PollEvent(&e) != 0) {
            // Check if there was an SDL event
            if (e.type == SDL_QUIT) {
                // If it was the command to exit, stop the program.
                return;
            }
        }
        DrawToScreen();
    }
}
#pragma endregion Run

#pragma endregion CPU

#pragma endregion Computer

int main(int argc, char* argv[]){
    // Open the program file
    FILE *file;
    byte *ROM;
    word file_size;
    file = fopen("program.bin", "rb");

    // Look for the file
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);            // Get the size of the file
    rewind(file);                       // Idk, makes it work

    ROM = (byte *)malloc(file_size);    // Get an array of bytes based on the size of the file

    fread(ROM, 1, file_size, file);     // Read the file and write its data to the ROM

    // Get the length of the ROM array
    int arrayLen = file_size / sizeof(ROM[0]);

    // Initialize the SDL screen
    initSDL();

    // Load and execute the program
    LoadProgram(ROM, arrayLen);
    ExecuteProgram(arrayLen);

    closeSDL();

    // Print the values of the registers and the program's memory for debug 
    PrintRegisters();
    PrintRAMDebug(arrayLen);

    free(ROM);                  // After program execution, free the memory taken up by the ROM
    fclose(file);               // Close the file

    return 0;                   // Gracefully exit (syscall 60, 1)
}
