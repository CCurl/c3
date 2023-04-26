# c3 - A minimal Forth-like VM written in C.

The main goals for this project are as follows:
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To have as much flexibility as possible.
- To be able to run on both Windows and Linux (and Apple).
- To be deployable to development boards via the Arduino IDE.

## Notes:
- This is NOT an ANSI-standard Forth system.
- This is a byte-coded implementation.
- The Linux version is 64-bit but can also be 32-bit.
- c3 provides 10 "virtual registers", r0 thru r9.
- c3 provides 10 temporary words, T0 thru T9.
- Not many primitives are built into the base executable.
- The rest is built using those primitives (see core.f).
- The VARIABLE space is separated from the CODE space.
- VHERE ("(vhere) @") is the address of the first available byte in the VARIABLE space.
- Strings are both counted and null-terminated.
- The dictionary starts at the end of the CODE area and grows down.
- The WORD length is defined by NAME_LEN (in c3.c) as 13 chars.
- A dictionary entry looks like this:
    - xt:      cell_t
    - flags:   byte
    - len:     byte
    - name:    char[NAME_LEN+1] (NULL terminated)

## Registers
c3 provides 10 "virtual registers", r0 thru r9.
There are 6 register operations: +regs, rX, sX, iX, dX, -regs.
- +regs   allocates 10 new current registers.
- r4      pushes register #4.
- s4      sets register #4 from TOS.
- i4      increments register #4.
- d4      decrements register #4.
- -regs   restores the registers to their previous values.

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
   : T0 ( n--sqrt ) dup 4 / begin >r dup r@ / r@ + 2 / dup r> - while nip ;
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
NOTE: many of the core words are defined in file 'core.f'

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
key?      (--F)            F: FALSE if no char available, else TRUE.

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
loop     (--)              Increment I, jump to DO if I < T.
-loop    (--)              Decrement I, jump to DO if I > T.
' xxx    (--xt fl f)       Find word 'xxx' in the dictionary.
        NOTE: Words like IF/THEN/EXIT and BEGIN/WHILE are not in the base c3.
              They are just words that are defined in core.f

*** REGISTERS ***
+regs    (--)              Save the current registers.
rX       (--n)             n: the value of register #X (X: [0-9]).
sX       (n--)             n: new value for register #X (X: [0-9]).
iX       (--)              Increment register #X (X: [0-9]).
dX       (--)              Decrement register #X (X: [0-9]).
-regs    (--)              Restore the last saved registers.
        NOTES: The registers are stored in an array/stack with a "register-base".
               +regs simply adds 10 to "register-base", so it is a very efficient operation.

*** SYSTEM ***
version  (--n)   n: c3 version*10 (e.g. - 11 => v1.1)
(exit)   (--n)   n: The byte-code value for EXIT.
(jmp)    (--n)   n: The byte-code value for JMP.    On execute: (?--?)    JUMP
(jmpz)   (--n)   n: The byte-code value for JMPZ.   On execute: (N--)     JUMP if N=0 (Consumes N)
(jmp=0)  (--n)   n: The byte-code value for JMP=0.  On execute: (N--N)    JUMP if N=0
(jmp<0)  (--n)   n: The byte-code value for JMP<0.  On execute: (N--N)    JUMP if N<0
(jmp>0)  (--n)   n: The byte-code value for JMP>0.  On execute: (N--N)    JUMP if N>0
(jmp-gt) (--n)   n: The byte-code value for JMP-GT. On execute: (A B--A)  JUMP if A>B (Consumes B)
(jmp-lt) (--n)   n: The byte-code value for JMP-LT. On execute: (A B--A)  JUMP if A<B (Consumes B)
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
(vhere)  (--a)   a: Address of the VHERE variable.
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
