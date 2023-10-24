# c3 - A stack-based VM written in C.

## What is c3?
- c3 is a stack-based, byte-coded VM.
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
- Strings in c3 are null-terminated, not counted.
- The user can add counted strings if desired.
- There are 2 separate memory areas: CODE and VARIABLE.
- The dictionary starts at the end of the CODE area and grows down.
- The dictionary search is not case-sensitive.

## Registers
c3 exposes 10 "virtual registers", r0 thru r9. There are 8 register operations: +regs, rX, rX+, rX-, sX, iX, dX, -regs.
The names of the register words are case-sensitive: (r0-r9, not R0-R9).

**Note:** The support for registers is built into c3, so they do NOT show up in "WORDS".

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
- The names of the temporary words are case-sensitive (T0-T9, not t0-t9).

An example usage of temporary words:
```
\ The Babylon square root algorithm
: T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - 0= until nip ;
: sqrt ( n--0|sqrt ) dup 0 > if T0 else drop 0 then ;
```

## Inline words
In c3, an "INLINE" word is like a macro ... when compiling INLINE words, c3 copies the contents of the word (up to, but not including the first EXIT) to the target, as opposed to compiling a CALL to the word. This improves performance and often saves space as well. This is especially true on a 64-bit system, where the CELL size is 8. **Note that if a word might have an embedded 3 (EXIT) in its implementation (like an address for example), then it should not be marked as INLINE.**

## Notes on output formatting in ZTYPE and '."':
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

```
  -ML- INLINE 47 0 3 -MLX-
  -ML- INLINE 47 1 3 -MLX-
  -ML- INLINE 47 6 3 -MLX-
  -ML- INLINE 47 7 3 -MLX-
```

c3 also defines some 'system-info' words (the addresses of system variables and sizes of buffers).

Everything in c3 is defined from those. See file 'sys-load.h' for details.

Note that this approach gives the user the maximum flexibility. Opcode 12 does not have to be called "DUP", it could just as easily be "(N--NN)" (or "foo--foo/foo", or whatever). But DUP is clear and concise, so that its default name. :)

## The dictionary
### NOTE: c3 does NOT do a case-sensitive dictionary search.
- A dictionary entry looks like this:
  - xt:     cell_t               (either 32-bit or 64-bit)
  - flags:  byte                 (IMMEDIATE=$01, INLINE=$02)
  - len:    byte
  - name:   char[NAME_LEN+1]     (NULL terminated)

## Default sizes for PC-based systems
- The default NAME_LEN is 13.
- The default CODE_SZ is 128K bytes (code and dictionary).
- The default VARS_SZ is  4MB bytes (strings and variables).
- The default STK_SZ  is   64 CELLS (data and return stacks).
- The default LSTK_SZ is   30 CELLS (loop stack, multiple of 3).
- The default REGS_SZ is  100 CELLS (register stack, multiple of 10).
- These are defined in the sys-init.h file.

## Building c3:
- Windows: there is a c3.sln file for Visual Studio
  - Use the x86 configuration (32-bit)
- Linux: there is a makefile
  - It uses clang -O3 and builds a 64-bit version
- Apple: I do not have an Apple, so I cannot build for Apples
  - But c3 is minimal enough that it should be easy to port to an Apple system
- Arduino: there is a c3.ino file
  - I use the Arduino IDE v2.0
  - Edit the section where isBOARD is defined to set the configuration for the board
  - For the RPI Pico and Teensy 4.0, I use:
    - CODE_SZ:   64K
    - VARS_SZ:   96K
    - STK_SZ:    32
    - LSTK_SZ:   30
    - REGS_SZ;  100
    - NAME_LEN:  13

## c3 Opcode / Word reference

### NOTE: See the sys-load.h file for the implementation of the words defined in the base c3 system.

|Opcode |Word        |Stack         |Description|
| :--   | :--        | :--          | :-- |
|  0    | STOP       | (--)         | Stops the runtime engine|
|  1    | LIT1       | (--B)        | Pushes next BYTE onto the stack|
|  2    | LIT        | (--N)        | Pushes next CELL onto the stack|
|  3    | EXIT       | (--)         | Exit subroutine|
|  4    | CALL       | (--)         | Call: next CELL is address, handles call-tail optimization|
|  5    | JUMP       | (--)         | Jump: next CELL is address|
|  6    | JUMPZ      | (N--)        | Jump if TOS==0: next CELL is address|
|  7    | JUMPNZ     | (N--N)       | Jump if TOS!=0: next CELL is address (no POP!)|
|  8    | !          | (N A--)      | Store CELL N to address A|
|  9    | C!         | (B A--)      | Store BYTE B to address A|
| 10    | @          | (A--N)       | Fetch CELL N FROM address A|
| 11    | C@         | (A--B)       | Fetch BYTE B FROM address A|
| 12    | DUP        | (N--N N)     | Duplicate TOS|
| 15    | DROP       | (A B--A)     | Drop TOS|
| 13    | SWAP       | (A B--B A)   | Swap TOS and NOS|
| 14    | OVER       | (A B--A B A) | Push a copy of NOS|
| 16    | +          | (A B--C)     | C: A + B|
| 17    | *          | (A B--C)     | C: A * B|
| 18    | /MOD       | (A B--M Q)   | M: A modulo B, Q: A divided by B|
| 19    | -          | (A B--C)     | C: A - B|
| 20    | 1+         | (A--B)       | Increment TOS|
| 21    | 1-         | (A--B)       | Decrement TOS|
| 22    | <          | (A B--F)     | If A<B, F=1, else F=0|
| 23    | =          | (A B--F)     | If A=B, F=1, else F=0|
| 24    | >          | (A B--F)     | If A>B, F=1, else F=0|
| 25    | 0= , NOT   | (N--F)       | If N=0, F=1, else F=0|
| 26    | >R         | (N--)        | Move N to return stack|
| 27    | R@         | (--N)        | N: Copy of top of return stack|
| 28    | R>         | (--N)        | N: Top of return stack (popped)|
| 29    | DO         | (T F--)      | Begin a loop from F to T, set I = F|
| 30    | LOOP       | (--)         | Increment I. Jump to DO if I<T|
| 31    | -LOOP      | (--)         | Decrement I. Jump to DO if I>T|
| 32    | (I)        | (--A)        | A: Address of I, the loop index|
| 33    | COM        | (A--B)       | B: Ones-complement of A|
| 34    | AND        | (A B--C)     | C: A bitwise-AND B|
| 35    | OR         | (A B--C)     | C: A bitwise-OR B|
| 36    | XOR        | (A B--C)     | C: A bitwise-XOR B|
| 37    | TYPE       | (A N--)      | EMIT N chars from address A (Standard Forth TYPE)|
| 38    | ZTYPE      | (A--)        | Output formatted chars at address A to (output_fp)|
| 39,X  | iX         | (--)         | Increment register X|
| 30,X  | dX         | (--)         | Decrement register X|
| 41,X  | rX         | (--N)        | N: value of register X|
| 42,X  | rX+        | (--N)        | N: value of register X, then decrement it|
| 43,X  | rX-        | (--N)        | N: value of register X, then increment it|
| 44,X  | sX         | (N--)        | Set regiser X to TOS|
| 45    | +REGS      | (--)         | Allocate 10 new registers (add 10 to REG-BASE)|
| 46    | -REGS      | (--)         | Restore last set of registers (subtract 10 from REG-BASE)|

### System opcodes are 2-bytes, starting with 47
|Opcode |Word        |Stack         |Description|
| :--   | :--        | :--          | :-- |
| 47,0  | INLINE     | (--)         | Mark the last word in the dictionary as INLINE|
| 47,1  | IMMEDIATE  | (--)         | Mark the last word in the dictionary as IMMEDIATE|
| 47,2  | (.)        | (I--)        | Perform ITOA on I, then QTYPE it (no trailing space)|
| 47,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 47,4  | ITOA       | (I--SZ)      | Convert I to string SZ in the current BASE|
| 47,5  | ATOI       | (SZ--I F)    | Convert string SZ to I. If successful, (I 1) else only (0)|
| 47,6  | :          | (--)         | Execute NEXT-WORD, add A to the dictionary, set STATE=1|
| 47,7  | ;          | (--)         | Compile EXIT to code,then set STATE=0|
| 47,8  | CREATE     | (--)         | Execute NEXT-WORD, add A to the dictionary|
|       |            |              | -- NOTE: when new word is executed, pushes VHERE |
|       |            |              | -- NOTE: must use with DOES> or compile EXIT |
| 47,9  | '          | (--XT FL F)  | Execute NEXT-WORD, search for A. If found, (XT FL 1), else only (0)|
| 47,10 | NEXT-WORD  | (--A N)      | A: Address of the next word from the input stream, N: length of A|
| 47,11 | TIMER      | (--N)        | N: current system time in milliseconds|
| 47,12 | C,         | (B--)        | Standard Forth "C,"|
| 47,13 | ,          | (N--)        | Standard Forth ","|
| 47,14 | KEY        | (--B)        | B: next keypress, wait if necessary|
| 47,15 | ?KEY       | (--F)        | If key was pressed, F=1, else F=0|
| 47,16 | EMIT       | (C--)        | Output CHAR C to (output_fp)|
| 47,17 | QTYPE      | (A--)        | Quick-type: Output string A to (output_fp), no formatting|

### String opcodes are 2-bytes, starting with 48
|Opcode |Word        |Stack         |Description|
| :--   | :--        | :--          | :-- |
| 48,0  | S-TRUNC    | (S--)        | Truncate string S|
| 48,1  | LCASE      | (C1--C2)     | C2: char C1 converted to lowercase|
| 48,2  | UCASE      | (C1--C2)     | C2: char C1 converted to uppercase|
| 48,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 48,4  | S-CPY      | (D S--)      | Copy string S to string D|
| 48,5  | S-CAT      | (D S--)      | Concatenate string S to string D|
| 48,6  | S-CATC     | (D C--)      | Concatenate char C to string D|
| 48,7  | S-LEN      | (S--N)       | N: length of string S|
| 48,8  | S-EQ       | (S1 S2--F)   | F: 1 if S1 = S2, else 0 (case sensitive)|
| 48,9  | S-EQI      | (S1 S2--F)   | F: 1 if S1 = S2, else 0 (not case sensitive)|

### Floating point opcodes are 2-bytes, starting with 49
|Opcode |Word        |Stack         |Description|
| :--   | :--        | :--          | :-- |
| 49,0  | F+         | (F1 F2--F3)  | Add F1 and F2, leaving F3|
| 49,1  | F-         | (F1 F2--F3)  | Subtract F2 from F1, leaving F3|
| 49,2  | F*         | (F1 F2--F3)  | Multiply F1 and F2, leaving F3|
| 49,3  | **UNUSED** |              | Not used so words can be marked as INLINE|
| 49,4  | F/         | (F1 F2--F3)  | Divide F1 by F2, leaving F3|
| 49,5  | F=         | (F1 F2--F)   | F: 1 if F1 = F2, else 0|
| 49,6  | F<         | (F1 F2--F)   | F: 1 if F1 < F2, else 0|
| 49,7  | F>         | (F1 F2--F)   | F: 1 if F1 > F2, else 0|
| 49,8  | F2I        | (F1--N)      | Convert double F1 into an integer N|
| 49,9  | I2F        | (N--F1)      | Convert integer N into a double F1|
| 49,10 | F.         | (F1--)       | Output F1 using the "%g" C format string|
| 49,11 | SQRT       | (F1--F2)     | F2: the square root of F1
| 49,12 | TANH       | (F1--F2)     | F2: the hyperbolic tangent of F1

### Opcodes for PCs (Windows and Linux)
|Opcode|Word    |Stack       |Description|
| :--  | :--    | :--        | :-- |
| 100  | SYSTEM | (A--)      | Call system(A)|
| 101  | FOPEN  | (NM MD--H) | NM: FileName, MD: Mode (e.g. - "rt"), H: Handle|
| 102  | FCLOSE | (H--)      | Close file with handle H|
| 103  | FREAD  | (A N H--R) | Read N bytes from file H to address A, R: num-read, 0 means EOF|
| 104  | FWRITE | (A N H--)  | Write N bytes to file H from address A|
| 105  | (LOAD) | (NM--)     | Load from file NM|

### Opcodes for Development Boards
|Opcode|Word        |Stack    |Description|
| :--  | :--        | :--     | :-- |
| 110  | PIN-INPUT  | (P--)   | pinMode(P, INPUT)|
| 111  | PIN-OUTPUT | (P--)   | pinMode(P, OUTPUT)|
| 112  | PIN-PULLUP | (P--)   | pinMode(P, INPUT_PULLUP)|
| 113  | DPIN@      | (P--N)  | N = digitalRead(P)|
| 114  | APIN@      | (P--N)  | N = analogRead(P)|
| 115  | DPIN!      | (N P--) | digitalWrite(P, N)|
| 116  | APIN!      | (N P--) | analogWrite(P, N)|

### Built-in c3 system-information words
|Word           |Stack     |Description|
| :--           | :--      | :-- |
| VERSION       | (--N)    | N: c3 version*100 (e.g. - 147 => v1.47).|
| CODE          | (--A)    | A: Start address for the CODE area.|
| CODE-SZ       | (--N)    | A: The size of the CODE area in bytes.|
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

### Other built-1n c3 words
| WORD |STACK |Description|
| :-- | :-- | :--|
| (LIT)        | (--N)          | OPCODE for LITERAL (INLINE) |
| (EXIT)       | (--N)          | OPCODE for EXIT (INLINE) |
| (CALL)       | (--N)          | OPCODE for CALL (INLINE) |
| (JMP)        | (--N)          | OPCODE for JMP |
| (JMPZ)       | (--N)          | OPCODE for JMPZ |
| (JMPNZ)      | (--N)          | OPCODE for JMPNZ |
| (STORE)      | (--N)          | OPCODE for STORE |
| (FETCH)      | (--N)          | OPCODE for FETCH |
| (DUP)        | (--N)          | OPCODE for DUP |
| (ZTYPE)      | (--N)          | OPCODE for ZTYPE |
| \\           | (--)           | Line comment |
| [            | (--)           | Set STATE=0 |
| ]            | (--)           | SET STATE=1 |
| LAST         | (--A)          | Address of the most recently created WORD |
| HERE         | (--A)          | Address of the next free byte in the CODE area |
| code-end     | (--A)          | Address of the end of the CODE area |
| vars-end     | (--A)          | Address of the end of the VARS area |
| ++           | (A--)          | Invrement CELL at A |
| --           | (A--)          | Decrement CELL At A |
| VHERE        | (--A)          | Address of the next free byte in the VARS area |
| ALLOT        | (N--)          | Add N to VHERE |
| vc,          | (B--)          | C, to the VARS area |
| v,           | (N--)          | , to the VARS area |
| CELLS        | (A--B)         | B: A * CELL |
| DOES>        | (--)           | Defines the behavior of words created using "CREATE" |
| CONSTANT nm  | (N--)          | Defines word "nm" that pushes N when executed. |
| VARIABLE nm  | (--)           | Defines word "nm" that pushes an addess when executed. ALLOTs a CELL  |
| val nm1      | (--)           | Defines "nm1" to push a number onto the stack when executed. |
| >val nm2     | (N--)          | Defines "nm2" to set N to nm1 |
| (val) nm3    | (--A)          | Defines "nm3" to push the address of nm1 |
| :NONAME      | (--A)          | A: HERE. Sets STATE=1 |
| EXEC         | (A--)          | Jumps to address A |
| IF           | (F--)          | If F=0, jump to ELSE or THEN |
| ELSE         | (--)           | If F<>0 (from IF), jump here  |
| THEN         | (--)           | End of IF or IF/ELSE |
| BEGIN        | (--)           | Start a LOOP |
| UNTIL        | (F--)          | If F<>0 jump to BEGIN |
| AGAIN        | (--)           | Jump to BEGIN |
| WHILE        | (F--)          | If F=0, jump to instruction after REPEAT |
| REPEAT       | (--)           | Jump to BEGIN (resolves WHILE) |
| FOR          | (N--)          | Begin a loop of N iterations |
| NEXT         | (--)           | Next iteration |
| -if          | (F--F)         | Non-destructive IF |
| -until       | (F--F)         | Non-destructive UNTIL |
| -while       | (F--F)         | Non-destructive WHILE |
| TUCK         | (A B--B A B)   | Tuck TOS before NOS |
| NIP          | (A B--B)       | Drop NOS |
| 2DUP         | (A B--A B A B) | Duplicate top 2 items |
| 2DROP        | (A B--)        | Drop top 2 items |
| ?DUP         | (F--F?)        | If F<>0, duplicate it |
| /            | (A B--C)       | C: A/B |
| mod          | (A B--C)       | C: A modulo B |
| +!           | (N A--)        | Add N to value at A |
| c++          | (A--)          | Increment BYTE at A |
| 2*           | (A--B)         | B: A*2 |
| 2/           | (A--B)         | B: A/2 |
| 2+           | (A--B)         | B: A+2 |
| <=           | (A B--F)       | F: if A<=B then 1 else 0 |
| >=           | (A B--F)       | F: if A>=B then 1 else 0 |
| <>           | (A B--F)       | F: if A<>B then 1 else 0 |
| RDROP        | (R:A--)        | Drop top of RETURN stack |
| ROT          | (A B C--B C A) | Rotate A to TOS |
| -ROT         | (A B C--C A B) | Rotate C before A |
| (            | (--)           | Skip until ')' or EOL |
| bl           | (--C)          | C: 32 (SPACE) |
| tab          | (--C)          | C: 9 (TAB) |
| cr           | (--)           | Output a NL (CR/LF) |
| space        | (--)           | Output a SPACE |
| .            | (N--)          | Print N in the current BASE |
| NEGATE       | (A--B)         | B: -A |
| ABS          | (A--B)         | B: if A<0 then -A else A |
| min          | (A B--C)       | C: if A<B then A else B |
| max          | (A B--C)       | C: if A>B then A else B |
| btw          | (N L H--F)     | F: if N is between L and H then 1 else 0   |
| I            | (--N)          | Index of the current loop |
| J            | (--N)          | Index of the next outer loop |
| +I           | (N--)          | Add N to the index (+1 is still added at the end) |
| UNLOOP       | (--)           | Unwind the loop stack (does NOT exit the loop) |
| 0SP          | (--)           | Empty/reset the stack |
| DEPTH        | (--N)          | N: the number of items on the stack |
| .S           | (--)           | Output the stack using the current BASE |
| dump         | (F N--)        | Output N bytes starting from F in the current BASE |
| " str"       | (--A)          | A: the address of str |
| ." hi"       | (--)           | Output "hi" |
| .word        | (A--)          | Output the name of the word at A |
| word-len     | (A--N)         | N: the length of the word at A |
| WORDS        | (--C           | Output the words in the dictionary |
| BINARY       | (--)           | Set the BASE to 2 |
| DECIMAL      | (--)           | Set the BASE to 10 |
| HEX          | (--)           | Set the BASE to 16 |
| ?            | (A--)          | Output the CELL value at A |
| LSHIFT       | (A B--C)       | C: A << B |
| RSHIFT       | (A B--C)       | C: A >> B |
| LOAD <FN>    | (--)           | Load from file "fn" |
| LOAD-ABORT   | (--)           | Stop loading from file |
| LOADED?      | (A B C--)      | Stops loading from file if C <> 0 |
| MARKER       | (--)           | Remember HERE, LAST, and VHERE |
| FORGET       | (--)           | Reset HERE, LAST, and VHERE to remembered values |
| FORGET-1     | (--)           | Remove the most recent entry from the dictionary |

## c3 startup behavior
When c3 starts:
- For every parameter on the command line:
  - If c3 can open the parameter as a file, queue it up to be loaded.
  - Else, set the (numeric only) value to a register based on the parameter's position.

## Adding new opcodes to c3
If for some reason, there is a need/desire to add more opcodes to c3, this section describes how it can be accomplished. 

For example, there might be some functionality in a library you want to make available, or maybe there is a bottleneck in performance you want to improve.

Here is the process:

- For a global opcode:
  - In c3.c, add the new opcode(s) to the appropriate enum.
  - In c3.c, add a NCASE to run() to for each new opcode.
  - In sys-load.h, add a "-ML-" line to LoadStartupWords() for each new opcode.
  - Update your README.md.

- For a target-specific opcode:
  - All work is done in the target's *.h file (e.g. - sys-pc.h).
  - Add the new opcodes(s) to the enum.
  - Target-specific opcodes should have values above 100.
  - Edit LoadStartupWords() and add a "-ML-" line for each new opcode.
  - For example: to define opcode 120 as "NEWOP" ... ParseLine("-ML- NEWOP 120 3 -MLX- INLINE");
  - In doUser(), add cases for the new opcode(s).
  - Update your README.md.
