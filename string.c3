\ string.c3 - some string words

' COUNT loaded?

\ Just some temp addresses
: pad1 VHERE  256 + ;
: pad2 pad1   256 + ;

\ Words for NULL-Terminated strings
: S-CATN  ( str num -- )  ITOA S-CAT  ; INLINE
: S-SCATN ( num str -- )  SWAP S-CATN ; INLINE
: S-SCPY  ( src dst -- )  SWAP S-CPY  ; INLINE
: S-SCAT  ( src dst -- )  SWAP S-CAT  ; INLINE

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
: sc-len ( str -- len ) C@ ; INLINE
: COUNT ( sc -- a n ) 1+ DUP 1- sc-len ; INLINE

: sz->sc ( sz sc -- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz -- ) SWAP 1+ S-CPY ; INLINE
