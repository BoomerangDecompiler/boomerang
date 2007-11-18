#line 690 "../specs/mips.nw"
assembly opcode
  jr                is j
  jalr              is jal
  mov               is move
  {sll,srl,sra}v    is $1
  tested_{*}        is $1
assembly operand
  [ rs rt rd base ] is "$%d"
  [ fs ft fd ]      is "$%s"
