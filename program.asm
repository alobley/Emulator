; Example program: put a line on the top and bottom of the screen
_start:
    LDI, A, 6       ; Load 6 into A

    CPY, B, A       ; Copy the value of A to B

    ADDI, 10        ; Add the immediate value of 10 to A

    CPY, C, A       ; Copy the value of A to C

    SUBR, B         ; Subtract the value of B from the value of A. Result in A.

    XORR, A, A      ; Clear A

    LDI, B, 251     ; Load 251 into B

    LDI, P, 11       ; Set P to 11

    BSWCHR, B       ; Go to the 251st RAM bank (start of VRAM)

; The loop label. Tells the assembler where to jump to so that I don't have to calculate it myself.
_makeTopLine:
    MOVMI, 255      ; Move 255 into the address P points to (whis will create a white 16x16 square)

    INCR, P         ; Increment P

    CMPI, P, 21    ; Compare P to 10
    JNEI, _makeTopLine     ; If the equal flag is false, repeat the loop

LDI, P, 233
LDI, B, 254
BSWCHR, B

_makeBottomLine:
    MOVMI, 255

    INCR, P

    CMPI, P, 243
    JNEI, _makeBottomLine

_done:
JMPI, _done         ; Infinite loop to prevent the processor from shutting down
