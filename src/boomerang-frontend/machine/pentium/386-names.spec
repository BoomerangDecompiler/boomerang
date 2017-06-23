#
# Copyright (C) 1996?? Norman Ramsey???
# Copyright (C) 1998, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# Jan 98 - Cristina
#	original file by Norman pentium-names.spec
#	removed all constructors dealing with floating point as per 386-core.spec.
# 2 Feb 98 - Cristina
# 	changed argument of MOVZX and MOVSX from Eaddr to Mem as per 386-core.spec.
#	toolkit emits warning for LDS, LES, LFS, LGS and LSS (10 combinations) 
#		as these names are not in this spec.  It doesn't matter as the
#		generated name for these is the right one (i.e. LDS, LES, etc).


assembly component 
    {iAL,AL}                is b
    {iAX,AX}                is w
    {iEAX,eAX}              is l
    {o,a}d                  is l
    {o,a}w                  is w
    {.I32,.R64,.lsI32,.lsR64} is l
    {.I16,.R32,.lsR32}      is s
    .lsI16                  is w
    b.*                     is b
    {b,w}                   is $1
    d                       is l
    B.{Eb.1,Eb.CL,Eb.Ib,Ev.Ib}   is b
    B.{Ev.1,Ev.CL}          is w
    {.STi,.ST.STi,.STi.St}  is ""
    P.STi.ST                is P
    .{O,NO,B,NB,Z,NZ,BE,NBE,S,NS,P,NP,L,NL,LE,NLE} is $1
assembly component
    {CALL}.*                is $1
    {CALL}l                 is $1
    CMPXCHG8B               is CMPXCHG
    {CMPXCHG,XADD,XCHG,TEST}.Eb.Gb is $1b
    CMPSv                   is CMPS
    {CMP*}.*                is $1
    {DEC,INC}.*             is $1
    {DIV}.*                 is $1
    {*}.st                  is $1
    {IDIV,IMUL}.*           is $1
    {IN,INT,J}.*            is $1
    JMP.Ep                  is lJMP
    {JMP}.*                 is $1
    MOV{.Eb.Ib,.AL.Ob,.Ob.AL} is MOVb
    {MOViv,MOV.Ev.Iv}       is MOV
    MOVSX.Gv.Ew             is MOVSwl
    {MOV.Ew.Sw,MOV.Sw.Ew} is MOVw
    {MOVS,MOVZ}X.Gv.Eb      is $1b
    {MOVSv,MOVSX.*}         is MOVS
    {MOV,MOVS}.*            is $1
    {MOVSX,MOVZX}.*         is $1
    MOVi{b,w}               is MOV$1
    MOVid                   is MOVl
    {*}.AX                  is $1
    {OUT.Ib.AL,OUT.DX.AL}   is OUTb
    {OUT,OUTS}.*            is $1
    {RET.far}*              is lRET
    {POP,PUSH,RET}.*        is $1
    {SCAS,STOS}v            is $1
    {SHRD,SHLD}.*           is $1
    SHRSAL                  is SHR
    TEST{.*.Ib,.Eb.*}       is TESTb
    TEST.*.Iw               is TESTw
    TEST.*.Id               is TESTl
    TEST.*                  is TEST
    {XADD*}.*               is $1
    {XCHG*}.*               is $1
    {*}i                    is $1
assembly component 
    {"mr","rm"}             is ""
    {"mrb","rmb"}           is b
    {*}64                   is $1
    {IDIV,DIV}"AL"          is $1
    {IDIV,DIV}"AX"          is $1
    {IDIV,DIV}"eAX"         is $1
    {IMULrm}                is IMUL
    INT3                    is INT
    FLD.ext                 is FLDLL
    {Jv,Jb}                 is J
    {INC}.Eb                is INCb
    {INS,LODS}v             is $1
    MUL.AL                  is MULb
    OUTSv                   is OUTS
    SHLSAL                  is SHL
    TEST.Ew.Iw              is TESTw
    TEST.Ed.Id              is TESTl
    SETb                    is SET
assembly opcode
  CALL.{Ev}{od}	            is CALL
  CALL.{Jv,Ep}{od,ow}       is lCALL
  CALL.aP{od}               is CALL
  CMPSv{od,ow}ad            is CMPSl
  CMPSv{od,ow}aw            is CMPSw
  JMP.Epod                  is lJMP
  MOVSX.Gv.Ebod             is MOVSbl
  MOVSX.Gv.Ebow		    is MOVSbw
  {ROL,ROR,RCL,RCR,SHR,SAR}{B.Ev.*}od is $1l
  {ROL,ROR,RCL,RCR,SHR,SAR}{B.Ev.*}ow is $1w
  SHLSAL{B.Ev.*}od          is SHLl
  SHLSAL{B.Ev.*}ow          is SHLw
  XCHGeAXow                 is XCHGw
  XCHGeAXod                 is XCHGl
assembly operand
    [count i8 i16 i32]           is "$%d"
    [r32 sr16 r16 r8 base index] is "%%%s"
assembly operand
    [reg reg8 sreg cr dr]      	is "%%%s" using field base
assembly operand
    dx    is "%%dx"
    ax    is "%%ax"
assembly component 
    {Indir,{Disp*},Abs32,Reg,{*Index*},E,rel{8,16,32}} is ""
    {*}  is   	$1
assembly syntax
  arith^"iAL"     i8!, "%al"
  arith^"iAX"     i16!, "%ax"
  arith^"iEAX"    i32!, "%eax"
  DIV^"AL"       Eaddr, "%al"
  DIV^"AX"       Eaddr, "%ax"
  DIV^"eAX"      Eaddr, "%eax"

  arithI^"b"     i8!,  Eaddr
  arithI^"w"     i16!, Eaddr
  arithI^"d"     i32!, Eaddr
  arithI^ov^"b"  i8!,  Eaddr
  MOV.Eb.Ib      i8!,  Eaddr
  MOV.Ev.Iv^ow   i16!, Eaddr
  MOV.Ev.Iv^od   i32!, Eaddr
assembly syntax
  arith^"rmb"    Eaddr, reg8
  arith^"rm"^ov  Eaddr, reg
  IMULrm^ov      Eaddr, reg
  MOV^"rmb"      Eaddr, reg
  MOV^"rm"^ov    Eaddr, reg
  MOVZX.Gv.Ew    Mem, r16
  MOVSX.Gv.Ew    Mem, r16
  MOVZX.Gv.Eb^ov Mem, r32
  MOVSX.Gv.Eb^ov Mem, r32
  BSF^ov  Eaddr, reg
  BSR^ov  Eaddr, reg
  LAR^ov  Eaddr, reg

  arith^"mrb"   reg8, Eaddr
  arith^"mr"^ov  reg, Eaddr
  MOV^"mr"^ov    reg, Eaddr
  MOV^"mrb"      reg, Eaddr
  TEST.Ev.Gv^ov  reg, Eaddr
  BT^ov          reg, Eaddr
  BTi^ov         i8!, Eaddr
  BTC^ov         reg, Eaddr
  BTCi^ov        i8!, Eaddr
  BTR^ov         reg, Eaddr
  BTRi^ov        i8!, Eaddr
  BTS^ov         reg, Eaddr
  BTSi^ov        i8!, Eaddr
  CMPXCHG.Eb.Gb  reg, Eaddr
  CMPXCHG.Ev.Gv^ov reg, Eaddr

assembly syntax
  IDIV^"AX"     Eaddr, "%ax"
  IDIV^"eAX"    Eaddr, "%eax"

  IN.AL.Ib     i8!, "%al", i8!
  IN.eAX.Ib^ov i8!, "%eax", i8!
  IN.AL.DX     "%dx, %al"
  IN.eAX.DX^ov "%dx, %eax"

  IMUL.Iv^"d"    i32!,  Eaddr,  reg 
  INT3           "$3"
  lfp^ov         Mem, reg
  LEA^ov         Mem, reg

  MOVib          i8!, r8
  MOViw          i16!, r16
  MOVid          i32!, r32

  MOV.AL.Ob      offset, "%al"
  MOV.eAX.Ov^ov  offset, "%eax"
  MOV.Ob.AL      "%al", offset
  MOV.Ov.eAX^ov  "%eax", offset

  OUT.Ib.AL      "%al", i8!
  OUT.Ib.eAX^ov  "%eax", i8!
  OUT.DX.AL      "%al", "%dx"
  OUT.DX.eAX^ow  "%al", "%dx"
  OUT.DX.eAX^od  "%eax", "%dx"

patterns
  pES is POP.ES | PUSH.ES
  pSS is POP.SS | PUSH.SS
  pDS is POP.DS | PUSH.DS
  pFS is POP.FS | PUSH.FS
  pGS is POP.GS | PUSH.GS
assembly syntax
  pES "%ES"
  pSS "%SS"
  pDS "%DS"
  pFS "%FS"
  pGS "%GS"
  PUSH.CS "%CS"

  rot^B.Eb.1     "$1", Eaddr
  rot^B.Ev.1^ov  "$1", Eaddr

  rot^B.Eb.CL    "%cl", Eaddr
  rot^B.Ev.CL^ov "%cl", Eaddr

  rot^B.Eb.Ib    i8!, Eaddr
  rot^B.Ev.Ib^ov i8!, Eaddr
 
  shdIb^ov  count, reg, Eaddr
  shdCL^ov  "%cl", reg, Eaddr

  TEST.AL.Ib     i8, "%al"
  TEST.eAX.Iv^ow i16, "%ax"
  TEST.eAX.Iv^od i32, "%ax"
  TEST.Eb.Ib     i8,  Eaddr
  TEST.Ew.Iw     i16, Eaddr
  TEST.Ed.Id     i32, Eaddr
  TEST.Eb.Gb     reg, Eaddr
  TEST.Ev.Gv^ov  reg, Eaddr
  XADD.Eb.Gb     reg, Eaddr
  XADD.Ev.Gv^ov  reg, Eaddr
  XCHG.Eb.Gb     reg, Eaddr
  XCHG^"eAX"^ov  "%eax", r32
  XCHG.Ev.Gv^ov  reg, Eaddr

assembly syntax
  Indir       (reg)
  Disp32      d(reg)
  Index       (base,index,ss)
  Index32     d(base,index,ss)
  ShortIndex  d(,index,ss)

