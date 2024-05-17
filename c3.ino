// Support for development boards

#include "c3.h"

#define mySerial Serial // Teensy and Pico

enum { xFOPEN=101, xFCLOSE, xFREAD, xFWRITE, xFGETS, FLOAD, BLOAD,
    OPEN_INPUT=110, OPEN_OUTPUT, OPEN_PULLUP,
    PIN_READ, PIN_READA, PIN_WRITE, PIN_WRITEA,
    EDIT_BLK
};

#ifdef mySerial
    void serialInit() { while (!mySerial) ; }
    void printChar(char c) { mySerial.print(c); }
    void printString(const char *s) { mySerial.print(s); }
    int qKey() { return mySerial.available(); }
    int key() { 
        while (!qKey()) {}
        return mySerial.read();
    }
#else
    void serialInit() { }
    void printChar(char c) {}
    void printString(char *s) {}
    int qKey() { return 0; }
    int key() { return 0; }
#endif

cell_t sysTime() { return micros(); }

// Cells are always 32-bit on dev boards (no 64-bit)
#define S1(x, y) (*(x)=((y)&0xFF))
#define G1(x, y) (*(x)<<y)
void Store(char *l, cell_t v) { S1(l,v); S1(l+1,v>>8); S1(l+2,v>>16); S1(l+3,v>>24); }
cell_t Fetch(const char *l) { return (*l)|G1(l+1,8)|G1(l+2,16)|G1(l+3,24); }

void loadUserWords() {
    parseF("-ML- PIN-INPUT   %d 3 -MLX- inline", OPEN_INPUT);
    parseF("-ML- PIN-OUTPUT  %d 3 -MLX- inline", OPEN_OUTPUT);
    parseF("-ML- PIN-PULLUP  %d 3 -MLX- inline", OPEN_PULLUP);
    parseF("-ML- DPIN@       %d 3 -MLX- inline", PIN_READ);
    parseF("-ML- APIN@       %d 3 -MLX- inline", PIN_READA);
    parseF("-ML- DPIN!       %d 3 -MLX- inline", PIN_WRITE);
    parseF("-ML- APIN!       %d 3 -MLX- inline", PIN_WRITEA);
    parseF("-ML- FOPEN       %d 3 -MLX- inline", xFOPEN);
    parseF("-ML- FCLOSE      %d 3 -MLX- inline", xFCLOSE);
    parseF("-ML- FREAD       %d 3 -MLX- inline", xFREAD);
    parseF("-ML- FWRITE      %d 3 -MLX- inline", xFWRITE);
    parseF("-ML- FGETS       %d 3 -MLX- inline", xFGETS);
    parseF("-ML- (LOAD)      %d 3 -MLX- inline", FLOAD);
    parseF("-ML- BLOAD       %d 3 -MLX- inline", BLOAD);
    parseF("-ML- EDIT        %d 3 -MLX- inline", EDIT_BLK);
    parseF(": isPC 0 ;");
}

char *doUser(char *pc, int ir) {
  cell_t t, n, x;
  switch (ir) {
    case OPEN_INPUT:   t=pop(); pinMode(t, INPUT);
    RCASE OPEN_OUTPUT: t=pop(); pinMode(t, OUTPUT);
    RCASE OPEN_PULLUP: t=pop(); pinMode(t, INPUT_PULLUP);
    RCASE PIN_READ:    t=pop(); push(digitalRead(t));
    RCASE PIN_READA:   t=pop(); push(analogRead(t));
    RCASE PIN_WRITE:   t=pop(); n=pop(); digitalWrite(t, n);
    RCASE PIN_WRITEA:  t=pop(); n=pop(); analogWrite(t, n);

    RCASE xFOPEN:  t=pop(); n=pop(); push(fOpen((char*)n, (char*)t));
    RCASE xFCLOSE: t=pop(); fClose(t);
    RCASE xFREAD:  t=pop(); n=pop(); x=pop(); push(fRead((char*)x, 1, (int)n, t));
    RCASE xFWRITE: t=pop(); n=pop(); x=pop(); push(fWrite((char*)x, 1, (int)n, t));
    RCASE xFGETS:  t=pop(); n=pop(); x=pop(); push(fGets(t, (char*)x, (int)n));
    RCASE FLOAD:   n=pop(); t=fOpen((char*)n, "rt");
            if (t && input_fp) { ipush(input_fp); }
            if (t) { input_fp = t; *in = 0; in = (char*)0; }
            else { printStringF("-noFile[%s]-", (char*)n); }
    RCASE BLOAD:    t=pop(); blockLoad((int)t);
    RCASE EDIT_BLK: t=pop(); editBlock(t);
    return pc; default: return 0;
  }
}

void setup() {
  serialInit();
  printString("Hello.");
  fileInit();
  c3Init();
  printString(" ok\r\n");
  in = (char*)0;
}

void idle() {
  // TODO
}

void loop() {
  if (qKey() == 0) { idle(); return; }
  int c = key();
  if (!in) {
      in = tib;
      fill(tib, 0, sizeof(tib));
  }

  if (c==9) { c = 32; }
  if (c==13) {
      *(in) = 0;
      ParseLine(tib);
      printString(" ok\r\n");
      in = 0;
  } else if ((c==8) || (c==127)) {
      if ((--in) < tib) { in = tib; }
      else { PC(8); PC(32); PC(8); }
  } else {
      if (BTW(c,32,126)) { *(in++) = c; PC(c); }
  }
}
