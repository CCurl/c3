// Support for files, either PC or None

#include "c3.h"

// BEGIN: These are shared for all file system versions
CELL_T inputStk[ISTK_SZ+1], input_sp, input_fp;

void ipush(CELL_T x) { if (input_sp < ISTK_SZ) inputStk[++input_sp] = x; }
CELL_T ipop() { return (0 < input_sp) ? inputStk[input_sp--] : 0; }

static char block_fn[16];
char* getBlockFN(int num) { sprintf(block_fn, "block-%03d.c3", num); return block_fn; }
// END: These are shared for all file system versions

#ifdef isPC

    void   fileInit() { input_fp = input_sp = 0; }
    CELL_T fOpen(const char *nm, const char *md) { return (CELL_T)fopen(nm, md); }
    void   fClose(CELL_T fp) { fclose((FILE*)fp); }
    
    int fRead(char *buf, int sz, int num, CELL_T fp) {
        return (int)fread(buf, sz, num, (FILE*)fp);
    }

    int fWrite(char *buf, int sz, int count, CELL_T fp) {
        return (int)fwrite(buf, sz, count, (FILE*)fp);
    }

    int writeBlock(int blk, char *data, int sz) {
        int nw = 0;
        CELL_T fh = fOpen(getBlockFN(blk), "wb");
        if (fh) {
            nw = fWrite(data, 1, sz, fh);
            fClose(fh);
        }
        return nw;
    }

    int readBlock(int blk, char *data, int sz) {
        int nr = 0;
        CELL_T fh = fOpen(getBlockFN(blk), "rb");
        if (fh) {
            nr = fRead(data, 1, sz, fh);
            fClose(fh);
        }
        return nr;
    }

    int fGets(CELL_T fh, char *buf, int sz) {
        if (fgets(buf, sz, (FILE*)fh) == buf) { return strLen(buf); }
        return -1;
    }

#elif defined (_NoFS_)

  void   fileInit() { }
  static void nf() { printString("-noFile-"); }
  CELL_T fOpen(const char *nm, const char *md) { nf(); return 0; }
  void   fClose(CELL_T fp) { nf(); }
  int fRead(char *buf, int sz, int num, CELL_T fp) { nf(); return 0; }
  int fWrite(char *buf, int sz, int num, CELL_T fp) { nf(); return 0; }
  void blockLoad(int num) { nf(); }
  int readBlock(int blk, char *buf, int sz) { nf(); return 0; }
  int writeBlock(int blk, char *buf, int sz) { nf(); return 0; }

#endif
