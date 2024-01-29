// editor.cpp - A simple block editor

#include "c3.h"

#define __EDITOR__

#ifndef __EDITOR__

    void editFile(char *fn) {}
    void editBlock(cell_t blockNum) {}

#else

#define MAX_LINES     500
#define LLEN          100
#define SCR_LINES      35
#define BLOCK_SZ      (MAX_LINES*LLEN)
#define EDCHAR(l,o)   edBuf[((l)*LLEN)+(o)]
#define EDCH(l,o)     EDCHAR(scrTop+l,o)
#define SHOW(l,v)     lineShow[(scrTop+l)]=v
#define DIRTY(l)      isDirty=1; SHOW(l,1)

enum { COMMAND = 1, INSERT, REPLACE, QUIT };
char theBlock[BLOCK_SZ];
int line, off, blkNum, edMode, scrTop;
int isDirty, lineShow[MAX_LINES];
char edBuf[BLOCK_SZ], tBuf[LLEN], mode[32], *msg = NULL;
char yanked[LLEN];

void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
void ClearEOL() { printString("\x1B[K"); }
void CursorOn() { printString("\x1B[?25h"); }
void CursorOff() { printString("\x1B[?25l"); }
void Color(int fg, int bg) { printStringF("\x1B[%d;%dm", (30+fg), bg?bg:40); }
void commandMode() { edMode=COMMAND; strCpy(mode, "command"); }
void insertMode()  { edMode=INSERT;  strCpy(mode, "insert"); }
void replaceMode() { edMode=REPLACE; strCpy(mode, "replace"); }
int edKey() { return key(); }

void NormLO() {
    line = min(max(line, 0), SCR_LINES-1);
    off = min(max(off,0), LLEN-1);
}

void showAll() {
    for (int i = 0; i < MAX_LINES; i++) { lineShow[i] = 1; }
}

char edChar(int l, int o) {
    char c = EDCH(l,o);
    if (c==0) { return c; }
    return BTW(c,32,126) ? c : ' ';
}

void showCursor() {
    char c = EDCH(line, off);
    if (c == 0) c = 'X';
    if (c < 32) c += ('a'-1);
    GotoXY(off + 1, line + 1);
    Color(0, 47);
    printChar(c);
    Color(7, 0);
}

void showLine(int l) {
    int sl = scrTop+l;
    if (!lineShow[sl]) { return; }
    SHOW(l,0);
    GotoXY(1, l+1);
    for (int o = 0; o < LLEN; o++) {
        int c = edChar(l, o);
        if (c) { printChar(c); }
        else { ClearEOL(); break; }
    }
    if (l == line) { showCursor(); }
}

void showStatus() {
    static int cnt = 0;
    GotoXY(1, SCR_LINES+2);
    printString("- Block Editor v0.1 - ");
    printStringF("Block# %03d%s", blkNum, isDirty ? " *" : "");
    printStringF("%s- %s", msg ? msg : " ", mode);
    ClearEOL();
    if (msg && (1 < ++cnt)) { msg = NULL; cnt = 0; }
}

void showHelp() {
	return;
    GotoXY(1, SCR_LINES+2);
    printString("  (h/j/k/l/tab/spc/bs/_/$) Move Cursor (g) Top (G) Bottom");
    printString("\r\n  (x) Del (X) Del-Prev (r) Replace 1 (i) Insert (R) Replace");
    printString("\r\n  (W) Write (L) Reload (+) Next (-) Prev (Q) Quit");
    printString("\r\n  (^A) Define (^B) Comment (^C) Inline (^D) Var");
    printString("\r\n  (^E) Machine (^F) Compile (^G) Interpret");
}

void showEditor() {
    for (int i = 0; i < SCR_LINES; i++) { showLine(i); }
}

void mv(int l, int o) {
    SHOW(line,1);
    line += l;
    off += o;
    NormLO();
    SHOW(line,1);
}

void gotoEOL() {
    mv(0, -99);
    while (EDCH(line, off) != 10) { ++off; }
}

int toBlock() {
    fill(theBlock, 0, BLOCK_SZ);
    for (int l=0; l<MAX_LINES; l++) {
        char *y=&EDCHAR(l,0);
        strCat(theBlock,y);
    }
    return strLen(theBlock);
}

void addLF(int l) {
    int o, lc = 0;
    for (o = 0; EDCH(l, o); ++o) { lc = EDCH(l, o); }
    if (lc != 10) { EDCH(l, o) = 10; }
}

void toBuf() {
    int o = 0, l = 0, ch;
    fill(edBuf, 0, BLOCK_SZ);
    for (int i = 0; i < BLOCK_SZ; i++) {
        ch = theBlock[i];
        if (ch == 0) { break; }
        if (ch ==10) {
            EDCHAR(l, o) = (char)ch;
            if (MAX_LINES <= (++l)) { return; }
            o=0;
            continue;
        } else if ((o < LLEN) && (ch!=13)) {
            EDCHAR(l,o++) = (char)ch;
        }
    }
    for (int i = 0; i < MAX_LINES; i++) { addLF(i); }
}

void edRdBlk(int force) {
    char fn[32];
    fill(theBlock, 0, BLOCK_SZ);
    sprintf(fn, "block-%03d.c3", (int)blkNum);
    cell_t fp = fOpen((cell_t)fn, (cell_t)"rb");
    if (fp) {
        fRead((cell_t)theBlock, BLOCK_SZ, 1, fp);
        fClose(fp);
    }
    toBuf();
    showAll();
    isDirty = 0;
}

void edSvBlk(int force) {
    if (isDirty || force) {
        toBlock();
        char fn[32];
        sprintf(fn, "block-%03d.c3", (int)blkNum);
        cell_t fp = fOpen((cell_t)fn, (cell_t)"wb");
        if (fp) {
            fWrite((cell_t)theBlock, 1, BLOCK_SZ, fp);
            fClose(fp);
        }
        isDirty = 0;
    }
}

void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        EDCH(line,o) = EDCH(line, o+1);
    }
    DIRTY(line);
    addLF(line);
}

void deleteLine() {
    EDCH(line,0) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

void insertSpace() {
    for (int o=LLEN-1; off<o; o--) {
        EDCH(line,o) = EDCH(line, o-1);
    }
    EDCH(line, off)=32;
}

void insertLine() {
    insertSpace();
    EDCH(line, off)=10;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

void joinLines() {
    gotoEOL();
    EDCH(line, off) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

void replaceChar(char c, int force, int mov) {
    for (int o=off-1; 0<=o; --o) {
        int ch = EDCH(line, o);
        if (ch && (ch != 10)) { break; }
        EDCH(line,o)=32;
    }
    EDCH(line, off)=c;
    DIRTY(line);
    addLF(line);
    if (mov) { mv(0, 1); }
}

int doInsertReplace(char c) {
    if (c==13) {
        if (edMode == REPLACE) { mv(1, -999); }
        else { insertLine(); mv(1,-99); }
        return 1;
    }
    if (!BTW(c,32,126)) { return 1; }
    if (edMode == INSERT) { insertSpace(); }
    replaceChar(c, 1, 1);
    return 1;
}

int doCommon(int c) {
    int l = line, o = off, st = scrTop;
    if ((c == 8) || (c == 127)) { mv(0, -1); }                  // <backspace>
    else if (c ==  4) { scrTop = min(scrTop+15,MAX_LINES-SCR_LINES+1); showAll(); } // <ctrl-d>
    else if (c ==  5) { scrTop = min(scrTop+1, MAX_LINES-SCR_LINES+1); showAll(); } // <ctrl-e>
    else if (c ==  9) { mv(0, 8); }                             // <tab>
    else if (c == 10) { mv(1, 0); }                             // <ctrl-j>
    else if (c == 11) { mv(-1, 0); }                            // <ctrl-k>
    else if (c == 12) { mv(0, 1); }                             // <ctrl-l>
    else if (c == 24) { deleteChar(); }                         // <ctrl-x>
    else if (c == 21) { scrTop = max(scrTop-15,0); showAll(); } // <ctrl-u>
    else if (c == 25) { scrTop = max(scrTop-1, 0); showAll(); } // <ctrl-y>
    return ((l != line) || (o != off) || (st != scrTop)) ? 1 : 0;
}

int processEditorChar(int c) {
    if (c==27) { commandMode(); return 1; }
    if (doCommon(c)) { return 1; }
    if (BTW(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c);
    }

    switch (c) {
        BCASE  13: mv(1,-999);
        case  ' ': mv(0, 1);
        BCASE 'h': mv(0,-1);
        BCASE 'l': mv(0,1);
        BCASE 'j': mv(1,0);
        BCASE 'k': mv(-1,0);
        BCASE '_': mv(0,-99);
        BCASE 'a': mv(0, 1); insertMode();
        BCASE 'A': gotoEOL(); insertMode();
        BCASE 'J': joinLines();
        BCASE '$': gotoEOL();
        BCASE 'g': mv(-99,-99);
        BCASE 'G': mv(99,-999);
        BCASE 'i': insertMode();
        BCASE 'I': mv(0, -99); insertMode();
        BCASE 'o': mv(1, -99); insertLine(); insertMode();
        BCASE 'O': mv(0, -99); insertLine(); insertMode();
        BCASE 'r': replaceChar(edKey(), 0, 1);
        BCASE 'R': replaceMode();
        BCASE 'c': deleteChar(); insertMode();
        BCASE 'C': c=off; while (c<LLEN) { EDCH(line, c) = 0; c++; }
                addLF(line); DIRTY(line); insertMode();
        BCASE 'D': deleteLine();
        BCASE 'x': deleteChar();
        BCASE 'X': if (0 < off) { --off; deleteChar(); }
        BCASE 'L': edRdBlk(1);
        BCASE 'W': isDirty = 1; edSvBlk(1);
        BCASE 'Y': strCpy(yanked, &EDCH(line, 0));
        BCASE 'p': mv(1,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
        BCASE 'P': mv(0,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
        BCASE '+': edSvBlk(0); ++blkNum; edRdBlk(0);
        BCASE '-': edSvBlk(0); blkNum = max(0, blkNum-1); edRdBlk(0);
        BCASE 'Q': toBlock(); edMode = QUIT;
    }
    return 1;
}

void editBlock(cell_t blk) {
    blkNum = max((int)blk, 0);
    line = off = scrTop = 0;
    msg = NULL;
    CLS();
    CursorOff();
    edRdBlk(0);
    commandMode();
    showAll();
    showHelp();
    while (edMode != QUIT) {
        showEditor();
        showStatus();
        processEditorChar(edKey());
    }
    GotoXY(1, SCR_LINES + 3);
    CursorOn();
}

void editFile(char *fn) {}

#endif // __EDITOR__