// c3.cpp - a minimal Forth-like VM

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef long cell_t;
typedef unsigned long ucell_t;
typedef unsigned char byte;

#include "sys-init.inc"

typedef struct { cell_t xt; byte f; byte len; char name[NAME_LEN+1]; } dict_t;

enum {
    STOP = 0, LIT1, LIT4, EXIT, CALL, JMP, JMPZ, JMPNZ,
    STORE, CSTORE, FETCH, CFETCH, DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, SUB, INC, DEC, LT, EQ, GT, NOT,
    RTO, RFETCH, RFROM, DO, LOOP, LOOP2, INDEX,
    COM, AND, OR, XOR, EMIT, TIMER, KEY, QKEY,
    TYPE, TYPEZ, DEFINE, ENDWORD, CREATE, FIND, WORD,
    REG_I, REG_D, REG_R, REG_RD, REG_RI, REG_S, 
    REG_NEW, REG_FREE, INLINE, IMMEDIATE,
    STOP_LOAD = 99, ALL_DONE = 999, VERSION = 84
};

#include "sys-enum.inc"

#define BTW(a,b,c)    ((b<=a) && (a<=c))
#define CELL_SZ       sizeof(cell_t)
#define CpAt(x)       (char*)Fetch((char*)x)
#define ToCP(x)       (char*)(x)
#define ClearTib      fill(tib, 0, sizeof(tib))
#define SC(x)         strCat(tib, x)
#define TOS           (stk[sp])
#define NOS           (stk[sp-1])
#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]
#define IS_IMMEDIATE  1
#define IS_INLINE     2
#define NCASE         goto next; case
#define PRINT1(a)     printString(a)
#define PRINT3(a,b,c) { PRINT1(a); PRINT1(b); PRINT1(c); }

cell_t stk[STK_SZ+1], sp, rsp;
char *rstk[STK_SZ+1];
cell_t lstk[LSTK_SZ+1], lsp;
cell_t fileStk[10], fileSp, input_fp, output_fp;
cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
char mem[MEM_SZ], vars[VARS_SZ], tib[128], WD[32];
char *here, *vhere, *in, *y;
dict_t tempWords[10], *last;

void push(cell_t x) { stk[++sp] = (cell_t)(x); }
cell_t pop() { return stk[sp--]; }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { Store(here, x); here += CELL_SZ; }

void fill(char *d, char val, int num) { for (int i=0; i<num; i++) { d[i]=val; } }
char *strEnd(char *s) { while (*s) ++s; return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCpy(char *d, const char *s) { *d = 0; strCat(d, s); }
int strLen(const char *d) { int len = 0; while (*d++) { ++len; } return len; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }

int strEq(const char *d, const char *s, int caseSensitive) {
    while (*s || *d) {
        if (caseSensitive) { if (*s != *d) return 0; }
        else { if (lower(*s) != lower(*d)) return 0; }
        s++; d++;
    }
    return -1;
}

char *iToA(byte N) {
    tib[120] = N / 100 % 10 + '0';
    tib[121] = N /  10 % 10 + '0';
    tib[122] = N /   1 % 10 + '0';
    tib[123] = 0;
    return &tib[120];
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
    if (sp < 0) { PRINT1("-under-"); sp=0; }
    if (STK_SZ < sp) { PRINT1("-over-"); sp=0; }
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
    if (NAME_LEN < l) { l=NAME_LEN; nm[l]=0; PRINT1("-name-trunc-"); }
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
    while (dp < (dict_t*)&mem[MEM_SZ]) {
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
int isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { return 0; }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { return 0; }
    push(isNeg ? -x : x);
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
    if (b == 10) { return isDecimal(wd); }
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

void Run(char *pc) {
next:
    switch (*(pc++)) {
        NCASE EXIT: if (rsp<1) { rsp=0; return; } pc=rstk[rsp--];
        NCASE CALL: y=pc+CELL_SZ; if (*y!=EXIT) { rstk[++rsp]=y; }          // fall-thru
        case  JMP: pc = CpAt(pc);
        NCASE JMPZ:  if (pop()==0) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE JMPNZ: if (TOS) { pc=CpAt(pc); } else { pc+=CELL_SZ; }
        NCASE LIT1: push(*(pc++));
        NCASE LIT4: push(Fetch(pc)); pc += CELL_SZ;
        NCASE STORE: Store(ToCP(TOS), NOS); sp-=2;
        NCASE CSTORE: *ToCP(TOS) = (char)NOS; sp-=2;
        NCASE FETCH: TOS = Fetch(ToCP(TOS));
        NCASE CFETCH: TOS = *(byte*)(TOS);
        NCASE DUP: push(TOS);
        NCASE SWAP: t1 = TOS; TOS = NOS; NOS = t1;
        NCASE OVER: push(NOS);
        NCASE DROP: if (--sp < 0) { sp = 0; }
        NCASE ADD:   t1=pop(); TOS += t1;
        NCASE SUB:   t1=pop(); TOS -= t1;
        NCASE MULT:  t1=pop(); TOS *= t1;
        NCASE SLMOD: t1=TOS; TOS = (NOS/t1); NOS %= t1;
        NCASE LT: t1=pop(); TOS = (TOS<t1);
        NCASE EQ: t1=pop(); TOS = (TOS==t1);
        NCASE GT: t1=pop(); TOS = (TOS>t1);
        NCASE NOT: TOS = (TOS==0);
        NCASE EMIT: printChar((char)pop());
        NCASE TIMER: push(clock());
        NCASE INC: ++TOS;
        NCASE DEC: --TOS;
        NCASE DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();
        NCASE INDEX: push((cell_t)&L0);
        NCASE LOOP: if (++L0<L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE LOOP2: if (--L0>L1) { pc=ToCP(L2); } else { lsp-=3; };
        NCASE RTO: rstk[++rsp] = ToCP(pop());
        NCASE RFETCH: push((cell_t)rstk[rsp]);
        NCASE RFROM: push((cell_t)rstk[rsp--]);
        NCASE WORD: t1=nextWord(); push((cell_t)WD); push(t1);
        NCASE DEFINE: doCreate(0); state=1;
        NCASE CREATE: doCreate(0);
        NCASE FIND: push(doFind(0));
        NCASE ENDWORD: state=0; CComma(EXIT);
        NCASE AND: t1=pop(); TOS = (TOS & t1);
        NCASE OR:  t1=pop(); TOS = (TOS | t1);
        NCASE XOR: t1=pop(); TOS = (TOS ^ t1);
        NCASE COM: TOS = ~TOS;
        NCASE QKEY: push(qKey());
        NCASE KEY:  push(key());
        NCASE REG_D: reg[*(pc++)+reg_base]--;
        NCASE REG_I: reg[*(pc++)+reg_base]++;
        NCASE REG_R:  push(reg[*(pc++)+reg_base]);
        NCASE REG_RD: push(reg[*(pc++)+reg_base]--);
        NCASE REG_RI: push(reg[*(pc++)+reg_base]++);
        NCASE REG_S: reg[*(pc++)+reg_base] = pop();
        NCASE REG_NEW: reg_base += (reg_base < (REGS_SZ-10)) ? 10 : 0;
        NCASE REG_FREE: reg_base -= (9 < reg_base) ? 10 : 0;
        NCASE TYPE: t1=pop(); y=ToCP(pop()); for (int i=0; i<t1; i++) { printChar(*(y++)); }
        NCASE TYPEZ: PRINT1(ToCP(pop()));
        NCASE INLINE: last->f = IS_INLINE;
        NCASE IMMEDIATE: last->f = IS_IMMEDIATE;
#include "sys-exec.inc"
        NCASE STOP: return;
        default: PRINT3("-[", iToA((cell_t)*(pc-1)), "]?-")
    }
}

int doNum(const char *w) {
    if (isNum(w) == 0) { return 0; }
    if (state == 0) { return 1; }
    if (BTW(TOS,0,127)) { CComma(LIT1); CComma(pop()); }
    else { CComma(LIT4); Comma(pop()); }
    return 1;
}

int doML(const char *w) {
    if ((state) || (!strEq(w,"-ML-",1))) { return 0; }
    doCreate(0);
    while (nextWord()) {
        if (strEq(WD,"-MLX-",1)) { return 1; }
        if (doNum(WD) == 0) { PRINT3("[",WD,"]?"); return 1; }
        CComma(pop());
    }
    return 1;
}

int doReg(const char *w) {
    char t = isRegOp(w);
    if (t == 0) { return 0; }
    if (state) { CComma(t); CComma(w[1]-'0'); }
    else {
        tib[120]=t; tib[121]=w[1]-'0'; tib[122]=EXIT;
        Run(&tib[120]);
    }
    return 1;
}

int doWord(const char *w) {
    if (doFind(w)==0) { return 0; }
    cell_t f = pop();
    char *xt = ToCP(pop());
    if ((state == 0) || (f & IS_IMMEDIATE)) { Run(xt); return 1; }
    if (f & IS_INLINE) {
        CComma(*(xt++));
        while (*xt != EXIT) { CComma(*(xt++)); }
    } else { CComma(CALL); Comma((cell_t)xt); }
    return 1;
}

void ParseLine(const char *x) {
    in = (char *)x;
    while ((state != ALL_DONE) && nextWord()) {
        if (doNum(WD)) { continue; }
        if (doML(WD)) { continue; }
        if (doReg(WD)) { continue; }
        if (doWord(WD)) { continue; }
        PRINT3("[", WD, "]??")
        if (state) { here = ToCP(last++); state = 0; }
        return;
    }
}

struct { long op; const char *opName;  const char *c3Word; } prims[] = { 
    { DEFINE,             "",              ":" },
    { ENDWORD,            "",              ";" },
    { INLINE,             "",              "INLINE" },
    { IMMEDIATE,          "",              "IMMEDIATE" },
    { VERSION,            "VERSION",       "" },
    { (cell_t)&sp,        "(sp)",          "" },
    { (cell_t)&rsp,       "(rsp)",         "" },
    { (cell_t)&lsp,       "(lsp)",         "" },
    { (cell_t)&here,      "(here)",        "" },
    { (cell_t)&vhere,     "(vhere)",       "" },
    { (cell_t)&last,      "(last)",        "" },
    { (cell_t)&stk[0],    "(stk)",         "" },
    { (cell_t)&rstk[0],   "(rstk)",        "" },
    { (cell_t)&tib[0],    "tib",           "" },
    { (cell_t)&in,        ">in",           "" },
    { (cell_t)&mem[0],    "mem",           "" },
    { MEM_SZ,             "mem-sz",        "" },
    { (cell_t)&vars[0],   "vars",          "" },
    { VARS_SZ,            "vars-sz",       "" },
    { (cell_t)&reg[0],    "regs",          "" },
    { (cell_t)&output_fp, "(output_fp)",   "" },
    { (cell_t)&input_fp,  "(input_fp)",    "" },
    { (cell_t)&state,     "state",         "" },
    { sizeof(dict_t),     "word-sz",       "" },
    { CELL_SZ,            "CELL",          "" },
    { (cell_t)&base,      "base",          "" },
    { 0, 0, 0 }
};

void loadNum(const char *name, cell_t val, int isLit) {
    doCreate((char*)name);
    if (isLit) { 
        if (BTW(val,1,127)) {
            last->f = IS_INLINE;
            CComma(LIT1); CComma(val);
        } else { CComma(LIT4); Comma(val); }
    } else {
        last->f = (val==ENDWORD) ? IS_IMMEDIATE : IS_INLINE;
        CComma(val);
    }
    CComma(EXIT);
}

void init() {
    here = &mem[0];
    vhere = &vars[0];
    last = (dict_t*)&mem[MEM_SZ];
    base = 10;
    sp = rsp = reg_base = 0;
    for (int i = 0; prims[i].opName;i++) {
        if (prims[i].opName[0]) { loadNum(prims[i].opName, prims[i].op, 1); }
        if (prims[i].c3Word[0]) { loadNum(prims[i].c3Word, prims[i].op, 0); }
    }
    for (int i=0; i<6; i++) { tempWords[i].f = 0; }
    for (int i=6; i<9; i++) { tempWords[i].f = IS_INLINE; }
    tempWords[9].f = IS_IMMEDIATE;
    loadStartupWords();
}

#ifdef isPC
#include "sys-pc.inc"
#endif
