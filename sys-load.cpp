// Load the base c3 system

// Use this nimbleText script to convert block-001.fth
//   ParseLine("<%row.replace(/[\\]/ig,"\\\\").replace(/["]/ig,"\\\"")%>");
// Don't forget to remove the '\' comments - they cause a crash.

#include "c3.h"

#ifndef _SYS_LOAD_

    void sysLoad() {}

#else

    void sysLoad() {
        ParseLine(": \\ 0 >IN  @ ! ; IMMEDIATE");
        ParseLine(": [ 0 state ! ; IMMEDIATE");
        ParseLine(": ] 1 state ! ;");
        ParseLine(": HERE  (HERE)  @ ;");
        ParseLine(": LAST  (LAST)  @ ;");
        ParseLine(": VHERE (VHERE) @ ;");
        ParseLine(": IF    (JMPZ) C, HERE 0 , ; IMMEDIATE");
        ParseLine(": ELSE  (JMP)  C, HERE SWAP 0 , HERE SWAP ! ; IMMEDIATE");
        ParseLine(": THEN  HERE SWAP ! ; IMMEDIATE");
        ParseLine(": BEGIN  HERE               ; IMMEDIATE");
        ParseLine(": UNTIL  (JMPZ) C, ,        ; IMMEDIATE");
        ParseLine(": AGAIN  (JMP)  C, ,        ; IMMEDIATE");
        ParseLine(": WHILE  (JMPZ) C, HERE 0 , ; IMMEDIATE");
        ParseLine(": REPEAT (JMP)  C, SWAP , HERE SWAP ! ; IMMEDIATE");
        ParseLine(": -EXIT (-REGS) C, (EXIT) C, ; IMMEDIATE");
        ParseLine(": TUCK  SWAP OVER ; INLINE");
        ParseLine(": NIP   SWAP DROP ; INLINE");
        ParseLine(": ?DUP  DUP IF DUP THEN ;");
        ParseLine(": +! DUP >R @ + R> ! ; INLINE");
        ParseLine(": 2+ 1+ 1+ ; INLINE");
        ParseLine(": CELL+ CELL + ; INLINE");
        ParseLine(": CELLS CELL * ; INLINE");
        ParseLine(": T3  "); // \\ ( --zstring end )");
        ParseLine("    +regs   VHERE DUP s8 s9   >IN  @ 1+ s5");
        ParseLine("    BEGIN r5+ C@ s1");
        ParseLine("        r1 IF r5 >IN  ! THEN");
        ParseLine("        r1 0= r1 '\"' = OR IF");
        ParseLine("            0 r8+ C!   r9 r8 -EXIT");
        ParseLine("        THEN r1   r8+ C!");
        ParseLine("    AGAIN ;");
        ParseLine(": \"  T3 STATE @ 0= IF DROP EXIT THEN");
        ParseLine("        (VHERE) ! (LIT) C, , ; IMMEDIATE");
        ParseLine(": .\" T3 STATE @ 0= IF DROP ZTYPE EXIT THEN");
        ParseLine("        (VHERE) ! (LIT) C, , (ZTYPE) C, ; IMMEDIATE");
        ParseLine(": code-end  code code-sz + ;");
        ParseLine(": vars-end  vars vars-sz + ;");
        ParseLine(": bl   #32 ;               INLINE");
        ParseLine(": tab   #9 EMIT ;          INLINE");
        ParseLine(": cr   #13 EMIT #10 EMIT ; INLINE");
        ParseLine(": space bl EMIT ;          INLINE");
        ParseLine(": . (.) space ;            INLINE");
        ParseLine(": LEXICON  (LEXICON) ! ;");
        ParseLine(": LEX@     (LEXICON) @ ;");
        ParseLine(": LEX-C3     0 LEXICON ;");
        ParseLine(": .word     CELL+ 1+ 2+ QTYPE ; INLINE");
        ParseLine(": word-lex  CELL+ 1+ C@ ; INLINE");
        ParseLine(": word-len  CELL+ 2+ C@ ; INLINE");
        ParseLine(": lex-match?  LEX@ >R  word-lex R@ =  R> 0= OR ;");
        ParseLine(": WORDS +REGS 0 DUP s1 s3 LAST s2 BEGIN");
        ParseLine("        r2 code-end < WHILE");
        ParseLine("        r2 lex-match? IF");
        ParseLine("            r1+ #9 > IF 0 s1 cr THEN");
        ParseLine("            r2 word-len #7 > IF i1 THEN");
        ParseLine("            i3 r2 .word tab");
        ParseLine("        THEN");
        ParseLine("        r2 WORD-SZ + s2");
        ParseLine("    REPEAT");
        ParseLine("    r3 .\" (%d words)\" -REGS ;");
        ParseLine(": ( BEGIN");
        ParseLine("      >IN @ C@ DUP 0= IF DROP EXIT THEN");
        ParseLine("      1 >IN +! ')' = IF EXIT THEN");
        ParseLine("    AGAIN ; IMMEDIATE");
        ParseLine(": ALLOT  VHERE + (VHERE) ! ;");
        ParseLine(": VC, VHERE C! 1 ALLOT ;");
        ParseLine(": V,  VHERE ! CELL ALLOT ;");
        ParseLine(": DOES>  R> (JMP) C, , ;");
        ParseLine(": CONSTANT  CREATE HERE CELL - ! (EXIT) C, ;");
        ParseLine(": VARIABLE  CREATE 0 V, (EXIT) C, ;");
        //ParseLine("\\ usage: val line   (val) (line)  >val >line ... 23 >line");
        ParseLine(": val    CREATE 0 V, (FETCH) C, (EXIT) C, ;");
        ParseLine(": >val   VHERE CELL - CONSTANT (STORE) HERE 1- C! (EXIT) C, ;");
        ParseLine(": (val)  VHERE CELL - CONSTANT ;");
        //ParseLine("\\ These use DOES> ... they might be more 'elegant',");
        //ParseLine("\\ but they are longer and less efficient");
        //ParseLine("\\ : val   CREATE 0 v, DOES> @ ;");
        //ParseLine("\\ : >val  CREATE DOES> CELL - ! ;");
        ParseLine(": :NONAME  HERE 1 STATE ! ;");
        ParseLine(": EXECUTE  >R ;");
        ParseLine(": FOR 0 SWAP DO ; INLINE");
        ParseLine(": NEXT -LOOP ; INLINE");
        ParseLine(": -if (DUP) C, (jmpz) C, here 0 , ; IMMEDIATE");
        ParseLine(": -until (DUP) C,  (jmpz) C, , ; IMMEDIATE");
        ParseLine(": -while (jmpnz) C, , ; IMMEDIATE");
        ParseLine(": /   /MOD NIP  ; INLINE");
        ParseLine(": MOD /MOD DROP ; INLINE");
        ParseLine(": 2DUP  OVER OVER ; INLINE");
        ParseLine(": 2DROP DROP DROP ; INLINE");
        ParseLine(": 2*  DUP + ; INLINE");
        ParseLine(": 2/  2 /   ; INLINE");
        ParseLine(": <=  > 0= ; INLINE");
        ParseLine(": >=  < 0= ; INLINE");
        ParseLine(": <>  = 0= ; INLINE");
        ParseLine(": RDROP R> DROP ; INLINE");
        ParseLine(": ROT   >R SWAP R> SWAP ; INLINE");
        ParseLine(": -ROT  SWAP >R SWAP R> ; INLINE");
        ParseLine(": NEGATE  INVERT 1+ ; INLINE");
        ParseLine(": ABS  DUP 0 < IF NEGATE THEN ;");
        ParseLine(": MIN  2DUP > IF SWAP THEN DROP ;");
        ParseLine(": MAX  2DUP < IF SWAP THEN DROP ;");
        ParseLine(": BTW +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;");
        ParseLine(": I  (I) @ ; INLINE");
        ParseLine(": J  (I) 3 CELLS - @ ;");
        ParseLine(": +I (I) +! ; INLINE");
        ParseLine(": +LOOP 1- +I LOOP ; INLINE");
        ParseLine(": UNLOOP (lsp) @ 3 - 0 MAX (lsp) ! ;");
        ParseLine(": 0SP 0 (sp) ! ;");
        ParseLine(": DEPTH (sp) @ 1- ;");
        ParseLine(": .S '(' EMIT space depth ?DUP IF");
        ParseLine("        0 DO (stk) I 1+ CELLS + @ . LOOP");
        ParseLine("    THEN ')' EMIT ;");
        ParseLine(": BINARY  %10 BASE ! ;");
        ParseLine(": DECIMAL #10 BASE ! ;");
        ParseLine(": HEX     $10 BASE ! ;");
        ParseLine(": ? @ . ;");
        ParseLine(": RSHIFT ( N1 S--N2 ) 0 DO 2/ LOOP ;");
        ParseLine(": LSHIFT ( N1 S--N2 ) 0 DO 2* LOOP ;");
        ParseLine(": THRU >R 1- R> DO I LOAD -LOOP ;");
        ParseLine(": INCLUDE next-word DROP (load) ;");
        ParseLine(": load-abort #99 state ! ;");
        ParseLine(": loaded? IF 2drop load-abort THEN ;");
        ParseLine("VARIABLE T0 2 CELLS allot");
        ParseLine(": T1 CELLS T0 + ;");
        ParseLine(": MARKER HERE 0 T1 ! VHERE 1 T1 ! LAST 2 T1 ! ;");
        ParseLine(": FORGET C3 0 T1 @ (HERE) ! 1 T1 @ (VHERE) ! 2 T1 @ (LAST) ! ;");
        ParseLine(": FORGET-1 LAST @ (HERE) ! LAST WORD-SZ + (LAST) ! ;");
        ParseLine("MARKER");
        ParseLine(".\" c3 - \"  version 10000 /mod s0 100 /mod r0  .\" v%d.%d.%d - Chris Curl%n\"");
        ParseLine("HERE CODE - .\" %d code bytes used, \" LAST HERE - .\" %d bytes free.%n\"");
        ParseLine("CODE-END LAST - dup s0 .\" %d dictionary bytes used, \" r0 word-sz / .\" %d words.%n\"");
        ParseLine("VHERE VARS - .\" %d variable bytes used, \" VARS-END VHERE - .\" %d bytes free.\"");
        // ParseLine("cr 999 load");
    }

#endif // _SYS_LOAD_
