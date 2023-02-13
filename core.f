: \ 0 >in @ ! ;

include test.f

: last (last) @ ;
: here (here) @ ;
: vhere (vhere) @ ;

: inline 2 last c! ;
: immediate 1 last c! ;
: [ 0 state ! ; immediate
: ] 1 state ! ;
: bye 999 state ! ;
: cells cell * ; inline

: c, here c! here 1+     (here) ! ;
: ,  here !  here cell + (here) ! ;

: const create (lit4) c, , (exit) c, ;
: var vhere const  ;
: allot vhere + (vhere) ! ;
: vc, vhere c! 1 allot ;
: v,  vhere ! cell allot ;

: if (jmpz) c, here 0 , ; immediate
: then here swap !      ; immediate
: exit (exit) c,        ; immediate

: tuck swap over ; inline
: nip  swap drop ; inline
: ?dup dup if dup then ;

: begin here         ; immediate
: while (jmpnz) c, , ; immediate
: until (jmpz)  c, , ; immediate
: again (jmp)   c, , ; immediate

: ( begin 
        >in @ c@ >in @ 1+ >in !
        dup  0= if drop exit then
        ')' = if drop exit then
    again ; immediate

: and [ (bitop) c, 11 c, ] ; inline
: or  [ (bitop) c, 12 c, ] ; inline
: xor [ (bitop) c, 13 c, ] ; inline
: com [ (bitop) c, 14 c, ] ; inline

: >r [ (retop) c, 11 c, ] ; inline
: r@ [ (retop) c, 12 c, ] ; inline
: r> [ (retop) c, 13 c, ] ; inline
: rdrop r> drop ; inline
: rot  >r swap r> swap ;
: -rot swap >r swap r> ;

: bl 32 ; inline
: space bl emit ; inline
: tab 9 emit ; inline
: cr 13 emit 10 emit ; inline

: negate com 1+ ; inline
: abs dup 0 < if negate then ;
: +!  dup @   + swap !  ; inline
: ++  dup @  1+ swap !  ; inline
: c++ dup c@ 1+ swap c! ; inline

: i (i) @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - (lsp) ! ;

: /   /mod nip  ; inline
: mod /mod swap ; inline

var (neg) cell allot
var (len) cell allot
: len (len) @ ;
: #digit '0' + dup '9' > if 7 + then ;
: <# 0 (neg) c! 0 (len) ! dup 0 < 
    if negate 1 (neg) ! then 0 swap ;         \ ( n1 -- 0 n2 )
: # base @ /mod swap #digit swap (len) ++ ;   \ ( u1 -- c u2 )
: #S begin # dup while ;                      \ ( u1 -- u2 )
: #> ;
: #- drop (neg) @ if '-' emit then ;
: #P #- begin emit dup while drop ;           \ ( 0 ... n 0 -- )
: (.) <# #S #> #P ;
: . (.) space ;

: count dup 1+ swap c@ ; inline
: type 0 do dup c@ emit 1+ loop drop ;

: S" (lit4) c, vhere ,
    vhere >r 0 vc,
    begin >in @ c@ >in ++
        dup 0= over '"' = or
        if drop 0 vc, rdrop exit then
        vc, r@ c++
    again ; immediate

: ." [ (call) c, ' S" drop drop , ]
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: words last begin
        dup mem-end < 0= if drop exit then
        dup 1+ count type tab word-sz +
    again ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $16 base ! ;
: ? @ . ;

\ temp for testing
: ms (.) ."  usec " ;
: elapsed timer swap - ms ;
: bm1 timer swap begin 1- dup while drop elapsed ;
: bm2 timer swap 0 do loop elapsed ;
: mil #1000 dup * * ;
500 mil const sz

sz bm1 sz bm2
: back ." -back" ; back
\ bye
