// System initialization logic for different types of systems
// NOTE: this is a *.h file because the Arduino IDE doesn't like *.inc files

#ifndef __C3_H__
#define __C3_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#if (defined __x86_64 || defined _WIN64)
#define FLOAT_T   double
#define CELL_T    int64_t 
#define UCELL_T   uint64_t 
#else
#define FLOAT_T   float
#define CELL_T    int32_t 
#define UCELL_T   uint32_t 
#endif

typedef CELL_T   cell_t;
typedef UCELL_T  ucell_t;
typedef FLOAT_T  flt_t;
typedef uint8_t  byte;

#if (defined _WIN32 || defined _WIN64)

    // Support for Windows
    #define isPC
    #include <conio.h>
    int qKey() { return _kbhit(); }
    int key() { return _getch(); }

#elif (defined __i386 || defined __x86_64)

// Support for Linux
#include <unistd.h>
#include <termios.h>
#define isPC
static struct termios normT, rawT;
static int isTtyInit = 0;
void ttyInit() {
    tcgetattr( STDIN_FILENO, &normT);
    cfmakeraw(&rawT);
    isTtyInit = 1;
}
void ttyModeNorm() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr( STDIN_FILENO, TCSANOW, &normT);
}
void ttyModeRaw() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr( STDIN_FILENO, TCSANOW, &rawT);
}
int qKey() {
    struct timeval tv;
    fd_set rdfs;
    ttyModeRaw();
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    int x = FD_ISSET(STDIN_FILENO, &rdfs);
    ttyModeNorm();
    return x;
}
int key() {
    ttyModeRaw();
    int x = getchar();
    ttyModeNorm();
    return x;
}

#else

    // Not a PC, must be a development board
    // NOTE: Change these values for a specific board
    //       I use these for the Teensy-4 and the Pico
    #define isBOARD 1

    extern int qKey();
    extern int key();
    #define CODE_SZ            64*1024
    #define VARS_SZ            96*1024
    #define STK_SZ             32
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           13
    #define NEEDS_ALIGN

#endif

enum { STOP_LOAD = 99, ALL_DONE = 999, VERSION = 99 };

#ifndef NEEDS_ALIGN
    void Store(const char *loc, cell_t x) { *(cell_t*)loc = x; }
    cell_t Fetch(const char *loc) { return *(cell_t*)loc; }
#else
    // 32-bit only
    #define S(x, y) (*(x)=((y)&0xFF))
    #define G(x, y) (*(x)<<y)
    void Store(char *l, cell_t v) { S(l,v); S(l+1,v>>8); S(l+2,v>>16); S(l+3,v>>24); }
    cell_t Fetch(char *l) { return (*l)|G(l+1,8)|G(l+2,16)|G(l+3,24); }
#endif

#ifndef CODE_SZ
    #define CODE_SZ           128*1024
    #define VARS_SZ        4*1024*1024
    #define STK_SZ             64
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           21
#endif

//extern void printString(const char *s);
//extern void printChar(const char c);
//extern void ParseLine(const char *x);
//extern void loadStartupWords();
//extern void loadUserWords();
//extern char *doUser(char *pc, int ir);
//extern cell_t sysTime();


// These are defined in vm.c
extern cell_t lstk[LSTK_SZ+1], lsp;
extern cell_t fileStk[10], fileSp, input_fp, output_fp;
extern cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
extern cell_t inputStk[10], fileSp, input_fp, output_fp;
extern char tib[256], *in, *y;

extern void push(cell_t x);
extern cell_t pop();
extern char *cpop();
extern void fpush(flt_t x);
extern flt_t fpop(); 
extern void rpush(char *x); 
extern char *rpop(); 
extern void CComma(cell_t x); 
extern void Comma(cell_t x); 
extern void fill(char *d, char val, int num); 
extern char *strEnd(char *s); 
extern void strCat(char *d, const char *s); 
extern void strCatC(char *d, const char c); 
extern void strCpy(char *d, const char *s); 
extern int strLen(const char *d); 
extern char *lTrim(char *d); 
extern char *rTrim(char *d);
extern int lower(int x); 
extern int upper(int x);

extern int strEqI(const char *s, const char *d);
extern int strEq(const char *d, const char *s);
extern void printStringF(const char *fmt, ...);
extern char *iToA(cell_t N, int b);
extern int isTempWord(const char *w);
extern char isRegOp(const char *w);
extern int nextWord();
extern void doDefine(char *wd);
extern int doFind(const char *nm);
extern int isBase10(const char *wd);
extern int isNum(const char *wd);
extern void doType(const char *str);
extern char *doStringOp(char *pc);
extern char *doSysOp(char *pc);
extern char *doFloatOp(char *pc);
extern void Run(char *pc);
extern int doReg(const char *w);
extern void vmInit();

// These are functions vm.c needs to be defined
extern void Store(const char *addr, cell_t val);
extern cell_t Fetch(const char *addr);
extern void printString(const char *str);
extern void printChar(char c);
extern char *doUser(char *pc, int ir);
extern int key();
extern int qKey();
extern cell_t sysTime();
extern void loadStartupWords();
extern void loadUserWords();

#endif // __C3_H__
