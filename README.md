# c3 - A minimal stack-based VM written in C.

## What is c3?
- c3 is a stack-based, byte-coded VM whose "Virtual CPU" does not have alot of opcodes.
- c3's opcodes implement many of the standard Forth operations.
- c3 supports IEEE 754 double-precision (64-bit) floating point numbers.
- c3 provides 10 "virtual registers", r0 thru r9.
  - Each register has 6 operations: rX, sX, iX, dX, rX+, and rX-.
- c3 provides 10 temporary words, T0 thru T9.
  - T0-T5 are "normal" words, T6-T8 are INLINE, and T9 is IMMEDIATE.

## Goals
The goals for c3 are:
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To be very easy to extend as desired.
- To provide as much flexibility to the programmer as possible.
- To be able to run on Windows, Linux, Apple, and development boards via the Arduino IDE.

## Notes about c3:
- c3 is NOT an ANSI-standard Forth system.
- Counted strings are also null-terminated.
- The dictionary starts at the end of the MEM area and grows down.
- The dictionary search is not case-sensitive.
- The VARIABLE space is separated from the MEM space.

## Registers
c3 exposes 10 "virtual registers", r0 thru r9. There are 8 register operations: +regs, rX, rX+, rX-, sX, iX, dX, -regs.

Note: The support for registers is built into c3, so they do NOT show up in "WORDS".

- +regs   allocate 10 new registers.
- r4      push register 4 to the stack.
- r4+     push register 4 to the stack, then increment it.
- r4-     push register 4 to the stack, then decrement it.
- s4      set register 4 from TOS.
- i4      increment register 4.
- d4      decrement register 4.
- -regs   restore the registers to their previous values.

Some example uses of registers:
```
   : btw  ( n l h--f )  +regs  s3 s2 s1  r2 r1 <   r1 r3 <  and  -regs ;
   : btwi ( n l h--f )  +regs  s3 s2 s1  r2 r1 <=  r1 r3 <= and  -regs ;
   : 2swap ( a b c d--c d a b )  +regs  s4 s3 s2 s1  r3 r4 r1 r2  -regs ;
   : move ( f t n-- ) +regs  s3 s2 s1  r3 0 do  r1+ c@ r2+ c!  loop  -regs ;
   : fill ( a c n-- ) +regs  s3 s2 s1  r3 0 do  r2 r1+ c!      loop  -regs ;
```

## Temporary words
c3 provides 10 temporary words, T0 thru T9.
- Defining a temporary word does not add an entry to the dictionary.
- Temporary words are intended to be helpful in factoring code.
- A temporary word can be redefined as often as desired.
- When redefined, code references to the previous definition are unchanged.
- T0-T5 are "normal" words, T6-T8 are INLINE, and T9 is IMMEDIATE.
- The names of the temporary words is case-sensitive (T0-T9, not t0-t9).

An example usage of temporary words:
```
\ The Babylon square root algorithm
: T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - 0= until nip ;
: sqrt ( n--0|sqrt ) dup 0 > if T0 else drop 0 then ;
```

## Inline words
In c3, an "INLINE" word is like a macro ... when compiling INLINE words, c3 copies the contents of the word (up to, but not including the EXIT) to the target, as opposed to compiling a CALL to the word. This improves performance and often saves space too. This is especially true on a 64-bit system, where the CELL size is 8. **Note that if a word might have an embedded 3 (EXIT) in its implementation (like in an address for example), then it should not be marked as INLINE.**

## Notes on output formatting in TYPE (aka - ."):
- %b: output TOS as a binary number
- %c: output TOS as a character
- %d: output TOS as an integer (base 10)
- %e: output an ESCAPE (27)
- %f: output FTOS as a floating point number
- %g: output FTOS as a scientific number
- %i: output TOS as an integer (current base)
- %n: output a CR/LF (13,10)
- %q: output the quote (") character
- %s: output TOS as a string (null terminated, no count byte)
- %t: output a TAB (9)
- %x: output TOS as a hex number

For example: : ascii 127 32 DO I I I I ." %n%d: (%c) %x %b" LOOP ;

## Bootstrapping c3
To bootstrap itself, c3 has a simple "machine language parser" that can create words in c3's "machine language". The keyword for that is "-ML-". For example, the c3 opcode for "return from subroutine" is 3, and "duplicate the top of the stack" is 12. So in the beginning of sys-load.h, I define aliases for the opcodes.

- The first four words defined in c3 are:
  - 'INLINE'    - mark the last word as inline
  - 'IMMEDIATE' - mark the last word as immediate
  - ':'         - define a new c3 word
  - ';'         - end word definition

c3 also defines some 'system-info' words (the addresses of system variables and sizes of buffers).

Everything in c3 can defined from those.

See the sys-load.h file for details.

Note that this approach gives the user the maximum flexibility. Opcode 12 does not have to be "DUP", it could just as easily be "(N--NN)" (or "foo--foo/foo", or whatever). But DUP is clear and concise, so that its default name. :)

## The dictionary
- A dictionary entry looks like this:
  - xt:     cell_t
  - flags:  byte (IMMEDIATE=$01, INLINE=$02)
  - len:    byte
  - name:   char[NAME_LEN+1] (NULL terminated)

## Default sizes for PC-based systems
- The default NAME_LEN is 13.
- The default MEM_SZ is 128K bytes (code and distionary).
- The default VARS_SZ is 4MB bytes (strings and variables).
- The default STK_SZ is 64 CELLS (data and return stacks).
- The default LSTK_SZ is 30 CELLS (loop stack, multiple of 3).
- The default REGS_SZ is 100 CELLS (register stack, multiple of 10).
- These can be easily changed in the sys-init.h file.

## Building c3:
- Windows: there is a c3.sln file for Visual Studio
  - Use the x86 configuration (32-bit)
- Linux: there is a 'make' script
  - It uses clang and builds a 64-bit version
  - Use -m32 to build a 32-bit version
  - You can also use gcc if you desire
- Apple: I do not have an Apple, so I cannot build for Apples
  - But c3 is minimal enough that it should be easy to port to an Apple system
- Arduino: there is a c3.ino file
  - I use the Arduino IDE v2.0
  - Edit the section where isBOARD is defined to set the sizes for the board
  - For the RPI Pico and Teensy 4.0, I use:
    - MEM_SZ:    64K
    - VARS_SZ:   96K
    - STK_SZ:    32
    - LSTK_SZ:   30
    - REGS_SZ;  100
    - NAME_LEN:  13

## c3 Opcode / Word reference
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
|  0 | STOP       | (--)         | Stops the runtime engine|
|  1 | LIT1       | (--n)        | Pushes next BYTE onto the stack|
|  2 | LIT        | (--N)        | Pushes next CELL onto the stack|
|  3 | EXIT       | (--)         | Exit subroutine|
|  4 | CALL       | (--)         | Call: next CELL is address, handles call-tail optimization|
|  5 | JUMP       | (--)         | Jump: next CELL is address|
|  6 | JUMPZ      | (N--)        | Jump if TOS==0: next CELL is address|
|  7 | JUMPNZ     | (N--N)       | Jump if TOS!=0: next CELL is address|
|  8 | !          | (N A--)      | Store CELL N to address A|
|  9 | C!         | (B A--)      | Store BYTE B to address A|
| 10 | @          | (--)         | Fetch CELL N FROM address A|
| 11 | C@         | (--)         | Fetch BYTE B FROM address A|
| 12 | DUP        | (N--N N)     | Duplicate TOS|
| 15 | DROP       | (A B--A)     | Drop TOS|
| 13 | SWAP       | (A B--B A)   | Swap TOS and NOS|
| 14 | OVER       | (A B--A B A) | Push a copy of NOS|
| 16 | +          | (A B--C)     | C: A + B|
| 17 | *          | (A B--C)     | C: A * B|
| 18 | /MOD       | (A B--C D)   | C: A modulo B, D: A divided by B|
| 19 | -          | (A B--C)     | C: A - B|
| 20 | 1+         | (A--B)       | Increment TOS|
| 21 | 1-         | (A--B)       | Decrement TOS|
| 22 | <          | (A B--F)     | If A<B, F=1, else F=0|
| 23 | =          | (A B--F)     | If A=B, F=1, else F=0|
| 24 | >          | (A B--F)     | If A>B, F=1, else F=0|
| 25 | 0= , NOT   | (N--F)       | If N=0, F=1, else F=0|
| 26 | >R         | (N--)        | Move N to return stack|
| 27 | R@         | (--N)        | N: Copy of top of return stack|
| 28 | R>         | (--N)        | N: Top of return stack (popped)|
| 29 | DO         | (T F--)      | Begin a loop from F to T, set I = F|
| 30 | LOOP       | (--)         | Increment I. Jump to DO if I<T|
| 31 | -LOOP      | (--)         | Decrement I. Jump to DO if I>T|
| 32 | (I)        | (--)         | Address of I, the loop index|
| 33 | COM        | (A--B)       | B: Ones-complement of A|
| 34 | AND        | (A B--C)     | C: A bitwise-AND B|
| 35 | OR         | (A B--C)     | C: A bitwise-OR B|
| 36 | XOR        | (A B--C)     | C: A bitwise-XOR B|
| 37 | COUNT      | (A1--A2 N)   | Standard Forth COUNT|
| 38 | TYPE       | (A N--)      | Output N formatted chars at address A to (output_fp)|
| 39 | iX         | (--)         | Increment register X|
| 30 | dX         | (--)         | Decrement register X|
| 41 | rX         | (--N)        | N: value of register X|
| 42 | rX+        | (--N)        | N: value of register X, then decrement it|
| 43 | rX-        | (--N)        | N: value of register X, then increment it|
| 44 | sX         | (N--)        | Set regiser X to TOS|
| 45 | +REGS      | (--)         | Allocate 10 new registers|
| 46 | -REGS      | (--)         | Restore last set of registers|

### System opcodes are 2-bytes, starting with 47
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
| 47,0  | INLINE     | (--)         | Mark the last word in the dictionary as INLINE|
| 47,1  | IMMEDIATE  | (--)         | Mark the last word in the dictionary as IMMEDIATE|
| 47,2  | (.)        | (N--)        | Perform ITOA on N, then TYPEZ it (no trailing space)|
| 47,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 47,4  | ITOA       | (N--A)       | A: a string version of N in the current BASE|
| 47,5  | :          | (--)         | Execute CREATE, then set STATE=1|
| 47,6  | ;          | (--)         | Append EXIT to code,then set STATE=0|
| 47,7  | CREATE     | (--)         | Execute NEXT-WORD, add A to the dictionary|
| 47,8  | '          | (--XT FL F)  | Execute NEXT-WORD, search for A. Push (XT FL 1) if found, else push only (0) |
| 47,9  | NEXT-WORD  | (--A)        | A: Address of the next word from the input stream|
| 47,10 | TIMER      | (--N)        | N: current system time in milliseconds|
| 47,11 | C,         | (C--)        | Standard Forth "C,"|
| 47,12 | ,          | (N--)        | Standard Forth ","|
| 47,13 | KEY        | (--B)        | B: next keypress, wait if necessary|
| 47,14 | ?KEY       | (--F)        | If key was pressed, F=1, else F=0|
| 47,15 | EMIT       | (B--)        | Output BYTE B to (output_fp)|
| 47,16 | TYPEZ      | (A--)        | Output NULL-terminated a address A to (output_fp)|

### String opcodes are 2-bytes, starting with 48
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
| 48,0  | TRUNC      | (S--)        | Truncate string S|
| 48,1  | STRCPY     | (S --)       | Copy string S to string D|
| 48,2  | STRCAT     | (S D--)      | Concatenate string S to string D|
| 48,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 48,4  | STRLEN     | (S--N)       | N: length of string S|
| 48,5  | STREQ      | (S1 S2--FL)  | FL: 1 if F1 = F2, else 0 (case sensitive)|
| 48,6  | STREQI     | (S1 S2--FL)  | FL: 1 if F1 = F2, else 0 (not case sensitive)|
| 48,7  | LCASE      | (C1--C2)     | Convert C1 to lowercase|
| 48,8  | UCASE      | (C1--C2)     | Convert C1 to uppercase|

### Floating point opcodes are 2-bytes, starting with 49
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
| 49,0  | F+         | (F1 F2--F3)  | Add F1 and F2, leaving F3|
| 49,1  | F-         | (F1 F2--F3)  | Subtract F2 from F1, leaving F3|
| 49,2  | F*         | (F1 F2--F3)  | Multiply F1 and F2, leaving F3|
| 49,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 49,4  | F/         | (F1 F2--F3)  | Divide F1 by F2, leaving F3|
| 49,5  | F=         | (F1 F2--FL)  | FL: 1 if F1 = F2, else 0|
| 49,6  | F<         | (F1 F2--FL)  | FL: 1 if F1 < F2, else 0|
| 49,7  | F>         | (F1 F2--FL)  | FL: 1 if F1 > F2, else 0|
| 49,8  | F2I        | (F1--N)      | Convert double F1 into an integer N|
| 49,9  | I2F        | (N--F1)      | Convert integer N into a double F1|
| 49,10 | F.         | (F1--)       | Output F1 using the "%g" C format string|
| 49,11 | SQRT       | (F1--F2)     | F2: the square root of F1
| 49,12 | TANH       | (F1--F2)     | F2: the hyperbolic tangent of F1

### Opcodes for PCs (Windows and Linux)
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
| 100 | SYSTEM | (A--)      | Call system(A)|
| 101 | FOPEN  | (N M--H)   | N: FileName, M: OpenMode (R/W/A), H: Handle|
| 102 | FCLOSE | (H--)      | Close file with handle H|
| 103 | FREAD  | (A N H--R) | Read N bytes from file H to address A, R: num-read|
| 104 | FWRITE | (A N H--)  | Write N bytes to file H to address A|
| 105 | (LOAD) | (A--)      | Load from file A|

### Opcodes for Development Boards
|Opcode|Word|Stack|Description|
| :-- | :-- | :-- | :-- |
| 58 | PIN-INPUT  | (P--)   | pinMode(P, INPUT)|
| 59 | PIN-OUTPUT | (P--)   | pinMode(P, OUTPUT)|
| 60 | PIN-PULLUP | (P--)   | pinMode(P, INPUT_PULLUP)|
| 61 | DPIN@      | (P--N)  | N = digitalRead(P)|
| 62 | APIN@      | (P--N)  | N = analogRead(P)|
| 63 | DPIN!      | (N P--) | digitalWrite(P, N)|
| 64 | APIN!      | (N P--) | analogWrite(P, N)|

## Built-in c3 system-information words
|Word|Stack|Description|
| :-- | :-- | :-- |
| VERSION       | (--N)    | N: c3 version*100 (e.g. - 147 => v1.47).|
| MEM           | (--A)    | A: Start address for the MEMORY area.|
| MEM-SZ        | (--N)    | A: The size of the MEMORY area in bytes.|
| VARS          | (--A)    | A: Start address for the VARIABLES area.|
| VARS-SZ       | (--N)    | N: The size of the VARIABLES area in bytes.|
| (REGS)        | (--A)    | A: Start address for the REGISTERS (REGS_SZ CELLs).|
| (INPUT_FP)    | (--A)    | A: Address of the input file handle.|
| (OUTPUT_FP)   | (--A)    | A: Address of the output file handle.|
| (HERE)        | (--A)    | A: Address of the HERE variable.|
| (LAST)        | (--A)    | A: Address of the LAST variable.|
| (VHERE)       | (--A)    | A: Address of the VHERE variable.|
| (STK)         | (--A)    | A: Address of the stack.|
| (SP)          | (--A)    | A: Address of the stack pointer.|
| (RSP)         | (--A)    | A: Address of the return stack pointer.|
| (LSP)         | (--A)    | A: Address of the loop stack pointer.|
| BASE          | (--A)    | A: Address of the BASE variable.|
| STATE         | (--A)    | A: Address of the STATE variable.|
| TIB           | (--A)    | A: Address of TIB (text input buffer).|
| >IN           | (--A)    | A: Address of >IN.|
| WORD-SZ       | (--N)    | N: size of a dictionary entry in bytes.|
| CELL          | (--N)    | N: size of a CELL in bytes.|

## c3 startup behavior
When c3 starts:
- For every parameter on the command line:
  - If c3 can open the parameter as a file, load it.
  - Else, set the (numeric only) value to a register based on the parameter's position.

## Adding new opcodes to c3
If for some reason, there is a need/desire to add more opcodes to c3, this describes how it can be accomplished. 

For example, there might be some functionality in a library you want to make available, or maybe there is a bottleneck in performance you want to improve.

Here is the process:

- Global opcode:
  - In c3.c, add the new opcode(s) to the appropriate enum.
  - In c3.c, add a NCASE to run() to for each new opcode.
  - In sys-load.h, add a "-ML-" line to LoadStartupWords() for each new opcode.
  - Update your README.md.

- Target-specific opcode:
  - All work is done in the target's *.h file (e.g. - sys-pc.h).
  - Add the new opcodes(s) to the enum.
  - Target-specific opcodes should have values above 100.
  - Edit LoadStartupWords() and add a "-ML-" line for each new opcode.
  - For example: to define opcode 120 as "NEWOP" ... ParseLine("-ML- NEWOP 120 3 -MLX- INLINE");
  - In doUser(), add cases for the new opcode(s).
  - Update your README.md.
