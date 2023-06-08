#include <Arduino.h>

#define mySerial Serial
#define PC(x) printChar(x)
#define BTW(x,l,h) ((l<=x) && (x<=h))

extern "C" {
    typedef long cell_t;
    enum { PIN_INPUT = IMMEDIATE+1, PIN_OUTPUT, PIN_PULLUP,
        PIN_READ, PIN_WRITE,
        PIN_READA, PIN_WRITEA
    };
    extern void push(cell_t);
    extern cell_t pop();
    extern void init();
    extern void fill(char *d, char val, int num);
    extern void ParseLine(const char *s);
    extern char tib[128], *in;

    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }

    cell_t clock() { return millis(); }
    void printChar(char c) {}
    void printString(char *s) {
        while (*s) {printChar(*(s++)); }
    }

    void loadStartupWords() {
        ParseLine(" : isPC 0 ;");
    }

    char *doUser(char *pc, int ir) {
        cell_t t, n;
        switch (ir) {
        case PIN_INPUT:  t = pop();                                 return pc;
        case PIN_OUTPUT: t = pop();                                 return pc;
        case PIN_PULLUP: t = pop();                                 return pc;
        case PIN_READ:   t = pop(); push(digitalRead(t));           return pc;
        case PIN_WRITE:  t = pop(); n = pop(); digitalWrite(t,n);   return pc;
        case PIN_READA:  t = pop(); push(analogRead(t));            return pc;
        case PIN_WRITEA: t = pop(); n = pop(); analogWrite(t,n);    return pc;
        default: return 0;
        }
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
