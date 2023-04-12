// Support for Windows
#include <conio.h>
#define isPC
int qKey() { return _kbhit(); }
int key() { return _getch(); }
