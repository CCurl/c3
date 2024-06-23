\ block 006 - utility words

2 load  \ File words
3 load  \ String words
5 load  \ Screen words

' LEX-UTIL loaded?

LEX-C3 : LEX-UTIL 6 LEXICON ;
LEX-UTIL

: p pad1 ;
: ls   " ls -l"   system ;
: pwd  " pwd"     system ;
: lg   " lazygit" system ;
: rl   forget 6 load ;
: ed   35 (scr-h) ! 6 edit ;

: vt 10 0 do key . loop ;

." words added: %n" words
