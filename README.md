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
|------|----|-----|-----------|
|  0 | (stop)     | (--) | DESC|
|  1 | (lit1)     | (--) | DESC|
|  2 | (lit4)     | (--) | DESC|
|  3 | EXIT       | (--) | DESC|
|  3 | (exit)     | (--) | DESC|
|  4 | (call)     | (--) | DESC|
|  5 | (jmp)      | (--) | DESC|
|  6 | (jmpz)     | (--) | DESC|
|  7 | (jmpnz)    | (--) | DESC|
|  8 | !          | (--) | DESC|
|  9 | c!         | (--) | DESC|
| 10 | @          | (--) | DESC|
| 11 | c@         | (--) | DESC|
| 12 | DUP        | (--) | DESC|
| 15 | DROP       | (--) | DESC|
| 13 | SWAP       | (--) | DESC|
| 14 | OVER       | (--) | DESC|
| 16 | +          | (--) | DESC|
| 17 | *          | (--) | DESC|
| 18 | /MOD       | (--) | DESC|
| 19 | -          | (--) | DESC|
| 20 | 1+         | (--) | DESC|
| 21 | 1-         | (--) | DESC|
| 22 | <          | (--) | DESC|
| 23 | =          | (--) | DESC|
| 24 | >          | (--) | DESC|
| 25 | 0=         | (--) | DESC|
| 26 | >R         | (--) | DESC|
| 27 | R@         | (--) | DESC|
| 28 | R>         | (--) | DESC|
| 29 | DO         | (--) | DESC|
| 30 | LOOP       | (--) | DESC|
| 31 | -LOOP      | (--) | DESC|
| 32 | (I)        | (--) | DESC|
| 33 | COM        | (--) | DESC|
| 34 | AND        | (--) | DESC|
| 35 | OR         | (--) | DESC|
| 36 | XOR        | (--) | DESC|
| 37 | EMIT       | (--) | DESC|
| 38 | TIMER      | (--) | DESC|
| 39 | KEY        | (--) | DESC|
| 40 | ?KEY       | (--) | DESC|
| 41 | (TYPE)     | (--) | DESC|
| 41 | TYPE       | (--) | DESC|
| 42 | TYPEZ      | (--) | DESC|
| 43 | (define)   | (--) | DESC|
| 44 | (end-word) | (--) | DESC|
| 45 | CREATE     | (--) | DESC|
| 46 | '          | (--) | DESC|
| 47 | NEXT-WORD  | (--) | DESC|
| 48 | (iX)       | (--) | DESC|
| 49 | (dX)       | (--) | DESC|
| 50 | (rX)       | (--) | DESC|
| 51 | (rX-)      | (--) | DESC|
| 52 | (rX+)      | (--) | DESC|
| 53 | (sX)       | (--) | DESC|
| 54 | +REGs      | (--) | DESC|
| 55 | -REGS      | (--) | DESC|
| 56 | INLINE     | (--) | DESC|
| 57 | IMMEDIATE  | (--) | DESC|

### Opcodes for PCs (Windows and Linux)
|Opcode|Name|Stack|Description|
|------|----|-----|-----------|
| 58 | SYSTEM |(--) | DESC|
| 59 | FOPEN  |(--) | DESC|
| 60 | FCLOSE |(--) | DESC|
| 61 | FREAD  |(--) | DESC|
| 62 | FWRITE |(--) | DESC|
| 63 | (load) |(--) | DESC|

### Opcodes for Development Boards
|Opcode|Name|Stack|Description|
|------|----|-----|-----------|
| 58 | PIN-INPUT  | (n--)     | DESC|
| 59 | PIN-OUTPUT | (n--)     | DESC|
| 60 | PIN-PULLUP | (n--)     | DESC|
| 61 | DPIN@      | (n1--n2)  | DESC|
| 62 | APIN@      | (n1--n2)  | DESC|
| 63 | DPIN!      | (n1 n2--) | DESC|
| 64 | APIN!      | (n1 n2--) | DESC|

## c3 startup behavior
When c3 starts:
- It tries to open 'core.c3', then '../core.c3'. If successful, c3 loads that file.
  - See file 'core.c3' for definitions defined in that file.
- For every parameter on the command line:
  - If c3 can open the parameter as a file, load it.
  - Else, set the (numeric only) value to a register based on the parameter's position.

## c3 built-in system-information words
|Word|Stack|Description|
|----|-----|-----------|
| version  | (--n)   | n: c3 version*100 (e.g. - 147 => v1.47).|
| mem      | (--a)   | a: Start address for the MEMORY area.|
| mem-sz   | (--n)   | a: The size of the MEMORY area in bytes.|
| vars     | (--a)   | a: Start address for the VARIABLES area.|
| vars-sz  | (--n)   | n: The size of the VARIABLES area in bytes.|
| regs     | (--a)   | a: Start address for the REGISTERS (REGS_SZ CELLs).|
| (vhere)  | (--a)   | a: Address of the VHERE variable.|
| (here)   | (--a)   | a: Address of the HERE variable.|
| (last)   | (--a)   | a: Address of the LAST variable.|
| (stk)    | (--a)   | a: Address of the stack.|
| (sp)     | (--a)   | a: Address of the stack pointer.|
| (rsp)    | (--a)   | a: Address of the return stack pointer.|
| (lsp)    | (--a)   | a: Address of the loop stack pointer.|
| word-sz  | (--n)   | n: The size of a dictionary entry in bytes.|
| base     | (--a)   | a: Address of the BASE variable.|
| state    | (--a)   | a: Address of the STATE variable.|
| tib      | (--a)   | a: Address of TIB (text input buffer).|
| >in      | (--a)   | a: Address of >IN.|
| cell     | (--n)   | n: size of a CELL in bytes.|

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
