\ File handling words

' fopen-rt loaded?

: fopen-rt ( fn--fh ) " rt"  FOPEN ;
: fopen-rb ( fn--fh ) " rb"  FOPEN ;
: fopen-wt ( fn--fh ) " wt"  FOPEN ;
: fopen-wb ( fn--fh ) " wb"  FOPEN ;
: fopen-rw ( fn--fh ) " r+b" FOPEN ;

variable T0
: fgetc ( fh--ch num-read )
    0 T0 C! T0 1 FREAD T0 C@ SWAP ;

: fgets ( a fh--eof )
    +REGS  s2 s1  0 s4
    r1 S-TRUNC
    BEGIN
        r2 fgetc s5 s3
        r5 0=    IF  r4 0= -REGS EXIT  THEN
        r3 10 =  IF  0     -REGS EXIT  THEN
        r3 13 <> IF  r1 r3 S-CATC  i4  THEN
    AGAIN ;

: T1 ( fh-- ) (output_fp) DUP @ T0 ! ! ;  \ Save and set output_fp
: T2 ( -- )   T0 @ (output_fp) ! ;        \ Restore output_fp

: fputc ( c  fh-- ) T1  EMIT   T2 ;
: fputs ( sz fh-- ) T1  ZTYPE  T2 ;

: T0 vhere #256 + ;
: cat +REGS
    NEXT-WORD DROP fopen-rt s1
    BEGIN
        r1 0= IF -REGS EXIT THEN
        T0 r1 fgets IF r1 FCLOSE 0 s1 ELSE T0 QTYPE CR THEN
    AGAIN ;
