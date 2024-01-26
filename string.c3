\ string.c3 - some string words

' count loaded?

: pad1 VHERE  512 + ;
: pad2 VHERE 1024 + ;

\ Words for NULL-Terminated strings
: S-CATN  ( str num-- )  ITOA S-CAT  ; INLINE
: S-SCATN ( num str-- )  SWAP S-CATN ; INLINE
: S-SCPY  ( src dst -- ) SWAP S-CPY  ; INLINE
: S-SCAT  ( src dst -- ) SWAP S-CAT  ; INLINE

\ Words for Counted strings
: sc-len ( str--len ) C@ ; INLINE

: sz->sc ( sz sc-- ) +REGS s2 s1
    r1 S-LEN r2+ C!
    r2 r1 S-CPY
    -REGS ;

: sc->sz ( sc sz-- ) SWAP 1+ S-CPY ; INLINE

: count ( sc--a n ) 1+ DUP 1- C@ ; INLINE
