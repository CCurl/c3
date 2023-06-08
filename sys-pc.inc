// Support for PCs

void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }
void loadStartupWords() { ParseLine(" : isPC 1 ;"); }

void getInput() {
    ClearTib;
    if ((state == STOP_LOAD) && input_fp) {
        fclose((FILE*)input_fp);
        input_fp = (0 < fileSp) ? fileStk[fileSp--] : 0;
        state = 0;
    }
    if (input_fp) {
        in = fgets(tib, sizeof(tib), (FILE*)input_fp);
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

int main(int argc, char *argv[]) {
    init();
    output_fp = 0;
    for (int i=1; i<argc; i++) {
        FILE *fp = fopen(argv[i],"rt");
        if (fp) { fileStk[++fileSp] = (cell_t)fp; }
        else {
            char x[2] = {i+'0',0};
            ClearTib; SC(argv[i]); SC(" s"); SC(x);
            ParseLine(tib);
        }
    }
    input_fp = (cell_t)fopen("core.c3", "rt");
    if (!input_fp) { input_fp = (cell_t)fopen("..\\core.c3", "rt"); }
    if (!input_fp && fileSp) { input_fp = fileStk[fileSp--]; }
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
