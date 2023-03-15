// c3.cpp - a minimal Forth system

// Windows PC (Visual Studio)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define isPC
#endif

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MEM_SZ            128*1024
#define VARS_SZ           512*1024
#define STK_SZ             64
#define LSTK_SZ            30

#ifdef _MSC_VER
    #include <conio.h>
    int qKey() { return _kbhit(); }
    int key() { return _getch(); }
#elif IS_LINUX
    int qKey() { return 0; }
    int key() { return 0; }
#else
    // Dev board
    extern int qKey();
    extern int key();
    #define MEM_SZ             64*1024
    #define VARS_SZ            96*1024
    #define STK_SZ             32
    #define LSTK_SZ            30
#endif

typedef long cell_t;
typedef unsigned long ucell_t;
typedef struct { char *xt; char f; char len; char name[14]; } dict_t;
typedef struct { int op; int flg; const char *name; } opcode_t;

extern void printString(const char *s);
extern void printChar(const char c);

enum {
    STOP = 0,
    EXIT, CALL, JMP, JMPZ, JMPNZ,
    STORE, CSTORE, FETCH, CFETCH,
    LIT1, LIT4,
    DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, SUB, 
    LT, EQ, GT, NOT,
    DO, LOOP, LOOP2, INDEX,
    RTO, RFETCH, RFROM,
    INC, INCA, DEC, DECA,
    COM, AND, OR, XOR,
    EMIT, TIMER, SYSTEM,
    KEY, QKEY, IS_NUM,
    DEFINE, ENDWORD, CREATE, FIND, WORD,
    FOPEN, FCLOSE, FLOAD, FREAD, FWRITE,
    REG_I, REG_D, REG_R, REG_S, REG_NEW, REG_FREE
};

#define IS_IMMEDIATE  1
#define IS_INLINE     2
#define STOP_LOAD    99
#define ALL_DONE    999

opcode_t opcodes[] = { 
    { DEFINE,   IS_INLINE, ":" },      { ENDWORD, IS_IMMEDIATE, ";" }
    , { CREATE, IS_INLINE, "create" }, { FIND,     IS_INLINE, "'" }
    , { DUP,    IS_INLINE, "dup" },    { SWAP,     IS_INLINE, "swap" }
    , { OVER,   IS_INLINE, "over" },   { DROP,     IS_INLINE, "drop" }
    , { EMIT,   IS_INLINE, "emit" },   { TIMER,    IS_INLINE, "timer" }
    , { ADD,    IS_INLINE, "+" },      { SUB,      IS_INLINE, "-" }
    , { MULT,   IS_INLINE, "*" },      { SLMOD,    IS_INLINE, "/mod" }
    , { RTO, IS_INLINE, ">r" },  { RFETCH, IS_INLINE, "r@" }, { RFROM, IS_INLINE, "r>" }
    , { LT,  IS_INLINE, "<" },   { EQ,     IS_INLINE, "=" },  { GT,     IS_INLINE, ">" }
    , { AND, IS_INLINE, "and" }, { OR,     IS_INLINE, "or" }, { XOR,    IS_INLINE, "xor" }
    , { KEY,     IS_INLINE, "key" },    { QKEY, IS_INLINE, "?key" } 
    , { FOPEN,   IS_INLINE, "fopen" },  { FCLOSE,   IS_INLINE, "fclose" }
    , { FREAD,   IS_INLINE, "fread" },  { FWRITE,   IS_INLINE, "fwrite" }
    , { FLOAD,   IS_INLINE, "(load)" }, { WORD,     IS_INLINE, "next-word" }
    , { COM,     IS_INLINE, "com" },    { NOT,      IS_INLINE, "0=" }
    , { INC,     IS_INLINE, "1+" },     { INCA,     IS_INLINE, "++" }
    , { DEC,     IS_INLINE, "1-" },     { DECA,     IS_INLINE, "--" }
    , { DO, IS_INLINE, "do" }, { LOOP, IS_INLINE, "loop" }, { LOOP2, IS_INLINE, "-loop" }
    , { INDEX,   IS_INLINE, "(i)" },    { SYSTEM,   IS_INLINE, "system" }
    , { STORE,   IS_INLINE, "!" },      { CSTORE,   IS_INLINE, "c!" }
    , { FETCH,   IS_INLINE, "@" },      { CFETCH,   IS_INLINE, "c@" }
    , { REG_NEW, IS_INLINE, "+regs" },  { REG_FREE, IS_INLINE, "-regs" }
    , { IS_NUM,  IS_INLINE, "number?" }
    , { 0, 0, 0 }
};

#define TOS           (stk[sp])
#define NOS           (stk[sp-1])
#define PUSH(x)       push((cell_t)(x))
#define DROP1         sp--
#define DROP2         sp-=2
#define NEXT          goto next

#define BTW(a,b,c)    ((b<=a) && (a<=c))
#define CELL_SZ       sizeof(cell_t)
#define clearTib      fill(tib, 0, sizeof(tib))
#define PRINT1(a)     printString(a)
#define PRINT3(a,b,c) { PRINT1(a); PRINT1(b); PRINT1(c); }
#define CpAt(x)       (char*)Fetch((char*)x)

#define L0            lstk[lsp]
#define L1            lstk[lsp-1]
#define L2            lstk[lsp-2]

cell_t stk[STK_SZ+1], sp, rsp;
char *rstk[STK_SZ+1];
cell_t lstk[LSTK_SZ+1], lsp;
cell_t fileStk[10], fileSp, input_fp, output_fp;
cell_t state, base, reg[100], reg_base, t1, t2;
char mem[MEM_SZ];
char vars[VARS_SZ], *vhere;
char *here, *pc, tib[128], *in;
dict_t tempWords[10], *last;

void push(cell_t x) { stk[++sp] = (cell_t)(x); }
cell_t pop() { return stk[sp--]; }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { *(cell_t*)here = x; here += CELL_SZ; }

void Store(char *loc, cell_t x) { *(cell_t*)loc = x; }
cell_t Fetch(char *loc) { return *(cell_t*)loc; }

void fill(char *d, char val, int num) { for (int i=0; i<num; i++) { *(d++)=val; } }
char *strEnd(char *s) { while (*s) ++s; return s; }
void strCat(char *d, const char *s) { d=strEnd(d); while (*s) { *(d++)=*(s++); } *d=0; }
void strCpy(char *d, const char *s) { *d = 0; strCat(d, s); }
int strLen(char *d) { int len = 0; while (*d++) { ++len; } return len; }
int lower(int x) { return BTW(x,'A','Z') ? x+32: x; }

int strEq(char *d, char *s, int caseSensitive) {
    while (*s || *d) {
        if (caseSensitive) { if (*s != *d) return 0; }
        else { if (lower(*s) != lower(*d)) return 0; }
        s++; d++;
    }
    return -1;
}

char *iToA(ucell_t N, int base) {
    static char ret[CELL_SZ*8];
    char *x = &ret[CELL_SZ*8-1];
    *(x) = 0;
    int neg = (((cell_t)N<0) && (base==10)) ? 1 : 0;
    if (neg) N = (~N) + 1;
    do {
        int r = (N % base) + '0';
        *(--x) = ('9'<r) ? r+7 : r;
        N /= base;
    } while (N);
    if (neg) { *(--x)='-'; }
    return x;
}

int isTempWord(char *w) {
    if ((w[0]=='T') && BTW(w[1],'0','9') && (w[2]==0)){ return 1; }
    return 0;
}

char isRegOp(char *w) {
    if ((strLen(w) != 2) || !BTW(w[1],'0','9')) { return 0; }
    if (w[0]=='r') { return REG_R; }
    if (w[0]=='s') { return REG_S; }
    if (w[0]=='i') { return REG_I; }
    if (w[0]=='d') { return REG_D; }
    return 0;
}

void Create(char *w) {
    if (isTempWord(w)) { tempWords[w[1]-'0'].xt = here; return; }
    int l = strLen(w);
    --last;
    if (13 < l) { l=13; w[l]=0; }
    strCpy(last->name, w);
    last->len = l;
    last->xt = here;
    last->f = 0;
}

// ( nm--xt flags 1 | 0 )
void find() {
    char *nm = (char*)pop();
    if (isTempWord(nm)) {
        PUSH(tempWords[nm[1]-'0'].xt);
        push(0); push(1); return;
    }
    int len = strLen(nm);
    // PRINT3("-LF-(",nm,"):")
    dict_t *dp = last;
    dict_t *stop = (dict_t*)&mem[0];
    while (dp < (dict_t*)&mem[MEM_SZ]) {
        // PRINT3("-LF-(",nm,"):")
        if ((len==dp->len) && strEq(nm, dp->name, 0)) {
            PUSH(dp->xt);
            push(dp->f);
            push(1); return;
        }
        ++dp;
    }
    push(0);
}

// ( --n? )
int isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { return 0; }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { return 0; }
    push(isNeg ? -x : x);
    return 1;
}

// ( nm--n? )
int isNum() {
    char *wd = (char*)pop();
    if ((wd[0] == '\'') && (wd[2] == '\'') && (wd[3] == 0)) { push(wd[1]); return 1; }
    int b = base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
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

// ( --addr | <null> )
int getword() {
    int len = 0;
    if (sp < 0) { PRINT1("-under-"); sp=0; }
    if (STK_SZ < sp) { PRINT1("over"); sp=0; }
    while (*in && (*in < 33)) { ++in; }
    if (*in == 0) { return 0; }
    PUSH(in);
    while (32 < *in) { ++in; ++len; }
    *(in++) = 0;
    return len;
}

void Run(char *y) {
    pc = y;

next:
    switch (*(pc++)) {
    case STOP:                                                             return;
    case EXIT: if (rsp<1) { rsp=0; return; } pc=rstk[rsp--];                NEXT;
    case CALL: y=pc+CELL_SZ; if (*y!=EXIT) { rstk[++rsp]=y; }          // fall-thru
    case JMP: pc=CpAt(pc);                                                  NEXT;
    case JMPZ: if (pop()==0) { pc=CpAt(pc); } else { pc+=CELL_SZ; }         NEXT;
    case JMPNZ: if (pop()) { pc=CpAt(pc); } else { pc+=CELL_SZ; }           NEXT;
    case LIT1: push(*(pc++));                                               NEXT;
    case LIT4: push(*(cell_t*)pc); pc += CELL_SZ;                           NEXT;
    case STORE: t1=pop(); t2=pop(); Store((char*)t1, t2);                   NEXT;
    case CSTORE: *(char*)TOS = (char)NOS; DROP2;                            NEXT;
    case FETCH: TOS = Fetch((char*)TOS);                                    NEXT;
    case CFETCH: TOS = *(char*)TOS;                                         NEXT;
    case DUP: push(TOS);                                                    NEXT;
    case SWAP: t1=TOS; TOS=NOS; NOS=t1;                                     NEXT;
    case OVER: push(NOS);                                                   NEXT;
    case DROP: DROP1;                                                       NEXT;
    case ADD: NOS += TOS; DROP1;                                            NEXT;
    case SUB: NOS -= TOS; DROP1;                                            NEXT;
    case MULT: NOS *= TOS; DROP1;                                           NEXT;
    case SLMOD: t1=NOS; t2=TOS; TOS=t1/t2; NOS=t1%t2;                       NEXT;
    case LT: NOS = (NOS <  TOS) ? -1 : 0; DROP1;                            NEXT;
    case EQ: NOS = (NOS == TOS) ? -1 : 0; DROP1;                            NEXT;
    case GT: NOS = (NOS >  TOS) ? -1 : 0; DROP1;                            NEXT;
    case NOT: TOS = (TOS) ? 0: -1;                                          NEXT;
    case EMIT: printChar((char)pop());                                      NEXT;
    case TIMER: push(clock());                                              NEXT;
    case INC: ++TOS;                                                        NEXT;
    case INCA: y=(char*)pop(); Store(y, Fetch(y)+1);                        NEXT;
    case DEC: --TOS;                                                        NEXT;
    case DECA: y=(char*)pop(); Store(y, Fetch(y)-1);                        NEXT;
    case DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();                     NEXT;
    case INDEX: PUSH(&L0);                                                  NEXT;
    case LOOP: if (++L0<L1) { pc=(char*)L2; } else { lsp-=3; };             NEXT;
    case LOOP2: if (--L0>L1) { pc=(char*)L2; } else { lsp-=3; };            NEXT;
    case WORD: t1=getword(); push(t1);                                      NEXT;
    case DEFINE: getword(); Create((char*)pop()); state=1;                  NEXT;
    case CREATE: getword(); Create((char*)pop());                           NEXT;
    case FIND: getword(); find();                                           NEXT;
    case ENDWORD: state=0; CComma(EXIT);                                    NEXT;
    case SYSTEM: y=(char*)pop(); system(y+1);                               NEXT;
    case AND: NOS &= TOS; DROP1;                                            NEXT;
    case OR:  NOS |= TOS; DROP1;                                            NEXT;
    case XOR: NOS ^= TOS; DROP1;                                            NEXT;
    case COM: TOS = ~TOS;                                                   NEXT;
    case RTO:    rstk[++rsp] = (char*)pop();                                NEXT; // >r
    case RFETCH: PUSH(rstk[rsp]);                                           NEXT; // r@
    case RFROM:  PUSH(rstk[rsp--]);                                         NEXT; // r>
    case FOPEN:  NOS=(cell_t)fopen((char*)(NOS+1), (char*)TOS+1); DROP1;    NEXT;
    case FCLOSE: fclose((FILE*)pop());                                      NEXT;
    case FREAD: t2=pop(); t1=pop(); y=(char*)TOS;
        TOS=fread(y, 1, t1, (FILE*)t2);                                     NEXT;
    case FWRITE: t2=pop(); t1=pop(); y=(char*)TOS;
        TOS=fwrite(y, 1, t1, (FILE*)t2);                                    NEXT;
    case FLOAD:  y=(char*)pop(); t1=(cell_t)fopen(y+1, "rt");
            if (t1 && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t1) { input_fp = t1; clearTib; }
            else { PRINT1("-noFile-"); }                                    NEXT;
    case QKEY: push(qKey());                                                NEXT;
    case KEY: push(key());                                                  NEXT;
    case REG_D: --reg[*(pc++)+reg_base];                                    NEXT;
    case REG_I: ++reg[*(pc++)+reg_base];                                    NEXT;
    case REG_R: push(reg[*(pc++)+reg_base]);                                NEXT;
    case REG_S: reg[*(pc++)+reg_base] = pop();                              NEXT;
    case REG_NEW: reg_base += (reg_base < 90) ? 10 : 0;                     NEXT;
    case REG_FREE: reg_base -= (0 < reg_base) ? 10 : 0;                     NEXT;
    case IS_NUM: ++TOS; push(isNum());                                            NEXT;
    default: PRINT3("-[", iToA((cell_t)*(pc-1),10), "]?-")                  break;
    }
}

int ParseWord() {
    char *w = (char*)pop(), t = isRegOp(w);
    if (t) {
        if (state) { CComma(t); CComma(w[1]-'0'); }
        else {
            tib[120]=t; tib[121]=w[1]-'0'; tib[122]=EXIT;
            Run(&tib[120]);
        }
        return 1;
    }

    PUSH(w);
    if (isNum()) {
        if (state) {
            if (BTW(TOS,0,127)) { CComma(LIT1); CComma(pop()); }
            else { CComma(LIT4); Comma(pop()); }
        }
        return 1;
    }

    PUSH(w); find();
    if (pop()) {
        cell_t f = pop();
        char *xt = (char*)pop();
        if ((state == 0) || (f & IS_IMMEDIATE)) { Run(xt); return 1; }
        if (f & IS_INLINE) {
            CComma(*(xt++));
            while ((*xt) && (*xt != EXIT)) { CComma(*(xt++)); }
        }
        else { CComma(CALL); Comma((cell_t)xt); }
        return 1;
    }

    PRINT3("[", w, "]??")
    if (state) {
        here = (char*)last;
        ++last;
        state = 0;
    }
    base = 10;
    return 0;
}

void ParseLine(char *x) {
    in = x;
    if (in==0) { in=tib; clearTib; }
    // PRINT3("-", in, "-")
    while (state != ALL_DONE) {
        if (getword() == 0) { return; }
        if (ParseWord() == 0) { return; }
    }
}

#define SC(x) strCat(tib, x)
void loadNum(const char *name, cell_t addr, int makeInline) {
    clearTib;
    strCpy(tib, ": ");
    SC(name); SC(" "); SC(iToA(addr, 10)); SC(" "); SC(";");
    ParseLine(tib);
    if (makeInline) { last->f = IS_INLINE; }
}

void init() {
    here = &mem[0];
    vhere = &vars[0];
    last = (dict_t*)&mem[MEM_SZ];
    base = 10;
    sp = rsp = reg_base = 0;
    opcode_t *op = opcodes;
    while (op->op) {
        Create((char*)op->name);
        last->f = op->flg;
        CComma(op->op);
        CComma(EXIT);
        ++op;
    }
    loadNum("version",  5,       1);
    loadNum("(exit)",   EXIT,    0);
    loadNum("(jmp)",    JMP,     1);
    loadNum("(jmpz)",   JMPZ,    1);
    loadNum("(jmpnz)",  JMPNZ,   1);
    loadNum("(call)",   CALL,    1);
    loadNum("(lit4)",   LIT4,    1);
    loadNum("(output_fp)", (cell_t)&output_fp, 0);
    loadNum("(input_fp)",  (cell_t)&input_fp, 0);
    loadNum("mem",      (cell_t)&mem[0], 0);
    loadNum("mem-end",  (cell_t)&mem[MEM_SZ], 0);
    loadNum("vars",     (cell_t)&vars[0], 0);
    loadNum("regs",     (cell_t)&reg[0], 0);
    loadNum("vars-end", (cell_t)&vars[VARS_SZ], 0);
    loadNum("(vhere)",  (cell_t)&vhere, 0);
    loadNum(">in",      (cell_t)&in, 0);
    loadNum("tib",      (cell_t)&tib[0], 0);
    loadNum("word-sz",  (cell_t)sizeof(dict_t), 1);
    loadNum("(stk)",    (cell_t)&stk[0], 0);
    loadNum("(sp)",     (cell_t)&sp, 0);
    loadNum("(rsp)",    (cell_t)&rsp, 0);
    loadNum("(lsp)",    (cell_t)&lsp, 0);
    loadNum("(last)",   (cell_t)&last, 0);
    loadNum("(here)",   (cell_t)&here, 0);
    loadNum("state",    (cell_t)&state, 0);
    loadNum("base",     (cell_t)&base, 0);
    loadNum("cell",     CELL_SZ, 1);
}

#ifdef isPC
void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }

void getInput() {
    clearTib;
    if ((state == STOP_LOAD) && input_fp) {
        fclose((FILE*)input_fp);
        input_fp =  (0 < fileSp) ? fileStk[fileSp--] : 0;
        state = 0;
    }
    if (input_fp) {
        in = fgets(tib, sizeof(tib), (FILE*)input_fp);
        if (in != tib) {
            fclose((FILE*)input_fp);
            input_fp =  (0 < fileSp) ? fileStk[fileSp--] : 0;
        }
    }
    if (! input_fp) {
        cell_t tmp = output_fp;
        output_fp = 0;
        printString((state) ? "... > " : " ok\n");
        in = fgets(tib, sizeof(tib), stdin);
        output_fp = tmp;
    }
    in = tib;
}

int main(int argc, char *argv[]) {
    init();
    input_fp = (cell_t)fopen("core.f", "rt");
    output_fp = 0;
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
#endif
