// Load the base c3 system

#include "c3.h"

#ifndef _SYS_LOAD_

void sysLoad() {}

#else

void sysLoad() {
    ParseLine(": \\ 0 >in @ ! ; IMMEDIATE");
    ParseLine(": [ 0 state ! ; IMMEDIATE");
    ParseLine(": ] 1 state ! ;");
    ParseLine(": LAST (last) @ ;");
    ParseLine(": HERE (here) @ ;");
    ParseLine(": code-end  code code-sz + ;");
    ParseLine(": vars-end  vars vars-sz + ;");
    ParseLine(": ++ DUP @ 1+ SWAP ! ; INLINE");
    ParseLine(": -- DUP @ 1- SWAP ! ; INLINE");
    ParseLine(": VHERE  (vhere) @ ;");
    ParseLine(": ALLOT  vhere + (vhere) ! ;");
    ParseLine(": VC, vhere c! (vhere) ++ ;");
    ParseLine(": V,  vhere ! CELL allot ;");
    ParseLine(": CELL+ CELL + ; INLINE");
    ParseLine(": CELLS CELL * ; INLINE");
    ParseLine(": DOES>  R> (JMP) C, , ;");
    ParseLine(": CONSTANT  CREATE HERE CELL - ! (EXIT) C, ;");
    ParseLine(": VARIABLE  CREATE 0 V, (EXIT) C, ;");
    ParseLine(": val    CREATE 0 v, DOES> @ ;");
    ParseLine(": >val   CREATE DOES> CELL - ! ;");
    ParseLine(": (val)  vhere CELL - CONSTANT ;");
    ParseLine(": :NONAME  HERE 1 STATE ! ;");
    ParseLine(": EXEC  >R ;");
    ParseLine(": IF    (jmpz) c, here 0 , ; IMMEDIATE");
    ParseLine(": ELSE  (jmp) c, here SWAP 0 , here SWAP ! ; IMMEDIATE");
    ParseLine(": THEN  here SWAP ! ; IMMEDIATE");
    ParseLine(": BEGIN  here        ; IMMEDIATE");
    ParseLine(": UNTIL  (jmpz) c, , ; IMMEDIATE");
    ParseLine(": AGAIN  (jmp) c, ,  ; IMMEDIATE");
    ParseLine(": WHILE  (jmpz) c, here 0 , ; IMMEDIATE");
    ParseLine(": REPEAT SWAP (jmp) c, , here SWAP ! ; IMMEDIATE");
    ParseLine(": FOR 0 SWAP do ; INLINE");
    ParseLine(": NEXT -loop ; INLINE");
    ParseLine(": -if (DUP) c, (jmpz) c, here 0 , ; IMMEDIATE");
    ParseLine(": -until (DUP) c,  (jmpz) c, , ; IMMEDIATE");
    ParseLine(": -while (jmpnz) c, , ; IMMEDIATE");
    ParseLine(": TUCK  SWAP OVER ; INLINE");
    ParseLine(": NIP   SWAP DROP ; INLINE");
    ParseLine(": 2DUP  OVER OVER ; INLINE");
    ParseLine(": 2DROP DROP DROP ; INLINE");
    ParseLine(": ?DUP  DUP IF DUP THEN ;");
    ParseLine(": /   /mod nip  ; INLINE");
    ParseLine(": MOD /mod DROP ; INLINE");
    ParseLine(": +! SWAP OVER @ + SWAP ! ; INLINE");
    ParseLine(": c++ DUP c@ 1+ SWAP c! ; INLINE");
    ParseLine(": 2*  DUP + ; INLINE");
    ParseLine(": 2/  2 / ; INLINE");
    ParseLine(": 2+  1+ 1+ ; INLINE");
    ParseLine(": <=  > 0= ; INLINE");
    ParseLine(": >=  < 0= ; INLINE");
    ParseLine(": <>  = 0= ; INLINE");
    ParseLine(": RDROP R> DROP ; INLINE");
    ParseLine(": ROT   >R SWAP r> SWAP ; INLINE");
    ParseLine(": ( begin");
    ParseLine("    >in @ c@ DUP 0= IF DROP EXIT THEN");
    ParseLine("    >in ++ ')' = IF EXIT THEN");
    ParseLine("    again ; IMMEDIATE");
    ParseLine(": bl  #32 ; INLINE");
    ParseLine(": tab  #9 EMIT ; INLINE");
    ParseLine(": cr  #13 EMIT #10 EMIT ; INLINE");
    ParseLine(": space bl EMIT ; INLINE");
    ParseLine(": . (.) space ; INLINE");
    ParseLine(": NEGATE  INVERT 1+ ; INLINE");
    ParseLine(": ABS  DUP 0 < IF negate THEN ;");
    ParseLine(": MIN  OVER OVER > IF SWAP THEN DROP ;");
    ParseLine(": MAX  OVER OVER < IF SWAP THEN DROP ;");
    ParseLine(": BTW +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;");
    ParseLine(": I  (I) @ ; INLINE");
    ParseLine(": J  (I) 3 CELLS - @ ;");
    ParseLine(": +I (I) @ + (I) ! ; INLINE");
    ParseLine(": +LOOP 1- +I LOOP ; INLINE");
    ParseLine(": UNLOOP (lsp) @ 3 - 0 MAX (lsp) ! ;");
    ParseLine(": 0SP 0 (sp) ! ;");
    ParseLine(": DEPTH (sp) @ 1- ;");
    ParseLine(": .S '(' EMIT space depth ?DUP IF");
    ParseLine("      0 do (stk) i 1+ CELL * + @ . loop");
    ParseLine("    THEN ')' EMIT ;");
    ParseLine(": dump ( a n-- ) for DUP c@ . 1+ next DROP ;");
    ParseLine(": T3 ( --zstr end )   +regs   >in ++");
    ParseLine("    vhere DUP s8 s9");
    ParseLine("    begin >in @ c@ s1");
    ParseLine("        r1 IF >in ++ THEN");
    ParseLine("        r1 0= r1 '\"' = or");
    ParseLine("        IF 0 r8+ c!   r9 r8 -regs   EXIT THEN");
    ParseLine("        r1   r8+ c!");
    ParseLine("    again ;");
    ParseLine(": \" ( --SZ )  T3 state @ 0= IF DROP EXIT THEN (vhere) ! (lit) c, , ; IMMEDIATE");
    ParseLine(": .\" ( -- )   T3 state @ 0= IF DROP ztype EXIT THEN");
    ParseLine("    (vhere) ! (lit) c, , (ztype) c, ; IMMEDIATE");
    ParseLine(": .word CELL + 2+ qtype ; INLINE");
    ParseLine(": word-len ( a--n ) CELL + 2+ s-len ; INLINE");
    ParseLine(": words +regs 0 s1 0 s3 last s2 begin");
    ParseLine("        r2 code-end < while");
    ParseLine("        r1+ #9 > IF 0 s1 cr THEN");
    ParseLine("        r2 word-len #7 > IF i1 THEN");
    ParseLine("        i3 r2 .word tab r2 word-sz + s2");
    ParseLine("    repeat");
    ParseLine("    r3 .\" (%d words)\" -regs ;");
    ParseLine(": BINARY  %10 BASE ! ;");
    ParseLine(": DECIMAL #10 BASE ! ;");
    ParseLine(": HEX     $10 BASE ! ;");
    ParseLine(": ? @ . ;");
    ParseLine(": RSHIFT ( N1 S--N2 ) 0 DO 2/ LOOP ;");
    ParseLine(": LSHIFT ( N1 S--N2 ) 0 DO 2* LOOP ;");
    ParseLine(": load next-word DROP (load) ;");
    ParseLine(": load-abort #99 state ! ;");
    ParseLine(": loaded? IF 2drop load-abort THEN ;");
    ParseLine("variable T0 2 CELLS allot");
    ParseLine(": T1 CELLS T0 + ;");
    ParseLine(": marker here 0 T1 ! vhere 1 T1 ! last 2 T1 ! ;");
    ParseLine(": forget 0 T1 @ (here) ! 1 T1 @ (vhere) ! 2 T1 @ (last) ! ;");
    ParseLine(": forget-1 last @ (here) ! last word-sz + (last) ! ;");
    ParseLine("marker");
    ParseLine(": benches forget \" benches.c3\" (load) ;");
    ParseLine(": sb forget \" sandbox.c3\" (load) ;");
}

#endif // _SYS_LOAD_
