\ block-002.c3 - File handling words

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
    BEGIN
        0 r1 C!  r2 fgetc s5 s3
        r5 0=    IF  r4 0= -REGS EXIT  THEN
        r3 10 =  IF  0     -REGS EXIT  THEN
        r3 13 <> IF  r3 r1+ C!  i4  THEN
    AGAIN ;

\ Words to redirect output
: output-fp ( --H )  (output_fp) @ ;
: ->file    ( H-- )  (output_fp) ! ;
: ->stdout  ( -- )   0 ->file ;

VARIABLE T0
: redirect    ( H-- )  output-fp T0 ! ->file ;  \ Save and redirect
: restore     ( -- )   T0 @ ->file ;            \ Restore output_fp

: fputc ( ch H-- )  redirect  EMIT   restore ;
: fputs ( sz H-- )  redirect  QTYPE  restore ;
