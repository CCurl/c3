# c3 - A minimal Forth-like VM written in C.

The goals for this project are as follows:
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To provide as much flexibility to the programmer as possible.
- To be able to run on both Windows and Linux (and Apple).
- To be deployable to development boards via the Arduino IDE.

## Notes:
- This is NOT an ANSI-standard Forth system.
- The Linux version is 64-bit but can also be 32-bit.
- This is a byte-coded implementation.
- This is a toolkit to create any environment the programmer desires.
- There are 64 operations built into the base executable.
- Six of the operations are exposed as c3 words:
    - ':'         - define a c3 word
    - ';'         - end word definition
    - 'INLINE'    - mark the last word as inline
    - 'IMMEDIATE' - mark the last word as immediate
    - '-ML-'      - define a c3 "Machine Language" word
    - '//'        - comment to end of line
- Additionally, c3 system information is exposed by c3.
- Everything is built using those primitives (see core.c3).
- For example, the standard Forth IF/THEN is defined as follows:
    - : if (jmpz) c, here 0 , ; immediate
    - : then here swap ! ; immediate
    - Since the words are not built-in, the programmer has total control over them.
- c3 provides 10 "virtual registers", r0 thru r9.
- c3 provides 10 temporary words, T0 thru T9.
- The VARIABLE space is separated from the CODE space.
- Counted strings are also null-terminated.
- The dictionary starts at the end of the CODE area and grows down.
- The WORD length is defined by NAME_LEN (in c3.c) as 13 chars.

## The dictionary
- A dictionary entry looks like this:
    - xt:      cell_t
    - flags:   byte
    - len:     byte
    - name:    char[NAME_LEN+1] (NULL terminated)

## Registers
c3 provides 10 "virtual registers", r0 thru r9.
There are 8 register operations: +regs, rX, rX+, rX-, sX, iX, dX, -regs.
- +regs   allocate 10 new current registers.
- r4      push register #4.
- r4+     push register #4, then increment it.
- r4-     push register #4, then decrement it.
- s4      set register #4 from TOS.
- i4      increment register #4.
- d4      decrement register #4.
- -regs   restore the registers to their previous values.

An example usage of registers:
```
   : btw  ( n l h--f )  +regs s3 s2 s1  r2 r1 <   r1 r3 <  and -regs ;
   : btwi ( n l h--f )  +regs s3 s2 s1  r2 r1 <=  r1 r3 <= and -regs ;
```

## Temporary words
c3 provides 10 temporary words, T0 thru T9.
- Defining a temporary word does not add an entry to the dictionary.
- Temporary words are intended to be helpful in factoring code.
- A temporary word can be redefined as often as desired.
- When redefined, code references to the previous definition are unchanged.
- A temporary word cannot be INLINE or IMMEDIATE.

An example usage of temporary words:
```
   \ The Babylon square root algorithm
   : T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - 0= until nip ;
   : sqrt ( n--0|sqrt ) dup 0 > if T0 else drop 0 then ;
```

## Building c3:
- Windows: there is a c3.sln file
  - use the x86 configuration
- Linux: there is a 'make' script
  - it uses clang and builds a 64-bit version
  - you can also use gcc or -m32 to build a 2 bit version
- Apple: I do not have an Apple, so I cannot build for Apple
  - But c3 is minimal enough that it should be easy to port to an Apple system
- Arduino: there is a c3.ino file (FUTURE)
  - I use the Arduino IDE v 1.8 or 2.0

## c3 Base system reference
```
NOTE: Since this is a toolkit, many of the core words are defined in file 'core.c3'

*** MATH ***
+        (a b--c)          Addition
-        (a b--c)          Subtraction
*        (a b--c)          Multiplication
/mod     (a b--r q)        r: modulo(a,b), q: quotient(a,b)
+!       (N A--)           Add N to CELL at A

*** STACK ***
1+       (a--b)            Increment TOS
1-       (a--b)            Decrement TOS
drop     (a b--a)          Drop TOS
dup      (a--a a)          Duplicate TOS
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
fopen    (n m--fh)         n: name, m: mode (eg - rt), fh: file-handle.
fclose   (--fh)            fh: file-handle.
fread    (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars read.
fwrite   (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars written.
(load)   (str--)           str: a counted string/filename to load
(input-fp)  (--a)          a: address of the input-file pointer (PC only; used by "(load)").
(output-fp) (--a)          a: address of the output-file pointer (PC only; redirects EMIT).

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

*** WORDS and FLOW CONTROL ***
: word   (--)              Begin definition of word. 
: T[0-9] (--)              Begin definition of a temporary word.
;        (--)              End current definition.
create x (--)              Creates a definition for x word.
do       (T F--)           Begin DO/LOOP loop.
(i)      (--a)             a: address of the index variable.
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
version  (--n)   n: c3 version*10 (e.g. - 4 => v0.4)
(exit)   (--n)   n: The byte-code value for EXIT.
(jmp)    (--n)   n: The byte-code value for JMP.    On execute: (?--?)  JUMP
(jmpz)   (--n)   n: The byte-code value for JMPZ.   On execute: (N--)   JUMP if N==0 (Consumes N)
(jmpnz)  (--n)   n: The byte-code value for JMPNZ.  On execute: (N--N)  JUMP if N!=0
(call)   (--n)   n: The byte-code value for CALL.
(lit1)   (--n)   n: The byte-code value for LIT1.
(lit4)   (--n)   n: The byte-code value for LIT4.
mem      (--a)   a: Start address for the MEMORY area.
mem-end  (--a)   a: End address for the MEMORY area.
vars     (--a)   a: Start address for the VARIABLES area.
vars-end (--a)   a: End address for the VARIABLES area.
regs     (--a)   a: Start address for the REGISTERS (10 CELLs).
(vhere)  (--a)   a: Address of the VHERE variable.
(here)   (--a)   a: Address of the HERE variable.
(last)   (--a)   a: Address of the LAST variable.
(stk)    (--a)   a: Address of the stack.
(sp)     (--a)   a: Address of the stack pointer.
(rsp)    (--a)   a: Address of the return stack pointer.
(lsp)    (--a)   a: Address of the loop stack pointer.
word-sz  (--n)   n: The size in bytes of a dictionary entry.
base     (--a)   a: Address of the BASE variable.
state    (--a)   a: Address of the STATE variable.
tib      (--a)   a: Address of TIB (text input buffer).
>in      (--a)   a: Address of >IN.
cell     (--n)   n: size in bytes of a CELL.
timer    (--n)   n: return value from clock() function call.
```

## Extending c3
(TODO)
