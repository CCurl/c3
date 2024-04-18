: \ 0 >IN  @ ! ; IMMEDIATE
: [ 0 state ! ; IMMEDIATE
: ] 1 state ! ;

: HERE  (HERE)  @ ;
: LAST  (LAST)  @ ;
: VHERE (VHERE) @ ;

: IF    (JMPZ) C, HERE 0 , ; IMMEDIATE
: ELSE  (JMP)  C, HERE SWAP 0 , HERE SWAP ! ; IMMEDIATE
: THEN  HERE SWAP ! ; IMMEDIATE

: BEGIN  HERE               ; IMMEDIATE
: UNTIL  (JMPZ) C, ,        ; IMMEDIATE
: AGAIN  (JMP)  C, ,        ; IMMEDIATE
: WHILE  (JMPZ) C, HERE 0 , ; IMMEDIATE
: REPEAT (JMP)  C, SWAP , HERE SWAP ! ; IMMEDIATE
: -EXIT (-REGS) C, (EXIT) C, ; IMMEDIATE

: TUCK  SWAP OVER ; INLINE
: NIP   SWAP DROP ; INLINE
: ?DUP  DUP IF DUP THEN ;
: +! DUP >R @ + R> ! ; INLINE
: 2+ 1+ 1+ ; INLINE
: CELL+ CELL + ; INLINE
: CELLS CELL * ; INLINE

: T3  \ ( --zstring end )
    +regs   VHERE DUP s8 s9   >IN  @ 1+ s5
    BEGIN r5+ C@ s1
        r1 IF r5 >IN  ! THEN
        r1 0= r1 '"' = OR IF
            0 r8+ C!   r9 r8 -EXIT
        THEN r1   r8+ C!
    AGAIN ;
: "  T3 STATE @ 0= IF DROP EXIT THEN
        (VHERE) ! (LIT) C, , ; IMMEDIATE
: ." T3 STATE @ 0= IF DROP ZTYPE EXIT THEN
        (VHERE) ! (LIT) C, , (ZTYPE) C, ; IMMEDIATE

: code-end  code code-sz + ;
: vars-end  vars vars-sz + ;

: bl   #32 ;               INLINE
: tab   #9 EMIT ;          INLINE
: cr   #13 EMIT #10 EMIT ; INLINE
: space bl EMIT ;          INLINE
: . (.) space ;            INLINE

: .word     CELL+ 2+ QTYPE ; INLINE
: word-len  CELL+ 1+ C@ ; INLINE
: WORDS +REGS 0 DUP s1 s3 LAST s2 BEGIN
        r2 code-end < WHILE
        r1+ #9 > IF 0 s1 cr THEN
        r2 word-len #7 > IF i1 THEN
        i3 r2 .word tab
        r2 WORD-SZ + s2
    REPEAT
    r3 ." (%d words)" -REGS ;

: ( BEGIN
      >IN @ C@ DUP 0= IF DROP EXIT THEN
      1 >IN +! ')' = IF EXIT THEN
    AGAIN ; IMMEDIATE

: ALLOT  VHERE + (VHERE) ! ;
: VC, VHERE C! 1 ALLOT ;
: V,  VHERE ! CELL ALLOT ;
: DOES>  R> (JMP) C, , ;
: CONSTANT  CREATE HERE CELL - ! (EXIT) C, ;
: VARIABLE  CREATE 0 V, (EXIT) C, ;
\ usage: val line   (val) (line)  >val >line ... 23 >line
: val    CREATE 0 V, (FETCH) C, (EXIT) C, ;
: >val   VHERE CELL - CONSTANT (STORE) HERE 1- C! (EXIT) C, ;
: (val)  VHERE CELL - CONSTANT ;
\ These use DOES> ... they might be more 'elegant',
\ but they are longer and less efficient
\ : val   CREATE 0 v, DOES> @ ;
\ : >val  CREATE DOES> CELL - ! ;
: :NONAME  HERE 1 STATE ! ;
: EXECUTE  >R ;
: FOR 0 SWAP DO ; INLINE
: NEXT -LOOP ; INLINE
: -if (DUP) C, (jmpz) C, here 0 , ; IMMEDIATE
: -until (DUP) C,  (jmpz) C, , ; IMMEDIATE
: -while (jmpnz) C, , ; IMMEDIATE
: /   /MOD NIP  ; INLINE
: MOD /MOD DROP ; INLINE
: 2DUP  OVER OVER ; INLINE
: 2DROP DROP DROP ; INLINE
: 2*  DUP + ; INLINE
: 2/  2 /   ; INLINE
: <=  > 0= ; INLINE
: >=  < 0= ; INLINE
: <>  = 0= ; INLINE
: RDROP R> DROP ; INLINE
: ROT   >R SWAP R> SWAP ; INLINE
: NEGATE  INVERT 1+ ; INLINE
: ABS  DUP 0 < IF NEGATE THEN ;
: MIN  2DUP > IF SWAP THEN DROP ;
: MAX  2DUP < IF SWAP THEN DROP ;
: BTW +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;
: I  (I) @ ; INLINE
: J  (I) 3 CELLS - @ ;
: +I (I) +! ; INLINE
: +LOOP 1- +I LOOP ; INLINE
: UNLOOP (lsp) @ 3 - 0 MAX (lsp) ! ;
: 0SP 0 (sp) ! ;
: DEPTH (sp) @ 1- ;
: .S '(' EMIT space depth ?DUP IF
        0 DO (stk) I 1+ CELLS + @ . LOOP
    THEN ')' EMIT ;
: BINARY  %10 BASE ! ;
: DECIMAL #10 BASE ! ;
: HEX     $10 BASE ! ;
: ? @ . ;
: RSHIFT ( N1 S--N2 ) 0 DO 2/ LOOP ;
: LSHIFT ( N1 S--N2 ) 0 DO 2* LOOP ;
: load-f next-word DROP (load) ;
: load-abort #99 state ! ;
: loaded? IF 2drop load-abort THEN ;
VARIABLE T0 2 CELLS allot
: T1 CELLS T0 + ;
: MARKER HERE 0 T1 ! VHERE 1 T1 ! LAST 2 T1 ! ;
: FORGET 0 T1 @ (HERE) ! 1 T1 @ (VHERE) ! 2 T1 @ (LAST) ! ;
: FORGET-1 LAST @ (HERE) ! LAST WORD-SZ + (LAST) ! ;

MARKER

." c3 - "  version 10000 /mod s0 100 /mod r0  ." v%d.%d.%d - Chris Curl%n"
HERE CODE - ." %d code bytes used, " LAST HERE - ." %d bytes free.%n"
CODE-END LAST - dup s0 ." %d dictionary bytes used, " r0 word-sz / ." %d words.%n"
VHERE VARS - ." %d variable bytes used, " VARS-END VHERE - ." %d bytes free."

: help 999 load ;
: benches 200 load ;
: sb 900 load ;
