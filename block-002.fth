\ block-002 - File handling words

' LEX-FILE loaded?

LEX-C3 : LEX-FILE 2 LEXICON ;
LEX-FILE

: fopen-rt  ( fn--fh ) " rt"  FOPEN ;
: fopen-rb  ( fn--fh ) " rb"  FOPEN ;
: fopen-wt  ( fn--fh ) " wt"  FOPEN ;
: fopen-wb  ( fn--fh ) " wb"  FOPEN ;
: fopen-rw  ( fn--fh ) " r+b" FOPEN ;

VHERE CONSTANT T0
1 ALLOT
: fgetc ( fh--ch num-read )
    0 T0 C! >R T0 1 R> FREAD T0 C@ SWAP ;

\ Words to redirect output
: output-fp   ( --H )  (output_fp) @ ;
: >output-fp  ( H-- )  (output_fp) ! ;
: ->stdout    ( -- )   0 >output-fp ;

VARIABLE T0
: redirect  ( H-- )  output-fp T0 ! >output-fp ;  \ Save and redirect
: restore   ( -- )   T0 @ >output-fp ;            \ Restore output_fp

: fputc  ( ch H-- )  redirect  EMIT   restore ;
: fputs  ( sz H-- )  redirect  QTYPE  restore ;
