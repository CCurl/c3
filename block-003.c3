\ block-003.c3 - some string words

' COUNT loaded?

\ Just some temp addresses
: pad1 VHERE  256 + ;
: pad2 pad1   256 + ;
: pad3 pad2   256 + ;

\ Words for NULL-Terminated strings
: S-CATN  ( str num -- )  ITOA S-CAT  ; INLINE
: S-SCATN ( num str -- )  SWAP S-CATN ; INLINE
: S-SCPY  ( src dst -- )  SWAP S-CPY  ; INLINE
: S-SCAT  ( src dst -- )  SWAP S-CAT  ; INLINE
: S-END   ( str -- end )  DUP S-LEN + ; INLINE

\ S-CATLEN ( dst src len -- ) concat <len> chars from src to dst
: S-CATLEN +regs s3 s2 S-END s1
    r3 0 DO r2 C@ ?DUP IF r1+ C! i2 THEN LOOP
    0 r1 C!  -regs ;

\ S-CPYLEN ( dst src len -- ) copy <len> chars from src to dst
: S-CPYLEN >R >R 0 OVER C! R> R> S-CATLEN ;

\ S-FIND ( LF STR -- ADDR|0 ) looks for string LF in string at STR
\   ADDR: 0 if not found, else address in STR
: S-FIND +regs s2 s1
    r1 C@ s6  r1 S-LEN s3
    BEGIN
        r6 r2 S-FINDC DUP s2  0= IF 0 -EXIT THEN
        r1 r2 r3 S-EQN  IF r2 -EXIT THEN
        i2
     AGAIN ;

\ Words for Counted strings
: SC-LEN ( str -- len ) C@ ; INLINE
: COUNT  ( sc -- a n ) 1+ DUP 1- SC-LEN ; INLINE

: sz->sc ( sz sc -- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz -- ) SWAP 1+ S-CPY ; INLINE
