\ Words for screen handling

' cls loaded?

: t       ( -- )       27 emit '[' emit ; inline
: T0      ( n1 n2-- )  t (.) ';' emit (.) ;
: cur-on  ( -- )       t ." ?25h" ;
: cur-off ( -- )       t ." ?25l" ;
: ->yx    ( y x-- )    T0 'H' emit ;
: ->xy    ( x y-- )    swap ->yx ;
: cls     ( -- )       t ." 2J" 1 dup ->yx ;
: clr-eol ( -- )       t ." 0K" ;
: color   ( bg fg-- )  T0 'm' emit ;
: fg      ( -- )       40 swap color ;
