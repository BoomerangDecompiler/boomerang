#
# Copyright (C) 1996???, Norman Ramsey???
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#line 267 "sparc.nw"
patterns
  B    is BA
  BGEU is BCC
  BLU  is BCS
  BNZ  is BNE
  branch.synonyms is B | BGEU | BLU | BNZ
#line 591 "sparc.nw"
constructors
  cmp rs1, reg_or_imm   is SUBcc(rs1, reg_or_imm, "%g0")
  jmp address_          is JMPL (address_, "%g0")
  call_ address_        is JMPL (address_, "%o7")
  tst  rs2              is ORcc ("%g0", rmode(rs2), "%g0")
  ret                   is JMPL (dispA("%i7",8), "%g0")
  retl                  is JMPL (dispA("%o7",8), "%g0")
  restore_              is RESTORE ("%g0", rmode("%g0"), "%g0")
  save_                 is SAVE("%g0", rmode("%g0"), "%g0")
  not   rd              is XNOR(rd,    rmode("%g0"), rd)
  not2  rs1, rd         is XNOR(rs1,   rmode("%g0"), rd)
  neg   rd              is SUB ("%g0", rmode(rd),    rd)
  neg2  rs2, rd         is SUB ("%g0", rmode(rs2),   rd)
  inc   simm13, rd         is ADD (rd, imode(simm13), rd)
  inccc simm13, rd         is ADDcc (rd, imode(simm13), rd)
  dec   simm13, rd         is SUB (rd, imode(simm13), rd)
  deccc simm13, rd         is SUBcc (rd, imode(simm13), rd)
  btst reg_or_imm, rs1  is ANDcc(rs1, reg_or_imm, "%g0")
  bset reg_or_imm, rd   is OR  (rd, reg_or_imm, rd)
  bclr reg_or_imm, rd   is ANDN(rd, reg_or_imm, rd)
  btog reg_or_imm, rd   is XOR (rd, reg_or_imm, rd)
  clr  rd               is OR  ("%g0", rmode("%g0"), rd)
  clr_ [address_]       is ST  ("%g0", address_)
  clrb [address_]       is STB ("%g0", address_)
  clrh [address_]       is STH ("%g0", address_)
  mov  reg_or_imm, rd   is OR  ("%g0", reg_or_imm, rd)
  mov_ rs2, rd          is OR  ("%g0", rmode(rs2),   rd)

#line 636 "sparc.nw"
#constructors
#  set val, rd  
#    when { val@[0:9] = 0 }     is  sethi(val, rd)
#    when { val = val@[0:12]! } is  OR("%g0", imode(val), rd)
#    otherwise                  is  sethi(val, rd); OR(rd, imode(val@[0:9]), rd)
