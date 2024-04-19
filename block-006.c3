\ block 006 - utility words

' lg loaded?

marker

3 5 thru

: ls   " ls -l"   system ;
: pwd  " pwd"     system ;
: lg   " lazygit" system ;
: rl   forget 6 load ;
: ed   35 (scr-h) ! 6 edit ;
: ?cr r1+ #9 > IF CR 0 s1 ELSE TAB THEN ;
: words-n +REGS last s2 0 s1 0 DO
        r2 .word ?cr r2 word-sz + s2
    LOOP -REGS ;
." words added: %n" 10 words-n
