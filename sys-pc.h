// Support for PCs
// NOTE: this is a *.h file because the Arduino IDE doesn't like *.inc files
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
    case SYSTEM: system(cpop());                                                return pc;
    case FOPEN:  y=cpop(); TOS=(cell_t)fopen(CTOS, y);                          return pc;
    case FCLOSE: t1=pop(); fclose((FILE*)t1);                                   return pc;
    case FREAD:  t1=pop(); n1=pop(); y = CTOS;
        TOS = feof((FILE*)t1) ? 0 : fread( y, 1, n1, (FILE*)t1);                return pc;
    case FWRITE: t1=pop(); n1=pop(); TOS = fwrite(CTOS, 1, n1, (FILE*)t1);      return pc;
    case FLOAD:  y=cpop(); t1=(cell_t)fopen(y, "rt");
            if (t1 && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t1) { input_fp = t1; ClearTib; }
            else { printStringF("-noFile[%s]-", y); }                           return pc;
    default:                                                                    return 0;
    }
}

void loadStartupWords() {
    parseF("-ML- SYSTEM %d 3 -MLX- inline", SYSTEM);
    parseF("-ML- FOPEN  %d 3 -MLX- inline", FOPEN);
    parseF("-ML- FCLOSE %d 3 -MLX- inline", FCLOSE);
    parseF("-ML- FREAD  %d 3 -MLX- inline", FREAD);
    parseF("-ML- FWRITE %d 3 -MLX- inline", FWRITE);
    parseF("-ML- (LOAD) %d 3 -MLX- inline", FLOAD);
}

void loadUserWords() {
    ParseLine(": isPC 1 ;");
}

int main(int argc, char *argv[]) {
    input_fp = output_fp = 0;
    c3Init();
    for (int i=1; i<argc; i++) {
        FILE *fp = fopen(argv[i], "rt");
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
