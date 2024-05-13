// editor.cpp - A simple block editor

#include "c3.h"
#include <string.h>

#define CELL          cell_t

#ifndef __EDITOR__
void editBlock(CELL Blk) { printString("-noEdit-"); }
CELL edScrH = 0;
#else

#define LLEN          100
#define SCR_HEIGHT     35

#define SCR_LINES     (int)edScrH
#define BLOCK_SZ      (MAX_LINES*LLEN)
#define EDCHAR(l,o)   edBuf[((l)*LLEN)+(o)]
#define EDCH(l,o)     EDCHAR(scrTop+l,o)
#define SHOW(l,v)     lineShow[(scrTop+l)]=v
#define DIRTY(l)      isDirty=1; SHOW(l,1)

enum { NORMAL = 1, INSERT, REPLACE, QUIT };
static char theBlock[BLOCK_SZ];
static int line, off, blkNum, edMode, scrTop;
static int isDirty, lineShow[MAX_LINES];
static char edBuf[BLOCK_SZ], tBuf[LLEN], mode[32], *msg = NULL;
static char yanked[LLEN];
CELL edScrH = SCR_HEIGHT; // can be set from c3 using '50 (scr-h) !'

static void GotoXY(int x, int y) { printStringF("\x1B[%d;%dH", y, x); }
static void CLS() { printString("\x1B[2J"); GotoXY(1, 1); }
static void ClearEOL() { printString("\x1B[K"); }
static void CursorOn() { printString("\x1B[?25h"); }
static void CursorOff() { printString("\x1B[?25l"); }
static void Color(int fg, int bg) { printStringF("\x1B[%d;%dm", (30+fg), bg?bg:40); }
static void normalMode() { edMode=NORMAL; strCpy(mode, "normal"); }
static void insertMode()  { edMode=INSERT;  strCpy(mode, "insert"); }
static void replaceMode() { edMode=REPLACE; strCpy(mode, "replace"); }
static void toggleInsert() { if (edMode!=INSERT) insertMode(); else normalMode(); }

static int edKey() {
    int x = key();
    if (x == 224) {  // Windows: start char
         x = (x << 5) ^ key();
    } else if (x==27) {
        CELL_T stopTime = sysTime() + 50;
        while (sysTime() < stopTime) {
            if (qKey()) { x = (x << 5) ^ key(); }
        }
    }
    return x;
}

static void NormLO() {
    line = MIN(MAX(line, 0), SCR_LINES-1);
    off = MIN(MAX(off, 0), LLEN-1);
    scrTop = MIN(MAX(scrTop, 0), MAX_LINES-SCR_LINES);
}

static void showAll() {
    for (int i = 0; i < MAX_LINES; i++) { lineShow[i] = 1; }
}

static char edChar(int l, int o) {
    char c = EDCH(l,o);
    if (c==0) { return c; }
    return BTW(c,32,126) ? c : ' ';
}

static void showCursor() {
    char c = EDCH(line, off);
    GotoXY(off + 1, line + 1);
    Color(0, 47);
    printChar(MAX(c,32));
    Color(7, 0);
}

static void showLine(int l) {
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
    printStringF("  [%d:%d]", (line+scrTop)+1, off+1);
    ClearEOL();
    if (msg && (1 < ++cnt)) { msg = NULL; cnt = 0; }
}

static void showEditor() {
    for (int i = 0; i < SCR_LINES; i++) { showLine(i); }
}

static void scroll(int amt) {
    int st = scrTop;
    scrTop += amt;
    if (st != scrTop) { line -= amt; showAll(); }
    NormLO();
}

static void mv(int l, int o) {
    SHOW(line,1);
    line += l;
    off += o;
    if (line < 0) { scroll(line); }
    if (SCR_LINES <= line) { scroll(line-SCR_LINES+1); }
    NormLO();
    SHOW(line,1);
}

static void gotoEOL() {
    char *ln = &EDCH(line, 0);
    off = 0;
    while (31<ln[off]) { ++off; }
    mv(0,0);
}

static int toBlock() {
    fill(theBlock, 0, BLOCK_SZ);
    for (int i=0; i<MAX_LINES; i++) {
        char *y = &EDCHAR(i,0);
        strCat(theBlock, y);
    }
    return strLen(theBlock);
}

static void addLF(int l) {
    char *ln = &EDCH(l, 0);
    int x=0;
    while (31 < ln[x]) { ++x; }
    ln[x++] = 10; ln[x] = 0;
}

static void toBuf() {
    int o = 0, l = 0, ch;
    fill(edBuf, 0, BLOCK_SZ);
    for (int i = 0; i < BLOCK_SZ; i++) {
        ch = theBlock[i];
        if (ch == 0) { break; }
        if (ch == 10) {
            EDCHAR(l, o) = (char)ch;
            if (MAX_LINES <= (++l)) { return; }
            o=0;
            continue;
        } else if ((o < LLEN) && BTW(ch,32,126)) {
            EDCHAR(l,o++) = (char)ch;
        }
    }
    l = scrTop; scrTop = 0;
    for (int i = 0; i < MAX_LINES; i++) { addLF(i); }
    scrTop = l;
}

static void edRdBlk(int force) {
    fill(theBlock, 0, BLOCK_SZ);
    readBlock(blkNum, theBlock, BLOCK_SZ);
    toBuf();
    showAll();
    isDirty = 0;
}

static void edSvBlk(int force) {
    if (isDirty || force) {
        int len = toBlock();
        while (1<len) {
            if (theBlock[len-2] == 10) { theBlock[len-1]=0; --len; }
            else { break; }
        }
        writeBlock(blkNum, theBlock, len);
        sprintf(tBuf, " - %d bytes ", (int)len);
        msg = &tBuf[0];
        isDirty = 0;
    }
}

static void deleteChar() {
    for (int o = off; o < (LLEN - 2); o++) {
        EDCH(line,o) = EDCH(line, o+1);
    }
    DIRTY(line);
    addLF(line);
}

static void deleteWord() {
    char ch = lower(EDCH(line, off));
    while (BTW(ch,'a','z') || BTW(ch,'0','9')) {
        deleteChar();
        ch = lower(EDCH(line, off));
    }
    while (EDCH(line,off)==' ') { deleteChar(); }
}

static void deleteLine() {
    EDCH(line,0) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

static void insertSpace() {
    for (int o=LLEN-1; off<o; o--) {
        EDCH(line,o) = EDCH(line, o-1);
    }
    EDCH(line, off)=32;
}

static void insertLine() {
    insertSpace();
    EDCH(line, off)=10;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

static void joinLines() {
    gotoEOL();
    EDCH(line, off) = 0;
    toBlock();
    toBuf();
    showAll();
    isDirty = 1;
}

static void replaceChar(char c, int force, int mov) {
    if (!BTW(c,32,126) && (!force)) { return; }
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

static int doInsertReplace(char c) {
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

static void edDelX(int c) {
    if (c==0) { c = key(); }
    if (c=='d') { strCpy(yanked, &EDCH(line, 0)); deleteLine(); }
    else if (c=='w') { deleteWord(); }
    else if (c=='.') { deleteChar(); }
    else if (c=='X') { if (0<off) { --off; deleteChar(); } }
    else if (c=='$') {
        c=off; while ((c<LLEN) && EDCH(line,c)) { EDCH(line,c)=0; c++; }
        addLF(line); DIRTY(line);
    }
}

static int edReadLine(char *buf, int sz) {
    int len = 0;
    CursorOn();
    while (len<(sz-1)) {
        char c = key();
        if (c==27) { len=0; break; }
        if (c==13) { break; }
        if ((c==127) || ((c==8) && (len))) { --len; printStringF("%c %c",8,8); }
        if (BTW(c,32,126)) { buf[len++]=c; printChar(c); }
    }
    CursorOff();
    buf[len]=0;
    return len;
}

static void edCommand() {
    char buf[32];
    GotoXY(1, SCR_LINES+3); ClearEOL();
    printChar(':');
    edReadLine(buf, sizeof(buf));
    GotoXY(1, SCR_LINES+3); ClearEOL();
    if (strEq(buf,"w")) { edSvBlk(0); }
    else if (strEq(buf,"w!")) { edSvBlk(1); }
    else if (strEq(buf,"wq")) { edSvBlk(0); edMode=QUIT; }
    else if (strEq(buf,"q!")) { edMode=QUIT; }
    else if (strEq(buf,"q")) {
        if (isDirty) { printString("(use 'q!' to quit without saving)"); }
        else { edMode=QUIT; }
    }
}

static int doCommon(int c) {
    int l = line, o = off, st = scrTop;
    if (((c == 8) || (c == 127)) && (0 < off)) {      // <backspace>
        mv(0, -1); if (edMode == INSERT) { deleteChar(); }
    }
    else if (c ==  4) { scroll(SCR_LINES/2); }              // <ctrl-d>
    else if (c ==  5) { scroll(1); }                        // <ctrl-e>
    else if (c ==  9) { mv(0, 8); }                         // <tab>
    else if (c == 10) { mv(1, 0); }                         // <ctrl-j>
    else if (c == 11) { mv(-1, 0); }                        // <ctrl-k>
    else if (c == 12) { mv(0, 1); }                         // <ctrl-l>
    else if (c == 24) { edDelX('X'); return 1; }            // <ctrl-x>
    else if (c == 21) { scroll(-SCR_LINES/2); }             // <ctrl-u>
    else if (c == 25) { scroll(-1); }                       // <ctrl-y>
    else if (c == 26) { edDelX('.'); return 1; }            // <ctrl-z>
    else if (c == 7240) { mv(-1, 0); }                      // Windows: Up
    else if (c == 7243) { mv(0, -1); }                      // Windows: Left
    else if (c == 7245) { mv(0, 1);   }                     // Windows: Right
    else if (c == 7248) { mv(1, 0);    }                    // Windows: Down
    else if (c == 7239) { mv(0, -99);  }                    // Windows: Home
    else if (c == 7247) { gotoEOL();  }                     // Windows: End
    else if (c == 7241) { mv(-(SCR_LINES - 1), -99);   }    // Windows: PgUp
    else if (c == 7249) { mv(SCR_LINES - 1, -99);         } // Windows: PgDn
    else if (c == 7251) { edDelX('.'); return 1; }          // Windows: Delete
    else if (c == 7250) { toggleInsert(); return 1; }       // Windows: Insert
    else if (c == 7287) { mv(-999, -99); return 1; }        // Windows: <ctrl>-Home

    return ((l != line) || (o != off) || (st != scrTop)) ? 1 : 0;
}

static int processEditorChar(int c) {
    if (c==27) { normalMode(); return 1; }
    if (doCommon(c)) { return 1; }
    if (BTW(edMode,INSERT,REPLACE)) {
        return doInsertReplace((char)c);
    }

    switch (c) {
        case   13: mv(1,-99);
        BCASE ' ': mv(0, 1);
        BCASE 'h': mv(0,-1);
        BCASE 'l': mv(0, 1);
        BCASE 'j': mv(1, 0);
        BCASE 'k': mv(-1,0);
        BCASE '_': mv(0,-99);
        BCASE 'a': mv(0, 1); insertMode();
        BCASE 'A': gotoEOL(); insertMode();
        BCASE 'J': joinLines();
        BCASE '$': gotoEOL();
        BCASE 'g': mv(-(SCR_LINES-1),-99);
        BCASE 'G': mv(SCR_LINES-1,-99);
        BCASE 'i': insertMode();
        BCASE 'I': mv(0, -99); insertMode();
        BCASE 'o': mv(1, -99); insertLine(); insertMode();
        BCASE 'O': mv(0, -99); insertLine(); insertMode();
        BCASE 'r': replaceChar(edKey(), 0, 1);
        BCASE 'R': replaceMode();
        BCASE 'c': edDelX('.'); insertMode();
        BCASE 'C': edDelX('$'); insertMode();
        BCASE 'd': edDelX(0);
        BCASE 'D': edDelX('$');
        BCASE 'x': edDelX('.');
        BCASE 'X': edDelX('X');
        BCASE 'L': edRdBlk(1);
        BCASE 'Y': strCpy(yanked, &EDCH(line, 0));
        BCASE 'p': mv(1,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
        BCASE 'P': mv(0,-99); insertLine(); strCpy(&EDCH(line,0), yanked);
        BCASE '+': edSvBlk(0); ++blkNum; edRdBlk(0); scrTop=line=off=0;
        BCASE '-': edSvBlk(0); blkNum = MAX(0, blkNum-1); edRdBlk(0); scrTop=line=off=0;
        BCASE ':': edCommand();
    }
    return 1;
}

void editBlock(CELL Blk) {
    blkNum = MAX((int)Blk, 0);
    line = off = scrTop = 0;
    msg = NULL;
    CLS();
    CursorOff();
    edRdBlk(0);
    normalMode();
    showAll();
    while (edMode != QUIT) {
        showEditor();
        showStatus();
        processEditorChar(edKey());
    }
    GotoXY(1, SCR_LINES+3);
    CursorOn();
}

#endif //  __EDITOR__
