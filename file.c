#include "c3.h"

#ifdef _LITTLEFS_
#include <Arduino.h>
#include <LittleFS.h>

void   fileInit() { }
cell_t fOpen(cell_t nm, cell_t md) { return 0; }
void   fClose(cell_t fp) { }
cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) { return 0; }
cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) { return 0; }

#elif defined (isPC)

void   fileInit() { }
cell_t fOpen(cell_t nm, cell_t md) { return (cell_t)fopen((char*)nm, (char*)md); }
void   fClose(cell_t fp) { fclose((FILE*)fp); }
cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) {
  return (cell_t)fread((char*)addr, sz, num, (FILE*)fp);
}
cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) {
  return (cell_t)fwrite((char*)addr, sz, num, (FILE*)fp);
}

#else

static void nf() { printString("-noFile-"); }
void   fileInit() { }
cell_t fOpen(cell_t nm, cell_t md) { nf(); return 0; }
void   fClose(cell_t fp) { nf(); }
cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) { nf(); return 0; }
cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) { nf(); return 0; }

#endif
