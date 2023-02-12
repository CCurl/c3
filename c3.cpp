// c3.cpp - a minimal Forth system

// Windows PC (Visual Studio)
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define isPC
#endif

#include <stdio.h>
#include <time.h>

extern void printString(const char *s);
extern void printChar(const char c);

#define MEM_SZ          64000
#define VARS_SZ        256000
#define STK_SZ             64
#define LSTK_SZ            30
#define NAME_LEN            9

enum {
    STOP = 0,
    EXIT, JMP, JMPZ, JMPNZ,
    CALL, LIT1, LIT4, 
    BITOPS, RETOPS, FILEOPS,
    DUP, SWAP, OVER, DROP,
    ADD, MULT, SLMOD, INC, DEC, SUB, 
    LT, EQ, GT, NOT,
    DO, LOOP, INDEX,
    EMIT, TIMER,
    DEFINE, ENDWORD, CREATE, FIND,
    STORE, CSTORE, FETCH, CFETCH
};

#define IS_IMMEDIATE  1
#define IS_INLINE     2

typedef struct { int op; int flg; const char *name; } opcode_t;
opcode_t opcodes[] = { 
    { DEFINE,    IS_INLINE,    ":" },       { ENDWORD, IS_IMMEDIATE, ";" }
    , { CREATE,  IS_INLINE,    "create" },  { FIND,    IS_INLINE,    "'" }
    , { DUP,     IS_INLINE,    "dup" },     { SWAP,    IS_INLINE,    "swap" }
    , { OVER,    IS_INLINE,    "over" },    { DROP,    IS_INLINE,    "drop" }
    , { EMIT,    IS_INLINE,    "emit" },    { TIMER,   IS_INLINE,    "timer" }
    , { ADD,     IS_INLINE,    "+" },       { SUB,     IS_INLINE,    "-" }
    , { MULT,    IS_INLINE,    "*" },       { SLMOD,   IS_INLINE,    "/mod" }
    , { LT,      IS_INLINE,    "<" },       { EQ,      IS_INLINE,    "=" }
    , { GT,      IS_INLINE,    ">" },       { NOT,     IS_INLINE,    "0=" }
    , { INC,     IS_INLINE,    "1+" },      { DEC,     IS_INLINE,    "1-" }
    , { DO,      IS_INLINE,    "do" },      { LOOP,    IS_INLINE,    "loop" }
    , { INDEX,   IS_INLINE,    "(i)" }
    , { STORE,   IS_INLINE,    "!" },       { CSTORE,  IS_INLINE,    "c!" }
    , { FETCH,   IS_INLINE,    "@" },       { CFETCH,  IS_INLINE,    "c@" }
    , { 0, 0, 0 }
};

#define TOS (stk[sp])
#define NOS (stk[sp-1])
#define PUSH(x) push((cell_t)(x))
#define DROP1 sp--
#define DROP2 sp-=2
#define RET(x) push(x); return;
#define NEXT goto next

#define BTW(a,b,c) ((b<=a) && (a<=c))
#define clearTib fill(tib, 0, sizeof(tib))
#define PRINT1(a)     printString(a)
#define PRINT2(a,b)   PRINT1(a); PRINT1(b)
#define PRINT3(a,b,c) PRINT2(a,b); PRINT1(c)

#define L0           lstk[lsp]
#define L1           lstk[lsp-1]
#define L2           lstk[lsp-2]

#define BYTES(x)      mem[x]

typedef long cell_t;
typedef unsigned char byte;
typedef struct { char f; char len; char name[NAME_LEN+1]; char *xt; } dict_t;

cell_t stk[STK_SZ+1], sp, rsp;
char *rstk[STK_SZ+1];
cell_t lstk[LSTK_SZ+1], lsp;
char mem[MEM_SZ];
char vars[VARS_SZ], *vhere;
cell_t state, base;
char *here, *pc, tib[128], *in;
dict_t *last;

#ifdef isPC
FILE *input_fp;
#endif

void push(cell_t x) { stk[++sp] = (cell_t)(x); }
cell_t pop() { return stk[sp--]; }

void CComma(cell_t x) { *(here++) = (char)x; }
void Comma(cell_t x) { *(cell_t*)here = x; here += sizeof(cell_t); }

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

void Create(char *w) {
    int l = strLen(w);
    --last;
    if (NAME_LEN < l) {
        PRINT2("-trunc:[", w);
        l=NAME_LEN; w[l]=0;
        PRINT3("->", w, "]-");
    }
    strCpy(last->name, w);
    last->len = l;
    last->xt = here;
    last->f = 0;
}

// ( nm--xt flags 1 )
// ( nm--0 )
void find() {
    char *nm = (char*)pop();
    int len = strLen(nm);
    dict_t *x = last;
    dict_t *end = (dict_t*)&BYTES(MEM_SZ);
    while (x < end) {
        if ((len==x->len) && strEq(nm, x->name, 0)) {
            PUSH(x->xt);
            push(x->f);
            RET(1);
        }
        ++x;
    }
    push(0);
}

// ( --n 1 )
// ( --0 )
void isDecimal(const char *wd) {
    cell_t x = 0, isNeg = (*wd == '-') ? 1 : 0;
    if (isNeg && (*(++wd) == 0)) { RET(0); }
    while (BTW(*wd, '0', '9')) { x = (x * 10) + (*(wd++) - '0'); }
    if (*wd) { RET(0); }
    push(isNeg ? -x : x);
    RET(1);
    // For Floating point support
    // if (*wd && (*wd != '.')) { RET(0); }
    // if (*wd == 0) { push(isNeg ? -x : x); RET(1); }
    // // Must be a '.', make it a float
    // ++wd;
    // float fx = (float)x, d = 10;
    // while (BTW(*wd, '0', '9')) { fx += (*(wd++) - '0') / d; d *= 10; }
    // if (*wd) { RET(0); }
    // push(0);
    // // FTOS = isNeg ? -fx : fx;
    // RET(1);
}

// ( nm--n 1 )
// ( nm--0 )
void isNum() {
    char *wd = (char*)pop();
    if ((wd[0] == '\'') && (wd[2] == '\'') && (wd[3] == 0)) { push(wd[1]); RET(1); }
    int b = base, lastCh = '9';
    if (*wd == '#') { b = 10;  ++wd; }
    if (*wd == '$') { b = 16;  ++wd; }
    if (*wd == '%') { b = 2;  ++wd; lastCh = '1'; }
    if (*wd == 0) { RET(0); }
    if (b == 10) { isDecimal(wd); return; }
    if (b < 10) { lastCh = '0' + b - 1; }
    cell_t x = 0;
    while (*wd) {
        cell_t t = -1, c = *(wd++);
        if (BTW(c, '0', lastCh)) { t = c - '0'; }
        if ((b == 16) && (BTW(c, 'A', 'F'))) { t = c - 'A' + 10; }
        if ((b == 16) && (BTW(c, 'a', 'f'))) { t = c - 'a' + 10; }
        if (t < 0) { RET(0); }
        x = (x * b) + t;
    }
    push(x);
    RET(1);
}

void getInput() {
    clearTib;
    if (input_fp) {
        in = fgets(tib, sizeof(tib), input_fp);
        if (in != tib) { fclose(input_fp); input_fp = NULL; }
    }
    if (! input_fp) {
        if (state) { printString("... > "); }
        else { printString(" ok\n"); }
        in = fgets(tib, sizeof(tib), stdin);
    }
}

// ( --addr len )
// ( --0 )
int getword(int stopOnNull) {
    int len = 0;
    if (sp < 0) { PRINT1("-under-"); sp=0;}
    if (STK_SZ < sp) { PRINT1("over"); sp=0; }
gl1:
    while (*in && (*in < 33)) { ++in; }
    if (*in == 0) { 
        if (stopOnNull) { return 0; }
        getInput(); goto gl1;
    }
    PUSH(in);
    while (32 < *in) { ++in; ++len; }
    *(in++) = 0;
    return len;
}

void Run(char *y) {
    cell_t t1, t2;
    pc = y;

next:
    switch (*(pc++)) {
    case STOP:                                                 return;
    case LIT1: push(*(pc++));                                               NEXT;
    case LIT4: push(*(cell_t*)pc); pc += sizeof(cell_t);                    NEXT;
    case CALL: if (*pc != EXIT) { rstk[++rsp] = (pc+sizeof(cell_t)); }
            pc = *(char**)pc;                                               NEXT;
    case EXIT: if (rsp<1) { rsp=0; return; } pc=rstk[rsp--];                NEXT;
    case JMP: pc = *(char**)pc;                                             NEXT;
    case JMPZ: if (pop()==0) { pc = *(char**)pc; }
             else { pc += sizeof(cell_t); }                                 NEXT;
    case JMPNZ: if (pop()) { pc=*(char**)pc; } else { pc+=sizeof(cell_t); } NEXT;
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
    case DEC: --TOS;                                                        NEXT;
    case INC: ++TOS;                                                        NEXT;
    case DO: lsp+=3; L2=(cell_t)pc; L0=pop(); L1=pop();                     NEXT;
    case INDEX: PUSH(&L0);                                                  NEXT;
    case LOOP: if (++L0<L1) { pc=(char*)L2; } else { lsp-=3; };             NEXT;
    case DEFINE: getword(0); Create((char*)pop()); state=1;                 NEXT;
    case CREATE: getword(0); Create((char*)pop());                          NEXT;
    case FIND: getword(0); find();                                          NEXT;
    case ENDWORD: state=0; CComma(EXIT);                                    NEXT;
    case BITOPS: t1 = *(pc++);
        if (t1==11) { NOS &= TOS; DROP1; }               // and
        else if (t1==12) { NOS |= TOS; DROP1; }          // or
        else if (t1==13) { NOS ^= TOS; DROP1; }          // xor
        else if (t1==14) { TOS = ~TOS; }                 // com
        NEXT;
    case RETOPS: t1 = *(pc++);
        if (t1==11) { rstk[++rsp] = (char*)pop(); }      // >r
        else if (t1==12) { PUSH(rstk[rsp]); }            // r@
        else if (t1==13) { PUSH(rstk[rsp--]); }          // r>
        NEXT;
    case FILEOPS: t1 = *(pc++);
        if (t1==11) { NOS=(cell_t)fopen((char*)TOS, (char*)NOS); DROP1; }
        else if (t1==12) { fclose((FILE*)pop()); }
        else if (t1==13) { PUSH(65); PUSH(1); }
        NEXT;
    default: printf("-[%d]?-",(int)*(pc-1));  break;
    }
}

int ParseWord() {
    char *w = (char*)TOS;
    isNum();
    if (pop()) {
        if (state) {
            if (BTW(TOS,0,127)) { CComma(LIT1); CComma(pop()); }
            else { CComma(LIT4); Comma(pop()); }
        }
        return 1;
    }
    PUSH(w);
    find();
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
    PRINT3("[", w, "]??");
    if (state) {
        ++last;
        state = 0;
    }
    base = 10;
    return 0;
}

void ParseLine(char *x, int stopOnNull) {
    in = x;
    if (in==0) { in=tib; clearTib; }
    while (state != 999) {
        if (getword(stopOnNull) == 0) { return; }
        ParseWord();
    }
}

void loadNum(const char *name, cell_t addr, int makeInline=0) {
    clearTib;
    sprintf(tib, ": %s %ld ;", name, addr);
    ParseLine(tib, 1);
    if (makeInline) { last->f = IS_INLINE; }
}

void init() {
    here = &BYTES(0);
    vhere = &vars[0];
    last = (dict_t*)&BYTES(MEM_SZ);
    base = 10;
    sp = rsp = 0;
    opcode_t *op = opcodes;
    while (op->op) {
        Create((char*)op->name);
        last->f = op->flg;
        CComma(op->op);
        CComma(EXIT);
        ++op;
    }
    loadNum("(exit)",   EXIT,    0);
    loadNum("(jmp)",    JMP,     1);
    loadNum("(jmpz)",   JMPZ,    1);
    loadNum("(jmpnz)",  JMPNZ,   1);
    loadNum("(call)",   CALL,    1);
    loadNum("(lit4)",   LIT4,    1);
    loadNum("(bitop)",  BITOPS,  1);
    loadNum("(retop)",  RETOPS,  1);
    loadNum("(fileop)", FILEOPS, 1);
    loadNum("mem",      (cell_t)&BYTES(0));
    loadNum("mem-end",  (cell_t)&BYTES(MEM_SZ));
    loadNum("vars",     (cell_t)&vars[0]);
    loadNum("vars-end", (cell_t)&vars[VARS_SZ]);
    loadNum("word-sz",  sizeof(dict_t), 1);
    loadNum("cell",     sizeof(cell_t), 1);
    loadNum("(vhere)",  (cell_t)&vhere);
    loadNum("(stk)",    (cell_t)&stk[0]);
    loadNum("(sp)",     (cell_t)&sp);
    loadNum("(rsp)",    (cell_t)&rsp);
    loadNum("(lsp)",    (cell_t)&lsp);
    loadNum("(last)",   (cell_t)&last);
    loadNum("(here)",   (cell_t)&here);
    loadNum(">in",      (cell_t)&in);
    loadNum("state",    (cell_t)&state);
    loadNum("base",     (cell_t)&base);
}

#ifdef isPC
void printChar(const char c) { putchar(c); }
void printString(const char* s) { fputs(s, stdout); }

int main(int argc, char *argv[]) {
    // int r='A';
    // for (i=1; i<argc; ++i) { y=argv[i]; RG(r++) = atoi(y); }
    init();
    input_fp = fopen("core.f", "rt");
    ParseLine(0, 0);
    return 0;
}
#endif
