\ Some benchmarks

: ms ." (%d usec)" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime? BEGIN ( n 3--f )
        OVER OVER /MOD SWAP IF
            OVER < IF DROP EXIT THEN
        ELSE 
            DROP = EXIT
        THEN
        1+ 1+
    AGAIN ;

variable num
: num-primes 4 s9  11 DO
        i 3 prime? IF i9 THEN
    2 +LOOP r9 ;

: T0 dup ." %d iterations ..." ;
: bm1 cr ." Bench 1: decrement loop, " T0
    timer swap begin 1- -while drop elapsed ;
: bm2 cr ." Bench 2: register decrement loop, " T0
    s1 timer begin r1- while repeat elapsed ;
: bm3 cr ." Bench 3: empty do/loop, " T0
    timer swap 0 do loop elapsed ;
: bm4 cr ." Bench 4: empty for/next, " T0
    timer swap for next elapsed ;
: T1 1- DUP IF T1 EXIT THEN DROP ;
: bm5 cr ." Bench 5: empty tail-call, " T0
    timer swap T1 elapsed ;
: bm6 cr ." Bench 6: number of primes in " dup . ." ... "
    timer swap num-primes . elapsed ;

\ load-abort

250 mil bm1
250 mil bm2
250 mil bm3
250 mil bm4
250 mil bm5
  1 mil bm6
