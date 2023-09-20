// Support for PCs
// NOTE: this is a *.ipp file because the Arduino IDE doesn't like *.inc files
#include <time.h>

void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }
cell_t sysTime() { return clock(); }

void getInput() {
    ClearTib;
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

enum { SYSTEM = 100, FOPEN, FCLOSE, FREAD, FWRITE, FLOAD };

char *doUser(char *pc, int ir) {
    switch (ir) {
    case SYSTEM: t1=pop(); system(ToCP(t1+1));                                      return pc;
    case FOPEN:  t1=pop(); TOS=(cell_t)fopen(ToCP(TOS+1), ToCP(t1+1));              return pc;
    case FCLOSE: t1=pop(); fclose((FILE*)t1);                                       return pc;
    case FREAD:  t1=pop(); n1=pop(); TOS =  fread(ToCP(TOS), 1, n1, (FILE*)t1);     return pc;
    case FWRITE: t1=pop(); n1=pop(); TOS = fwrite(ToCP(TOS), 1, n1, (FILE*)t1);     return pc;
    case FLOAD:  y=ToCP(pop()); t1=(cell_t)fopen(y+1, "rt");
            if (t1 && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t1) { input_fp = t1; ClearTib; }
            else { printStringF("-noFile[%s]-", y+1); }                                   return pc;
    default:                                                                        return 0;
    }
}

void loadStartupWords() {
    ParseLine(": isPC 1 ;");
    //#include "sys-load.ipp"
    input_fp = (cell_t)fopen("core.c3", "rt");
    if (!input_fp) { input_fp = (cell_t)fopen("..\\core.c3", "rt"); }
}

int main(int argc, char *argv[]) {
    input_fp = output_fp = 0;
    c3Init();
    for (int i=1; i<argc; i++) {
        FILE *fp = fopen(argv[i],"rt");
        if (fp) { fileStk[++fileSp] = (cell_t)fp; }
        else {
            char x[2] = {i+'0',0};
            ClearTib; SC(argv[i]); SC(" s"); SC(x);
            ParseLine(tib);
        }
    }
    if (!input_fp && fileSp) { input_fp = fileStk[fileSp--]; }
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
