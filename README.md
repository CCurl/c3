# c3 - A minimal Forth written in C.

The main goals for this project are as follows:
- To have an implementation that is minimal and "intuitively obvious upon casual inspection".
- To be able to run on both Windows and Linux (and Macintosh).
- To be deployable to development boards via the Arduino IDE.

Notes:
- This is NOT an ANSI-standard Forth system.
- This is a byte-coded implementation.
- The Linux version is 64-bit but can also be 32-bit.
- Not many primitives are built into the base executable.
- The rest is built using those words (see core.f).
- The VARIABLE space is separated from the CODE space.
- VHERE ("(vhere) @") is the address of the first available byte in the VARIABLE space.
- A dictionary entry looks like this:
    - xt:      cell_t
    - flags:   byte
    - len:     byte
    - name:    char[14] (NULL terminated)

## c3 Base system reference
```
NOTE: many of the core words are defined in file 'core.f'

*** MATH ***
+        (a b--c)          Addition
-        (a b--c)          Subtraction
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
[0-x]*   (--N)             Input N as a number in the current BASE.
#[0-9]*  (--N)             Input N as a decimal number.
$[0-f]*  (--N)             Input N as a hexadecimal number.
%[0-1]*  (--N)             Input N as a binary number.
'x'      (--N)             Input N as the ascii value of 'x'.
emit     (C--)             Output C as a character.
key      (--C)             C: Next keyboard char, wait if no char available.
key?     (--F)             F: FALSE if no char available, else TRUE.
                NOTE: key and ?key are currently only implemented for WINDOWS.
                      They are not yet implemented under LINUX, 

*** FILES ***
fopen    (n m--fh)         n: name, m: mode (eg - rt), fh: file-handle.
fclose   (--fh)            fh: file-handle.
fread    (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars read.
fwrite   (a sz fh--n)      a: buf, sz: max size, fh: file-handle, n: num chars written.
(input-fp)  (--a)          a: address of the input-file pointer (PC only; used by "load").
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
;        (--)              End current definition.
create x (--)              Creates a definition for x word.
do       (T F--)           Begin DO/LOOP loop.
(i)      (--a)             a: address of the index variable.
loop     (--)              Increment I, jump to DO if I < T.
' xxx    (--xt fl f)       Find word 'xxx' in the dictionary.
            NOTE: Words like IF/THEN and BEGIN/WHILE are not in the base c3.
                  They are defined in core.f

*** SYSTEM ***
(exit)   (--n)   n: The byte-code value for EXIT.
(jmp)    (--n)   n: The byte-code value for JMP.
(jmpz)   (--n)   n: The byte-code value for JMPZ.
(jmpnz)  (--n)   n: The byte-code value for JMPNZ.
(call)   (--n)   n: The byte-code value for CALL.
(lit4)   (--n)   n: The byte-code value for LIT4.
mem      (--a)   a: Start address for the MEMORY area.
mem-end  (--a)   a: End address for the MEMORY area.
vars     (--a)   a: Start address for the VARIABLES area.
vars-end (--a)   a: End address for the VARIABLES area.
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
