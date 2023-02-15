\ some tests
: ms (.) ."  usec " ;
: elapsed timer swap - ms ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap 0 do loop elapsed ;
: mil #1000 dup * * ;
100 mil const sz

: prime? begin ( n 3--f )
        over over /mod swap if  \ test 2
            over < if drop exit then
        else 
            drop = exit
        then
        1+ 1+
    again ;

var num cell allot
: num-primes cr dup (.) ." : num primes ... "
    4 num ! 11 do
        i 3 prime? if num ++ then 1 +i
    loop num ? ;

\  prime? if i . num ++ then 1 +i 

: bm3 timer swap num-primes elapsed ;

sz bm1 sz bm2
cr 1 mil bm3
\ bye
