#line 370 "mips.nw"
constructors
  nop         	is  sll (r0, r0, 0)
  mov rd, rs    is  addu(rd, rs, r0)
  b reloc     	is  beq (r0, r0, reloc)
#line 381 "mips.nw"
constructors
  bge  rs, rt, reloc  is  slt (r1, rs, rt);  beq(r1, r0, reloc)
  bgeu rs, rt, reloc  is  sltu(r1, rs, rt);  beq(r1, r0, reloc)
  blt  rs, rt, reloc  is  slt (r1, rs, rt);  bne(r1, r0, reloc)
  bltu rs, rt, reloc  is  sltu(r1, rs, rt);  bne(r1, r0, reloc)
  bleu rs, rt, reloc  is  sltu(r1, rt, rs);  beq(r1, r0, reloc)
  ble  rs, rt, reloc  is  slt (r1, rt, rs);  beq(r1, r0, reloc)
  bgt  rs, rt, reloc  is  slt (r1, rt, rs);  bne(r1, r0, reloc)
  bgtu rs, rt, reloc  is  sltu(r1, rt, rs);  bne(r1, r0, reloc)
#line 393 "mips.nw"
constructors
  mul rd, rs, rt  is  multu(rs, rt); mflo(rd)
#line 414 "mips.nw"
constructors
  li rt, imm  
    when { imm@[16:31]! = imm@[15]! } is addiu(rt, r0, imm@[0:15]!)
    when { imm@[0:15] = 0 }           is lui  (rt, imm@[16:31])
    otherwise is lui(rt, imm@[16:31] + imm@[15]); addiu(rt, rt, imm@[0:15]!)
#line 548 "mips.nw"
constructors
  l.d ft,offset!(base) # { ft = 2 * _ } 
      is  lwc1(ft, offset!, base); lwc1(ft+1, offset!+4, base)
  s.d ft,offset!(base) # { ft = 2 * _ } 
      is  swc1(ft, offset!, base); swc1(ft+1, offset!+4, base)
  mtc1.d rt, fs  # { rt = 2 * _, fs = 2 * _ } 
      is  mtc1(rt, fs); mtc1(rt+1, fs+1)
  mfc1.d rt, fs  # { rt = 2 * _, fs = 2 * _ } 
      is  mfc1(rt, fs); mfc1(rt+1, fs+1)
