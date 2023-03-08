\ Some benchmarks

: ms . ." usec" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime? begin ( n 3--f )
        over over /mod swap if  \ test 2
            over < if drop exit then
        else 
            drop = exit
        then
        1+ 1+
    again ;

var num cell allot
: num-primes 4 num ! 11 do
        i 3 prime? if num ++ then 1 +i
    loop num ? ;

: bm1 cr ." Bench: decrement loop, " dup . ." iterations ... "
    timer swap begin 1- dup while drop elapsed ;
: bm2 cr ." Bench: empty do loop, " dup . ." iterations ... "
    timer swap 0 do loop elapsed ;
: bm3 cr ." Bench: number of primes in " dup . ." ... "
    timer swap num-primes elapsed ;

99 state ! \ Abort/stop the load
250 mil bm1
250 mil bm2
2 mil bm3
