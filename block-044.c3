' accept loaded?

: bs 8 emit ; INLINE
: T0 r9 if  bs space bs  d1 d9  then ;

: accept ( a--len ) +regs DUP s1 s2 0 s9
  begin
    key s4
    r4 13 = if 0 r1 c!  r9 -exit then
    r4  8 =  r4 127 = or if T0 then
    r4 31 >  r4 127 < and if
       r4 r1+ c!  r4 emit  i9
    then
  again ;
