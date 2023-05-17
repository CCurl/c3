' cmove loaded?

: cmove ( src dst num-- )
    +regs s3 s2 s1
    r3 if 
        r3 0 do r1+ c@ r2+ c! loop
    then
    -regs ;

: cmove> ( src dst num-- )
    +regs s3 r3 + s2  r3 + s1
    r3 if
        r3 0 do r1- c@ r2- c! loop
    then
    -regs ;

: fill ( dst num ch-- )
    +regs s3 s2 s1
    r2 0 do r3 r1+ c! loop
    -regs ;
