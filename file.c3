\ File handling words

' fopen-rt loaded?

: fopen-rt ( fn--fh ) " rt"  FOPEN ;
: fopen-rb ( fn--fh ) " rb"  FOPEN ;
: fopen-wt ( fn--fh ) " wt"  FOPEN ;
: fopen-wb ( fn--fh ) " wb"  FOPEN ;
: fopen-rw ( fn--fh ) " r+b" FOPEN ;

VARIABLE T0
: fgetc ( fh--ch num-read )
    0 T0 C! >R T0 1 R> FREAD T0 C@ SWAP ;

: fgets ( a fh--eof )
    +REGS  s2 s1  0 s4
    r1 S-TRUNC
    BEGIN
        r2 fgetc s5 s3
        r5 0=    IF  r4 0= -REGS EXIT  THEN
        r3 10 =  IF  0     -REGS EXIT  THEN
        r3 13 <> IF  r1 r3 S-CATC  i4  THEN
    AGAIN ;

\ Words to redirect output
: output-fp ( --H )  (output_fp) @ ;
: ->file    ( H-- )  (output_fp) ! ;
: ->stdout  ( -- )   0 ->file ;

VARIABLE T0
: T1 ( H-- )  output-fp T0 ! ->file ;  \ Save and redirect
: T2 ( -- )   T0 @ ->file ;            \ Restore output_fp

: fputc ( ch H-- )  T1  EMIT   T2 ;
: fputs ( sz H-- )  T1  QTYPE  T2 ;

: T0 vhere #256 + ;
: cat +REGS
    NEXT-WORD DROP fopen-rt s1
    BEGIN
        r1 0= IF -REGS EXIT THEN
        T0 r1 fgets IF r1 FCLOSE 0 s1 ELSE T0 QTYPE CR THEN
    AGAIN ;
