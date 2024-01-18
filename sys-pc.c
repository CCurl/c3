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

char *doUser(char *pc, int ir) {
    char *cp;
    switch (ir) {
    case SYSTEM:  system(cpop());
    RCASE FOPEN : y = cpop(); cp = cpop(); push((cell_t)fopen(cp, y));
    RCASE FCLOSE: t1=pop(); fclose((FILE*)t1);
    RCASE FREAD:  t1=pop(); n1=pop(); y=cpop(); push(fread( y, 1, n1, (FILE*)t1));
    RCASE FWRITE: t1=pop(); n1=pop(); y=cpop(); push(fwrite(y, 1, n1, (FILE*)t1));
    RCASE FLOAD:  y=cpop(); t1=(cell_t)fopen(y, "rt");
            if (t1 && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t1) { input_fp = t1; fill(tib, 0, sizeof(tib)); }
            else { printStringF("-noFile[%s]-", y); }
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

int tryOpen(char *loc, char *fn) {
    tib[0]=0;
    if (loc && *loc) { strCpy(tib, loc); }
    strCat(tib, fn);
    FILE *fp = fopen(tib, "rb");
    if (!fp) { return 0; }
    fileStk[++fileSp] = (cell_t)fp;
    return 1;
}

void lookForStartUpFile() {
    y="block-000.c3";
#ifdef IS_LINUX
    if (tryOpen("./", y)) { return; }
    if (tryOpen("~/.local/c3/", y)) { return; }
    if (tryOpen("~/.local/bin/", y)) { return; }
#elif (defined  IS_WINDOWS)
    if (tryOpen(".\\", y)) { return; }
    if (tryOpen("c:\\bin\\", y)) { return; }
    if (tryOpen("c:\\bin\\c3\\", y)) { return; }
    if (tryOpen("c:\\c3\\", y)) { return; }
#endif
}

int main(int argc, char *argv[]) {
    input_fp = output_fp = 0;
    c3Init();

    for (int i=1; i<argc; i++) {
        if (tryOpen("", argv[i])) { continue; }
        if (isNum(argv[i])) { reg[i] = pop(); }
        else { reg[i] = (cell_t)argv[i]; }
    }
    lookForStartUpFile();
    if (fileSp) { input_fp = fileStk[fileSp--]; }
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}

#endif // isPC
