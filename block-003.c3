\ block-003.c3 - some string words

' COUNT loaded?

\ Some temp buffers
: pad1 VHERE  256 + ;
: pad2 pad1   256 + ;
: pad3 pad2   256 + ;

\ Words for NULL-Terminated strings
: S-CATN  ( str num -- )  ITOA S-CAT   ; INLINE
: S-SCATN ( num str -- )  SWAP S-CATN  ; INLINE
: S-SCPY  ( src dst -- )  SWAP S-CPY   ; INLINE
: S-SCAT  ( src dst -- )  SWAP S-CAT   ; INLINE
: S-SCATC ( ch dst -- )   SWAP S-CATC  ; INLINE
: S-END   ( str -- end )  DUP  S-LEN + ; INLINE

: FILL ( DST NUM CH -- ) +REGS s3 s2 s1
    r2 0 DO  r3 r1+ C!  LOOP
    -REGS ;

: CMOVE ( SRC DST NUM -- ) +REGS  s3 s2 s1
    r3 0 DO  r1+ C@ r2+ C!  LOOP
    -REGS ;

: S-CPYLEN ( DST SRC LEN -- ) \ Copy LEN bytes from SRC to DST
    >R SWAP R> CMOVE ;

: S-CATLEN ( DST SRC LEN -- ) \ Concat LEN bytes from SRC to DST
    >R >R  S-END  R> R>  S-CPYLEN ;

: S-FIND ( LF STR -- ADDR|0 ) \ Look for string LF in STR
    \ Returns 0 if not found, else the address in STR
    +regs s2 s1
    r1 C@ s6  r1 S-LEN s3
    BEGIN
        r6 r2 S-FINDC DUP s2  0= IF 0 -EXIT THEN
        r1 r2 r3 S-EQN  IF r2 -EXIT THEN
        i2
     AGAIN ;

\ Words for Counted strings
: sc-len  ( str -- len )  C@ ; INLINE
: COUNT   ( sc -- a n )   1+ DUP 1- sc-len ; INLINE
: sc->sz  ( sc sz -- )    SWAP 1+ S-CPY ; INLINE
: sz->sc  ( sz sc -- )    +REGS s2 s1
    r1 S-LEN r2+ C!    r2 r1 S-CPY
    -REGS ;
