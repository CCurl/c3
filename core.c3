-ML- !           8 3 -MLX- inline
-ML- @          10 3 -MLX- inline
: \ 0 >in @ ! ; immediate

\ ***********************************************
\ c3 opcodes
\ ***********************************************

\ (stop)      is 0
\ (lit1)      is 1
: (lit4)         2 ;       inline
-ML- EXIT        3 3 -MLX- inline
: (exit)         3 ;       \ NOT inline
: (call)         4 ;       inline
: (jmp)          5 ;       inline
: (jmpz)         6 ;       inline
: (jmpnz)        7 ;       inline
\ !   (line 1)   8         \ defined above
-ML- c!          9 3 -MLX- inline
\ @   (line 2)  10         \ defined above
-ML- c@         11 3 -MLX- inline
: (dup)         12 ;       inline
-ML- DUP        12 3 -MLX- inline
-ML- SWAP       13 3 -MLX- inline
-ML- OVER       14 3 -MLX- inline
-ML- DROP       15 3 -MLX- inline
-ML- +          16 3 -MLX- inline
-ML- *          17 3 -MLX- inline
-ML- /MOD       18 3 -MLX- inline
-ML- -          19 3 -MLX- inline
-ML- 1+         20 3 -MLX- inline
-ML- 1-         21 3 -MLX- inline
-ML- <          22 3 -MLX- inline
-ML- =          23 3 -MLX- inline
-ML- >          24 3 -MLX- inline
-ML- 0=         25 3 -MLX- inline
-ML- >R         26 3 -MLX- inline
-ML- R@         27 3 -MLX- inline
-ML- R>         28 3 -MLX- inline
-ML- DO         29 3 -MLX- inline
-ML- LOOP       30 3 -MLX- inline
-ML- -LOOP      31 3 -MLX- inline
-ML- (I)        32 3 -MLX- inline
-ML- COM        33 3 -MLX- inline
-ML- AND        34 3 -MLX- inline
-ML- OR         35 3 -MLX- inline
-ML- XOR        36 3 -MLX- inline
-ML- EMIT       37 3 -MLX- inline
-ML- TIMER      38 3 -MLX- inline
-ML- KEY        39 3 -MLX- inline
-ML- ?KEY       40 3 -MLX- inline
: (TYPE)        41 ;       inline
-ML- TYPE       41 3 -MLX- inline
-ML- TYPEZ      42 3 -MLX- inline
\ (define)   is 43         \ defined in c3
\ (end-word) is 44         \ defined in c3
-ML- CREATE     45 3 -MLX- inline
-ML- '          46 3 -MLX- inline
-ML- NEXT-WORD  47 3 -MLX- inline
\ (iX)       is 48         \ defined in c3
\ (dX)       is 49         \ defined in c3
\ (rX)       is 50         \ defined in c3
\ (rX-)      is 51         \ defined in c3
\ (rX+)      is 52         \ defined in c3
\ (sX)       is 53         \ defined in c3
-ML- +REGs      54 3 -MLX- inline
-ML- -REGS      55 3 -MLX- inline
\ : INLINE      56 ;       \ defined in c3
\ : IMMEDIATE   57 ;       \ defined in c3

\ Words for the PC version
-ML- SYSTEM     58 3 -MLX- inline
-ML- FOPEN      59 3 -MLX- inline
-ML- FCLOSE     60 3 -MLX- inline
-ML- FREAD      61 3 -MLX- inline
-ML- FWRITE     62 3 -MLX- inline
-ML- (load)     63 3 -MLX- inline

\ ***********************************************
\ OK ... now we can get things going
\ ***********************************************

: [ 0 state ! ; immediate
: ] 1 state ! ;

: last (last) @ ;
: here (here) @ ;
: c, here c! here 1+     (here) ! ;
: ,  here !  here cell + (here) ! ;

: mem-end   mem  mem-sz  + ;
: vars-end  vars vars-sz + ;

: ++ dup @ 1+ swap ! ; inline
: -- dup @ 1- swap ! ; inline

: vhere  (vhere) @ ;
: allot  vhere + (vhere) ! ;
: vc, vhere c! (vhere) ++ ;
: v,  vhere ! cell allot ;

: bye 999 state ! ;
: cells cell * ; inline

: constant  create (lit4) c, , (exit) c, ;
: variable  vhere constant cell allot ;
: val  vhere constant ;
: (val)  here 1- cell - constant ;

: :noname  here 1 state ! ;
: does>  r> last ! ;
: exec  >r ;

: if    (jmpz) c, here 0 , ; immediate
: else  (jmp) c, here swap 0 , here swap ! ; immediate
: then  here swap ! ; immediate
: exit  (exit) c,   ; immediate

: begin  here         ; immediate
: until  (jmpz) c, , ; immediate
: again  (jmp) c, , ; immediate
: while  (jmpz) c, here 0 , ; immediate
: repeat swap (jmp) c, , here swap ! ; immediate
: for 0 swap do ; inline
: next -loop ; inline

\ Some 'MachineForth' inspired words
: -if     (dup) c,  (jmpz) c, here 0 , ; immediate
: -until  (dup) c,  (jmpz) c, , ; immediate
: -while  (jmpnz) c, , ; immediate

: tuck  swap over ; inline
: nip   swap drop ; inline
: 2dup  over over ; inline
: 2drop drop drop ; inline
: ?dup  dup if dup then ;

: /   /mod nip  ; inline
: mod /mod drop ; inline

: +! swap over @ + swap ! ;  inline
: c++ dup c@ 1+ swap c! ; inline
: 2*  dup + ; inline
: 2/  2 / ; inline
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
: btw +regs s3 s2 s1 r1 r2 >= r1 r3 <= and -regs ;

: i  (i) @ ;
: j  (i) 3 cells - @ ;
: +i (i) +! ;
: unloop (lsp) @ 3 - 0 max (lsp) ! ;

variable (neg)
variable #bufp
: hold #bufp -- #bufp @ c! ;          \ ( c -- )
: #digit dup 9 > if 7 + then '0' + ;  \ ( n -- c )
: >neg dup 0 < (neg) ! abs ;          \ ( n1 -- u1 )
: <# here #66 + #bufp ! 0 hold >neg ; \ ( n1 -- u1 )
: # base @ /mod swap #digit hold ;    \ ( u1 -- u2 )
: #S begin # dup 0= until drop ;      \ ( u1 -- )
: #> (neg) @ if '-' hold then ;       \ ( -- )
: #P #bufp @ typez ;                  \ ( -- )
: (.) <# #S #> #P ;
: . (.) space ;

: 0sp 0 (sp) ! ;
: depth (sp) @ 1- ;
: .s '(' emit space depth ?dup if
        0 do (stk) i 1+ cells + @ . loop 
    then ')' emit ;

: count ( str--a n ) dup 1+ swap c@ ; inline
: dump ( a n-- ) for dup c@ . 1+ next drop ;

: T2 ( --str end )   +regs   >in ++
    vhere dup s8 s9  0 r8+ c!
    begin >in @ c@ s1
        r1 if >in ++ then
        r1 0= r1 '"' = or
        if 0 r8+ c!   r9 r8 -regs   exit then
        r1   r8+ c!   r9 c++
    again ;

: s" ( --str ) T2 state @ 0= if drop exit then (vhere) ! (lit4) c, , ; immediate

: ." ( -- ) T2 state @ 0= if drop count type exit then
    (vhere) ! (lit4) c, ,
    (call) c, [ (lit4) c, ' count drop drop , ] ,
    (type) c, ;  immediate

: .word cell + 1+ count type ;
: words +regs 0 s1 0 s3 last s2 begin
        r2 mem-end < while
            r1+ #10 > if 0 s1 cr then
            i3 r2 .word tab r2 word-sz + s2
    repeat
    ." (" r3 . ." words)" -regs ;

: binary  %10 base ! ;
: decimal #10 base ! ;
: hex     $10 base ! ;
: ? @ . ;

: lshift ( n1 s--n2 ) 0 do 2* loop ;
: rshift ( n1 s--n2 ) 0 do 2/ loop ;

: load next-word drop 1- (load) ;
: load-abort #99 state ! ;
: loaded? if 2drop load-abort then ;

variable (fg) 2 cells allot
: fg cells (fg) + ;
: marker here 0 fg ! vhere 1 fg ! last 2 fg ! ;
: forget 0 fg @ (here) ! 1 fg @ (vhere) ! 2 fg @ (last) ! ;
: forget-1 last @ (here) ! last word-sz + (last) ! ;
marker
: .ver version 100 /mod (.) '.' emit . ;

." c3 - v" .ver ." - Chris Curl" cr
here mem -   . ." code bytes used, " last here - . ." bytes free." cr
vhere vars - . ." variable bytes used, " vars-end vhere - . ." bytes free."
forget

: benches forget s" benches.c3" (load) ;
: sb forget s" sandbox.c3" (load) ;
: work forget s" work.c3" (load) ;
: ed forget s" editor.c3" (load) ;
marker
