\ File handling words

' fopen-rt loaded?

: fopen-rt ( fn--fh ) " rt" fopen ;
: fopen-rb ( fn--fh ) " rb" fopen ;
: fopen-wt ( fn--fh ) " wt" fopen ;
: fopen-wb ( fn--fh ) " wb" fopen ;
: fopen-rw ( fn--fh ) " r+b" fopen ;
: ->stdout 0 (output_fp) ! ;

variable T0
: fgetc ( fh--c f )
    >r T0 1 r> fread T0 c@ swap ;

: fgets ( a fh--len eof )
    +regs   s2 s1   0 s4
    begin 
        r2 fgetc   swap s3
        0= if 0 r1 c!   r4 1 -regs exit then
        r3 10 = if  0 r1 c!  r4 0 -regs exit then
        r3 13 = if  0 s3 then
        r3  9 = if 32 s3 then
        r3 if r3 r1 c!   i1 i4 then
    again ;

: fputc ( c fh-- )
    (output_fp) @ >R (output_fp) ! EMIT R> (output_fp) ! ;

: fputs ( sz fh-- )
    (output_fp) @ >R (output_fp) ! ZTYPE R> (output_fp) ! ;
