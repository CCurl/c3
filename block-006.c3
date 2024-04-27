\ block 006 - utility words

3 load  \ String words
5 load  \ Screen words

C3 ' UTILS loaded?

marker

: UTILS 6 LEXICON ;
UTILS

: p pad1 ;
: ls   " ls -l"   system ;
: pwd  " pwd"     system ;
: lg   " lazygit" system ;
: rl   forget 6 load ;
: ed   35 (scr-h) ! 6 edit ;

." words added: %n" words
