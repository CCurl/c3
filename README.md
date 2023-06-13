# c3 - A minimal, stack-based, VM written in C.

## What is c3?
- c3 is a stack-based VM whose "CPU" does not have alot of opcodes.
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
- Four of the operations are exposed as c3 words:
    - ':'         - define a c3 word
    - ';'         - end word definition
    - 'INLINE'    - mark the last word as inline
    - 'IMMEDIATE' - mark the last word as immediate
- In addition to the above, c3 also defines some 'system' words (the addresses of system variables and sizes of buffers).
- Everything else in c3 can be defined from those.
- On startup, c3 tried to load file "core.c3"; that is where the bootstrap code can be found.
- The default code I have put in core.c3 has a Forth feel to it, but it doesn't have to.
- For example, the standard Forth IF/THEN is defined as follows:
    - : if (jmpz) c, here 0 , ; immediate
    - : then here swap ! ; immediate
    - Since they are not built-in, they can be changed by modifying core.c3.
- Strings are both counted and null-terminated.
- The dictionary starts at the end of the MEM area and grows down.
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

## c3 Base system reference
When c3 starts, it can take a filename as the startup file/program.

c3 tries to open 'core.c3', then '../core.c3'. If successful, it loads that first.

NOTE: Most of these words are defined in 'core.c3'.
- The below list is not complete.
- See file core.c3 for full details.
```
*** MATH ***
+        (a b--c)          Addition
-        (a b--c)          Subtraction
*        (a b--c)          Multiplication
/mod     (a b--r q)        r: modulo(a,b), q: quotient(a,b)

*** STACK ***
1+       (a--b)            Increment TOS
1-       (a--b)            Decrement TOS
drop     (a b--a)          Drop TOS
dup      (a--a a)          Duplicate TOS
?dup     (a--a a?)         Duplicate TOS if TOS <> 0
over     (a b--a b a)      Copy NOS
swap     (a b--b a)        Swap TOS and NOS

*** INPUT/OUTPUT ***
[0-x]*    (--N)            Input N as a number in the current BASE.
#[0-9]*   (--N)            Input N as a decimal number.
$[0-f]*   (--N)            Input N as a hexadecimal number.
%[0-1]*   (--N)            Input N as a binary number.
'x'       (--N)            Input N as the ascii value of 'x'.
number?   (S--N F|F)       Parse string S as a number. N: number if F=1.
emit      (C--)            Output C as a character.
next-word (--A L)          A: the next word from the input stream, L: length.
key       (--C)            C: Next keyboard char, wait if no char available.
?key      (--F)            F: FALSE if no char available, else TRUE.

*** FILES ***
        NOTE: By default, these are defined in PC-based systems only.
              Boards that support LittleFS can have them as well.
fopen    (n m--fh)         n: name, m: mode (eg - rt), fh: file-handle.
fclose   (--fh)            fh: file-handle.
fread    (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars read.
fwrite   (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars written.
(load)   (str--)           str: a counted string/filename to load
(input-fp)  (--a)          a: address of the input-file pointer (used by "(load)").
(output-fp) (--a)          a: address of the output-file pointer (redirects EMIT).

*** LOGICAL ***
=        (a b--f)          Equality.
<        (a b--f)          Less-than.
>        (a b--f)          Greater-than.
0=       (n--f)            Logical NOT.

*** MEMORY ***
@        (a--n)            n: CELL at address a.
c@       (a--b)            b: BYTE at address a.
!        (n a--)           Store CELL n to address a.
c!       (b a--)           Store BYTE b to address a.
+!       (N A--)           Add N to CELL at A
++       (A--)             Incement value of CELL at A
--       (A--)             Decement value of CELL at A
c++      (A--)             Incement value of BYTE at A

*** WORDS and FLOW CONTROL ***
: word   (--)              Begin definition of word. 
: T[0-9] (--)              Begin definition of a temporary word.
        NOTE: T0-T5 are normal, T6-T8 are INLINE, and T9 is IMMEDIATE.
;        (--)              End current definition.
create x (--)              Create a definition for word "x".
for      (N--)             Begin a FOR loop. Set I = N.
next     (--)              Decrement I. If I>0, jump to for.
do       (T F--)           Begin DO/LOOP loop.
i        (--n)             n: the loop index variable I.
(i)      (--a)             a: address of the loop index variable I.
loop     (--)              Increment I, Jump to DO if I < T.
-loop    (--)              Decrement I, Jump to DO if I > T.
' xxx    (--xt fl f)       Find word 'xxx' in the dictionary.
        NOTE: Words like IF/THEN/EXIT and BEGIN/UNTIL are not in the base c3.
              They are just words that are defined in core.c3

*** REGISTERS ***
+regs    (--)              Save the current registers (register-base += 10).
rX       (--N)             N: the value of register #X.
rX+      (--N)             N: the value of register #X. Then increment register X.
rX-      (--N)             N: the value of register #X. Then decrement register X.
sX       (N--)             Set register #X to N.
iX       (--)              Increment register #X.
dX       (--)              Decrement register #X.
-regs    (--)              Restore the last saved registers (register-base -= 10).
        NOTES: 1. The registers are stored in an array/stack with a "register-base".
               2. +regs simply adds 10 to "register-base", so it is a very efficient operation.

*** SYSTEM ***
version  (--n)   n: c3 version*100 (e.g. - 147 => v1.47).
mem      (--a)   a: Start address for the MEMORY area.
mem-sz   (--n)   a: The size of the MEMORY area in bytes.
vars     (--a)   a: Start address for the VARIABLES area.
vars-sz  (--n)   n: The size of the VARIABLES area in bytes.
regs     (--a)   a: Start address for the REGISTERS (10 CELLs).
(vhere)  (--a)   a: Address of the VHERE variable.
(here)   (--a)   a: Address of the HERE variable.
(last)   (--a)   a: Address of the LAST variable.
(stk)    (--a)   a: Address of the stack.
(sp)     (--a)   a: Address of the stack pointer.
(rsp)    (--a)   a: Address of the return stack pointer.
(lsp)    (--a)   a: Address of the loop stack pointer.
word-sz  (--n)   n: The size of a dictionary entry in bytes.
base     (--a)   a: Address of the BASE variable.
state    (--a)   a: Address of the STATE variable.
tib      (--a)   a: Address of TIB (text input buffer).
>in      (--a)   a: Address of >IN.
cell     (--n)   n: size of a CELL in bytes.
```

## Adding new opcodes to c3
If for some reason, there is a need/desire to add more opcodes to c3, this describes how it can be accomplished. There might be some functionality in a library you want to make available, or maybe there is a bottleneck in performance you want to improve.
- Define the new opcodes(s) to the enum in the *.ipp file for the target system.
- Make sure they have values above the value for IMMEDIATE (57).
- In doUser(), add cases for your new opcodes (also in the *.ipp file).
- There are 2 ways to define them in the dictionary:
  - Edit core.c3 and add a -ML- in core.c3 for each new opcode.
  - Modify loadStartupWords() to add defines for the new opcodes.
    - For example: to define opcode 67 as "NEWOP" ... ParseLine("-ML- NEWOP 67 3 -MLX- INLINE");
