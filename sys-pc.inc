// Support for PCs

void printChar(const char c) { fputc(c, output_fp ? (FILE*)output_fp : stdout); }
void printString(const char* s) { fputs(s, output_fp ? (FILE*)output_fp : stdout); }

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
    for (int i=1; (i<argc) && (!input_fp); i++) {
        input_fp = (cell_t)fopen(argv[i],"rt");
        if (!input_fp) {
            ClearTib; SC(argv[i]); SC(" s"); SC(iToA(i,10));
            ParseLine(tib);
        }
    }
    if (!input_fp) { input_fp = (cell_t)fopen("core.f", "rt"); }
    if (!input_fp) { input_fp = (cell_t)fopen("..\\core.f", "rt"); }
    //input_fp = 0;
    output_fp = 0;
    while (state != ALL_DONE) {
        getInput();
        ParseLine(tib);
    }
    return 0;
}
