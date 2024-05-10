// c3 - a stack-based VM

#include <stdarg.h>
#include <math.h>
#include "c3.h"

enum {
    STOP = 0, LIT1, LIT, EXIT, CALL, JMP, JMPZ, JMPNZ,
    STORE, CSTORE, FETCH, CFETCH, DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, SUB, INC, DEC, LT, EQ, GT, EQ0,
    RTO, RFETCH, RFROM, DO, LOOP, LOOP2, INDEX,
    COM, AND, OR, XOR, TYPE, ZTYPE,
    REG_I, REG_D, REG_R, REG_RD, REG_RI, REG_S,
    REG_NEW, REG_FREE,
    SYS_OPS, STR_OPS, FLT_OPS
};

// NB: these skip #3 (EXIT), so they can be marked as INLINE
enum { // System opcodes
    INLINE=0, IMMEDIATE, DOT, ITOA=4, ATOI,
    COLONDEF, ENDWORD, CREATE, FIND, WORD, TIMER,
    CCOMMA, COMMA, KEY, QKEY, EMIT, QTYPE
};

enum { // String opcodes
    TRUNC=0, LCASE, UCASE, STRCPY=4, STRCAT, STRCATC, STRLEN,
    STREQ, STREQI, STREQN, LTRIM, RTRIM, FINDC
};

enum { // Floating point opcdes
    FADD=0, FSUB, FMUL, FDIV=4, FEQ, FLT, FGT, F2I, I2F, FDOT,
    SQRT, TANH
};

#define CpAt(x)       (char*)Fetch((char*)x)
#define DSP           ds.sp
#define TOS           (ds.stk[DSP].i)
#define NOS           (ds.stk[DSP-1].i)
#define FTOS          (ds.stk[DSP].f)
#define FNOS          (ds.stk[DSP-1].f)
#define CTOS          (ds.stk[DSP].c)
#define RSP           rs.sp
#define RTOS          (rs.stk[RSP].c)
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define IS_IMMEDIATE  0x01
#define IS_INLINE     0x02

stk_t ds, rs;
cell_t lstk[LSTK_SZ+1], lsp, output_fp;
cell_t state, base, reg[REGS_SZ], reg_base, t1, n1, lexicon;
char code[CODE_SZ], vars[VARS_SZ], tib[TIB_SZ], WD[32];
char *here, *vhere, *in, *y;
dict_t tempWords[10], *last;

void push(cell_t x) { ds.stk[++DSP].i = (cell_t)(x); }
cell_t pop() { return ds.stk[DSP--].i; }
char *cpop() { return ds.stk[DSP--].c; }

void fpush(flt_t x) { ds.stk[++DSP].f = (x); }
flt_t fpop() { return ds.stk[DSP--].f; }

void rpush(char *x) { rs.stk[++RSP].c = (x); }
char *rpop() { return rs.stk[RSP--].c; }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { Store(here, x); here += CELL_SZ; }

void fill(char *d, char val, int num) { for (int i=0; i<num; i++) { d[i]=val; } }
char *strEnd(char *s) { while (*s) { ++s; } return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCatC(char *d, const char c) { d=strEnd(d); *(d++)=c; *d=0; }
void strCpy(char *d, const char *s) { if (d != s) { *d = 0; strCat(d, s); } }
int strLen(const char *d) { return (int)(strEnd((char*)d)-d); }
char *lTrim(char *d) { while (*d && (*d<33)) { ++d; } return d; }
char *rTrim(char *d) { char *s=strEnd(d)-1; while ((d<=s) && (*s< 33)) { *(s--) = 0; } return d; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }
int upper(int x) { return BTW(x,'a','z') ? x-32: x; }

int strEq(const char *d, const char *s) {
    while (*(s++) == *(d++)) { if (*(s-1)==0) { return 1; } }
    return 0;
}

int strEqI(const char *s, const char *d) {
    while (lower(*s++) == lower(*d++)) { if (*(s-1)==0) { return 1; } }
    return 0;
}

int strEqN(const char *s, const char *d, cell_t n) {
    while (*(s++) == *(d++) && (n)) { --n; }
    return (n==0) ? 1 : 0;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)last;
    buf -= 256;
    if (buf<here) { buf=here+1; }
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 255, fmt, args);
    va_end(args);
    printString(buf);
}

char *iToA(cell_t N, int b) {
    static char buf[65];
    ucell_t X = (ucell_t)N;
    int isNeg = 0;
    if (b == 0) { b = (int)base; }
    if ((b == 10) && (N < 0)) { isNeg = 1; X = -N; }
    char c, *cp = &buf[64];
    *(cp) = 0;
    do {
        c = (char)(X % b) + '0';
        X /= b;
        c = (c > '9') ? c+7 : c;
        *(--cp) = c;
    } while (X);
    if (isNeg) { *(--cp) = '-'; }
    return cp;
}

int isTempWord(const char *w) {
    return ((w[0]=='T') && BTW(w[1],'0','9') && (w[2]==0)) ? 1 : 0;
}

char isRegOp(const char *w) {
    if (!BTW(w[1],'0','9')) { return 0; }
    if ((w[0]=='r') && (w[2]==0)) return REG_R;
    if ((w[0]=='s') && (w[2]==0)) return REG_S;
    if ((w[0]=='i') && (w[2]==0)) return REG_I;
    if ((w[0]=='d') && (w[2]==0)) return REG_D;
    if ((w[0]=='r') && (w[2]=='+') && (w[3]==0)) return REG_RI;
    if ((w[0]=='r') && (w[2]=='-') && (w[3]==0)) return REG_RD;
    return 0;
}

int nextWord() {
    int len = 0;
    if (DSP < 0) { printString("-under-"); DSP=0; }
    if (STK_SZ < DSP) { printString("-over-"); DSP=STK_SZ; }
    while (*in && (*in < 33)) { ++in; }
    while (32 < *in) { WD[len++] = *(in++); }
    WD[len] = 0;
    return len;
}

void addWord() {
    nextWord();
    if (isTempWord(WD)) { tempWords[WD[1]-'0'].xt = (cell_t)here; return; }
    int l = strLen(WD);
    if (NAME_LEN < l) { printStringF("-nameTrunc:%s-", WD); l=NAME_LEN; WD[l]=0; }
    --last;
    strCpy(last->name, WD);
    last->len = l;
    last->xt = (cell_t)here;
    last->f = 0;
    last->lex = (byte)lexicon;
}

// ( nm--xt flags | <null> )
int doFind(const char *nm) {
    if (nm == 0) { nextWord(); nm = WD; }
    if (isTempWord(nm)) {
        push(tempWords[nm[1]-'0'].xt);
        push(tempWords[nm[1]-'0'].f);
        return 1;
    }
    int len = strLen(nm);
    dict_t *dp = last;
    while (dp < (dict_t*)&code[CODE_SZ]) {
        if ((len==dp->len) && strEqI(nm, dp->name)) {
            push(dp->xt);
            push(dp->f);
            return 1;
        }
        ++dp;
    }
    return 0;
}

// ( --n | <null> )
int isBase10(const char *wd) {
    cell_t x = 0, isNeg = 0;
    if (*wd == '-') { isNeg=1; ++wd; }
    if (!BTW(*wd, '0', '9')) { return 0; }
    while (BTW(*wd, '0', '9')) { x = (x*10)+(*(wd++)-'0'); }
    if (*wd == 0) { push(isNeg ? -x : x); return 1; }
    if (*(wd++) != '.') { return 0; }
    flt_t fx = (flt_t)x, f = 0, fy = 1;
    while (BTW(*wd, '0', '9')) { f = (f*10)+(*(wd++)-'0'); fy*=10; }
    if (*wd) { return 0; }
    fx += f/fy;
    fpush(isNeg ? -fx : fx);
    return 1;
}

// ( --n | <null> )
int isNum(const char *wd) {
    if ((wd[0]=='\'') && (wd[2]=='\'') && (wd[3]==0)) { push(wd[1]); return 1; }
    int b = (int)base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b =  2;  ++wd; }
    if (*wd == 0) { return 0; }
    if (b == 10) { return isBase10(wd); }
    if (b < 10) { lastCh = '0' + b - 1; }
    cell_t x = 0;
    while (*wd) {
        cell_t t = -1, c = *(wd++);
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { return 0; }
        x = (x * b) + t;
    }
    push(x);
    return 1;
}

void doType(const char *str) {
    if (!str) { str=cpop(); }
    while (*str) {
        char c=*(str++);
        if (c == '%') {
            c = *(str++);
            if (c == 0) { return; }
            else if (c=='b') { printString(iToA(pop(), 2)); }
            else if (c=='c') { printChar((char)pop()); }
            else if (c=='d') { printString(iToA(pop(), 10)); }
            else if (c=='e') { printChar(27); }
            else if (c=='f') { printStringF("%f", fpop()); }
            else if (c=='g') { printStringF("%g", fpop()); }
            else if (c=='i') { printString(iToA(pop(), (int)base)); }
            else if (c=='n') { printString("\n\r"); }
            else if (c=='q') { printChar(34); }
            else if (c=='s') { printString(cpop()); }
            else if (c=='t') { printChar(9); }
            else if (c=='x') { printString(iToA(pop(), 16)); }
            else { printChar(c); }
        } else {
            printChar(c);
        }
    }
}

char *doStringOp(char *pc) {
    char *d=NULL, *s=NULL;
    switch(*pc++) {
        case TRUNC:    d=cpop(); d[0] = 0;
        RCASE LCASE:   TOS = lower((int)TOS);
        RCASE UCASE:   TOS = upper((int)TOS);
        RCASE STRCPY:  s=cpop(); d=cpop(); strCpy(d, s);
        RCASE STRCAT:  s=cpop(); d=cpop(); strCat(d, s);
        RCASE STRCATC: t1=pop(); d=cpop(); strCatC(d, (char)t1);
        RCASE STRLEN:  TOS=strLen(CTOS);
        RCASE STREQ:   s=cpop(); d=CTOS; TOS=strEq(d, s);
        RCASE STREQI:  s=cpop(); d=CTOS; TOS=strEqI(d, s);
        RCASE STREQN:  t1=pop(); s=cpop(); d=cpop(); push(strEqN(d, s, t1));
        RCASE LTRIM:   CTOS=lTrim(CTOS);
        RCASE RTRIM:   rTrim(CTOS);
        RCASE FINDC:   s=cpop(); while (*s && (*s!=TOS)) { ++s; }
                       TOS = (*s) ? (cell_t)s : 0;
        return pc; default: printStringF("-strOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doSysOp(char *pc) {
    switch(*pc++) {
        case INLINE: last->f = IS_INLINE;
        RCASE IMMEDIATE: last->f = IS_IMMEDIATE;
        RCASE DOT:   printString(iToA(pop(), (int)base));
        RCASE ITOA:  TOS = (cell_t)iToA(TOS, (int)base);
        RCASE ATOI:  push(isNum(cpop()));
        RCASE COLONDEF: addWord(); state=1;
        RCASE ENDWORD:  state=0; CComma(EXIT);
        RCASE CREATE:   addWord(); CComma(LIT); Comma((cell_t)vhere);
        RCASE FIND:   push(doFind(ToCP(0)));
        RCASE WORD:   t1=nextWord(); push((cell_t)WD); push(t1);
        RCASE TIMER:  push(sysTime());
        RCASE CCOMMA: CComma(pop());
        RCASE COMMA:  Comma(pop());
        RCASE KEY:    push(key());
        RCASE QKEY:   push(qKey());
        RCASE EMIT:   printChar((char)pop());
        RCASE QTYPE:  printString(cpop());
            return pc; 
        default: printStringF("-sysOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doFloatOp(char *pc) {
    flt_t x;
    switch(*pc++) {
        case  FADD: x=fpop(); FTOS += x;
        RCASE FSUB: x=fpop(); FTOS -= x;
        RCASE FMUL: x=fpop(); FTOS *= x;
        RCASE FDIV: x=fpop(); FTOS /= x;
        RCASE FEQ:  x=fpop(); TOS = (x == FTOS);
        RCASE FLT:  x=fpop(); TOS = (x > FTOS);
        RCASE FGT:  x=fpop(); TOS = (x < FTOS);
        RCASE F2I:  TOS = (cell_t)FTOS;
        RCASE I2F:  FTOS = (flt_t)TOS;
        RCASE FDOT: printStringF("%g", fpop());
        RCASE SQRT: FTOS = (flt_t)sqrt(FTOS);
        RCASE TANH: FTOS = (flt_t)tanh(FTOS);
            return pc; 
        default: printStringF("-fltOp:[%d]?-", *(pc-1));
    }
    return pc;
}

void Run(char *pc) {
    flt_t f1;
next:
    switch (*(pc++)) {
        case STOP: return;
        NCASE LIT1: push(*(pc++));
        NCASE LIT: push(Fetch(pc)); pc += CELL_SZ;
        NCASE EXIT: if (RSP<1) { RSP=0; return; } pc=rpop();
        NCASE CALL: y=pc+CELL_SZ; if (*y!=EXIT) { rpush(y); }          // fall-thru
        case  JMP:  pc = CpAt(pc);
        NCASE JMPZ:  if (pop()==0) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE JMPNZ: if (TOS) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE STORE:  Store(CTOS, NOS);  DSP-=2; if (DSP < 1) { DSP = 0; }
        NCASE CSTORE: *CTOS = (char)NOS; DSP-=2; if (DSP < 1) { DSP = 0; }
        NCASE FETCH: TOS = Fetch(CTOS);
        NCASE CFETCH: TOS = *(byte*)(TOS);
        NCASE DUP: fpush(FTOS);
        NCASE SWAP: f1 = FTOS; FTOS = FNOS; FNOS = f1;
        NCASE OVER: fpush(FNOS);
        NCASE DROP: if (--DSP < 0) { DSP = 0; }
        NCASE ADD:   t1=pop(); TOS += t1;
        NCASE MULT:  t1=pop(); TOS *= t1;
        NCASE SLMOD: t1=TOS; TOS = (NOS/t1); NOS %= t1;
        NCASE SUB:   t1=pop(); TOS -= t1;
        NCASE INC: ++TOS;
        NCASE DEC: --TOS;
        NCASE LT: t1=pop(); TOS = (TOS<t1);
        NCASE EQ: t1=pop(); TOS = (TOS==t1);
        NCASE GT: t1=pop(); TOS = (TOS>t1);
        NCASE EQ0: TOS = (TOS==0);
        NCASE RTO: rpush((char *)pop());
        NCASE RFETCH: push((cell_t)RTOS);
        NCASE RFROM: push((cell_t)rpop());
        NCASE DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();
        NCASE LOOP:  if (++L0<L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE LOOP2: if (--L0>L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE INDEX: push((cell_t)&L0);
        NCASE COM: TOS = ~TOS;
        NCASE AND: t1=pop(); TOS &= t1;
        NCASE OR:  t1=pop(); TOS |= t1;
        NCASE XOR: t1=pop(); TOS ^= t1;
        NCASE TYPE: t1=pop(); y=cpop(); for (int i=0; i<t1; i++) { printChar(*(y++)); }
        NCASE ZTYPE: doType(0);
        NCASE REG_I: reg[*(pc++)+reg_base]++;
        NCASE REG_D: reg[*(pc++)+reg_base]--;
        NCASE REG_R:  push(reg[*(pc++)+reg_base]);
        NCASE REG_RD: push(reg[*(pc++)+reg_base]--);
        NCASE REG_RI: push(reg[*(pc++)+reg_base]++);
        NCASE REG_S: reg[*(pc++)+reg_base] = pop();
        NCASE REG_NEW: reg_base += (reg_base < (REGS_SZ-10)) ? 10 : 0;
        NCASE REG_FREE: reg_base -= (9 < reg_base) ? 10 : 0;
        NCASE SYS_OPS: pc = doSysOp(pc);
        NCASE STR_OPS: pc = doStringOp(pc);
        NCASE FLT_OPS: pc = doFloatOp(pc);
            goto next;
        default: pc = doUser(pc, *(pc-1));
            if (pc) { goto next; }
            printStringF("-op:[%d]?-", *(pc-1));
    }
}

int doNum(const char *w) {
    if (isNum(w) == 0) { return 0; }
    if (state == 0) { return 1; }
    if (BTW(TOS, 0, 127)) { CComma(LIT1); CComma(pop()); }
    else { CComma(LIT); Comma(pop()); }
    return 1;
}

int doML(const char *w) {
    if ((state) || (!strEq(w,"-ML-"))) { return 0; }
    addWord();
    while (nextWord()) {
        if (strEq(WD,"-MLX-")) { return 1; }
        if (doNum(WD) == 0) { printStringF("-ML:[%s]?-", WD); return 1; }
        CComma(pop());
    }
    return 1;
}

int doReg(const char *w) {
    char t = isRegOp(w);
    if (t == 0) { return 0; }
    if (state) { CComma(t); CComma(w[1]-'0'); }
    else {
        int h=TIB_SZ-10;
        tib[h]=t; tib[h+1]=w[1]-'0'; tib[h+2]=EXIT;
        Run(&tib[h]);
    }
    return 1;
}

int doWord(const char *w) {
    if (doFind(w)==0) { return 0; }
    cell_t f = pop();
    char *xt = cpop();
    if ((state == 0) || (f & IS_IMMEDIATE)) { Run(xt); return 1; }
    if (f & IS_INLINE) {
        CComma(*(xt++));
        while (*xt != EXIT) { CComma(*(xt++)); }
    } else { CComma(CALL); Comma((cell_t)xt); }
    return 1;
}

void ParseLine(const char *x) {
    if (DSP < 1) { DSP = 0; }
    // printStringF("%s\r\n", x); // Debug
    if (state == ALL_DONE) { return; }
    in = (char *)x;
    while ((state != ALL_DONE) && nextWord()) {
        if (doNum(WD)) { continue; }
        if (doML(WD)) { continue; }
        if (doReg(WD)) { continue; }
        if (doWord(WD)) { continue; }
        printStringF("-[word:%s]?-", WD);
        if (state) { here = ToCP((last++)->xt); state = 0; }
        while (input_fp) { fClose(input_fp); input_fp = ipop(); }
        return;
    }
}

void parseF(const char *fmt, ...) {
    char buf[128];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 128, fmt, args);
    va_end(args);
    ParseLine(buf);
}

void loadC3Words() {
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
    parseF(lit, "(EXIT)", EXIT); last->f = 0;
    parseF(lit, "(CALL)", CALL);
    parseF(lit, "(JMP)", JMP);
    parseF(lit, "(JMPZ)", JMPZ);
    parseF(lit, "(JMPNZ)", JMPNZ);
    parseF(lit, "(STORE)", STORE);
    parseF(lit, "(FETCH)", FETCH);
    parseF(lit, "(ZTYPE)", ZTYPE);
    parseF(lit, "(-REGS)", REG_FREE);

    parseF(m1i, "EXIT", EXIT);
    parseF(m1i, "!", STORE);
    parseF(m1i, "C!", CSTORE);
    parseF(m1i, "@", FETCH);
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
    // rX, sX, iX, dX, iX+, dX+ are hard-coded in c3.c
    parseF(m1i, "+REGS", REG_NEW);
    parseF(m1i, "-REGS", REG_FREE);

    // System opcodes ...(INLINE and IMMEDIATE) were defined above
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
    parseF(m2i, "S-EQI",   STR_OPS, STREQI);
    parseF(m2i, "S-EQN",   STR_OPS, STREQN);
    parseF(m2i, "S-LTRIM", STR_OPS, LTRIM);
    parseF(m2i, "S-RTRIM", STR_OPS, RTRIM);
    parseF(m2i, "S-FINDC", STR_OPS, FINDC);

    // Float opcodes ...
    parseF(m2i, "F+",    FLT_OPS, FADD);
    parseF(m2i, "F-",    FLT_OPS, FSUB);
    parseF(m2i, "F*",    FLT_OPS, FMUL);
    parseF(m2i, "F/",    FLT_OPS, FDIV);
    parseF(m2i, "F=",    FLT_OPS, FEQ);
    parseF(m2i, "F<",    FLT_OPS, FLT);
    parseF(m2i, "F>",    FLT_OPS, FGT);
    parseF(m2i, "F>I",   FLT_OPS, F2I);
    parseF(m2i, "I>F",   FLT_OPS, I2F);
    parseF(m2i, "F.",    FLT_OPS, FDOT);
    parseF(m2i, "FSQRT", FLT_OPS, SQRT);
    parseF(m2i, "FTANH", FLT_OPS, TANH);

    // System information words
    parseF(lit, "VERSION", VERSION);
    parseF(lit, "VARS-SZ", VARS_SZ);
    parseF(lit, "CODE-SZ", CODE_SZ);
    parseF(lit, "WORD-SZ", sizeof(dict_t));
    parseF(addrFMT, "(scr-h)",     &edScrH);
    parseF(addrFMT, "(SP) ",       &DSP);
    parseF(addrFMT, "(RSP)",       &RSP);
    parseF(addrFMT, "(LSP)",       &lsp);
    parseF(addrFMT, "(HERE)",      &here);
    parseF(addrFMT, "(LAST)",      &last);
    parseF(addrFMT, "(STK) ",      &ds.stk[0].i);
    parseF(addrFMT, "(RSTK)",      &rs.stk[0].c);
    parseF(addrFMT, "CODE",        &code[0]);
    parseF(addrFMT, "VARS",        &vars[0]);
    parseF(addrFMT, "(VHERE)",     &vhere);
    parseF(addrFMT, "(REGS)",      &reg[0]);
    parseF(addrFMT, "(OUTPUT_FP)", &output_fp);
    parseF(addrFMT, "(INPUT_FP)",  &input_fp);
    parseF(addrFMT, "(LEXICON)",   &lexicon);
    parseF(addrFMT, "TIB",         &tib[0]);
    parseF(addrFMT, ">IN",         &in);
    parseF(addrFMT, "STATE",       &state);
    parseF(addrFMT, "BASE",        &base);
    parseF(lit, "CELL", CELL_SZ);
}

void c3Init() {
    here = &code[0];
    vhere = &vars[0];
    cell_t x = (cell_t)&code[CODE_SZ];
    while ((long)x & 0x03) { --x; }
    last = (dict_t*)x;
    base = 10;
    DSP = RSP = reg_base = lexicon = 0;

    for (int i=0; i<6; i++) { tempWords[i].f = 0; }
    for (int i=6; i<9; i++) { tempWords[i].f = IS_INLINE; }
    tempWords[9].f = IS_IMMEDIATE;
    
    loadC3Words();
    loadUserWords();
    sysLoad();
}
