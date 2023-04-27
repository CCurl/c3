: here (here) @ ;
: c, here c! 1 (here) +! ;
: , here ! here cell + (here) ! ;

: vhere  (vhere) @ ;
: allot  vhere + (vhere) ! ;
: vc, vhere c! 1 (vhere) +! ;
: v,  vhere ! cell allot ;

: last (last) @ ;
: immediate 1 last cell + c! ;
: inline 2 last cell + c! ;

: \ 0 >in @ ! ; immediate
: [ 0 state ! ; immediate
: ] 1 state ! ;
: -; (jmp) here cell - 1- c! 0 state ! ; immediate
: bye 999 state ! ;
: cells cell * ; inline

: constant  create (lit4) c, , (exit) c, ;
: variable  vhere constant cell allot ;
: val  vhere constant ;
: (val)  here 1- cell - constant ;

: does>  r> last ! ;
: :noname  here 1 state ! ;
: exec  >r ;

: if    (jmpz) c, here 0 , ; immediate
: else  (jmp) c, here swap 0 , here swap ! ; immediate
: then  here swap ! ; immediate
: exit  (exit) c,   ; immediate

: begin  here         ; immediate
: until  (jmpz)  c, , ; immediate
: again  (jmp)   c, , ; immediate
: while  (jmpz)  c, here 0 , ; immediate
: repeat swap (jmp) c, ,
    here swap ! ; immediate

: tuck  swap over ; inline
: nip   swap drop ; inline
: 2dup  over over ; inline
: 2drop drop drop ; inline
: ?dup  dup if dup then ;

: ++  dup @  1+ swap ! ; inline
: --  dup @  1- swap ! ; inline
: c++ dup c@ 1+ swap c! ; inline
: 2*  dup + ; inline
: 2+  1+ 1+ ; inline
: <=  > 0= ; inline
: >=  < 0= ; inline
: <>  = 0= ; inline

: rdrop r> drop ; inline
: rot   >r swap r> swap ;
: -rot  swap >r swap r> ;

: ( begin 
        >in @ c@ dup 0= if drop exit then
        >in ++ ')' = if exit then
    again ; immediate

: bl  #32 ; inline
: tab  #9 emit ; inline
: cr  #13 emit #10 emit ; inline
: space bl emit ; inline

: negate  com 1+ ; inline
: abs  dup 0 < if negate then ;
: min  over over > if swap then drop ;
: max  over over < if swap then drop ;

: i  (i) @ ;
: j  (i) 3 cells - @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - 0 max (lsp) ! ;

: /   /mod nip  ; inline
: mod /mod drop ; inline

: T0 emit cr ;
variable (neg)
variable #buf 32 allot
variable #bufp
: hold #bufp -- #bufp @ c! ;          \ ( c -- )
: #digit '0' + dup '9' > if 7 + then ;
: >neg dup 0 < (neg) ! abs ;          \ ( n1 -- u1 )
: <# #buf 35 + #bufp ! 0 hold >neg -; \ ( n1 -- u1 )
: # base @ /mod swap #digit hold -;   \ ( u1 -- u2 )
: #S begin # dup 0= until ;           \ ( u1 -- 0 )
: #> drop (neg) @ if '-' hold then ;
: #P #bufp @ typez ;                  \ ( 0 ... n 0 -- )
: (.) <# #S #> #P -;
: . (.) space ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: count ( str--a n ) dup 1+ swap c@ ; inline

: T8 ( ch-- )   r8 c! i8 ;
: T2 ( --str end )   +regs
    vhere dup s8 s9   0 T8
    begin >in @ c@ s1
        r1 if >in ++ then
        r1 0= r1 '"' = or
        if 0 T8   r9 r8 -regs   exit then
        r1 T8   r9 c++
    again ;

: s" ( --str ) T2 state @ 0= if drop exit then (vhere) ! (lit4) c, , ; immediate

: ." ( -- ) T2 state @ 0= if drop count type exit then
    (vhere) ! (lit4) c, ,
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (call) c, [ (lit4) c, ' type  drop drop , ] , ;  immediate

: .word cell + 1+ count type ;
: words +regs 0 s1 last s2 begin
        r2 mem-end < while
            i1 r1 #11 mod 0= if cr then
            r2 .word tab r2 word-sz + s2
    repeat
    ." (" r1 . ." words)" -regs ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $10 base ! ;
: ? @ . ;

: rshift ( n1 s--n2 ) 0 do 2 / loop ;
: lshift ( n1 s--n2 ) 0 do 2* loop ;

: load next-word drop 1- (load) ;
: load-abort 99 state ! ;
: loaded? if 2drop load-abort then ;

variable (fg) 2 cells allot
: fg cells (fg) + ;
: marker here 0 fg ! vhere 1 fg ! last 2 fg ! ;
: forget 0 fg @ (here) ! 1 fg @ (vhere) ! 2 fg @ (last) ! ;
: forget-1 last @ (here) ! last word-sz + (last) ! ;
marker
: .ver version 10 /mod (.) '.' emit . ;

." c3 - v" .ver ." - Chris Curl" cr
here mem -   . ." code bytes used, " last here - . ." bytes free." cr
vhere vars - . ." variable bytes used, " vars-end vhere - . ." bytes free."
forget

: benches forget s" benches.f" (load) ;
: sb forget s" sandbox.f" (load) ;
marker
