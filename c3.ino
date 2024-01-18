// Support for development boards

#include <Arduino.h>

#define BTW(x,l,h) ((l<=x) && (x<=h))
#define PC(c) printChar(c)
#define RCASE return pc; case

// #define __LITTLEFS__

#define mySerial Serial
extern "C" {
    typedef long cell_t;
    enum { OPEN_INPUT=110, OPEN_OUTPUT, OPEN_PULLUP,
        PIN_READ, PIN_READA, PIN_WRITE, PIN_WRITEA
    };
#ifdef __LITTLEFS__
    #include "LittleFS.h"
    enum { FOPEN=101, FCLOSE, FREAD, FWRITE, FLOAD };
#endif
    extern void push(cell_t);
    extern cell_t pop();
    extern void c3Init();
    extern void fill(char *d, char val, int num);
    extern void parseF(const char* fmt, ...);
    extern void ParseLine(const char *s);
    extern void printStringF(const char *fmt, ...);

    extern char tib[128], *in;
    extern cell_t fileStk[10], fileSp, input_fp, output_fp;

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

// Store and Fetch are 32-bit only on dev boards
#define S1(x, y) (*(x)=((y)&0xFF))
#define G1(x, y) (*(x)<<y)
void Store(char *l, cell_t v) { S1(l,v); S1(l+1,v>>8); S1(l+2,v>>16); S1(l+3,v>>24); }
cell_t Fetch(char *l) { return (*l)|G1(l+1,8)|G1(l+2,16)|G1(l+3,24); }

void loadUserWords() {
    parseF("-ML- PIN-INPUT   %d 3 -MLX- inline", OPEN_INPUT);
    parseF("-ML- PIN-OUTPUT  %d 3 -MLX- inline", OPEN_OUTPUT);
    parseF("-ML- PIN-PULLUP  %d 3 -MLX- inline", OPEN_PULLUP);
    parseF("-ML- DPIN@       %d 3 -MLX- inline", PIN_READ);
    parseF("-ML- APIN@       %d 3 -MLX- inline", PIN_READA);
    parseF("-ML- DPIN!       %d 3 -MLX- inline", PIN_WRITE);
    parseF("-ML- APIN!       %d 3 -MLX- inline", PIN_WRITEA);
    #ifdef __LITTLEFS__
    parseF("-ML- FOPEN       %d 3 -MLX- inline", FOPEN);
    parseF("-ML- FCLOSE      %d 3 -MLX- inline", FCLOSE);
    parseF("-ML- FREAD       %d 3 -MLX- inline", FREAD);
    parseF("-ML- FWRITE      %d 3 -MLX- inline", FWRITE);
    parseF("-ML- FLOAD       %d 3 -MLX- inline", FLOAD);
    parseF("-ML- BLOAD       %d 3 -MLX- inline", BLOAD);
    #endif
    parseF(": isPC 0 ;");
}

char *doUser(char *pc, int ir) {
  cell_t t, n;
  switch (ir) {
    case OPEN_INPUT:   t = pop(); pinMode(t, INPUT);
    RCASE OPEN_OUTPUT: t = pop(); pinMode(t, OUTPUT);
    RCASE OPEN_PULLUP: t = pop(); pinMode(t, INPUT_PULLUP);
    RCASE PIN_READ:    t = pop(); push(digitalRead(t));
    RCASE PIN_READA:   t = pop(); push(analogRead(t));
    RCASE PIN_WRITE:   t = pop(); n = pop(); digitalWrite(t,n);
    RCASE PIN_WRITEA:  t = pop(); n = pop(); analogWrite(t,n);

  #ifdef __LITTLEFS__
    RCASE FOPEN:  t=pop(); push(fOpen(pop(), t));
    RCASE FCLOSE: t=pop(); fClose(t);
    RCASE FREAD:  t=pop(); n=pop(); push(fRead(pop(), 1, n, t));
    RCASE FWRITE: t=pop(); n=pop(); push(fWrite(pop(), 1, n, t));
    RCASE FLOAD:  n=pop(); t=fOpen(n, (cell_t)"rt");
            if (t && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t) { input_fp = t; fill(tib, 0, sizeof(tib)); }
            else { printStringF("-noFile[%s]-", (char*)n); }
    RCASE BLOAD:  n=pop(); t=fOpen(n, (cell_t)"rt");
            if (t && input_fp) { fileStk[++fileSp]=input_fp; }
            if (t) { input_fp = t; fill(tib, 0, sizeof(tib)); }
            else { printStringF("-noFile[%s]-", (char*)n); }
  #endif
    return pc; default: return 0;
  }
}

void setup() {
  c3Init();
  #ifdef __LITTLEFS__
    fileInit();
  #endif
  in = (char*)0;
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
}
