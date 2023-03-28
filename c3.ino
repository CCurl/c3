#include <Arduino.h>

#define mySerial Serial
#define PC(x) printChar(x)

extern "C" {
extern void init();
extern void fill(char *d, char val, int num);
extern void ParseLine(char *s);
extern char tib[128], *in;

    long clock() { return millis(); }
    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }
    void printChar(char c) {}
    void printString(char *s) {
        while (*s) {printChar(*(s++)); }
    }
}

void setup() {
    init();
}

void loop() {
    if (qKey() == 0) { return; }
    int c = key();
    if (!in) {
        in = tib;
        fill(tib, 0, sizeof(tib));
    }

    if (c == 13) {
        *(in) = 0;
        ParseLine(tib);
        in = 0;
    } else if ((c==8) || (c==127)) {
        if (--in < tib) { in = tib; }
        else { PC(8); PC(32); PC(8); }
    } else {
        *(in++) = (31<c) ? 32 : c;
    }
}
