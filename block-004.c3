\ block-004.c3 - memory words

' cmove> loaded?

: cmove> ( src dst num-- )
    +regs s3 r3 + s2  r3 + s1
    r3 0 do r1- c@ r2- c! loop
    -regs ;
