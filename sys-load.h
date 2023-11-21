// Load the base c3 system
// NOTE: this is a *.h file because the Arduino IDE doesn't like *.inc files

void sysLoad() {
    const char *m2n = "-ML- %s %d %d 3 -MLX-";
    const char *m2i = "-ML- %s %d %d 3 -MLX- INLINE";
    const char *m1i = "-ML- %s %d    3 -MLX- INLINE";
    const char *lit = ": %s %d ; INLINE";

    // Bootstrap ...
    parseF(m2n, "INLINE", SYS_OPS, INLINE); last->f = IS_INLINE;
    parseF(m2i, "IMMEDIATE", SYS_OPS, IMMEDIATE);
    parseF(m2i, ":", SYS_OPS, COLONDEF);
    parseF(m2n, ";", SYS_OPS, ENDWORD); last->f = IS_IMMEDIATE;

    // Opcodes ...
    parseF(lit, "(LIT)", LIT);
    parseF(m1i, "EXIT", EXIT);
    parseF(lit, "(EXIT)", EXIT); last->f = 0;
    parseF(lit, "(CALL)", CALL);
    parseF(lit, "(JMP)", JMP);
    parseF(lit, "(JMPZ)", JMPZ);
    parseF(lit, "(JMPNZ)", JMPNZ);
    parseF(m1i, "!", STORE);
    parseF(lit, "(STORE)", STORE);
    parseF(m1i, "C!", CSTORE);
    parseF(m1i, "@", FETCH);
    parseF(lit, "(FETCH)", FETCH);
    parseF(m1i, "C@", CFETCH);
    parseF(m1i, "DUP", DUP);
    parseF(lit, "(DUP)", DUP);
    parseF(m1i, "SWAP", SWAP);
    parseF(m1i, "OVER", OVER);
    parseF(m1i, "DROP", DROP);
    parseF(m1i, "+", ADD);
    parseF(m1i, "*", MULT);
    parseF(m1i, "/MOD", SLMOD);
    parseF(m1i, "-", SUB);
    parseF(m1i, "1+", INC);
    parseF(m1i, "1-", DEC);
    parseF(m1i, "<", LT);
    parseF(m1i, "=", EQ);
    parseF(m1i, ">", GT);
    parseF(m1i, "0=", EQ0);
    parseF(m1i, ">R", RTO);
    parseF(m1i, "R@", RFETCH);
    parseF(m1i, "R>", RFROM);
    parseF(m1i, "DO", DO);
    parseF(m1i, "LOOP", LOOP);
    parseF(m1i, "-LOOP", LOOP2);
    parseF(m1i, "(I)", INDEX);
    parseF(m1i, "INVERT", COM);
    parseF(m1i, "AND", AND);
    parseF(m1i, "OR", OR);
    parseF(m1i, "XOR", XOR);
    parseF(m1i, "TYPE", TYPE);
    parseF(m1i, "ZTYPE", ZTYPE);
    parseF(lit, "(ZTYPE)", ZTYPE);
    // rX, sX, iX, dX, iX+, dX+ are hard-coded in c3.c
    parseF(m1i, "+REGS", REG_NEW);
    parseF(m1i, "-REGS", REG_FREE);
    // parseF(m1i, "SYS_OPS", SYS_OPS);
    // parseF(lit, "STR_OPS", STR_OPS);
    // parseF(lit, "FLT_OPS", FLT_OPS);

    // System opcodes ...(INLINE and IMMEDIATE) were defined above
    // parseF(m2n, "INLINE", SYS_OPS, INLINE); last->f = IS_INLINE;
    // parseF(m2i, "IMMEDIATE", SYS_OPS, IMMEDIATE);
    parseF(m2i, "(.)",       SYS_OPS, DOT);
    parseF(m2i, "ITOA",      SYS_OPS, ITOA);
    parseF(m2i, "ATOI",      SYS_OPS, ATOI);
    parseF(m2i, "CREATE",    SYS_OPS, CREATE);
    parseF(m2i, "'",         SYS_OPS, FIND);
    parseF(m2i, "NEXT-WORD", SYS_OPS, WORD);
    parseF(m2i, "TIMER",     SYS_OPS, TIMER);
    parseF(m2i, "C,",        SYS_OPS, CCOMMA);
    parseF(m2i, ",",         SYS_OPS, COMMA);
    parseF(m2i, "KEY",       SYS_OPS, KEY);
    parseF(m2i, "?KEY",      SYS_OPS, QKEY);
    parseF(m2i, "EMIT",      SYS_OPS, EMIT);
    parseF(m2i, "QTYPE",     SYS_OPS, QTYPE);

    // String opcodes ...
    parseF(m2i, "S-TRUNC", STR_OPS, TRUNC);
    parseF(m2i, "LCASE",   STR_OPS, LCASE);
    parseF(m2i, "UCASE",   STR_OPS, UCASE);
    parseF(m2i, "S-CPY",   STR_OPS, STRCPY);
    parseF(m2i, "S-CAT",   STR_OPS, STRCAT);
    parseF(m2i, "S-CATC",  STR_OPS, STRCATC);
    parseF(m2i, "S-LEN",   STR_OPS, STRLEN);
    parseF(m2i, "S-EQ",    STR_OPS, STREQ);
    parseF(m2i, "S-EQ-I",  STR_OPS, STREQI);
    parseF(m2i, "S-LTRIM", STR_OPS, LTRIM);
    parseF(m2i, "S-RTRIM", STR_OPS, RTRIM);

    // Float opcodes ...
    parseF(m2i, "F+",   FLT_OPS, FADD);
    parseF(m2i, "F-",   FLT_OPS, FSUB);
    parseF(m2i, "F*",   FLT_OPS, FMUL);
    parseF(m2i, "F/",   FLT_OPS, FDIV);
    parseF(m2i, "F=",   FLT_OPS, FEQ);
    parseF(m2i, "F<",   FLT_OPS, FLT);
    parseF(m2i, "F>",   FLT_OPS, FGT);
    parseF(m2i, "F2I",  FLT_OPS, F2I);
    parseF(m2i, "I2F",  FLT_OPS, I2F);
    parseF(m2i, "F.",   FLT_OPS, FDOT);
    parseF(m2i, "SQRT", FLT_OPS, SQRT);
    parseF(m2i, "TANH", FLT_OPS, TANH);

    loadStartupWords();

    // System information words
    parseF(": VERSION     #%d ;", VERSION);
    parseF(": (SP)        $%p ;", &DSP);
    parseF(": (RSP)       $%p ;", &RSP);
    parseF(": (LSP)       $%p ;", &lsp);
    parseF(": (HERE)      $%p ;", &here);
    parseF(": (LAST)      $%p ;", &last);
    parseF(": (STK)       $%p ;", &ds.stk[0].i);
    parseF(": (RSTK)      $%p ;", &rs.stk[0].c);
    parseF(": TIB         $%p ;", &tib[0]);
    parseF(": >IN         $%p ;", &in);
    parseF(": CODE        $%p ;", &code[0]);
    parseF(": CODE-SZ     #%d ;", CODE_SZ);
    parseF(": VARS        $%p ;", &vars[0]);
    parseF(": VARS-SZ     #%d ;", VARS_SZ);
    parseF(": (VHERE)     $%p ;", &vhere);
    parseF(": (REGS)      $%p ;", &reg[0]);
    parseF(": (OUTPUT_FP) $%p ;", &output_fp);
    parseF(": (INPUT_FP)  $%p ;", &input_fp);
    parseF(": STATE       $%p ;", &state);
    parseF(": BASE        $%p ;", &base);
    parseF(": WORD-SZ     #%d ;", sizeof(dict_t));
    parseF(": BYE  %d STATE !  ;", ALL_DONE);
    parseF(": CELL %d ; INLINE",   CELL_SZ);

    // Main system
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
    ParseLine(": :noname  here 1 state ! ;");
    ParseLine(": exec  >R ;");
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
    ParseLine(": btw +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;");
    ParseLine(": I  (I) @ ; INLINE");
    ParseLine(": J  (I) 3 CELLS - @ ;");
    ParseLine(": +I (I) @ + (I) ! ; INLINE");
    ParseLine(": +LOOP 1- +I LOOP ; INLINE");
    ParseLine(": UNLOOP (lsp) @ 3 - 0 MAX (lsp) ! ;");
    ParseLine(": 0SP 0 (sp) ! ;");
    ParseLine(": DEPTH (sp) @ 1- ;");
    ParseLine(": .s '(' EMIT space depth ?DUP IF");
    ParseLine("      0 do (stk) i 1+ 8 * + @ . loop");
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
    loadUserWords();

    ParseLine("marker");
}
