#
# Copyright (C) 1996, Princeton University or Owen C. Braun ?????
# Copyright (C) 2000, Sun Microsystems, Inc
# Copyright (C) 2000-2001, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# Motorola MC68000 Specification }
# Owen C. Braun \\ ocbraun@princeton.edu }
# Specification of the Motorola MC68000 Instruction Set
# for the New Jersey Machine Code Toolkit, Version 0.4.
# Created Mar/25/96 5:47 am. 
#
# 2-3 Feb 00 - Cristina
#   Added the "keep" list to make this spec specific to the MC68328.
#   Added ORItoCCR pattern
# 09 Feb 00 - Cristina
#   removed unlk from _reg2only as it takes an address register instead 
#   of a data register.
# 15 Feb 00 - Mike: Added moveFromCCR to clr set
# 16 Feb 00 - Mike: Renamed _b (branch pattern and constructor) to _br; was
#                       a name clash with some of the pattern matching code
# 25 Feb 00 - Mike: BTA (Branch Target Address) signed
# 01 Aug 01 - Mike: Branches use cond field instead of reg1 and sb; avoids
#               toolkit error


fields of iword (16)

    op  12:15       #  first 4 bits as opcode (standard)
    reg1    9:11    #  generally first of two register operands
    MDadrm  6:8     #  addressing mode for a MOVE destination operand
    sb  8:8         #  bit before size field (often part of opcode)
    sz  6:7         #  size
    adrm    3:5     #  effective addressing mode
    reg2    0:2     #  effective addressing register, or second of two register operands

    cond    8:11    #  experimental
    data8   0:7     #  experimental (but used by branches)
    adrb    4:5     #  experimental
    vect    0:3     #  experimental
    bot12   0:11    # experimental MVE

fields of exword (16)

    d16      0:15
    iType   15:15
    iReg    12:14
    iSize   11:11
    null     8:10
    disp8    0:7

fields of exlong (32)

    d32  0:31

# Instruction opcode (high-order nybble) patterns.

patterns

[ BitsImmMovep      #  Bit Manipulation/movep/Immediate
    MoveByte        #  Move Byte
    MoveLong        #  Move Long
    MoveWord        #  Move Word
    Misc            #  Miscellaneous
    QuickCond       #  addq/subq/Scc/DBcc
    Branch          #  Bcc/bsr/bra
    MoveQ           #  moveq
    OrDivSbcd       #  or/div/sbcd
    Sub             #  sub/subx
    Aline           #  (Unassigned, often system calls)
    CmpEor          #  cmp/eor
    AndMulAbcdExg   #  and/mul/abcd/exg
    Add             #  add/addx
    ShiftRotate     #  Shift/Rotate
    _               #  Coprocessor Interface (N/A)
      ] is op = { 0 to 15 }

# General instruction patterns.

patterns

    LegalSz     is  sz < 3

# Instruction patterns.

patterns

  [ orib  oriw  oril
    andib andiw andil
    subib subiw subil
    addib addiw addil
    _     _     _
    eorib eoriw eoril
    cmpib cmpiw cmpil
    _     _     _ ]
    is reg1 = { 0 to 7 } & sb = 0 & sz = { 0 to 2 } & BitsImmMovep

    ori     is  BitsImmMovep & reg1 = 0 & sb = 0
    andi    is  BitsImmMovep & reg1 = 1 & sb = 0
    subi    is  BitsImmMovep & reg1 = 2 & sb = 0
    addi    is  BitsImmMovep & reg1 = 3 & sb = 0
    eori    is  BitsImmMovep & reg1 = 5 & sb = 0
    cmpi    is  BitsImmMovep & reg1 = 6 & sb = 0

    oriToCCR is BitsImmMovep & reg1 = 0 & sb = 0 & sz = 0 & adrm = 7 & reg2 = 4
    oriToSR  is BitsImmMovep & reg1 = 0 & sb = 0 & sz = 1 & adrm = 7 & reg2 = 4
    andiToCCR is BitsImmMovep & reg1 = 1 & sb = 0 & sz = 0 & adrm = 7 & reg2 = 4
    andiToSR  is BitsImmMovep & reg1 = 1 & sb = 0 & sz = 1 & adrm = 7 & reg2 = 4
    eoriToCCR is BitsImmMovep & reg1 = 5 & sb = 0 & sz = 0 & adrm = 7 & reg2 = 4
    eoriToSR  is BitsImmMovep & reg1 = 5 & sb = 0 & sz = 1 & adrm = 7 & reg2 = 4

  [ btsti bchgi bclri bseti ]
    is sz = { 0 to 3 } & BitsImmMovep & reg1 = 4 & sb = 0

  [ btst bchg bclr bset ]
    is sz = { 0 to 3 } & BitsImmMovep & sb = 1
    
  [ movepmrw movepmrl moveprmw moveprml ]
    is sz = { 0 to 3 } & BitsImmMovep & sb = 1 & adrm = 1
    movepmr is  BitsImmMovep & sb = 1 & sz < 2 & adrm = 1
    moveprm is  BitsImmMovep & sb = 1 & sz > 1 & adrm = 1
    movep   is  BitsImmMovep & sb = 1 & adrm = 1

  [ _ moveb movel movew ]
    is op = { 0 to 3 }
    move    is  op > 0 & op < 4

  [ negxb negxw negxl moveFromSR ]
    is sz = { 0 to 3 } & Misc & reg1 = 0 & sb = 0
    negx    is  Misc & reg1 = 0 & sb = 0 & LegalSz

  [ clrb clrw clrl moveFromCCR ]
    is sz = { 0 to 3 } & Misc & reg1 = 1 & sb = 0
    clr     is  Misc & reg1 = 1 & sb = 0 & LegalSz

  [ negb negw negl moveToCCR ]
    is sz = { 0 to 3 } & Misc & reg1 = 2 & sb = 0
    neg     is  Misc & reg1 = 2 & sb = 0 & LegalSz

  [ notb notw notl moveToSR ]
    is sz = { 0 to 3 } & Misc & reg1 = 3 & sb = 0
    not     is  Misc & reg1 = 3 & sb = 0 & LegalSz

    extw    is  Misc & reg1 = 4 & sb = 0 & sz = 2 & adrm = 0
    extl    is  Misc & reg1 = 4 & sb = 0 & sz = 3 & adrm = 0
    nbcd    is  Misc & reg1 = 4 & sb = 0 & sz = 0
    swap    is  Misc & reg1 = 4 & sb = 0 & sz = 1 & adrm = 0
    pea     is  Misc & reg1 = 4 & sb = 0 & sz = 1

    illegal is  Misc & reg1 = 5 & sb = 0 & sz = 3 & adrm = 7 & reg2 = 4

  [ tstb tstw tstl tas ]
    is sz = { 0 to 3 } & Misc & reg1 = 5 & sb = 0
    tst     is  Misc & reg1 = 5 & sb = 0 & LegalSz

    trap    is  Misc & reg1 = 7 & sb = 0 & sz = 1 & adrb = 0
    link    is  Misc & reg1 = 7 & sb = 0 & sz = 1 & adrm = 2
    unlk    is  Misc & reg1 = 7 & sb = 0 & sz = 1 & adrm = 3
    moveFromUSP is  Misc & reg1 = 7 & sb = 0 & sz = 1 & adrm = 5
    moveToUSP   is  Misc & reg1 = 7 & sb = 0 & sz = 1 & adrm = 4

  [ reset nop stop rte _ rts trapv rtr ]
    is reg2 = { 0 to 7 } & Misc & reg1 = 7 & sb = 0 & sz = 1 & adrm = 6

    jsr     is  Misc & reg1 = 7 & sb = 0 & sz = 2
    jmp     is  Misc & reg1 = 7 & sb = 0 & sz = 3

    movermw is  Misc & reg1 = 4 & sb = 0 & sz = 2
    moverml is  Misc & reg1 = 4 & sb = 0 & sz = 3
    moverm  is  Misc & reg1 = 4 & sb = 0 & sz > 1
    movemrw is  Misc & reg1 = 6 & sb = 0 & sz = 2
    movemrl is  Misc & reg1 = 6 & sb = 0 & sz = 3
    movemr  is  Misc & reg1 = 6 & sb = 0 & sz > 1

    lea     is  Misc & sb = 1 & sz = 3
    chk     is  Misc & sb = 1 & sz = 2

  [ addqb addqw addql _ subqb subqw subql _ ]
    is sb = { 0 to 1 } & sz = { 0 to 3 } & QuickCond
    addq    is  QuickCond & sb = 0 & LegalSz
    subq    is  QuickCond & sb = 1 & LegalSz

    DBcc    is  QuickCond & sz = 3 & adrm = 1
    Scc     is  QuickCond & sz = 3

    bra     is  Branch & cond = 0
    bsr     is  Branch & cond = 1
    Bcc     is  Branch & cond > 1

    moveq   is  MoveQ & sb = 0

    divu    is  OrDivSbcd & sb = 0 & sz = 3
    divs    is  OrDivSbcd & sb = 1 & sz = 3
    sbcdr   is  OrDivSbcd & sb = 1 & sz = 0 & adrm = 0
    sbcdm   is  OrDivSbcd & sb = 1 & sz = 0 & adrm = 1
    sbcd    is  OrDivSbcd & sb = 1 & sz = 0

  [ orrb orrw orrl _ ]
    is sz = { 0 to 3 } & OrDivSbcd & sb = 0
    orr     is  OrDivSbcd & sb = 0 & LegalSz

  [ ormb ormw orml _ ]
    is sz = { 0 to 3 } & OrDivSbcd & sb = 1
    orm     is  OrDivSbcd & sb = 1 & LegalSz

  [ subxrb subxrw subxrl _ ]
    is sz = { 0 to 3 } & Sub & sb = 1 & adrm = 0
    subxr   is  Sub & sb = 1 & LegalSz & adrm = 0

  [ subxmb subxmw subxml _ ]
    is sz = { 0 to 3 } & Sub & sb = 1 & adrm = 1
    subxm   is  Sub & sb = 1 & LegalSz & adrm = 1

    subaw   is  Sub & sb = 0 & sz = 3 
    subal   is  Sub & sb = 1 & sz = 3 
    suba    is  Sub & sz = 3 

  [ subrb subrw subrl _ ]
    is sz = { 0 to 3 } & Sub & sb = 0 
    subr    is  Sub & sb = 0 & LegalSz 

  [ submb submw subml _ ]
    is sz = { 0 to 3 } & Sub & sb = 1
    subm    is  Sub & sb = 1 & LegalSz

  [ cmpmb cmpmw cmpml _ ]
    is sz = { 0 to 3 } & CmpEor & sb = 1 & adrm = 1
    cmpm    is  CmpEor & sb = 1 & LegalSz & adrm = 1

  [ cmpb cmpw cmpl _ ]
    is sz = { 0 to 3 } & CmpEor & sb = 0 
    cmp     is  CmpEor & sb = 0 & LegalSz 

    cmpaw   is  CmpEor & sb = 0 & sz = 3 
    cmpal   is  CmpEor & sb = 1 & sz = 3 
    cmpa    is  CmpEor & sz = 3 

  [ eorb eorw eorl _ ]
    is sz = { 0 to 3 } & CmpEor & sb = 1
    eor     is  CmpEor & sb = 1 & LegalSz

    mulu    is  AndMulAbcdExg & sb = 0 & sz = 3
    muls    is  AndMulAbcdExg & sb = 1 & sz = 3
    abcdr   is  AndMulAbcdExg & sb = 1 & sz = 0 & adrm = 0
    abcdm   is  AndMulAbcdExg & sb = 1 & sz = 0 & adrm = 1
    abcd    is  AndMulAbcdExg & sb = 1 & sz = 0

    exgdd   is  AndMulAbcdExg & sb = 1 & sz = 1 & adrm = 0
    exgaa   is  AndMulAbcdExg & sb = 1 & sz = 1 & adrm = 1
    exgda   is  AndMulAbcdExg & sb = 1 & sz = 2 & adrm = 1

  [ andrb andrw andrl _ ]
    is sz = { 0 to 3 } & AndMulAbcdExg & sb = 0
    andr    is  AndMulAbcdExg & sb = 0 & LegalSz 

  [ andmb andmw andml _ ]
    is sz = { 0 to 3 } & AndMulAbcdExg & sb = 1
    andm    is  AndMulAbcdExg & sb = 1 & LegalSz

  [ addxrb addxrw addxrl _ ]
    is sz = { 0 to 3 } & Add & sb = 1 & adrm = 0
    addxr   is  Add & sb = 1 & LegalSz & adrm = 0

  [ addxmb addxmw addxml _ ]
    is sz = { 0 to 3 } & Add & sb = 1 & adrm = 1
    addxm   is  Add & sb = 1 & LegalSz & adrm = 1

    addaw   is  Add & sb = 0 & sz = 3 
    addal   is  Add & sb = 1 & sz = 3 
    adda    is  Add & sz = 3 

  [ addrb addrw addrl _ ]
    is sz = { 0 to 3 } & Add & sb = 0 
    addr    is  Add & sb = 0 & LegalSz

  [ addmb addmw addml _ ]
    is sz = { 0 to 3 } & Add & sb = 1
    addm    is  Add & sb = 1 & LegalSz

  [ asrm aslm lsrm lslm roxrm roxlm rorm rolm ]
    is reg1 = { 0 to 3 } & sb = { 0 to 1 } & ShiftRotate & sz = 3

  [ asrib asriw asril aslib asliw aslil
    lsrib lsriw lsril lslib lsliw lslil
    roxrib roxriw roxril roxlib roxliw roxlil
    rorib roriw roril rolib roliw rolil
    asrrb asrrw asrrl aslrb aslrw aslrl
    lsrrb lsrrw lsrrl lslrb lslrw lslrl
    roxrrb roxrrw roxrrl roxlrb roxlrw roxlrl
    rorrb rorrw rorrl rolrb rolrw rolrl ]
    is adrm = { 0 to 7 } & sb = { 0 to 1 } & sz = { 0 to 2 } & ShiftRotate

  [ asri asli
    lsri lsli
    roxri roxli
    rori roli
    asrr aslr
    lsrr lslr
    roxrr roxlr
    rorr rolr ]
    is adrm = { 0 to 7 } & sb = { 0 to 1 } & LegalSz & ShiftRotate

#
#  This file contains patterns used to identify certain types of
#  instructions (i.e., branches) and information about a given
#  instruction (i.e., the size of the data it is loading/storing, the
#  registers it uses).
#

#  Note that for the moment the Compute pattern is vacuuous; this is
#  partially because an argument could be made for any M68k Instruction
#  doing "computation" and partially because Cinderella doesn't expect
#  to encounter an instruction of AsmInstr::UnknownType; thus I am making
#  this a "catchall".
#  

patterns

   Compute  is  op < 15

#
#  Addressing mode tuples - note that the constraints enforce an *actual*
#               memory access (modes 1, 2, and 7/4 don't count)
#

patterns

   amAddr   is  (adrm > 1 & adrm < 7) | (adrm = 7 & reg2 < 4)
   alAddr   is  (adrm > 1 & adrm < 7) | (adrm = 7 & reg2 < 2)
   dAddr    is  (adrm > 1 & adrm < 7) | (adrm = 7 & reg2 < 4)
   daAddr   is  (adrm > 1 & adrm < 7) | (adrm = 7 & reg2 < 2)
   maAddr   is  (adrm > 1 & adrm < 7) | (adrm = 7 & reg2 < 2)
   mdAddr   is  (MDadrm > 1 & MDadrm < 7) | (MDadrm = 7 & reg1 < 4)
   mrAddr   is  adrm = 2 | (adrm > 3 & adrm < 7) | (adrm = 7 & reg2 < 2)
   rmAddr   is  (adrm > 1 & adrm < 4) | (adrm > 4 & adrm < 7) | (adrm = 7 & reg2 < 4)

#
#  The MemSrcAn pattern matches any instruction which loads from
#  a memory location, where the location is specified by an address
#  register. 
#
#  Note. Patterns are represented internally in the toolkit in
#  normal form, as follows:
#
#  pattern: disjunct [ | pattern ]
#
#  disjunct: sequent [ ; disjunct ]
#
#  sequent: constraint [ & sequent ]
#
#  Thus expressions containing parenthesized ORs (i.e., (a | b) & c)
#  will be stored by distributing over the disjuncts (i.e.,
#  (a & c) | (b & c)). Thus we gain nothing by digging inside
#  instruction patterns, which contain no disjuncts, to define the
#  following patterns. What will be stored by the toolkit is
#  equivalent to (instr1 | instr2 | instr3 ...).
#

patterns

   memSrcAn is  abcdm |
        addxm | 
        cmpm | 
        sbcdm | 
        subxm

   memSrcEA is  addr & amAddr | 
        addm & maAddr | 
        adda & amAddr |
        andr & dAddr | 
        andm & maAddr |
        aslm & maAddr | 
        asrm & maAddr |
        bchg & daAddr | 
        bclr & daAddr | 
        bset & daAddr | 
        btst & dAddr |
        chk & dAddr |
        cmp & amAddr |
        divs & dAddr | 
        divu & dAddr |
        eor & daAddr | 
        eori & daAddr |
        lslm & maAddr | 
        lsrm & maAddr |
        moveb & amAddr | 
        movel & amAddr | 
        movew & amAddr | 
        moveToCCR & dAddr | 
        moveToSR & dAddr |
        movemr & mrAddr | 
        moverm & rmAddr | 
        movepmr | 
        muls & dAddr | 
        mulu & dAddr |
        nbcd & daAddr | 
        neg & daAddr | 
        negx & daAddr | 
        not & daAddr |
        orr & dAddr | 
        orm & maAddr |
        rolm & maAddr | 
        rorm & maAddr |
        roxlm & maAddr | 
        roxrm & maAddr |
        subr & amAddr | 
        subm & maAddr | 
        suba & amAddr |
        tas & daAddr | 
        tst & daAddr

#
#  The MemDstAn & MemDstEA patterns are analogous to those above
#  except that they pertain to stores to a memory location.
#

   memDstAn is  abcdm | 
        addxm | 
        sbcdm | 
        subxm

   memDstEA is  addm & maAddr | 
        addi & daAddr | 
        addq & alAddr |
        andm & maAddr | 
        andi & daAddr |
        aslm & maAddr | 
        asrm & maAddr |
        bchg & daAddr | 
        bclr & daAddr | 
        bset & daAddr | 
        clr & daAddr |
        eor & daAddr | 
        eori & daAddr |
        lslm & maAddr | 
        lsrm & maAddr |
        move & mdAddr | 
        moveFromSR & daAddr |
        moveFromCCR & daAddr |
        moverm & rmAddr | 
        moveprm |
        nbcd & daAddr| 
        neg & daAddr | 
        negx & daAddr | 
        not & daAddr |
        orm & maAddr | 
        ori & daAddr |
        rolm & maAddr | 
        rorm & maAddr |
        roxlm & maAddr | 
        roxrm & maAddr |
        Scc & daAddr | 
        subm & maAddr | 
        subi & daAddr | 
        subq & alAddr | 
        tas & daAddr

#
#  The following patterns are required by the target-independent
#  interface to cinderella as a means of classifying instruction
#  types.
#

   UncondBranch is bra | jmp

   CondBranch is Bcc | DBcc

   Call is bsr | jsr

   Return is rte | rtr | rts

   Load is memSrcAn | memSrcEA

   Store is memDstAn | memDstEA


constructors

  byteOffset    d8!  : BTA   is  data8 = d8
  wordOffset    d16! : BTA   is  data8 = 0; d16

patterns

  [ dDirect aDirect indirect postInc preDec aDisp aIndex _ ]
    is adrm = { 0 to 7 }
    abs.w       is  adrm = 7 & reg2 = 0
    abs.l       is  adrm = 7 & reg2 = 1
    pcDisp      is  adrm = 7 & reg2 = 2
    notPcDisp   is  adrm != 7 | reg2 != 2
    pcIndex     is  adrm = 7 & reg2 = 3
    Imm         is  adrm = 7 & reg2 = 4

constructors

#
# Modes tuples:
#
# al - Alterable
# am - Any Mode
# c - Control
# d - Data
# da - Data Alterable
# ma - Memory Alterable
# md - Move Destinations (Data Alterable in different fields)
# mr - MOVEM memory-to-register (Control + PostInc)
# rm - MOVEM register-to-memory (Control Alt + PreDec)
#

#
# Alterable modes
#
  alDDirect reg2            : alEA is dDirect & reg2
  alADirect reg2            : alEA is aDirect & reg2
  alIndirect reg2           : alEA is indirect & reg2
  alPostInc reg2            : alEA is postInc & reg2
  alPreDec  reg2            : alEA is preDec & reg2
  alADisp   reg2            : alEAX is aDisp & reg2
  alADisp.x d16!            : alX is d16
  alAIndex  reg2            : alEAX is aIndex & reg2
  alAIndex.x iType iReg iSize disp8! : alX is iType & iReg & iSize & disp8
  alAbs.w                   : alEAX is abs.w
  alAbs.w.x d16!            : alX is d16
  alAbs.l                   : alEAX is abs.l
  alAbs.l.x d32             : alX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# All modes
#
  amDDirect reg2            : amEA is dDirect & reg2
  amADirect reg2            : amEA is aDirect & reg2
  amIndirect reg2           : amEA is indirect & reg2
  amPostInc reg2            : amEA is postInc & reg2
  amPreDec  reg2            : amEA is preDec & reg2
  amADisp   reg2            : amEAX is aDisp & reg2
  amADisp.x d16!            : amX is d16
  amAIndex  reg2            : amEAX is aIndex & reg2
  amAIndex.x iType iReg iSize disp8!: amX is iType & iReg & iSize & disp8
  amPcDisp                  : amEAX is pcDisp
  amPcDisp.x label          : amX { label = PC + d16! } is PC: d16
  amPcIndex                 : amEAX is pcIndex
  amPcIndex.x iType iReg iSize disp8!   : amX is iType & iReg & iSize & disp8
  amAbs.w                   : amEAX is abs.w
  amAbs.w.x d16!            : amX is d16
  amAbs.l                   : amEAX is abs.l
  amAbs.l.x d32             : amX is d16 = d32@[16:31]; d16 = d32@[0:15]
  amImm.b                   : amEAX is Imm & sz = 0x0
  amImm.b.x d8!             : amX is iType = 0 & iReg = 0 & iSize = 0 & null = 0 & disp8 = d8
  amImm.w                   : amEAX is Imm & sz = 0x1
  amImm.w.x d16             : amX is d16
  amImm.l                   : amEAX is Imm & sz = 0x2
  amImm.l.x d32             : amX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Addressing modes used with _A instructions (ADDA/CMPA/SUBA) where the sb
# field, rather than the sz field, determines the size of an immediate
#
  awlDDirect    reg2        : awlEA is dDirect & reg2
  awlADirect    reg2        : awlEA is aDirect & reg2
  awlIndirect   reg2        : awlEA is indirect & reg2
  awlPostInc    reg2        : awlEA is postInc & reg2
  awlPreDec reg2            : awlEA is preDec & reg2
  awlADisp  reg2            : awlEAX is aDisp & reg2
  awlADisp.x    d16!        : awlX is d16
  awlAIndex reg2            : awlEAX is aIndex & reg2
  awlAIndex.x   iType iReg iSize disp8! : awlX is iType & iReg & iSize & disp8
  awlPcDisp                 : awlEAX is pcDisp
  awlPcDisp.x   label       : awlX { label = PC + d16! } is PC: d16
  awlPcIndex                : awlEAX is pcIndex
  awlPcIndex.x  iType iReg iSize disp8! : awlX is iType & iReg & iSize & disp8
  awlAbs.w                  : awlEAX is abs.w
  awlAbs.w.x    d16!        : awlX is d16
  awlAbs.l                  : awlEAX is abs.l
  awlAbs.l.x    d32         : awlX is d16 = d32@[16:31]; d16 = d32@[0:15]
  awlImm.w                  : awlEAX is Imm & sb = 0
  awlImm.w.x    d16         : awlX is d16
  awlImm.l                  : awlEAX is Imm & sb = 1
  awlImm.l.x    d32         : awlX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Control modes
#
  cIndirect reg2            : cEA is indirect & reg2
  cADisp    reg2            : cEAX is aDisp & reg2
  cADisp.x  d16!            : cX is d16
  cAIndex   reg2            : cEAX is aIndex & reg2
  cAIndex.x iType iReg iSize disp8! : cX is iType & iReg & iSize & disp8
  cPcDisp                   : cEAX is pcDisp
  cPcDisp.x label           : cX { label = PC + d16! } is PC: d16
  cPcIndex                  : cEAX is pcIndex
  cPcIndex.x    iType iReg iSize disp8! : cX is iType & iReg & iSize & disp8
  cAbs.w                    : cEAX is abs.w
  cAbs.w.x  d16!            : cX is d16         # Note: d16 sign extended to 32
  cAbs.l                    : cEAX is abs.l
  cAbs.l.x  d32             : cX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Data modes
#
  dDDirect  reg2            : dEA is dDirect & reg2
  dIndirect reg2            : dEA is indirect & reg2
  dPostInc  reg2            : dEA is postInc & reg2
  dPreDec   reg2            : dEA is preDec & reg2
  dADisp    reg2            : dEAX is aDisp & reg2
  dADisp.x  d16!            : dX is d16
  dAIndex   reg2            : dEAX is aIndex & reg2
  dAIndex.x iType iReg iSize disp8!  : dX is iType & iReg & iSize & disp8
  dPcDisp                   : dEAX is pcDisp
  dPcDisp.x label           : dX { label = PC + d16! } is PC: d16
  dPcIndex                  : dEAX is pcIndex
  dPcIndex.x    iType iReg iSize disp8!  : dX is iType & iReg & iSize & disp8
  dAbs.w                    : dEAX is abs.w
  dAbs.w.x  d16!            : dX is d16
  dAbs.l                    : dEAX is abs.l
  dAbs.l.x  d32             : dX is d16 = d32@[16:31]; d16 = d32@[0:15]
  dImm.b                    : dEAX is Imm & sz = 0x0
  dImm.b.x  d8!             : dX is iType = 0 & iReg = 0 & iSize = 0 & null = 0 & disp8 = d8
  dImm.w                    : dEAX is Imm & sz = 0x1
  dImm.w.x  d16             : dX is d16
  dImm.l                    : dEAX is Imm & sz = 0x2
  dImm.l.x  d32             : dX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Data modes except word-immediates only (i.e., DIVU/DIVS/MULU/MULS
# where sz field is constrained as part of opcode)
#
  dWDDirect reg2            : dWEA is dDirect & reg2
  dWIndirect    reg2        : dWEA is indirect & reg2
  dWPostInc reg2            : dWEA is postInc & reg2
  dWPreDec  reg2            : dWEA is preDec & reg2
  dWADisp   reg2            : dWEAX is aDisp & reg2
  dWADisp.x d16!            : dWX is d16
  dWAIndex  reg2            : dWEAX is aIndex & reg2
  dWAIndex.x    iType iReg iSize disp8! : dWX is iType & iReg & iSize & disp8
  dWPcDisp                  : dWEAX is pcDisp
  dWPcDisp.x    label       : dWX { label = PC + d16! } is PC: d16
  dWPcIndex                 : dWEAX is pcIndex
  dWPcIndex.x   iType iReg iSize disp8! : dWX is iType & iReg & iSize & disp8
  dWAbs.w                   : dWEAX is abs.w
  dWAbs.w.x d16!            : dWX is d16
  dWAbs.l                   : dWEAX is abs.l
  dWAbs.l.x d32             : dWX is d16 = d32@[16:31]; d16 = d32@[0:15]
  dWImm.w                   : dWEAX is Imm
  dWImm.w.x d16             : dWX is d16

#
# Same as data modes except byte-immediates only (BTST only)
#
  dBDDirect reg2            : dBEA is dDirect & reg2
  dBIndirect    reg2        : dBEA is indirect & reg2
  dBPostInc reg2            : dBEA is postInc & reg2
  dBPreDec  reg2            : dBEA is preDec & reg2
  dBADisp   reg2            : dBEAX is aDisp & reg2
  dBADisp.x d16!            : dBX is d16
  dBAIndex  reg2            : dBEAX is aIndex & reg2
  dBAIndex.x    iType iReg iSize disp8! : dBX is iType & iReg & iSize & disp8
  dBPcDisp                  : dBEAX is pcDisp
  dBPcDisp.x    label       : dBX { label = PC + d16! } is PC: d16
  dBPcIndex                 : dBEAX is pcIndex
  dBPcIndex.x   iType iReg iSize disp8! : dBX is iType & iReg & iSize & disp8
  dBAbs.w                   : dBEAX is abs.w
  dBAbs.w.x d16!            : dBX is d16
  dBAbs.l                   : dBEAX is abs.l
  dBAbs.l.x d32             : dBX is d16 = d32@[16:31]; d16 = d32@[0:15]
  dBImm.b                   : dBEAX is Imm
  dBImm.b.x d8!             : dBX is iType = 0 & iReg = 0 & iSize = 0 & null = 0 & disp8 = d8

#
# Data Alterable modes
#
  daDDirect reg2            : daEA is dDirect & reg2
  daIndirect    reg2        : daEA is indirect & reg2
  daPostInc reg2            : daEA is postInc & reg2
  daPreDec  reg2            : daEA is preDec & reg2
  daADisp   reg2            : daEAX is aDisp & reg2
  daADisp.x d16!            : daX is d16
  daAIndex  reg2            : daEAX is aIndex & reg2
  daAIndex.x    iType iReg iSize disp8! : daX is iType & iReg & iSize & disp8
  daAbs.w                   : daEAX is abs.w
  daAbs.w.x d16!            : daX is d16
  daAbs.l                   : daEAX is abs.l
  daAbs.l.x d32             : daX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Memory Alterable modes
#
  maIndirect    reg2        : maEA is indirect & reg2
  maPostInc reg2            : maEA is postInc & reg2
  maPreDec  reg2            : maEA is preDec & reg2
  maADisp   reg2            : maEAX is aDisp & reg2
  maADisp.x d16!            : maX is d16
  maAIndex  reg2            : maEAX is aIndex & reg2
  maAIndex.x    iType iReg iSize disp8! : maX is iType & iReg & iSize & disp8
  maAbs.w                   : maEAX is abs.w
  maAbs.w.x d16!            : maX is d16
  maAbs.l                   : maEAX is abs.l
  maAbs.l.x d32             : maX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# Move source modes (same as "any mode" except Imm size is determined by opcode)
#
  msDDirect reg2            : msEA is dDirect & reg2
  msADirect reg2            : msEA is aDirect & reg2
  msIndirect    reg2        : msEA is indirect & reg2
  msPostInc reg2            : msEA is postInc & reg2
  msPreDec  reg2            : msEA is preDec & reg2

  msADisp   reg2            : msEAX is aDisp & reg2
  msADisp.x d16!            : msX is d16
  msAIndex  reg2            : msEAX is aIndex & reg2
  msAIndex.x    iType iReg iSize disp8! : msX is iType & iReg & iSize & disp8
  msPcDisp                  : msEAX is pcDisp
  msPcDisp.x    label       : msX { label = PC + d16! } is PC: d16
  msPcIndex                 : msEAX is pcIndex
  msPcIndex.x   iType iReg iSize disp8! : msX is iType & iReg & iSize & disp8
  msAbs.w                   : msEAX is abs.w
  msAbs.w.x d16!            : msX is d16
  msImm.b                   : msEAX is Imm & op = 1
  msImm.b.x d8!             : msX is iType = 0 & iReg = 0 & iSize = 0 & null = 0 & disp8 = d8
  msImm.w                   : msEAX is Imm & op = 3
  msImm.w.x d16             : msX is d16

  msAbs.l                   : msEAXL is abs.l
  msImm.l                   : msEAXL is Imm & op = 2

#
# Move Destination modes
#

patterns

  [ MDdDirect MDaDirect MDindirect MDpostInc MDpreDec MDaDisp MDaIndex _ ]
    is MDadrm = { 0 to 7 }
    MDabs.w     is  sb = 1 & sz = 3 & reg1 = 0
    MDabs.l     is  sb = 1 & sz = 3 & reg1 = 1

constructors

  mdDDirect reg1            : mdEA is MDdDirect & reg1
  mdADirect reg1            : mdEA is MDaDirect & reg1
  mdIndirect    reg1        : mdEA is MDindirect & reg1
  mdPostInc reg1            : mdEA is MDpostInc & reg1
  mdPreDec  reg1            : mdEA is MDpreDec & reg1

  mdADisp   reg1            : mdEAX is MDaDisp & reg1
  mdADisp.x d16!            : mdX is d16
  mdAIndex  reg1            : mdEAX is MDaIndex & reg1
  mdAIndex.x    iType iReg iSize disp8! : mdX is iType & iReg & iSize & disp8
  mdAbs.w                   : mdEAX is MDabs.w
  mdAbs.w.x d16             : mdX is d16
  mdAbs.l                   : mdEAX is MDabs.l
  mdAbs.l.x d32             : mdX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# MOVEM memory-to-register modes
#
  mrIndirect    reg2        : mrEA is indirect & reg2
  mrPostInc reg2            : mrEA is postInc & reg2
  mrADisp   reg2            : mrEAX is aDisp & reg2
  mrADisp.x d16!            : mrX is d16
  mrAIndex  reg2            : mrEAX is aIndex & reg2
  mrAIndex.x    iType iReg iSize disp8! : mrX is iType & iReg & iSize & disp8
  mrPcDisp                  : mrEAX is pcDisp
  mrPcDisp.x    label       : mrX { label = PC + d16! } is PC: d16
  mrPcIndex                 : mrEAX is pcIndex
  mrPcIndex.x   iType iReg iSize disp8! : mrX is iType & iReg & iSize & disp8
  mrAbs.w                   : mrEAX is abs.w
  mrAbs.w.x d16!            : mrX is d16
  mrAbs.l                   : mrEAX is abs.l
  mrAbs.l.x d32             : mrX is d16 = d32@[16:31]; d16 = d32@[0:15]

#
# MOVEM register-to-memory modes
#
  rmIndirect    reg2        : rmEA is indirect & reg2
  rmPreDec  reg2            : rmEA is preDec & reg2
  rmADisp   reg2            : rmEAX is aDisp & reg2
  rmADisp.x d16!            : rmX is d16
  rmAIndex  reg2            : rmEAX is aIndex & reg2
  rmAIndex.x    iType iReg iSize disp8! : rmX is iType & iReg & iSize & disp8
  rmAbs.w                   : rmEAX is abs.w
  rmAbs.w.x d16!            : rmX is d16
  rmAbs.l                   : rmEAX is abs.l
  rmAbs.l.x d32             : rmX is d16 = d32@[16:31]; d16 = d32@[0:15]


patterns

#
# Patterns for factored constructors
#

  _toCCR    is  andiToCCR | eoriToCCR | oriToCCR
  _toSR     is  andiToSR  | eoriToSR | oriToSR

  _twoRegRB is  addxrb | subxrb | abcdr | sbcdr
  _twoRegRW is  addxrw | subxrw
  _twoRegRL is  addxrl | subxrl
  _twoRegMB is  addxmb | cmpmb | subxmb | abcdm | sbcdm
  _twoRegMW is  addxmw | cmpmw | subxmw
  _twoRegML is  addxml | cmpml | subxml

  _alurB    is  addrb | cmpb  | subrb
  _alurW    is  addrw | cmpw  | subrw
  _alurL    is  addrl | cmpl  | subrl

  _alurdB   is  andrb | orrb
  _alurdW   is  andrw | orrw | chk
  _alurdL   is  andrl | orrl

  _alurdw   is  divs | divu | muls | mulu

  _alumB    is  addmb | andmb | ormb  | submb
  _alumW    is  addmw | andmw | ormw  | submw
  _alumL    is  addml | andml | orml  | subml

  _aluaW    is  addaw | cmpaw | subaw
  _aluaL    is  addal | cmpal | subal

  _aluqB    is  addqb | subqb
  _aluqW    is  addqw | subqw
  _aluqL    is  addql | subql

  _immEAb   is  addib | andib | cmpib | eorib | orib | subib
  _immEAw   is  addiw | andiw | cmpiw | eoriw | oriw | subiw
  _immEAl   is  addil | andil | cmpil | eoril | oril | subil

  _shiftMR  is  asrm | lsrm | rorm | roxrm
  _shiftML  is  aslm | lslm | rolm | roxlm
  _shiftIRB is  asrib | lsrib | rorib | roxrib
  _shiftIRW is  asriw | lsriw | roriw | roxriw
  _shiftIRL is  asril | lsril | roril | roxril
  _shiftILB is  aslib | lslib | rolib | roxlib
  _shiftILW is  asliw | lsliw | roliw | roxliw
  _shiftILL is  aslil | lslil | rolil | roxlil
  _shiftRRB is  asrrb | lsrrb | rorrb | roxrrb
  _shiftRRW is  asrrw | lsrrw | rorrw | roxrrw
  _shiftRRL is  asrrl | lsrrl | rorrl | roxrrl
  _shiftRLB is  aslrb | lslrb | rolrb | roxlrb
  _shiftRLW is  aslrw | lslrw | rolrw | roxlrw
  _shiftRLL is  aslrl | lslrl | rolrl | roxlrl

  _bits     is  bchg  | bclr  | bset
  _bitsi    is  bchgi | bclri | bseti

  _oneEAdaB is  clrb | negb | negxb | notb | tstb | nbcd | tas
  _oneEAdaW is  clrw | negw | negxw | notw | tstw
  _oneEAdaL is  clrl | negl | negxl | notl | tstl

  _oneEAc   is  jsr | jmp | pea

#### is extb missing?
  _reg2only is  extw | extl | swap 

  _noArg    is  illegal | reset | nop | rte | rts | trapv | rtr

  _move     is  moveb | movew | movel

  _uBranch  is  bra | bsr

#
#  Using db^cond notation here somehow would be nice, but it seems that
#  you cannot combine it with the "is any of, which is" notation
#

  _dbcc     is any of   [ dbt  dbf  dbhi dbls dbcc dbcs dbne dbeq
                  dbvc dbvs dbpl dbmi dbge dblt dbgt dble ],
        which is    DBcc & cond = { 0 to 15 }

  _s        is any of   [ st  sf  shi sls scc scs sne seq
                  svc svs spl smi sge slt sgt sle ],
        which is    Scc & cond = { 0 to 15 }

  _br       is any of   [ _   _   bhi bls bcc bcs bne beq
                  bvc bvs bpl bmi bge blt bgt ble ],
        which is    Bcc & cond = { 0 to 15 }


constructors

#
# Factored constructors
#

  _toCCR        d8, "CCR"       is  _toCCR; iType = 0 & iReg = 0 & iSize = 0 &
                                null = 0 & disp8 = d8
  _toSR         d16, "SR"       is  _toSR; d16

  _twoRegRB     reg2, reg1
  _twoRegRW     reg2, reg1
  _twoRegRL     reg2, reg1
  _twoRegMB     reg2, reg1
  _twoRegMW     reg2, reg1
  _twoRegML     reg2, reg1

  _alurB        amEA, reg1      is  _alurB & reg1 & amEA
  _alurB^".ex"  amEAX amX, reg1     is  _alurB & reg1 & amEAX; amX
  _alurW        amEA, reg1      is  _alurW & reg1 & amEA
  _alurW^".ex"  amEAX amX, reg1     is  _alurW & reg1 & amEAX; amX
  _alurL        amEA, reg1      is  _alurL & reg1 & amEA
  _alurL^".ex"  amEAX amX, reg1     is  _alurL & reg1 & amEAX; amX

  _alurdB       dEA, reg1       is  _alurdB & reg1 & dEA
  _alurdB^".ex" dEAX dX, reg1       is  _alurdB & reg1 & dEAX; dX
  _alurdW       dEA, reg1       is  _alurdW & reg1 & dEA
  _alurdW^".ex" dEAX dX, reg1       is  _alurdW & reg1 & dEAX; dX
  _alurdL       dEA, reg1       is  _alurdL & reg1 & dEA
  _alurdL^".ex" dEAX dX, reg1       is  _alurdL & reg1 & dEAX; dX

  _alurdw       dWEA, reg1      is  _alurdw & reg1 & dWEA
  _alurdw^".ex" dWEAX dWX, reg1     is  _alurdw & reg1 & dWEAX; dWX

  _alumB        reg1, maEA      is  _alumB & reg1 & maEA
  _alumB^".ex"  reg1, maEAX maX     is  _alumB & reg1 & maEAX; maX
  _alumW        reg1, maEA      is  _alumW & reg1 & maEA
  _alumW^".ex"  reg1, maEAX maX     is  _alumW & reg1 & maEAX; maX
  _alumL        reg1, maEA      is  _alumL & reg1 & maEA
  _alumL^".ex"  reg1, maEAX maX     is  _alumL & reg1 & maEAX; maX

  _aluaW        awlEA, reg1     is  _aluaW & reg1 & awlEA
  _aluaW^".ex"  awlEAX awlX, reg1   is  _aluaW & reg1 & awlEAX; awlX
  _aluaL        awlEA, reg1     is  _aluaL & reg1 & awlEA
  _aluaL^".ex"  awlEAX awlX, reg1   is  _aluaL & reg1 & awlEAX; awlX

  _aluqB        d3!, alEA       is  _aluqB & reg1 = d3 & alEA
  _aluqB^".ex"  d3!, alEAX alX      is  _aluqB & reg1 = d3 & alEAX; alX
  _aluqW        d3!, alEA       is  _aluqW & reg1 = d3 & alEA
  _aluqW^".ex"  d3!, alEAX alX      is  _aluqW & reg1 = d3 & alEAX; alX
  _aluqL        d3!, alEA       is  _aluqL & reg1 = d3 & alEA
  _aluqL^".ex"  d3!, alEAX alX      is  _aluqL & reg1 = d3 & alEAX; alX

  _immEAb       d8!, daEA       is  _immEAb & daEA; iType = 0 & iReg = 0 & iSize = 0 &
                                   null = 0 & disp8 = d8
  _immEAb^".ex" d8!, daEAX daX      is  _immEAb & daEAX; iType = 0 & iReg = 0 & iSize = 0 &
                                null = 0 & disp8 = d8; daX
  _immEAw       d16, daEA       is  _immEAw & daEA; d16
  _immEAw^".ex" d16, daEAX daX      is  _immEAw & daEAX; d16; daX
  _immEAl       d32, daEA       is  _immEAl & daEA; d16 = d32@[16:31]; d16 = d32@[0:15]
  _immEAl^".ex" d32, daEAX daX      is  _immEAl & daEAX; d16 = d32@[16:31]; d16 = d32@[0:15]; daX

  _shiftMR      maEA            is  _shiftMR & maEA
  _shiftMR^".ex" maEAX maX      is  _shiftMR & maEAX; maX
  _shiftML      maEA            is  _shiftML & maEA
  _shiftML^".ex" maEAX maX      is  _shiftML & maEAX; maX
  _shiftIRB     d3, reg2        is  _shiftIRB & reg1 = d3 & reg2
  _shiftILB     d3, reg2        is  _shiftILB & reg1 = d3 & reg2
  _shiftIRW     d3, reg2        is  _shiftIRW & reg1 = d3 & reg2
  _shiftILW     d3, reg2        is  _shiftILW & reg1 = d3 & reg2
  _shiftIRL     d3, reg2        is  _shiftIRL & reg1 = d3 & reg2
  _shiftILL     d3, reg2        is  _shiftILL & reg1 = d3 & reg2
  _shiftRRB     reg1, reg2
  _shiftRLB     reg1, reg2
  _shiftRRW     reg1, reg2
  _shiftRLW     reg1, reg2
  _shiftRRL     reg1, reg2
  _shiftRLL     reg1, reg2

  _bits         reg1, daEA      is  _bits & reg1 & daEA
  _bits^".ex"   reg1, daEAX daX     is  _bits & reg1 & daEAX; daX
  _bitsi        d8, daEA        is  _bitsi & daEA; iType = 0 & iReg = 0 & iSize = 0 &
                                       null = 0 & disp8 = d8
  _bitsi^".ex"  d8, daEAX daX   is  _bitsi & daEAX; iType = 0 & iReg = 0 & iSize = 0 &
                                    null = 0 & disp8 = d8; daX

  _oneEAdaB     daEA            is  _oneEAdaB & daEA
  _oneEAdaB^".ex" daEAX daX     is  _oneEAdaB & daEAX; daX
  _oneEAdaW     daEA            is  _oneEAdaW & daEA
  _oneEAdaW^".ex" daEAX daX     is  _oneEAdaW & daEAX; daX
  _oneEAdaL     daEA            is  _oneEAdaL & daEA
  _oneEAdaL^".ex" daEAX daX     is  _oneEAdaL & daEAX; daX

  _oneEAc       cEA             is  _oneEAc & cEA
  _oneEAc^".ex" cEAX cX         is  _oneEAc & cEAX; cX

  _reg2only     reg2            is  _reg2only & reg2

  _noArg                        is  _noArg

  _move         msEA, mdEA      is  _move & msEA & mdEA
  _move^".ex"   msEAX msX, mdEA     is  _move & msEAX & mdEA; msX
  _move^".exl"  msEAXL d32, mdEA    is  _move & msEAXL & mdEA; d16 = d32@[16:31]; d16 = d32@[0:15]
  _move^".mx"   msEA, mdEAX mdX     is  _move & msEA & mdEAX; mdX
  _move^".emx"  msEAX msX, mdEAX mdX    is  _move & msEAX & mdEAX; msX; mdX
  _move^".emxl" msEAXL d32, mdEAX mdX   is  _move & msEAXL & mdEAX; d16 = d32@[16:31];
                                        d16 = d32@[0:15]; mdX


#
#  Branches
#

constructors

  _uBranch      BTA         is  BTA & _uBranch ...

  _br           BTA         is  BTA & _br ...

  _dbcc         reg2, d16   is  _dbcc & reg2; d16

  _s            daEA        is  _s & daEA
  _s^".ex"      daEAX daX   is  _s & daEAX; daX


#
# Non-factored constructors
#

  lea           cEA, reg1       is  lea & reg1 & cEA
  lea^".ex"     cEAX cX, reg1   is  lea & reg1 & cEAX; cX

  eorb          reg1, daEA      is  eorb & reg1 & daEA
  eorb^".ex"    reg1, daEAX daX is  eorb & reg1 & daEAX; daX
  eorw          reg1, daEA      is  eorw & reg1 & daEA
  eorw^".ex"    reg1, daEAX daX is  eorw & reg1 & daEAX; daX
  eorl          reg1, daEA      is  eorl & reg1 & daEA
  eorl^".ex"    reg1, daEAX daX is  eorl & reg1 & daEAX; daX

  btst          reg1, dBEA      is  btst & reg1 & dBEA
  btst^".ex"    reg1, dBEAX dBX is  btst & reg1 & dBEAX; dBX
  btsti         d8, dBEA        is  btsti & dBEA; iType = 0 & iReg = 0 & iSize = 0 &
                                      null = 0 & disp8 = d8
  btsti^".ex"   d8, dBEAX dBX   is  btsti & dBEAX; iType = 0 & iReg = 0 & iSize = 0 &
                                       null = 0 & disp8 = d8; dBX

  movepmr       d16,reg2,reg1   is  movepmr & reg1 & reg2; d16
  moveprm       reg1,d16,reg2   is  moveprm & reg1 & reg2; d16

  moveFromSR        "SR", daEA      is  moveFromSR & daEA
  moveFromSR^".ex"  "SR", daEAX daX is  moveFromSR & daEAX; daX

  moveToSR      dWEA, "SR"          is  moveToSR & dWEA
  moveToSR^".ex"    dWEAX dWX, "SR" is  moveToSR & dWEAX; dWX

  moveToCCR     dWEA, "CCR"         is  moveToCCR & dWEA
  moveToCCR^".ex" dWEAX dWX, "CCR"  is  moveToCCR & dWEAX; dWX

  moveFromCCR       "CCR", daEA     is  moveFromCCR & daEA
  moveFromCCR^".ex" "CCR", daEAX daX is moveFromCCR & daEAX; daX

  trap          d4              is  trap & vect = d4

  link          reg2, d16!      is  link & reg2; d16

  moveFromUSP   "USP", reg2     is  moveFromUSP & reg2
  moveToUSP     reg2, "USP"     is  moveToUSP & reg2
    
  stop          d16             is  stop; d16

  movermw       d16, rmEA       is  movermw & rmEA; d16
  movermw^".ex" d16, rmEAX rmX  is  movermw & rmEAX; d16; rmX
  movemrw       mrEA, d16       is  movemrw & mrEA; d16
  movemrw^".ex" mrEAX mrX, d16  is  movemrw & mrEAX; d16; mrX
  moverml       d16, rmEA       is  moverml & rmEA; d16
  moverml^".ex" d16, rmEAX rmX  is  moverml & rmEAX; d16; rmX
  movemrl       mrEA, d16       is  movemrl & mrEA; d16
  movemrl^".ex" mrEAX mrX, d16  is  movemrl & mrEAX; d16; mrX

  moveq         d8!, reg1       is  moveq & reg1 & data8 = d8

  exgdd         reg1, reg2
  exgaa         reg1, reg2
  exgda         reg1, reg2

  unlk          reg2    


#
# Constructors for procedure linkage
#
# Ordinary call. reloc is set to HOST address
  call_    reloc  { reloc = L + 2 + d16! }
                        is L: jsr & pcDisp ; d16
# ?
  regCall  cEA          is jsr & notPcDisp & cEA
  regJmp   cEA          is jmp & notPcDisp & cEA
# pea num(pc)
  peaPcDisp d16!        is pea & pcDisp; d16
# lea d16(a7),a7 (often used to restore stack after a call)
  leaSpSp   d16!        is lea & aDisp & reg1 = 7 & reg2 = 7; d16
# An aline instruction, e.g. one with opcode 0xA123 (here, d12 set to 0x123)
  Aline     d12         is Aline & bot12 = d12
# Push a single register, e.g. movel d3,-(a7)
  pushreg   reg         is movel & dDirect & MDpreDec & reg1 = 7 & reg2 = reg
# Pop a single register, e.g. movel (a7)+,d3
  popreg    reg         is movel & postInc & reg2 = 7 & MDdDirect & reg2 = reg
# addaw #d16, a7
  addaw_d16 d16!        is addaw & Imm & reg1 = 7; d16


#
# Constructors that are part of the MC68328 instruction set
# 

keep

# ABCD, ADDX, CMPM, SBCD, SUBX
    _twoRegRB
    _twoRegRW
    _twoRegRL
    _twoRegMB
    _twoRegMW
    _twoRegML

# ADD, AND, CHK, CMP, CMPA, DIVS, DIVU, MULS, MULU, OR, SUB, SUBA  
    _alurB
    _alurB^".ex"
    _alurW
    _alurW^".ex"
    _alurL
    _alurL^".ex"
    _alurdB
    _alurdB^".ex"
    _alurdW
    _alurdW^".ex"
    _alurdL
    _alurdL^".ex"
    _alurdw
    _alurdw^".ex"
    _alumB
    _alumB^".ex"
    _alumW
    _alumW^".ex"
    _alumL
    _alumL^".ex"

# ADDA, CMPA, SUBA
    _aluaW
    _aluaW^".ex"
    _aluaL
    _aluaL^".ex"

# ADDQ, SUBQ
    _aluqB
    _aluqB^".ex"
    _aluqW
    _aluqW^".ex"
    _aluqL
    _aluqL^".ex"

# ADDI, ANDI, CMPI, EORI, ORI, SUBI
    _immEAb
    _immEAb^".ex"
    _immEAw
    _immEAw^".ex"
    _immEAl
    _immEAl^".ex"

# ANDI to CCR, EORI to CCR, ORI to CCR  
    _toCCR

# ANDI to SR, EORI to SR, ORI to SR
    _toSR

# ASL, ASR, LSL, LSR, ROL, ROR, ROXL, ROXR 
    _shiftMR
    _shiftMR^".ex"
    _shiftML
    _shiftML^".ex"
    _shiftIRB
    _shiftIRW
    _shiftIRL
    _shiftILB
    _shiftILW
    _shiftILL
    _shiftRRB
    _shiftRRW
    _shiftRRL
    _shiftRLB
    _shiftRLW
    _shiftRLL

# Bcc  
    _br

# BRA, BSR
    _uBranch

# BCHG, BCLR, BSET
    _bits
    _bits^".ex"
    _bitsi  
    _bitsi^".ex"    

# BTST 
    btst
    btst^".ex"
    btsti
    btsti^".ex"

# CLR, NBCD, NEG, NEGX, NOT, TAS, TST
    _oneEAdaB
    _oneEAdaB^".ex"
    _oneEAdaW
    _oneEAdaW^".ex"
    _oneEAdaL
    _oneEAdaL^".ex"

# DBcc
    _dbcc

# EOR
    eorb
    eorb^".ex"
    eorw
    eorw^".ex"
    eorl
    eorl^".ex"

# EXG
    exgdd
    exgaa
    exgda

# EXT, SWAP
    _reg2only

# ILLEGAL, NOP, RESET, RTE, RTR, RTS, TRAPV (ILLEGAL not needed)
    _noArg

# JMP, JSR, PEA
    _oneEAc
    _oneEAc^".ex"   

# LEA
    lea
    lea^".ex"

# LINK 
    link

# MOVE
    _move
    _move^".ex"
    _move^".exl"
    _move^".mx"
    _move^".emx"
    _move^".emxl"

# MOVEA
### which ones are these? 

# MOVEM
    movermw
    movermw^".ex"
    movemrw
    movemrw"^.ex"
    moverml
    moverml^".ex"
    movemrl
    movemrl^".ex"

# MOVEP
    movepmr
    moveprm

# MOVEQ
    moveq

# MOVE from SR, MOVE to SR, MOVE to CCR, MOVE from CCR 
    moveFromSR
    moveFromSR^".ex"
    moveToSR
    moveToSR^".ex"
    moveToCCR
    moveToCCR^".ex"
    moveFromCCR
    moveFromCCR^".ex"

# MOVE USP 
    moveFromUSP
    moveToUSP

# Scc
    _s
    _s^".ex"

# STOP
    stop

# TRAP 
    trap

# UNLK
    unlk
