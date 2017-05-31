# Table F-9

constructors
   crset  bx      is creqv( bx, bx, bx )
   crclr  bx      is crxor( bx, bx, bx )
   crmove bx, by  is cror(  bx, by, by )
   crnot  bx, by  is crnor( bx, by, by )

# Section F.9

   nop            is ori( 0, 0, 0 )
   li    D, v     is addi( D, 0, v )
   lis   D, v     is addis( D, 0, v )
   la    D, d!, A is addi( D, A, d )
   mr    A, S     is ori( A, S, 0 )  # PPC 604 has better performance with ori than or
   mr.   A, S     is or.( A, S, S )
   not   A, S     is nor( A, S, S )
   not.  A, S     is nor.( A, S, S )
   mtcr  S        is mtcrf( 0xff, S ) 

# e additions
#
   lwi   D, x     is addis( D, 0, x@[16:31] ); ori(D, D, x@[0:15] )

#### end



