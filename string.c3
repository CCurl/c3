\ string.c3 - some string handling words

' s-len loaded?

load memory.c3

: s-len ( str--len ) c@ ; inline
: s-end ( str--end ) dup s-len + 1+ ; inline
: s-cpy ( dst src-- ) tuck s-len 2+ cmove ;
: s-trunc ( str-- ) 0 swap 2dup 1+ c! c! ;
: s-cat ( dst src-- )
    +regs s2 s1
    r2 1+ r1 s-end r2 s-len 1+ cmove
    r1 s-len r2 s-len + r1 c!
    -regs ;
: s-catc ( str ch-- )
    +regs s2 s1 r1 s-end s3
    r2 r3 c! 0 r3 1+ c!
    r1 c++ -regs ;
: s-catd ( str d-- )
    dup 9 > if 7 + then
    s-catc ;
: s-catn ( dst n-- ) 
    swap >r <# #s #>
    begin r@ swap s-catc ?dup while
    rdrop ;
: s-scatc ( c dst-- )   swap s-catc ;
: s-scatn ( n dst-- )   swap s-catn ;
: s-findc ( sz ch--a|0 )
    +regs s2 s1 0 s8
    r1 s-len 0 do
        r1 c@ s3 r3 r2 = r3 0= or
        if r1 s8 999 +i then i1
    loop r8 -regs ;
: lower ( c1--c2 )
    >r r@ 'A' >= r@ 'Z' <= and if r> 32 + >r then r> ;
: upper ( c1--c2 )
    >r r@ 'a' >= r@ 'z' <= and if r> 32 - >r then r> ;
: s-eq ( s1 s2--f )
    over c@ over c@ <> if 2drop 0 exit then
    +regs s2 s1 -1 s8
    r1 c@ 0 do
        r1 c@ r2 c@ <> if 0 s8 999 (i) ! then i1 i2
    loop r8 -regs ;
: s-eq-i ( s1 s2--f )
    over c@ over c@ <> if 2drop 0 exit then
    +regs s2 s1 -1 s8
    r1 c@ 0 do 
        r1 c@ lower r2 c@ lower <> 
        if 0 s8 999 (i) ! then i1 i2 
    loop r8 -regs ;
: s-from-sz ( s sz-- )
    +regs s2 s1   r1 s-trunc
    begin r2 c@ dup if r1 s-scatc i2 1 then while
    -regs ;
