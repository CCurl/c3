#ifndef __LITTLEFS_H__

#define __LITTLEFS_H__

void   fileInit() { }
cell_t fOpen(cell_t nm, cell_t md) { return 0; }
void   fClose(cell_t fp) { }
cell_t fRead(cell_t addr, cell_t sz, cell_t num, cell_t fp) { return 0; }
cell_t fWrite(cell_t addr, cell_t sz, cell_t num, cell_t fp) { return 0; }

#endif // __LITTLEFS_H__
