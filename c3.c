// c3.cpp - a minimal Forth-like VM

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

typedef long cell_t;
typedef unsigned long ucell_t;
typedef unsigned char byte;
typedef double flt_t;

#include "sys-init.h"

typedef union { cell_t i; flt_t f; char *c; } se_t;
typedef struct { cell_t sp; se_t stk[STK_SZ+1]; } stk_t;
typedef struct { cell_t xt; byte f; byte len; char name[NAME_LEN+1]; } dict_t;

enum {
    STOP = 0, LIT1, LIT, EXIT, CALL, JMP, JMPZ, JMPNZ,
    STORE, CSTORE, FETCH, CFETCH, DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, SUB, INC, DEC, LT, EQ, GT, EQ0,
    RTO, RFETCH, RFROM, DO, LOOP, LOOP2, INDEX,
    COM, AND, OR, XOR, UNUSED1, ZTYPE,
    REG_I, REG_D, REG_R, REG_RD, REG_RI, REG_S,
    REG_NEW, REG_FREE,
    SYS_OPS, STR_OPS, FLT_OPS
};

// NB: these skip #3 (EXIT), so they can be marked as INLINE
enum { // System opcodes
    INLINE=0, IMMEDIATE, DOT, ITOA = 4,
    DEFINE, ENDWORD, CREATE, FIND, WORD, TIMER,
    CCOMMA, COMMA, KEY, QKEY, EMIT, QTYPE
};

enum { // String opcodes
    TRUNC=0, LCASE, UCASE, STRCPY=4, STRCAT, STRCATC, STRLEN, STREQ, STREQI
};

enum { // Floating point opcdes
    FADD=0, FSUB, FMUL, FDIV = 4, FEQ, FLT, FGT, F2I, I2F, FDOT,
    SQRT, TANH
};

enum { STOP_LOAD = 99, ALL_DONE = 999, VERSION = 90 };

#define BTW(a,b,c)    ((b<=a) && (a<=c))
#define CELL_SZ       sizeof(cell_t)
#define CpAt(x)       (char*)Fetch((char*)x)
#define ToCP(x)       (char*)(x)
#define ClearTib      fill(tib, 0, sizeof(tib))
#define SC(x)         strCat(tib, x)
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
#define IS_IMMEDIATE  1
#define IS_INLINE     2
#define NCASE         goto next; case
#define RCASE         return pc; case

stk_t ds, rs;
cell_t lstk[LSTK_SZ+1], lsp;
cell_t fileStk[10], fileSp, input_fp, output_fp;
cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
char code[CODE_SZ], vars[VARS_SZ], tib[256], WD[32];
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
char *strEnd(char *s) { while (*s) ++s; return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCatC(char *d, const char c) { d=strEnd(d); *(d++)=c; *d=0; }
void strCpy(char *d, const char *s) { if (d != s) { *d = 0; strCat(d, s); } }
int strLen(const char *d) { int len = 0; while (*d++) { ++len; } return len; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }
int upper(int x) { return BTW(x,'a','z') ? x-32: x; }

int strEq(const char *d, const char *s, int caseSensitive) {
    while (*s || *d) {
        if (caseSensitive) { if (*s != *d) return 0; }
        else { if (lower(*s) != lower(*d)) return 0; }
        s++; d++;
    }
    return 1;
}

void printStringF(const char *fmt, ...) {
    char *buf = (char*)last;
    buf -= 256;
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, 200, fmt, args);
    va_end(args);
    printString(buf);
}

char *iToA(cell_t N, int b) {
    static char buf[65];
    ucell_t X = (ucell_t)N;
    int isNeg = 0;
    if (b == 0) { b = base; }
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
    if (w[0]=='r') { 
        if (w[2]==0) { return REG_R; }
        if ((w[2]=='-') && (w[3]==0))  { return REG_RD; }
        if ((w[2]=='+') && (w[3]==0))  { return REG_RI; }
        return 0;
    }
    if (w[2]) { return 0; }
    if (w[0]=='s') { return REG_S; }
    if (w[0]=='i') { return REG_I; }
    if (w[0]=='d') { return REG_D; }
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

void doCreate(char *nm) {
    if (nm == 0) { nextWord(); nm = WD; }
    if (isTempWord(nm)) { tempWords[nm[1]-'0'].xt = (cell_t)here; return; }
    int l = strLen(nm);
    --last;
    if (NAME_LEN < l) { l=NAME_LEN; nm[l]=0; printString("-nameTrunc-"); }
    strCpy(last->name, nm);
    last->len = l;
    last->xt = (cell_t)here;
    last->f = 0;
}

// ( nm--xt flags | <null> )
int doFind(const char *nm) {
    if (nm == 0) { nextWord(); nm = WD; }
    if (isTempWord(nm)) {
        n1 = nm[1]-'0';
        push(tempWords[n1].xt);
        push(tempWords[n1].f);
        return 1;
    }
    int len = strLen(nm);
    dict_t *dp = last;
    while (dp < (dict_t*)&code[CODE_SZ]) {
        if ((len==dp->len) && strEq(nm, dp->name, 0)) {
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
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { return 0; }
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
    int b = base, lastCh = '9';
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
    for (int i = 0; i < str[i]; i++) {
        char c = str[i];
        if (c == '%') {
            c = str[++i];
            if (c == 0) { return; }
            else if (c=='b') { printString(iToA(pop(), 2)); }
            else if (c=='c') { printChar((char)pop()); }
            else if (c=='d') { printString(iToA(pop(), 10)); }
            else if (c=='e') { printChar(27); }
            else if (c=='f') { printStringF("%f", fpop()); }
            else if (c=='g') { printStringF("%g", fpop()); }
            else if (c=='i') { printString(iToA(pop(), base)); }
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
    char *d, *s;
    switch(*pc++) {
        case TRUNC:    d=cpop(); d[0] = 0;
        RCASE LCASE:   TOS = lower((int)TOS);
        RCASE UCASE:   TOS = upper((int)TOS);
        RCASE STRCPY:  s=cpop(); d=cpop(); strCpy(d, s);
        RCASE STRCAT:  s=cpop(); d=cpop(); strCat(d, s);
        RCASE STRCATC: t1=pop(); d=cpop(); strCatC(d, (char)t1);
        RCASE STRLEN:  d=CTOS; TOS=strLen(d);
        RCASE STREQ:   s=cpop(); d=CTOS; TOS=strEq(d, s, 0);
        RCASE STREQI:  s=cpop(); d=CTOS; TOS=strEq(d, s, 1);
            return pc;
        default: printStringF("-strOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doSysOp(char *pc) {
    switch(*pc++) {
        case INLINE: last->f = IS_INLINE;
        RCASE IMMEDIATE: last->f = IS_IMMEDIATE;
        RCASE DOT: printString(iToA(pop(), base));
        RCASE ITOA: TOS = (cell_t)iToA(TOS, base);
        RCASE DEFINE: doCreate(ToCP(0)); state=1;
        RCASE ENDWORD: state=0; CComma(EXIT);
        RCASE CREATE: doCreate(ToCP(0));
        RCASE FIND: push(doFind(ToCP(0)));
        RCASE WORD: t1=nextWord(); push((cell_t)WD); push(t1);
        RCASE TIMER: push(sysTime());
        RCASE CCOMMA: CComma(pop());
        RCASE COMMA: Comma(pop());
        RCASE KEY:  push(key());
        RCASE QKEY: push(qKey());
        RCASE EMIT: printChar((char)pop());
        RCASE QTYPE: printString(cpop());
            return pc; 
        default: printStringF("-sysOp:[%d]?-", *(pc-1));
    }
    return pc;
}

char *doFloatOp(char *pc) {
    flt_t x;
    switch(*pc++) {
        case  FADD: x = fpop(); FTOS += x;
        RCASE FSUB: x = fpop(); FTOS -= x;
        RCASE FMUL: x = fpop(); FTOS *= x;
        RCASE FDIV: x = fpop(); FTOS /= x;
        RCASE FEQ:  x = fpop(); TOS = (x == FTOS);
        RCASE FLT:  x = fpop(); TOS = (x > FTOS);
        RCASE FGT:  x = fpop(); TOS = (x < FTOS);
        RCASE F2I:  TOS = (cell_t)FTOS;
        RCASE I2F:  FTOS = (flt_t)TOS;
        RCASE FDOT: printStringF("%g", fpop());
        RCASE SQRT: FTOS = sqrt(FTOS);
        RCASE TANH: FTOS = tanh(FTOS);
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
        case  JMP: pc = CpAt(pc);
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
        NCASE AND: t1=pop(); TOS = (TOS & t1);
        NCASE OR:  t1=pop(); TOS = (TOS | t1);
        NCASE XOR: t1=pop(); TOS = (TOS ^ t1);
        NCASE UNUSED1: printString("-unused1-");
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
    if ((state) || (!strEq(w,"-ML-",1))) { return 0; }
    doCreate((char*)0);
    while (nextWord()) {
        if (strEq(WD,"-MLX-",1)) { return 1; }
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
        int h=245;
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
    in = (char *)x;
    while ((state != ALL_DONE) && nextWord()) {
        if (doNum(WD)) { continue; }
        if (doML(WD)) { continue; }
        if (doReg(WD)) { continue; }
        if (doWord(WD)) { continue; }
        printStringF("-[word:%s]?-", WD);
        if (state) { here = ToCP((last++)->xt); state = 0; }
        while (fileSp) { fclose((FILE*)fileStk[fileSp--]); }
        input_fp = 0;
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

#include "sys-load.h"

void c3Init() {
    here = &code[0];
    vhere = &vars[0];
    last = (dict_t*)&code[CODE_SZ];
    base = 10;
    DSP = RSP = reg_base = 0;
    sysLoad();

    for (int i=0; i<6; i++) { tempWords[i].f = 0; }
    for (int i=6; i<9; i++) { tempWords[i].f = IS_INLINE; }
    tempWords[9].f = IS_IMMEDIATE;

    ParseLine("version 100 /mod .\" c3 - v%d.%d - Chris Curl%n\"");
    ParseLine("here code - .\" %d code bytes used, \" last here - .\" %d bytes free.%n\"");
    ParseLine("vhere vars - .\" %d variable bytes used, \" vars-end vhere - .\" %d bytes free.\"");
    ParseLine(": benches forget \" benches.c3\" (load) ;");
    ParseLine(": sb forget \" sandbox.c3\" (load) ;");
    ParseLine("marker");
}

#ifdef isPC
#include "sys-pc.h"
#endif
