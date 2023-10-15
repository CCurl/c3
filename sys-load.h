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
    parseF(m2i, ":", SYS_OPS, DEFINE);
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
    parseF(m1i, "NOT", EQ0);
    parseF(m1i, ">R", RTO);
    parseF(m1i, "R@", RFETCH);
    parseF(m1i, "R>", RFROM);
    parseF(m1i, "DO", DO);
    parseF(m1i, "LOOP", LOOP);
    parseF(m1i, "-LOOP", LOOP2);
    parseF(m1i, "(I)", INDEX);
    parseF(m1i, "COM", COM);
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
    parseF(m2i, "(CREATE)",  SYS_OPS, CREATE);
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
    parseF(": VERSION     #%ld ;", VERSION);
    parseF(": (SP)        $%lx ;", &DSP);
    parseF(": (RSP)       $%lx ;", &RSP);
    parseF(": (LSP)       $%lx ;", &lsp);
    parseF(": (HERE)      $%lx ;", &here);
    parseF(": (LAST)      $%lx ;", &last);
    parseF(": (STK)       $%lx ;", &ds.stk[0].i);
    parseF(": (RSTK)      $%lx ;", &rs.stk[0].c);
    parseF(": TIB         $%lx ;", &tib[0]);
    parseF(": >IN         $%lx ;", &in);
    parseF(": CODE        $%lx ;", &code[0]);
    parseF(": CODE-SZ     #%ld ;", CODE_SZ);
    parseF(": VARS        $%lx ;", &vars[0]);
    parseF(": VARS-SZ     #%ld ;", VARS_SZ);
    parseF(": (VHERE)     $%lx ;", &vhere);
    parseF(": (REGS)      $%lx ;", &reg[0]);
    parseF(": (OUTPUT_FP) $%lx ;", &output_fp);
    parseF(": (INPUT_FP)  $%lx ;", &input_fp);
    parseF(": STATE       $%lx ;", &state);
    parseF(": BASE        $%lx ;", &base);
    parseF(": WORD-SZ     #%ld ;", sizeof(dict_t));
    parseF(": BYE  %d STATE !  ;", ALL_DONE);
    parseF(": CELL %d ; inline",   CELL_SZ);

    // Main system
    ParseLine(": \\ 0 >in @ ! ; immediate");
    ParseLine(": [ 0 state ! ; immediate");
    ParseLine(": ] 1 state ! ;");
    ParseLine(": last (last) @ ;");
    ParseLine(": here (here) @ ;");
    ParseLine(": code-end  code code-sz + ;");
    ParseLine(": vars-end  vars vars-sz + ;");
    ParseLine(": ++ DUP @ 1+ SWAP ! ; inline");
    ParseLine(": -- DUP @ 1- SWAP ! ; inline");
    ParseLine(": vhere  (vhere) @ ;");
    ParseLine(": allot  vhere + (vhere) ! ;");
    ParseLine(": vc, vhere c! (vhere) ++ ;");
    ParseLine(": v,  vhere ! cell allot ;");
    ParseLine(": cells cell * ; inline");
    ParseLine(": create (create) vhere (lit) c, , (exit) here c! ;");
    ParseLine(": does>  R> (jmp) c, , ;");
    ParseLine(": constant  (create) (lit) c, , (exit) c, ;");
    ParseLine(": variable   create 0 v, (exit) c, ;");
    ParseLine(": val    create 0 v, does> @ ;");
    ParseLine(": >val   create does> cell - ! ;");
    ParseLine(": (val)  vhere cell - constant ;");
    ParseLine(": :noname  here 1 state ! ;");
    ParseLine(": exec  >R ;");
    ParseLine(": IF    (jmpz) c, here 0 , ; immediate");
    ParseLine(": ELSE  (jmp) c, here SWAP 0 , here SWAP ! ; immediate");
    ParseLine(": THEN  here SWAP ! ; immediate");
    ParseLine(": exit  (exit) c,   ; immediate");
    ParseLine(": begin  here        ; immediate");
    ParseLine(": until  (jmpz) c, , ; immediate");
    ParseLine(": again  (jmp) c, ,  ; immediate");
    ParseLine(": while  (jmpz) c, here 0 , ; immediate");
    ParseLine(": repeat SWAP (jmp) c, , here SWAP ! ; immediate");
    ParseLine(": for 0 SWAP do ; inline");
    ParseLine(": next -loop ; inline");
    ParseLine(": -if (DUP) c, (jmpz) c, here 0 , ; immediate");
    ParseLine(": -until (DUP) c,  (jmpz) c, , ; immediate");
    ParseLine(": -while (jmpnz) c, , ; immediate");
    ParseLine(": tuck  SWAP OVER ; inline");
    ParseLine(": nip   SWAP DROP ; inline");
    ParseLine(": 2dup  OVER OVER ; inline");
    ParseLine(": 2drop DROP DROP ; inline");
    ParseLine(": ?DUP  DUP IF DUP THEN ;");
    ParseLine(": /   /mod nip  ; inline");
    ParseLine(": mod /mod DROP ; inline");
    ParseLine(": +! SWAP OVER @ + SWAP ! ;  inline");
    ParseLine(": c++ DUP c@ 1+ SWAP c! ; inline");
    ParseLine(": 2*  DUP + ; inline");
    ParseLine(": 2/  2 / ; inline");
    ParseLine(": 2+  1+ 1+ ; inline");
    ParseLine(": <=  > 0= ; inline");
    ParseLine(": >=  < 0= ; inline");
    ParseLine(": <>  = 0= ; inline");
    ParseLine(": rdrop R> DROP ; inline");
    ParseLine(": rot   >R SWAP r> SWAP ;");
    ParseLine(": -rot  SWAP >R SWAP R> ;");
    ParseLine(": ( begin");
    ParseLine("    >in @ c@ DUP 0= IF DROP exit THEN");
    ParseLine("    >in ++ ')' = IF exit THEN");
    ParseLine("    again ; immediate");
    ParseLine(": bl  #32 ; inline");
    ParseLine(": tab  #9 EMIT ; inline");
    ParseLine(": cr  #13 EMIT #10 EMIT ; inline");
    ParseLine(": space bl EMIT ; inline");
    ParseLine(": . (.) space ; inline");
    ParseLine(": negate  com 1+ ; inline");
    ParseLine(": abs  DUP 0 < IF negate THEN ;");
    ParseLine(": min  OVER OVER > IF SWAP THEN DROP ;");
    ParseLine(": max  OVER OVER < IF SWAP THEN DROP ;");
    ParseLine(": btw +regs s3 s2 s1 r2 r1 <= r1 r3 <= and -regs ;");
    ParseLine(": i  (i) @ ;");
    ParseLine(": j  (i) 3 cells - 0 max @ ;");
    ParseLine(": +i (i) +! ;");
    ParseLine(": unloop (lsp) @ 3 - 0 max (lsp) ! ;");
    ParseLine(": 0sp 0 (sp) ! ;");
    ParseLine(": depth (sp) @ 1- ;");
    ParseLine(": .s '(' EMIT space depth ?DUP IF");
    ParseLine("    0 do (stk) i 1+ 8 * + @ . loop");
    ParseLine("    THEN ')' EMIT ;");
    ParseLine(": dump ( a n-- ) for DUP c@ . 1+ next DROP ;");
    ParseLine(": T3 ( --zstr end )   +regs   >in ++");
    ParseLine("    vhere DUP s8 s9");
    ParseLine("    begin >in @ c@ s1");
    ParseLine("        r1 IF >in ++ THEN");
    ParseLine("        r1 0= r1 '\"' = or");
    ParseLine("        IF 0 r8+ c!   r9 r8 -regs   exit THEN");
    ParseLine("        r1   r8+ c!");
    ParseLine("    again ;");
    ParseLine(": \" ( --SZ )  T3 state @ 0= IF DROP exit THEN (vhere) ! (lit) c, , ; immediate");
    ParseLine(": .\" ( -- )   T3 state @ 0= IF DROP ztype exit THEN");
    ParseLine("    (vhere) ! (lit) c, , (ztype) c, ; immediate");
    ParseLine(": .word cell + 2+ qtype ; inline");
    ParseLine(": words +regs 0 s1 0 s3 last s2 begin");
    ParseLine("        r2 code-end < while");
    ParseLine("        r1+ #10 > IF 0 s1 cr THEN");
    ParseLine("        i3 r2 .word tab r2 word-sz + s2");
    ParseLine("    repeat");
    ParseLine("    r3 .\" (%d words)\" -regs ;");
    ParseLine(": binary  %10 base ! ;");
    ParseLine(": decimal #10 base ! ;");
    ParseLine(": hex     $10 base ! ;");
    ParseLine(": ? @ . ;");
    ParseLine(": lshift ( n1 s--n2 ) 0 do 2* loop ;");
    ParseLine(": rshift ( n1 s--n2 ) 0 do 2/ loop ;");
    ParseLine(": load next-word DROP (load) ;");
    ParseLine(": load-abort #99 state ! ;");
    ParseLine(": loaded? IF 2drop load-abort THEN ;");
    ParseLine("variable T0 2 cells allot");
    ParseLine(": T1 cells T0 + ;");
    ParseLine(": marker here 0 T1 ! vhere 1 T1 ! last 2 T1 ! ;");
    ParseLine(": forget 0 T1 @ (here) ! 1 T1 @ (vhere) ! 2 T1 @ (last) ! ;");
    ParseLine(": forget-1 last @ (here) ! last word-sz + (last) ! ;");
    loadUserWords();

    ParseLine("marker");
}
