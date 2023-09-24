\ string.c3 - some string words

' type loaded?

\ Words for NULL-Terminated strings
: s-catn ( str n-- ) ITOA S-CAT ;

\ Words for Counted strings
: sz->sc ( sz sc-- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz-- ) SWAP 1+ S-CPY ; INLINE

: count ( sc--a n ) 1+ DUP 1- C@ ;
: type ( a n-- ) ?DUP IF
        0 DO DUP C@ EMIT 1+ LOOP
    THEN DROP ;
