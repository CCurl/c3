# c3 - A minimal Forth written in C.

The main goals for this project are as follows:
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To be able to run on both Windows and Linux (and Macintosh).
- To be deployable development boards, via the Arduino IDE.

Notes:
- This is NOT an ANSI-standard Forth system.
- This is a byte-coded implementation.
- The Linux version is 64-bit but can also be 32-bit.
- Not many primitives are built into the base executable.
- The rest is built using those words (see core.f).
- The VARIABLE space is separated from the CODE space.
- VHERE ("(vhere) @") is the address of the first available byte in the VARIABLE space.
- The maximum length of a word-name is configurable. (#define NAME_LEN 9)
- A dictionary entry looks like this:
    - flags:   byte
    - length:  byte
    - name:    char[NAME_LEN+1] (NULL terminated)
    - xt:      cell_t

## c3 Reference
```
NOTE: many words are defined in file 'core.f'

*** MATH ***
+        (a b--c)          Addition
-        (a b--c)          Subtraction
/        (a b--c)          Division
*        (a b--c)          Multiplication
/mod     (a b--r q)        q: quotient(a,b), r: modulo(a,b)

*** STACK ***
1+       (a--b)            Increment TOS
1-       (a--b)            Decrement TOS
drop     (a b--a)          Drop TOS
dup      (a--a a)          Duplicate TOS
over     (a b--a b a)      Copy NOS
swap     (a b--b a)        Swap TOS and NOS

*** INPUT/OUTPUT ***
emit     (c--)             Output c as a character.
key      (--c)             c: Next keyboard char, wait if no char available.
key?     (--f)             f: FALSE if no char available, else TRUE.

*** LOGICAL ***
=        (a b--f)          Equality.
<        (a b--f)          Less-than.
>        (a b--f)          Greater-than.
0=       (n--f)            Logical NOT.

*** MEMORY ***
@        (a--n)            n: CELL at a.
c@       (a--b)            b: BYTE at a.
!        (n a--)           Store CELL n at a.
c!       (b a--)           Store BYTE b at a.

*** WORDS and FLOW CONTROL ***
: word   (--)              Begin definition of word.
;        (--)              End current definition.
do       (T F--)           Begin DO/LOOP loop
(i)      (--a)             a: address of the index variable.
loop     (--)              Increment I, jump to DO if I < T
' word   (--xt fl f)       Find a word in the dictionary.

*** SYSTEM ***
(exit)   (--n)   n: The byte-code value for EXIT
(jmp)    (--n)   n: The byte-code value for JMP
(jmpz)   (--n)   n: The byte-code value for JMPZ
(jmpnz)  (--n)   n: The byte-code value for JMPNZ
(call)   (--n)   n: The byte-code value for CALL
(lit4)   (--n)   n: The byte-code value for LIT4
(bitop)  (--n)   n: The byte-code value for BITOP
(retop)  (--n)   n: The byte-code value for RETOP
(fileop) (--n)   n: The byte-code value for FILEOP
mem      (--a)   a: Start address for the MEMORY area.
mem-end  (--a)   a: End address for the MEMORY area.
vars     (--a)   a: Start address for the VARIABLES area.
vars-end (--a)   a: End address for the VARIABLES area.
word-sz  (--n)   n: Size in bytes of a dictionary entry.
(vhere)  (--a)   a: Address of VHERE.
(here)   (--a)   a: Address of HERE.
(last)   (--a)   a: Address of LAST.
(vhere)  (--a)   a: Address of VHERE.
(stk)    (--a)   a: Address of the stack.
(sp)     (--a)   a: Address of stack pointer.
(rsp)    (--a)   a: Address of return stack pointer.
(lsp)    (--a)   a: Address of the loop stack pointer.
base     (--a)   a: Address of BASE.
state    (--a)   a: Address of STATE.
>in      (--a)   a: Address of >IN.
cell     (--n)   n: size in bytes of a CELL.
```

## Extending c3
