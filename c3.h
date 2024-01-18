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

#define RCASE         return pc; case

#if (defined _WIN32 || defined _WIN64)
    #define isPC
    #define IS_WINDOWS
#elif (defined __i386 || defined __x86_64 || defined IS_LINUX)
    #define isPC
    #ifndef IS_LINUX
    #define IS_LINUX
    #endif
#else

    // Not a PC, must be a development board
    // NOTE: Change these values for a specific board
    //       I use these for the Teensy-4 and the Pico
    #define isBOARD 1

    #define CODE_SZ            64*1024
    #define VARS_SZ            96*1024
    #define STK_SZ             32
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           13
    #define NEEDS_ALIGN

#endif

enum { STOP_LOAD = 99, ALL_DONE = 999, VERSION = 99 };

#ifndef CODE_SZ
    #define CODE_SZ           128*1024
    #define VARS_SZ        4*1024*1024
    #define STK_SZ             64
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           21
#endif

// These are defined in the VM
extern cell_t lstk[LSTK_SZ+1], lsp;
extern cell_t fileStk[10], fileSp, input_fp, output_fp;
extern cell_t state, base, reg[REGS_SZ], reg_base, t1, n1;
extern cell_t inputStk[10], fileSp, input_fp, output_fp;
extern char tib[256], *in, *y;
extern char vars[VARS_SZ];

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
extern void ParseLine(const char* x);
extern void parseF(const char* fmt, ...);
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
extern void c3Init();

// These are functions the VM calls
extern void Store(const char *addr, cell_t val);
extern cell_t Fetch(const char *addr);
extern void printString(const char *str);
extern void printChar(char c);
extern char *doUser(char *pc, int ir);
extern int key();
extern int qKey();
extern cell_t sysTime();
extern void loadStartupWords();
extern void loadUserStartupWords();
extern void loadUserWords();

#endif // __C3_H__