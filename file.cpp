// NOTE: this is a .h file because the PC version is straight C, while the Arduino version is CPP
// We need to suport both at the same time.

#include "c3.h"

#ifdef isPC

#ifdef __cplusplus
extern "C" {
#endif

  void   fileInit() { }
  cell_t fOpen(cell_t nm, cell_t md) { return (cell_t)fopen((char*)nm, (char*)md); }
  void   fClose(cell_t fp) { fclose((FILE*)fp); }
  cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) {
      return (cell_t)fread((char*)addr, sz, num, (FILE*)fp);
  }
  cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) {
      return (cell_t)fwrite((char*)addr, sz, num, (FILE*)fp);
  }

#ifdef __cplusplus
}
#endif

#elif defined (_LITTLEFS_)

  #include <LittleFS.h>

  LittleFS_Program myFS;
  void fileInit() {
      myFS.begin(1 * 1024 * 1024);
      printString("\r\nLittleFS: initialized");
      printStringF("\r\nBytes Used: %llu, Bytes Total:%llu", myFS.usedSize(), myFS.totalSize());
  }
  cell_t fOpen(cell_t nm, cell_t md) { y=(char*)md; File x=myFS.open((char*)nm,*y=='w'?FILE_WRITE:FILE_READ); return (cell_t)x; }
  void   fClose(cell_t fp) { File x=File((FileImpl*)fp); x.close(); }
  cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) { File x=File((FileImpl*)fp); return x.read((char*)addr,(sz*num)); }
  cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) { File x=File((FileImpl*)fp); return x.write((char*)addr,(sz*num)); }

#else

  static void nf() { printString("-noFile-"); }
  void   fileInit() { }
  cell_t fOpen(cell_t nm, cell_t md) { nf(); return 0; }
  void   fClose(cell_t fp) { nf(); }
  cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) { nf(); return 0; }
  cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) { nf(); return 0; }

#endif
