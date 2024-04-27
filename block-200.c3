\ block-200.c3 - Some benchmarks

C3 ' BENCHES loaded?

: BENCHES 200 LEXICON ;
BENCHES

: ms ." (%d usec)" ;
: elapsed timer swap - ms ;
: mil #1000 dup * * ;

: prime? ( n--f )
    3 BEGIN
        2DUP /MOD SWAP IF
            OVER < IF DROP EXIT THEN
        ELSE 
            DROP = EXIT
        THEN
        2+
    AGAIN ;

: num-primes 4 s9 11 DO I prime? IF i9 THEN 2 +LOOP r9 ;

: fib ( n--fib ) DUP 2 < IF DROP 1 EXIT THEN 1- DUP fib SWAP 1- fib + ;

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
: bm6 cr DUP ." Bench 6: number of primes in %d ... "
    timer swap num-primes . elapsed ;
: bm7 cr DUP ." Bench 7: Fibonacci number for: %d ... "
    timer swap fib . elapsed ;

: run-all
    250 mil bm1
    250 mil bm2
    250 mil bm3
    250 mil bm4
    250 mil bm5
    1 mil bm6
    35     bm7 ;

run-all
