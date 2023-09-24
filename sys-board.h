// Support for development boards
// NOTE: this is a *.h file because the Arduino IDE doesn't like *.inc files

#include <Arduino.h>

#define BTW(x,l,h) ((l<=x) && (x<=h))
#define PC(c) printChar(c)
#define IMMEDIATE 57

#define mySerial Serial
extern "C" {
    typedef long cell_t;
    enum { OPEN_INPUT = 110, OPEN_OUTPUT, OPEN_PULLUP,
        PIN_READ, PIN_READA, PIN_WRITE, PIN_WRITEA
    };
    extern void push(cell_t);
    extern cell_t pop();
    extern void c3Init();
    extern void fill(char *d, char val, int num);
    extern void ParseLine(const char *s);
    extern char tib[128], *in;

#ifdef mySerial
    void printChar(char c) { mySerial.print(c); }
    void printString(char *s) { mySerial.print(s); }
    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }
#else
    void printChar(char c) {}
    void printString(char *s) {}
    int qKey() { return 0; }
    int key() { return 0; }
#endif

cell_t sysTime() { return micros(); }

void loadStartupWords() {
    ParseLine(": isPC 0 ;");
    ParseLine("-ML- PIN-INPUT   100 3 -MLX- inline");
    ParseLine("-ML- PIN-OUTPUT  101 3 -MLX- inline");
    ParseLine("-ML- PIN-PULLUP  102 3 -MLX- inline");
    ParseLine("-ML- DPIN@       103 3 -MLX- inline");
    ParseLine("-ML- APIN@       104 3 -MLX- inline");
    ParseLine("-ML- DPIN!       105 3 -MLX- inline");
    ParseLine("-ML- APIN!       106 3 -MLX- inline");
}

void loadUserWords() { }

char *doUser(char *pc, int ir) {
        cell_t t, n;
        switch (ir) {
        case OPEN_INPUT:  t = pop(); pinMode(t, INPUT);             return pc;
        case OPEN_OUTPUT: t = pop(); pinMode(t, OUTPUT);            return pc;
        case OPEN_PULLUP: t = pop(); pinMode(t, INPUT_PULLUP);      return pc;
        case PIN_READ:    t = pop(); push(digitalRead(t));          return pc;
        case PIN_READA:   t = pop(); push(analogRead(t));           return pc;
        case PIN_WRITE:   t = pop(); n = pop(); digitalWrite(t,n);  return pc;
        case PIN_WRITEA:  t = pop(); n = pop(); analogWrite(t,n);   return pc;
        default: return 0;
        }
    }
}

void setup() {
    c3Init();
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
