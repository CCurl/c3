// Support for PCs

#include "c3.h"
#include <time.h>

#ifdef isPC

enum { SYSTEM = 100, FOPEN, FCLOSE, FREAD, FWRITE, FLOAD, BLOAD,
        EDIT_BLK
};

extern void editBlock(cell_t blkNum);
extern void editFile(char *fn);

#ifdef IS_WINDOWS

    #include <conio.h>
    int qKey() { return _kbhit(); }
    int key() { return _getch(); }

#elif (defined IS_LINUX)

#include <unistd.h>
#include <termios.h>
static struct termios normT, rawT;
static int isTtyInit = 0;
void ttyInit() {
    tcgetattr(STDIN_FILENO, &normT);
    cfmakeraw(&rawT);
    isTtyInit = 1;
}
void ttyModeNorm() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr(STDIN_FILENO, TCSANOW, &normT);
}
void ttyModeRaw() {
    if (!isTtyInit) { ttyInit(); }
    tcsetattr(STDIN_FILENO, TCSANOW, &rawT);
}
int qKey() {
    struct timeval tv;
    fd_set rdfs;
    ttyModeRaw();
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&rdfs);
    FD_SET(STDIN_FILENO, &rdfs);
    select(STDIN_FILENO + 1, &rdfs, NULL, NULL, &tv);
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

#endif // IS_LINUX

void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }
cell_t sysTime() { return clock(); }
void Store(char* loc, cell_t x) { *(cell_t*)loc = x; }
cell_t Fetch(const char* loc) { return *(cell_t*)loc; }
char *root;

void getInput() {
    fill(tib, 0, sizeof(tib));
    if ((state == STOP_LOAD) && input_fp) {
        fclose((FILE*)input_fp);
        input_fp = (0 < fileSp) ? inputStk[fileSp--] : 0;
        state = 0;
    }
    if (input_fp) {
        in = fgets(tib, 190, (FILE*)input_fp);
        if (in != tib) {
            fclose((FILE*)input_fp);
            input_fp = (0 < fileSp) ? inputStk[fileSp--] : 0;
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

int tryOpen(const char *root, const char *loc, const char *fn) {
    char nm[64];
    strCpy(nm, root);
    strCat(nm, loc);
    strCat(nm, fn);
    // printf("try [%s]\n", nm);
    cell_t fp = fOpen((cell_t)nm, (cell_t)"rb");
    if (!fp) { return 0; }
    if (input_fp) { inputStk[++fileSp] = input_fp; }
    input_fp = fp;
    return 1;
}

int lookForFile(const char *name) {
#ifdef IS_LINUX
    if (tryOpen("", "", name)) { return 1; }
    if (tryOpen("", "./", name)) { return 1; }
    if (tryOpen(root, "/.local/c3/", name)) { return 1; }
    if (tryOpen(root, "/.local/bin/c3/", name)) { return 1; }
    if (tryOpen(root, "/.local/bin/", name)) { return 1; }
#elif (defined  IS_WINDOWS)
    if (tryOpen("", "", name)) { return 1; }
    if (tryOpen("", ".\\", name)) { return 1; }
    if (tryOpen(root, "\\c3\\", name)) { return 1; }
    if (tryOpen(root, "\\bin\\c3\\", name)) { return 1; }
    if (tryOpen(root, "\\bin\\", name)) { return 1; }
#endif
    return 0;
}

void LFF(char *fn) { if (!lookForFile(fn)) { printStringF("-file[%s]?-", fn); } }

char *doUser(char *pc, int ir) {
    cell_t t1, t2, t3;
    switch (ir) {
    case SYSTEM:  system(cpop());
    RCASE FOPEN:  t2=pop(); t1=pop(); push(fOpen(t1, t2));
    RCASE FCLOSE: fClose(pop());
    RCASE FREAD:  t3=pop(); t2=pop(); t1=pop(); push(fRead(t1, 1, t2, t3));
    RCASE FWRITE: t3=pop(); t2=pop(); t1=pop(); push(fWrite(t1, 1, t2, t3));
    RCASE FLOAD:  LFF(cpop());
    RCASE BLOAD:  y=&tib[TIB_SZ-16]; sprintf(y, "block-%03d.c3", (int)pop()); LFF(y);
    RCASE EDIT_BLK: t1=pop(); editBlock(t1);
    return pc; default: return 0;
    }
}

void loadUserWords() {
    parseF("-ML- SYSTEM %d 3 -MLX- inline", SYSTEM);
    parseF("-ML- FOPEN  %d 3 -MLX- inline", FOPEN);
    parseF("-ML- FCLOSE %d 3 -MLX- inline", FCLOSE);
    parseF("-ML- FREAD  %d 3 -MLX- inline", FREAD);
    parseF("-ML- FWRITE %d 3 -MLX- inline", FWRITE);
    parseF("-ML- (LOAD) %d 3 -MLX- inline", FLOAD);
    parseF("-ML- BLOAD  %d 3 -MLX- inline", BLOAD);
    parseF("-ML- EDIT   %d 3 -MLX- inline", EDIT_BLK);
    ParseLine(": isPC 1 ;");
}

int main(int argc, char *argv[]) {
    root=(char*)"";
    input_fp = output_fp = 0;
    c3Init();

    if (1 < argc) { root = argv[1]; }
    for (int i=2; i<argc; i++) {
        if (lookForFile(argv[i])) { continue; }
        if (isNum(argv[i])) { reg[i] = pop(); }
        else { reg[i] = (cell_t)argv[i]; }
    }
#ifndef _SYS_LOAD_
    lookForFile("block-000.c3");
#endif
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}

#endif // isPC
