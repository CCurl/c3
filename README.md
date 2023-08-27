# c3 - A minimal stack-based VM written in C.

## What is c3?
- c3 is a stack-based VM whose "CPU" does not have alot of opcodes.
- Since c3 is stack-based, it is very well suited to a Forth-like environment.
- c3 is a toolkit to create any environment the programmer desires.
- c3 provides 10 "virtual registers", r0 thru r9.
  - Each register has 6 operations.
- c3 provides 10 temporary words, T0 thru T9.
  - T0-T5 are "normal" words, T6-T8 are INLINE, and T9 is IMMEDIATE.

## Goals
The goals for c3 are:
- To have an implementation that is as minimal as possible.
- To have an implementation that is "intuitively obvious upon casual inspection".
- To to be very easy to extend as desired.
- To provide as much flexibility to the programmer as possible.
- To be able to run on Windows and Linux (and Apple).
- To be deployable to development boards via the Arduino IDE.

## Notes about c3:
- This is NOT an ANSI-standard Forth system.
- This is a byte-coded implementation.
- There are less than 64 operations built into the base executable.
- Four of the operations are pre-defined in the c3 dictionary:
    - ':'         - define a new c3 word
    - ';'         - end word definition
    - 'INLINE'    - mark the last word as inline
    - 'IMMEDIATE' - mark the last word as immediate
- In addition to the above, c3 also defines some 'system' words (the addresses of system variables and sizes of buffers).
- Everything else in c3 is defined from those.
- On startup, c3 tries to load file "core.c3"; that is where the bootstrap code can be found.
- The default code I have put in core.c3 has a Forth feel to it, but it doesn't have to.
- For example, the standard Forth IF/ELSE/THEN is defined as follows:
    - : if (jmpz) c, here 0 , ; immediate
    - : else  (jmp) c, here swap 0 , here swap ! ; immediate
    - : then here swap ! ; immediate
    - Since they are not built-in, they can be changed by modifying core.c3.
- Strings are both counted and null-terminated.
- The dictionary starts at the end of the MEM area and grows down.
- The dictionary search is not case-sensitive.
- The VARIABLE space is separated from the MEM space.

## Inline words
In c3, an "INLINE" word is like a macro ... when compiling INLINE words, c3 copies the contents of the word (up to, but not including the EXIT) to the target, as opposed to compiling a CALL to the word. This improves performance and often saves space too (especially on a 64-bit system, where the CELL size is 8). Note that if a word might have an embedded 3 (EXIT) in its implementation (like in an address for example), then it should not be marked as INLINE.

## Bootstrapping c3
To bootstrap, c3 has a simple "machine language parser" that can create words in c3's "machine language". The keyword for that is "-ML-". For example, the c3 opcode for "return from subroutine" is 3, and "duplicate the top of the stack" is 12. So in the beginning of core.c3, I define my aliases for the opcodes, like this:

```
...
-ML- EXIT 3 3 -MLX-
...
-ML- DUP 12 3 -MLX- inline
...
```

Note that this approach gives the user the maximum flexibility. Opcode 12 does not have to be "DUP", it could just as easily be "(A--AA)" (or "foo--foo/foo", or "WTF??", or whatever). But DUP is clear and concise, so that is what is used. :)

## The dictionary
- A dictionary entry looks like this:
    - xt:      cell_t
    - flags:   byte (IMMEDIATE=0x01, INLINE=0x02)
    - len:     byte
    - name:    char[NAME_LEN+1] (NULL terminated)

## Default sizes for PC-based systems
- The default NAME_LEN is 13.
- The default MEM_SZ is 128K bytes (code and distionary).
- The default VARS_SZ is 4MB bytes (strings and variables).
- The default STK_SZ is 64 CELLS (data and return stacks).
- The default LSTK_SZ is 30 CELLS (loop stack, multiple of 3).
- The default REGS_SZ is 100 CELLS (register stack, multiple of 10).
- These can be easily changed in the sys-init.ipp file.

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

## c3 Opcode reference
|Opcode|Name|Stack|Description|
| :-- | :-- | :-- | :-- |
|  0 | STOP       | (--)         | Stops the runtime engine|
|  1 | LIT1       | (--B)        | Pushes next BYTE onto the stack|
|  3 | EXIT       | (--)         | Exit subroutine|
|  2 | LIT        | (--N)        | Pushes next CELL onto the stack|
|  4 | CALL       | (--)         | Call: next CELL is address, handles call-tail optimization|
|  5 | JUMP       | (--)         | Jump: next CELL is address|
|  6 | JUMP-Z     | (N--)        | Jump if TOS==0: next CELL is address|
|  7 | JUMP-NZ    | (N--N)       | Jump if TOS!=0: next CELL is address|
|  8 | STORE      | (N A--)      | Store CELL N to address A|
|  9 | CSTORE     | (B A--)      | Store BYTE B to address A|
| 10 | FETCH      | (--)         | Fetch CELL N FROM address A|
| 11 | CFETCH     | (--)         | Fetch BYTE B FROM address A|
| 12 | DUP        | (N--N N)     | Duplicate TOS|
| 15 | DROP       | (A B--A)     | Drop TOS|
| 13 | SWAP       | (A B--B A)   | Swap TOS and NOS|
| 14 | OVER       | (A B--A B A) | Push a copy of NOS|
| 16 | ADD        | (A B--C)     | C: A + B|
| 17 | MULT       | (A B--C)     | C: A * B|
| 18 | SLASHMOD   | (A B--C D)   | C: A mod B, D: A divided by B|
| 19 | SUB        | (A B--C)     | C: A - B|
| 20 | INC        | (A--B)       | Increment TOS|
| 21 | DEC        | (A--B)       | Decrement TOS|
| 22 | LESS-THAN  | (A B--F)     | If A<B, F=1, else F=0|
| 23 | EQUALS     | (A B--F)     | If A=B, F=1, else F=0|
| 24 | GRT-THAN   | (A B--F)     | If A>B, F=1, else F=0|
| 25 | LOG-NOT    | (N--F)       | If A=0, B=1, else F=0|
| 26 | R-TO       | (N--)        | Move N to return stack|
| 27 | R-FETCH    | (--N)        | N: Copy of top of return stack|
| 28 | R-FROM     | (--N)        | N: Top of return stack (popped)|
| 29 | DO         | (T F--)      | Begin a loop from F to T, set I = F|
| 30 | LOOP       | (--)         | Increment I. Jump to beginning if I<T|
| 31 | -LOOP      | (--)         | Decrement I. Jump to beginning if I>T|
| 32 | (I)        | (--)         | Address of I|
| 33 | COM        | (A--B)       | B: Ones-complement of A|
| 34 | AND        | (A B--C)     | C: A bitwise-AND B|
| 35 | OR         | (A B--C)     | C: A bitwise-OR B|
| 36 | XOR        | (A B--C)     | C: A bitwise-XOR B|
| 37 | EMIT       | (B--)        | Output BYTE B to (output_fp)|
| 38 | TIMER      | (--N)        | N: current system time|
| 39 | KEY        | (--B)        | B: next keypress, wait if necessary|
| 40 | QKEY       | (--F)        | If key was pressed, F=1, else F=0|
| 41 | TYPE       | (A N--)      | Output N chars at address A to (output_fp)|
| 42 | TYPEZ      | (A--)        | Output NULL-terminated a address A to (output_fp)|
| 43 | DEFINE     | (--)         | Execute CREATE and set STATE=1|
| 44 | END-DEF    | (--)         | Append EXIT to code, set STATE=0|
| 45 | CREATE     | (--)         | Execute NEXT-WORD, add A to the dictionary|
| 46 | FIND-WORD  | (--XT FL F)  | Execute NEXT-WORD, search for A. Push (XT FL 1) if found, else push only (0) |
| 47 | NEXT-WORD  | (--A)        | A: Address of the next word from the input stream|
| 48 | INC-REG    | (--)         | Increment register X|
| 49 | DEC-REG    | (--)         | Decrement register X|
| 50 | REG        | (--N)        | N: value of register X|
| 51 | REG-INC    | (--N)        | N: value of register X, then decrement it|
| 52 | REG-DEC    | (--N)        | N: value of register X, then increment it|
| 53 | SET-REG    | (N--)        | Set regiser X to TOS|
| 54 | NEW-REGS   | (--)         | Allocate 10 new registers|
| 55 | FREE-REGS  | (--)         | Restore last set of registers|
| 56 | INLINE     | (--)         | Mark the last word in the dictionary as INLINE|
| 57 | IMMEDIATE  | (--)         | Mark the last word in the dictionary as IMMEDIATE|

### Opcodes for PCs (Windows and Linux)
|Opcode|Name|Stack|Description|
| :-- | :-- | :-- | :-- |
| 58 | SYSTEM | (A--)       Call system(a)|
| 59 | FOPEN  | (N M--H)   | N: FileName, M: OpenMode (R/W/A), H: Handle|
| 60 | FCLOSE | (H--)      | Close file with handle H|
| 61 | FREAD  | (A N H--R) | Read N bytes from file H to address A, R: num-read|
| 62 | FWRITE | (A N H--)  | Write N bytes to file H to address A|
| 63 | FLOAD  | (A--)      | Load from file A|

### Opcodes for Development Boards
|Opcode|Name|Stack|Description|
| :-- | :-- | :-- | :-- |
| 58 | PIN-INPUT  | (P--)   | pinMode(P, INPUT)|
| 59 | PIN-OUTPUT | (P--)   | pinMode(P, OUTPUT)|
| 60 | PIN-PULLUP | (P--)   | pinMode(P, INPUT_PULLUP)|
| 61 | DPIN@      | (P--N)  | N = digitalRead(P)|
| 62 | APIN@      | (P--N)  | N = analogRead(P)|
| 63 | DPIN!      | (N P--) | digitalWrite(P, N)|
| 64 | APIN!      | (N P--) | analogWrite(P, N)|

## c3 startup behavior
When c3 starts:
- It tries to open 'core.c3', then '../core.c3'. If successful, c3 loads that file.
  - See file 'core.c3' for definitions defined in that file.
- For every parameter on the command line:
  - If c3 can open the parameter as a file, load it.
  - Else, set the (numeric only) value to a register based on the parameter's position.

## c3 built-in system-information words
|Word|Stack|Description|
| :-- | :-- | :-- |
| version     | (--N) | N: c3 version*100 (e.g. - 147 => v1.47).|
| mem         | (--A) | A: Start address for the MEMORY area.|
| mem-sz      | (--N) | A: The size of the MEMORY area in bytes.|
| vars        | (--A) | A: Start address for the VARIABLES area.|
| vars-sz     | (--N) | N: The size of the VARIABLES area in bytes.|
| regs        | (--A) | A: Start address for the REGISTERS (REGS_SZ CELLs).|
| (vhere)     | (--A) | A: Address of the VHERE variable.|
| (input_fp)  | (--A) | A: Address of the input file handle.|
| (output_fp) | (--A) | A: Address of the output file handle.|
| (here)      | (--A) | A: Address of the HERE variable.|
| (last)      | (--A) | A: Address of the LAST variable.|
| (stk)       | (--A) | A: Address of the stack.|
| (sp)        | (--A) | A: Address of the stack pointer.|
| (rsp)       | (--A) | A: Address of the return stack pointer.|
| (lsp)       | (--A) | A: Address of the loop stack pointer.|
| word-sz     | (--N) | N: The size of a dictionary entry in bytes.|
| base        | (--A) | A: Address of the BASE variable.|
| state       | (--A) | A: Address of the STATE variable.|
| tib         | (--A) | A: Address of TIB (text input buffer).|
| >in         | (--A) | A: Address of >IN.|
| cell        | (--N) | N: size of a CELL in bytes.|

## Adding new opcodes to c3
If for some reason, there is a need/desire to add more opcodes to c3, this describes how it can be accomplished. 

For example, there might be some functionality in a library you want to make available, or maybe there is a bottleneck in performance you want to improve.

Here is the process I use:
- Define the new opcodes(s) to the enum in the *.ipp file for the target system.
- Make sure they have values above the value for IMMEDIATE (57).
- In doUser(), add cases for your new opcodes (also in the *.ipp file).
- There are 2 ways to define them in the dictionary:
  - edit core.c3 and add a -ML- in core.c3 for each new opcode.
  - or, modify loadStartupWords() to add -ML- definitions for the new opcodes.
    - For example: to define opcode 67 as "NEWOP" ... ParseLine("-ML- NEWOP 67 3 -MLX- INLINE");
