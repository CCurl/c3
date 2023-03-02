: here (here) @ ;
: c, here c! (here) ++ ;
: , here ! here cell + (here) ! ;

: last (last) @ ;
: immediate 1 last cell + c! ;
: inline 2 last cell + c! ;

: \ 0 >in @ ! ; immediate
: [ 0 state ! ; immediate
: ] 1 state ! ;
: bye 999 state ! ;

: vhere (vhere) @ ;
: const create (lit4) c, , (exit) c, ;
: var vhere const ;
: (var) here 1- cell - const ;
: does> r> last ! ;

: cells cell * ; inline
: allot vhere + (vhere) ! ;
: vc, vhere c! (vhere) ++ ;
: v,  vhere ! cell allot ;

: if  (jmpz) c, here 0 , ; immediate
: else (jmp) c, here swap 0 , here swap ! ; immediate
: then here swap ! ; immediate
: exit (exit) c,   ; immediate

: begin here         ; immediate
: while (jmpnz) c, , ; immediate
: until (jmpz)  c, , ; immediate
: again (jmp)   c, , ; immediate

: tuck swap over ; inline
: nip  swap drop ; inline
: 2dup over over ; inline
: ?dup dup if dup then ;

: +! tuck @ + swap ! ; inline
: c++ dup @ 1+ swap ! ;
: c-- dup @ 1- swap ! ;
: 2* dup + ; inline

: rdrop r> drop ; inline
: rot  >r swap r> swap ;
: -rot swap >r swap r> ;

: ( begin 
        >in @ c@ dup 0= if drop exit then
        >in ++ ')' = if exit then
    again ; immediate

: bl #32 ; inline
: space bl emit ; inline
: tab #9 emit ; inline
: cr #13 emit #10 emit ; inline

: negate com 1+ ; inline
: abs dup 0 < if negate then ;
: min over over > if swap then drop ;
: max over over < if swap then drop ;

: i (i) @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - (lsp) ! ;

: /   /mod nip  ; inline
: mod /mod drop ; inline

var (neg) cell allot
: #digit '0' + dup '9' > if 7 + then ;
: <# 0 swap dup 0 < (neg) ! abs ;    \ ( n1 -- 0 n2 )
: # base @ /mod swap #digit swap ;   \ ( u1 -- c u2 )
: #S begin # dup while ;             \ ( u1 -- u2 )
: #> drop (neg) @ if '-' then ;
: #P begin emit dup while drop ;     \ ( 0 ... n 0 -- )
: (.) <# #S #> #P ;
: . (.) space ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: count ( str--a n ) dup 1+ swap c@ ; inline
: type  ( a n-- ) ?dup if 0 do dup c@ emit 1+ loop then drop ;

var s (var) (s) : >s (s) ! ; : s++ s (s) ++ ;
var d (var) (d) : >d (d) ! ; : d++ d (d) ++ ;

: i" ( --str ) vhere dup >d 0 d++ c!
    begin >in @ c@ dup >s if >in ++ then
        s 0= s '"' = or
        if 0 d++ c! exit then
        s d++ c! vhere c++
    again ;

: s" ( --str ) i" state @ if (lit4) c, , d (vhere) ! then ; immediate

: ." ( -- ) i" state @ 0= if count type exit then
    (lit4) c, , d (vhere) !
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: .word dup cell + 1+ count type ;
: words last begin
        dup mem-end < if 
            .word tab word-sz +
        else drop exit
        then
    again ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $10 base ! ;
: ? @ . ;

: rshift ( n1 s--n2 ) 0 do 2 / loop ;
: lshift ( n1 s--n2 ) 0 do 2* loop ;

: fopen-rt s" rt" fopen ;
: fopen-wt s" wt" fopen ;
: ->stdout 0 (output_fp) ! ;

var (fg) 3 cells allot
: fg cells (fg) + ;
: marker here 0 fg ! vhere 1 fg ! last 2 fg ! ;
: forget 0 fg @ (here) ! 1 fg @ (vhere) ! 2 fg @ (last) ! ;
: forget-1 last @ (here) ! last word-sz + (last) ! ;
marker

." c3 - v0.0.1 - Chris Curl" cr
here mem -   . ." code bytes used, " last here - . ." bytes free." cr
vhere vars - . ." variable bytes used, " vars-end vhere - . ." bytes free."
