// Support for LittleFS on the Pico

#include "c3.h"

#ifdef _PicoFS_

#include <LittleFS.h>

#define myFS LittleFS

#define NFILES 20
File files[NFILES+1];

void fileInit() {
	printString("\r\nLittleFS: begin ...");
	myFS.begin();
	printString("done.");
    FSInfo64 fsinfo;
    LittleFS.info64(fsinfo);
	printString("\r\nLittleFS: initialized");
	//printStringF("\r\nBytes total: %llu, used: %llu", fsinfo.totalBytes, fsinfo.usedBytes);
	input_fp = input_sp = 0;
	for (int i=0;i<=NFILES;i++) { files[i] = File(); }
}

int freeFile() {
	for (int i=1;i<=NFILES;i++) {
		if ((bool)files[i] == false) { return i; }
	}
	return 0;
}

CELL_T fOpen(const char *fn, const char *mode) {
	int fh = freeFile();
	if (0 < fh) {
		files[fh] = myFS.open((char*)fn, mode);
        if (files[fh].name() == nullptr) { return 0; }
        if (mode[0] == 'w') { files[fh].truncate(0); }
	}
	return fh;
}

// fO (fn md -- fh) - FileOpen
void fOpenF() {
	char *md = (char *)pop();
	char *fn = (char *)pop();
	push(fOpen(fn, md));
}

// fC (fh--) - File Close
void fClose(CELL_T fh) {
	if (BTW(fh, 1, NFILES) &&((bool)files[fh])) {
		files[fh].close();
	}
}

// fD (nm--) - File Delete
void fDelete() {
	char *nm = (char *)pop();
    myFS.remove(nm);
}

// fR (fh--c n) - File Read
// fh: File handle, c: char read, n: num chars read
// n=0: End of file or file error
int fRead(char *buf, int num, int sz, CELL_T fh) {
	return (BTW(fh, 1, NFILES)) ? (int)files[fh].read((uint8_t*)buf, num*sz) : 0;
}

// fW (c fh--n) - File Write
// fh: File handle, c: char to write, n: num chars written
// n=0: File not open or error
int fWrite(char *buf, int num, int sz, CELL_T fh) {
	return (BTW(fh, 1, NFILES)) ? (int)files[fh].write(buf, num*sz) : 0;
}

int readBlock(int blk, char *buf, int sz) {
	char fn[32];
	int num = 0;
	sprintf(fn, getBlockFN(), blk);
	File x = myFS.open(fn, "r");
	if ((bool)x) {
		num = (int)x.read((uint8_t*)buf, sz);
		x.close();
	}
	return num;
}

// bR (blk buf sz--f)
void readBlock1() {
    CELL_T sz = pop();
    char *buf = (char*)pop();
    CELL_T blk = pop();
    push(readBlock(blk, (char*)buf, sz));
}

int writeBlock(int blk, char *buf, int sz) {
	char fn[32];
	int num = 0;
	sprintf(fn, getBlockFN(), blk);
	File x = myFS.open(fn, "w");
	if ((bool)x) {
        x.truncate(0);
        num = x.write((uint8_t*)buf, sz);
		x.close();
	}
	return num;
}

// bW (blk buf sz--f)
void writeBlock1() {
    CELL_T sz = pop();
    char *buf = (char*)pop();
    CELL_T blk = pop();
    push(writeBlock(blk, buf, sz));
}

// fL - File readLine
int fReadLine(CELL_T fh, char *buf, int sz) {
	int n = -1;
	buf[0] = 0;
	if (BTW(fh, 1, NFILES) && (0 < files[fh].available())) {
		n = files[fh].readBytesUntil(10, buf, sz);
		buf[n] = 0;
	}
	return n;
}

// bL - Block Load
void blockLoad(int blk) {
	char fn[32];
	sprintf(fn, getBlockFN(), blk);
	CELL_T fh = fOpen(fn, "r");
	if (files[fh].available()) {
		if (input_fp) {
			fpush(input_fp);
		}
		input_fp = fh;
	}
}

// bA - Block load Abort
void loadAbort() {}

#endif
