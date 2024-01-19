// Support for PCs

#include "c3.h"
#include <time.h>

#ifdef isPC

enum { SYSTEM = 100, FOPEN, FCLOSE, FREAD, FWRITE, FLOAD };

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

#endif

void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }
cell_t sysTime() { return clock(); }
void Store(const char* loc, cell_t x) { *(cell_t*)loc = x; }
cell_t Fetch(const char* loc) { return *(cell_t*)loc; }
char *root;

void getInput() {
    fill(tib, 0, sizeof(tib));
    if ((state == STOP_LOAD) && input_fp) {
        fclose((FILE*)input_fp);
        input_fp = (0 < fileSp) ? fileStk[fileSp--] : 0;
        state = 0;
    }
    if (input_fp) {
        in = fgets(tib, 190, (FILE*)input_fp);
        if (in != tib) {
            fclose((FILE*)input_fp);
            input_fp = (0 < fileSp) ? fileStk[fileSp--] : 0;
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

int tryOpen(char *root, char *loc, char *fn) {
    char nm[64];
    strCpy(nm, root);
    strCat(nm, loc);
    strCat(nm, fn);
    // printf("try [%s]\n", nm);
    FILE *fp = fopen(nm, "rb");
    if (!fp) { return 0; }
    if (input_fp) { fileStk[++fileSp] = input_fp; }
    input_fp = (cell_t)fp;
    return 1;
}

int lookForFile(char *name) {
#ifdef IS_LINUX
    if (tryOpen("", "", name)) { return 1; }
    if (tryOpen("", "./", name)) { return 1; }
    if (tryOpen(root, "/.local/c3/", name)) { return 1; }
    if (tryOpen(root, "/.local/bin/", name)) { return 1; }
#elif (defined  IS_WINDOWS)
    if (tryOpen("", "", name)) { return 1; }
    if (tryOpen("", ".\\", name)) { return 1; }
    if (tryOpen(root, "\\c3\\", name)) { return 1; }
    if (tryOpen(root, "\\bin\\", name)) { return 1; }
#endif
    return 0;
}

char *doUser(char *pc, int ir) {
    char *cp;
    switch (ir) {
    case SYSTEM:  system(cpop());
    RCASE FOPEN : y=cpop(); cp=cpop(); push((cell_t)fopen(cp, y));
    RCASE FCLOSE: t1=pop(); fclose((FILE*)t1);
    RCASE FREAD:  t1=pop(); n1=pop(); y=cpop(); push(fread( y, 1, n1, (FILE*)t1));
    RCASE FWRITE: t1=pop(); n1=pop(); y=cpop(); push(fwrite(y, 1, n1, (FILE*)t1));
    RCASE FLOAD:  y=cpop(); if (!lookForFile(y)) { printStringF("-file[%s]?-", y); }
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
    ParseLine(": isPC 1 ;");
}

int main(int argc, char *argv[]) {
    root="";
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
