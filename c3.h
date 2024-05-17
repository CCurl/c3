#ifndef __C3_H__
#define __C3_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

enum { STOP_LOAD = 99, ALL_DONE = 999, VERSION = 20240517 };

#if (defined __x86_64 || defined _WIN64)
    #define CELL_T    int64_t
    #define UCELL_T   uint64_t
    #define FLOAT_T   double
    #define CELL_SZ   8
    #define addrFMT ": %s $%llx ;"
#else
    #define CELL_T    int32_t 
    #define UCELL_T   uint32_t 
    #define FLOAT_T   float
    #define CELL_SZ   4
    #define addrFMT ": %s $%lx ;"
#endif

typedef CELL_T   cell_t;
typedef UCELL_T  ucell_t;
typedef FLOAT_T  flt_t;
typedef uint8_t  byte;

#define ToCP(x)       (char*)(x)
#define BTW(x,l,h)    ((l<=x) && (x<=h))
#define PC(c)         printChar(c)
#define BCASE         break; case
#define NCASE         goto next; case
#define RCASE         return pc; case

#ifndef MIN
    #define MIN(a,b)      ((a)<(b))?(a):(b)
    #define MAX(a,b)      ((a)>(b))?(a):(b)
#endif

#if (defined _WIN32 || defined _WIN64)
    #define isPC
    #define IS_WINDOWS
    #define __EDITOR__
    // #define _SYS_LOAD_
#elif (defined __i386 || defined __x86_64 || defined IS_LINUX)
    #define isPC
    #ifndef IS_LINUX
    #define IS_LINUX
    #endif
    #define __EDITOR__
    // #define _SYS_LOAD_
#else

    // Not a PC, must be a development board
    // NOTE: Change these values for a specific board
    //       For the Pico, I use: https://github.com/earlephilhower/arduino-pico
    //       I use these for the Teensy-4 and the Pico
#define isBOARD 1

    #define MAX_LINES         100
    #define CODE_SZ            64*1024
    #define VARS_SZ            96*1024
    #define STK_SZ            128
    #define LSTK_SZ             3*25      // 25 nested loops
    #define REGS_SZ            10*25      // 25 nested +REGS
    #define TIB_SZ            256
    #define NAME_LEN           16
    #define NEEDS_ALIGN
    #define __EDITOR__
    #define _SYS_LOAD_
    // #define _TeensyFS_     // LittleFS for Teensy
    #define _PicoFS_       // LittleFS for Pico
    // #define _NoFS_         // No FS

#endif

#ifndef CODE_SZ
    // Default for PCs
    #define MAX_LINES         250
    #define CODE_SZ            96*1024
    #define VARS_SZ             4*1024*1024
    #define STK_SZ            256
    #define LSTK_SZ            50*3       // 50 nested loops
    #define REGS_SZ            50*10      // 50 nested +REGS
    #define TIB_SZ           1024
    #define NAME_LEN           20
    // #define _SYS_LOAD_
#endif

typedef union { cell_t i; flt_t f; char *c; } se_t;
typedef struct { cell_t sp; se_t stk[STK_SZ+1]; } stk_t;
typedef struct { cell_t xt; byte f, lex, len; char name[NAME_LEN+1]; } dict_t;

// These are defined in the VM
extern cell_t lstk[LSTK_SZ+1], lsp, output_fp;
extern cell_t state, base, reg[REGS_SZ], reg_base, t1, n1, lexicon;
extern char tib[TIB_SZ], *in, *y, *here;
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
extern void ParseLine(const char *x);
extern void parseF(const char *fmt, ...);
extern int strEqI(const char *s, const char *d);
extern int strEq(const char *d, const char *s);
extern void printStringF(const char *fmt, ...);
extern char *iToA(cell_t N, int b);
extern int isTempWord(const char *w);
extern char isRegOp(const char *w);
extern int nextWord();
extern void doDefine(char *wd);
extern int doFind(const char *n);
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
extern void Store(char *addr, cell_t val);
extern cell_t Fetch(const char *addr);
extern void printString(const char *str);
extern void printChar(char c);
extern char *doUser(char *pc, int ir);
extern int key();
extern int qKey();
extern cell_t sysTime();
extern void sysLoad();
extern void loadUserWords();

// Files support
#include "file.h"

// Editor
extern void editBlock(cell_t blkNum);
extern cell_t edScrH;

#endif // __C3_H__
