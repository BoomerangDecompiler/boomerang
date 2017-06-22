#line 36 "mips.nw"
fields of instruction (32) 
#line 53 "mips.nw"
word 0:31 op 26:31 rs 21:25 rt 16:20 immed 0:15 offset 0:15 base 21:25 
target 0:25 rd 11:15 shamt 6:10 funct 0:5 cond 16:20 breakcode 6:25
#line 434 "mips.nw"
ft 16:20 fs 11:15 fd 6:10 format 21:24 bit25 25:25
#line 474 "mips.nw"
cop1code 22:25 copbcode 16:16
#line 70 "mips.nw"
fieldinfo [ rs rt rd base ] is [ 
#line 65 "mips.nw"
names [ r0  r1  r2  r3  r4  r5  r6  r7  r8  r9  r10 r11 r12 r13 r14 r15
        r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 sp  r30 r31 ]
#line 70 "mips.nw"
                                                                           ]
#line 436 "mips.nw"
fieldinfo [ fs ft fd ] is [ 
#line 438 "mips.nw"
names [ f0  f1  f2  f3  f4  f5  f6  f7  f8  f9  f10 f11 f12 f13 f14 f15
        f16 f17 f18 f19 f20 f21 f22 f23 f24 f25 f26 f27 f28 f29 f30 f31 ]
#line 436 "mips.nw"
                                                                            ]
#line 446 "mips.nw"
fieldinfo format is [ sparse [ s = 0, d = 1, w = 4 ] ]
#line 90 "mips.nw"
patterns
 [ special bcond   j       jal     beq     bne     blez    bgtz
   addi    addiu   slti    sltiu   andi    ori     xori    lui
   cop0    cop1    cop2    cop3    _       _       _       _
   _       _       _       _       _       _       _       _
   lb      lh      lwl     lw      lbu     lhu     lwr     _
   sb      sh      swl     sw      _       _       swr     _
   lwc0    lwc1    lwc2    lwc3    _       _       _       _
   swc0    swc1    swc2    swc3    _       _       _       _ ] 
 is op = {0 to 63}
#line 170 "mips.nw"
patterns
 [ sll     _       srl     sra     sllv    _       srlv    srav
   jr      jalr    _       _       syscall break   _       _
   mfhi    mthi    mflo    mtlo    _       _       _       _
   mult    multu   div     divu    _       _       _       _
   add     addu    sub     subu    and     or      xor     nor
   _       _       slt     sltu    _       _       _       _ ] 
 is  special & funct = {0 to 47}
#line 186 "mips.nw"
patterns
 [ bltz bgez bltzal bgezal ] is bcond & cond = [ 0 1 16 17 ]
#line 209 "mips.nw"
patterns
  load    is lb | lbu | lh | lhu | lw | lwl | lwr | sb | sh | sw | swl | swr 
  immedS  is addi | addiu | slti | sltiu
  immedU  is andi | ori | xori 
  arith3  is add | addu | sub | subu | slt | sltu | and | or | xor | nor 
  shift   is sll | srl | sra
  vshift  is sllv | srlv | srav 
  arith2  is mult | multu | div | divu
  mfhilo  is mfhi | mflo 
  mthilo  is mthi | mtlo
  jump    is j  | jal
  jumpr   is jr | jalr
  branch1 is blez | bgtz | bltz | bgez | bltzal | bgezal
  branch2 is beq | bne
  copls   is lwc0 | lwc1 | lwc2 | lwc3 | swc0 | swc1 | swc2 | swc3
#line 258 "mips.nw"
constructors
  load   rt, offset!(base)
  immedS rt, rs, offset!
  immedU rt, rs, offset
  lui    rt, offset
  arith3 rd, rs, rt
  shift  rd, rt, shamt
  vshift rd, rt, rs
  arith2 rs, rt
  mfhilo rd
  mthilo rs
  syscall
  break  breakcode
  copls  ft, offset!(base)
  jr     rs
  jalr   rd, rs
#line 333 "mips.nw"
placeholder for instruction is break(99)
#line 303 "mips.nw"
relocatable reloc
constructors
  branch1 rs, reloc
     { reloc = L + 4 * offset! } is branch1 & rs & offset; L: epsilon
  branch2 rs, rt, reloc
     { reloc = L + 4 * offset! } is branch2 & rs & rt & offset; L: epsilon
#line 350 "mips.nw"
constructors
  jump reloc { reloc@[28:31] = L@[28:31], 
               reloc@[2:27]  = target, 
               reloc@[0:1]   = 0 } 
         is L: jump & target
#line 457 "mips.nw"
patterns
 [ add.    sub.    mul.    div.    _       abs.    mov.    neg.   
   _       _       _       _       _       _       _       _
   _       _       _       _       _       _       _       _
   _       _       _       _       _       _       _       _
   cvt.s   cvt.d   _       _       cvt.w   _       _       _
   _       _       _       _       _       _       _       _
   c.f     c.un    c.eq    c.ueq   c.olt   c.ult   c.ole   c.ule
   c.sf    c.ngle  c.seq   c.ngl   c.lt    c.nge   c.le    c.ngt ] 
 is cop1 & funct = {0 to 63} & bit25 = 1
#line 476 "mips.nw"
patterns
  [ mfc1 cfc1 mtc1 ctc1 ] is cop1 & cop1code = {0 to 3} & funct = 0
  bc1x is cop1 & (cop1code = 4 | cop1code = 6)
  bc1f is bc1x & copbcode = 0
  bc1t is bc1x & copbcode = 1
#line 498 "mips.nw"
patterns
  arith3. is add. | div. | mul. | sub.
  arith2. is abs. | mov. | neg. 
  movec1  is mfc1 | mtc1 | cfc1 | ctc1
  c.cond  is c.f  | c.un   | c.eq  | c.ueq | c.olt | c.ult | c.ole | c.ule |
             c.sf | c.ngle | c.seq | c.ngl | c.lt  | c.nge | c.le  | c.ngt  
  lsc1    is lwc1 | swc1
  convert is cvt.s | cvt.d | cvt.w

#line 528 "mips.nw"
constructors
  arith3.^format fd, fs, ft { fd = 2 * _, fs = 2 * _, ft = 2 * _ }
  arith2.^format fd, fs     { fd = 2 * _, fs = 2 * _ }
  movec1         rt, fs     
  c.cond^"."^format  fs, ft { fs = 2 * _, ft = 2 * _ }
  convert^"."^format fd, fs { fd = 2 * _, fs = 2 * _ }
  "bc1f" reloc { reloc = L + 4 * offset! } is bc1f & offset; L: epsilon
  "bc1t" reloc { reloc = L + 4 * offset! } is bc1t & offset; L: epsilon
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
  l.d ft,offset!(base) { ft = 2 * _ } 
      is  lwc1(ft, offset!, base); lwc1(ft+1, offset!+4, base)
  s.d ft,offset!(base) { ft = 2 * _ } 
      is  swc1(ft, offset!, base); swc1(ft+1, offset!+4, base)
  mtc1.d rt, fs  { rt = 2 * _, fs = 2 * _ } 
      is  mtc1(rt, fs); mtc1(rt+1, fs+1)
  mfc1.d rt, fs  { rt = 2 * _, fs = 2 * _ } 
      is  mfc1(rt, fs); mfc1(rt+1, fs+1)
#line 663 "mips.nw"
fieldinfo [ rs rt rd base fs ft fd ] is [ guaranteed ]
#line 576 "mips.nw"
constructors
  trunc.w.d fs, ft, rt is 
    cfc1(rt, f31); 
    cfc1(rt, f31);   nop(); 
    ori(r1, rt, 3);
    xori(r1, r1, 2); 
    ctc1(r1, f31); 
    srlv(r0, r0, r0);
    cvt.w.d(fs, ft); nop();
    ctc1(rt, f31);   nop(); 
    mfc1(rt, fs);    nop()
