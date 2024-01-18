: \ 0 >in @ ! ; IMMEDIATE
: [ 0 state ! ; IMMEDIATE
: ] 1 state ! ;
: LAST (last) @ ;
: HERE (here) @ ;
: code-end  code code-sz + ;
: vars-end  vars vars-sz + ;
: ++ DUP @ 1+ SWAP ! ; INLINE
: -- DUP @ 1- SWAP ! ; INLINE
: VHERE  (vhere) @ ;
: ALLOT  vhere + (vhere) ! ;
: VC, vhere c! (vhere) ++ ;
: V,  vhere ! CELL allot ;
: CELL+ CELL + ; INLINE
: CELLS CELL * ; INLINE
: DOES>  R> (JMP) C, , ;
: CONSTANT  CREATE HERE CELL - ! (EXIT) C, ;
: VARIABLE  CREATE 0 V, (EXIT) C, ;
: val    CREATE 0 v, DOES> @ ;
: >val   CREATE DOES> CELL - ! ;
: (val)  vhere CELL - CONSTANT ;
: :NONAME  HERE 1 STATE ! ;
: EXEC  >R ;
: IF    (jmpz) c, here 0 , ; IMMEDIATE
: ELSE  (jmp) c, here SWAP 0 , here SWAP ! ; IMMEDIATE
: THEN  here SWAP ! ; IMMEDIATE
: BEGIN  here        ; IMMEDIATE
: UNTIL  (jmpz) c, , ; IMMEDIATE
: AGAIN  (jmp) c, ,  ; IMMEDIATE
: WHILE  (jmpz) c, here 0 , ; IMMEDIATE
: REPEAT SWAP (jmp) c, , here SWAP ! ; IMMEDIATE
: FOR 0 SWAP do ; INLINE
: NEXT -loop ; INLINE
: -if (DUP) c, (jmpz) c, here 0 , ; IMMEDIATE
: -until (DUP) c,  (jmpz) c, , ; IMMEDIATE
: -while (jmpnz) c, , ; IMMEDIATE
: TUCK  SWAP OVER ; INLINE
: NIP   SWAP DROP ; INLINE
: 2DUP  OVER OVER ; INLINE
: 2DROP DROP DROP ; INLINE
: ?DUP  DUP IF DUP THEN ;
: /   /mod nip  ; INLINE
: MOD /mod DROP ; INLINE
: +! SWAP OVER @ + SWAP ! ; INLINE
: c++ DUP c@ 1+ SWAP c! ; INLINE
: 2*  DUP + ; INLINE
: 2/  2 / ; INLINE
: 2+  1+ 1+ ; INLINE
: <=  > 0= ; INLINE
: >=  < 0= ; INLINE
: <>  = 0= ; INLINE
: RDROP R> DROP ; INLINE
: ROT   >R SWAP r> SWAP ; INLINE
: ( begin
    >in @ c@ DUP 0= IF DROP EXIT THEN
    >in ++ ')' = IF EXIT THEN
    again ; IMMEDIATE
: bl  #32 ; INLINE
: tab  #9 EMIT ; INLINE
: cr  #13 EMIT #10 EMIT ; INLINE
: space bl EMIT ; INLINE
: . (.) space ; INLINE
: NEGATE  INVERT 1+ ; INLINE
: ABS  DUP 0 < IF negate THEN ;
: MIN  OVER OVER > IF SWAP THEN DROP ;
: MAX  OVER OVER < IF SWAP THEN DROP ;
: BTW +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;
: I  (I) @ ; INLINE
: J  (I) 3 CELLS - @ ;
: +I (I) @ + (I) ! ; INLINE
: +LOOP 1- +I LOOP ; INLINE
: UNLOOP (lsp) @ 3 - 0 MAX (lsp) ! ;
: 0SP 0 (sp) ! ;
: DEPTH (sp) @ 1- ;
: .S '(' EMIT space depth ?DUP IF
      0 do (stk) i 1+ 8 * + @ . loop
    THEN ')' EMIT ;
: dump ( a n-- ) for DUP c@ . 1+ next DROP ;
: T3 ( --zstr end )   +regs   >in ++
    vhere DUP s8 s9
    begin >in @ c@ s1
        r1 IF >in ++ THEN
        r1 0= r1 '"' = or
        IF 0 r8+ c!   r9 r8 -regs   EXIT THEN
        r1   r8+ c!
    again ;
: " ( --SZ )  T3 state @ 0= IF DROP EXIT THEN (vhere) ! (lit) c, , ; IMMEDIATE
: ." ( -- )   T3 state @ 0= IF DROP ztype EXIT THEN
    (vhere) ! (lit) c, , (ztype) c, ; IMMEDIATE
: .word CELL + 2+ qtype ; INLINE
: word-len ( a--n ) CELL + 2+ s-len ; INLINE
: words +regs 0 s1 0 s3 last s2 begin
        r2 code-end < while
        r1+ #9 > IF 0 s1 cr THEN
        r2 word-len #7 > IF i1 THEN
        i3 r2 .word tab r2 word-sz + s2
    repeat
    r3 ." (%d words)" -regs ;
: BINARY  %10 BASE ! ;
: DECIMAL #10 BASE ! ;
: HEX     $10 BASE ! ;
: ? @ . ;
: RSHIFT ( N1 S--N2 ) 0 DO 2/ LOOP ;
: LSHIFT ( N1 S--N2 ) 0 DO 2* LOOP ;
: load next-word DROP (load) ;
: load-abort #99 state ! ;
: loaded? IF 2drop load-abort THEN ;
variable T0 2 CELLS allot
: T1 CELLS T0 + ;
: marker here 0 T1 ! vhere 1 T1 ! last 2 T1 ! ;
: forget 0 T1 @ (here) ! 1 T1 @ (vhere) ! 2 T1 @ (last) ! ;
: forget-1 last @ (here) ! last word-sz + (last) ! ;

marker
version 100 /mod ." c3 - v%d.%d - Chris Curl%n"
here code - ." %d code bytes used, " last here - ." %d bytes free.%n"
vhere vars - ." %d variable bytes used, " vars-end vhere - ." %d bytes free."
: benches forget " benches.c3" (load) ;
: sb forget " sandbox.c3" (load) ;
