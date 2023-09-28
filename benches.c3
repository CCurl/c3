\ Some benchmarks

: ms ." (%d usec)" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime? begin ( n 3--f )
        over over /mod swap if
            over < if drop exit then
        else 
            drop = exit
        then
        1+ 1+
    again ;

variable num
: num-primes 4 num ! 11 do
        i 3 prime? if num ++ then 1 +i
    loop num ? ;

: T0 dup ." %d iterations ..." ;
: bm1 cr ." Bench 1: decrement loop, " T0
    timer swap begin 1- -while drop elapsed ;
: bm2 cr ." Bench 2: register decrement loop, " T0
    s1 timer begin r1- while repeat elapsed ;
: bm3 cr ." Bench 3: empty do/loop, " T0
    timer swap 0 do loop elapsed ;
: bm4 cr ." Bench 4: empty for/next, " T0
    timer swap for next elapsed ;
: bm5 cr ." Bench 5: number of primes in " dup . ." ... "
    timer swap num-primes elapsed ;

\ load-abort

250 mil bm1
250 mil bm2
250 mil bm3
250 mil bm4
  2 mil bm5
