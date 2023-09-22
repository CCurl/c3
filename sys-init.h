// System initialization logic for different types of systems
// NOTE: this is a *.h file because the Arduino IDE doesn't like *.inc files

extern void printString(const char *s);
extern void printChar(const char c);
extern void ParseLine(const char *s);
extern void loadStartupWords();
extern void loadUserWords();
extern char *doUser(char *pc, int ir);
extern cell_t sysTime();

#ifdef _MSC_VER

    // Support for Windows
    #include <conio.h>
    #define isPC
    int qKey() { return _kbhit(); }
    int key() { return _getch(); }

#elif IS_LINUX

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

    // Not Windows or Linux, must be a development board
    #define isBOARD 1

    extern int qKey();
    extern int key();
    #define MEM_SZ             64*1024
    #define VARS_SZ            96*1024
    #define STK_SZ             32
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           13
    #define NEEDS_ALIGN

#endif

#ifndef NEEDS_ALIGN
    void Store(char *loc, cell_t x) { *(cell_t*)loc = x; }
    cell_t Fetch(char *loc) { return *(cell_t*)loc; }
#else
    #define S(x, y) (*(x)=((y)&0xFF))
    #define G(x, y) (*(x)<<y)
    void Store(char *l, cell_t v) { S(l,v); S(l+1,v>>8); S(l+2,v>>16); S(l+3,v>>24); }
    cell_t Fetch(char *l) { return (*l)|G(l+1,8)|G(l+2,16)|G(l+3,24); }
#endif

#ifndef MEM_SZ
    #define MEM_SZ            128*1024
    #define VARS_SZ        4*1024*1024
    #define STK_SZ             64
    #define LSTK_SZ            30
    #define REGS_SZ           100
    #define NAME_LEN           13
#endif