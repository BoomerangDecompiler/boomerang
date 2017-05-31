#
# Copyright (C) 1996???, Norman Ramsey ???
# Copyright (C) 1998,2000, The University of Queensland
# Copyright (C) 2001, Sun Microsystems, Inc
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# FILE:     sparc-core.spec
# OVERVIEW: This is the New Jersey Machine Code Toolkit core specification file
#           for the Sparc V8 processor
#
# $Revision$
#
# 3 Mar 98 - Cristina
#	changed branch^a constructor to return the 'a' value as well.
# 24 Jan 00 - Mike
#   changes to ensure that double and quad registers get the correct names
# 11 Feb 01 - Nathan: Fixed operand names for FTOd and FTOq
# 11 Feb 01 - Nathan: Renamed decode_sethi to sethi, and sethi to encode_sethi
# 22 Nov 02 - Mike: pbranch (V9 branches with prediction); RETT -> RETURN


fields of instruction (32) 
inst 0:31 op 30:31 disp30 0:29 rd 25:29 op2 22:24 imm22 0:21 a 29:29 cond 25:28
disp22 0:21 op3 19:24 rs1 14:18 i 13:13 asi 5:12 rs2 0:4 simm13 0:12 opf 5:13
cd 25:29 disp19 0:18 cc01 20:21
fds 25:29 fs1s 14:18 fs2s 0:4
fdd 25:29 fs1d 14:18 fs2d 0:4
fdq 25:29 fs1q 14:18 fs2q 0:4
fieldinfo
[ rd rs1 rs2 ] is [ 
names [ "%g0"  "%g1"  "%g2"  "%g3"  "%g4"  "%g5"  "%g6"  "%g7"
        "%o0"  "%o1"  "%o2"  "%o3"  "%o4"  "%o5"  "%sp"  "%o7"
        "%l0"  "%l1"  "%l2"  "%l3"  "%l4"  "%l5"  "%l6"  "%l7"
        "%i0"  "%i1"  "%i2"  "%i3"  "%i4"  "%i5"  "%fp"  "%i7" ]
                                                               ]
[ fds fs1s fs2s ] is [ 
names [ "%f0"  "%f1"  "%f2"  "%f3"  "%f4"  "%f5"  "%f6"  "%f7"
        "%f8"  "%f9"  "%f10" "%f11" "%f12" "%f13" "%f14" "%f15"
        "%f16" "%f17" "%f18" "%f19" "%f20" "%f21" "%f22" "%f23"
        "%f24" "%f25" "%f26" "%f27" "%f28" "%f29" "%f30" "%f31" ]
                                                                ]
[ fdd fs1d fs2d ] is [ 
names [ "%f0to1"   "a" "%f2to3"   "b" "%f4to5"   "c" "%f6to7"   "d"
        "%f8to9"   "e" "%f10to11" "f" "%f12to13" "g" "%f14to15" "h"
        "%f16to17" "i" "%f18to19" "j" "%f20to21" "k" "%f22to23" "l"
        "%f24to25" "m" "%f26to27" "n" "%f28to29" "o" "%f30to31" "p" ]
                                                                ]
[ fdq fs1q fs2q ] is [ 
names [ "%f0to3"   "q"  "r"  "s"  "%f4to7"   "t"  "u"  "v"
        "%f8to11"  "w"  "x"  "y"  "%f12to15" "z"  "A"  "B"
        "%f16to19" "C"  "D"  "E"  "%f20to23" "F"  "G"  "H"
        "%f24to27" "I"  "J"  "K"  "%f28to31" "L"  "M"  "N" ]
                                                     ]
            cd is [ 
names [ "%c0"  "%c1"  "%c2"  "%c3"  "%c4"  "%c5"  "%c6"  "%c7"
        "%c8"  "%c9"  "%c10" "%c11" "%c12" "%c13" "%c14" "%c15"
        "%c16" "%c17" "%c18" "%c19" "%c20" "%c21" "%c22" "%c23"
        "%c24" "%c25" "%c26" "%c27" "%c28" "%c29" "%c30" "%c31" ]
                                                                ]
fieldinfo a is [ names [ "" ",a" ] ]
patterns
 [ TABLE_F2 CALL TABLE_F3 TABLE_F4 ] is op  = {0 to 3}
patterns
 [ UNIMP Bpcc Bicc SETHI FBfcc CBccc ] is TABLE_F2 & op2 = [0 1 2 4 6 7]
 NOP                              is SETHI & rd = 0 & imm22 = 0
patterns
 [ ADD  ADDcc  TADDcc   WRxxx
   AND  ANDcc  TSUBcc   WRPSR
   OR   ORcc   TADDccTV WRWIM
   XOR  XORcc  TSUBccTV WRTBR
   SUB  SUBcc  MULScc   FPop1
   ANDN ANDNcc SLL      FPop2
   ORN  ORNcc  SRL      CPop1
   XNOR XNORcc SRA      CPop2
   ADDX ADDXcc RDxxx    JMPL
   _    _      RDPSR    RETURN
   UMUL UMULcc RDWIM    Ticc
   SMUL SMULcc RDTBR    FLUSH
   SUBX SUBXcc MOVcc    SAVE
   _    _      _        RESTORE
   UDIV UDIVcc _        _
   SDIV SDIVcc _        _       ] is TABLE_F3 & op3 = {0 to 63 columns 4}
patterns
  WRASR          is WRxxx & rd != 0   # should be rdi != 0
  WRY            is WRxxx & rd = 0
  RDASR          is RDxxx & rs1 != 0  # should be rs1i != 0
  RDY            is RDxxx & rs1 = 0
  STBAR          is RDxxx & rs1 = 15 & rd = 0
patterns
 [ LD     LDA     LDF   LDC
   LDUB   LDUBA   LDFSR LDCSR
   LDUH   LDUHA   _ _
   LDD    LDDA    LDDF  LDDC
   ST     STA     STF   STC
   STB    STBA    STFSR STCSR
   STH    STHA    STDFQ STDCQ
   STD    STDA    STDF  STDC
   _      _       _     _
   LDSB   LDSBA   _     _
   LDSH   LDSHA   _     _
   _      _       _     _
   _      _       _     _
   LDSTUB LDSTUBA _     _
   _      _       _     _
   SWAP.  SWAPA   _     _  ]  is TABLE_F4 & op3 = {0 to 63 columns 4}
patterns
  float2 is any of [ FMOVs FNEGs FABSs FSQRTs FSQRTd FSQRTq
                     FiTOs FdTOs FqTOs FiTOd  FsTOd  FqTOd
                     FiTOq FsTOq FdTOq FsTOi  FdTOi  FqTOi ],
  which is FPop1 & opf =  
                   [ 0x1   0x5   0x9   0x29   0x2a   0x2b
                     0xc4  0xc6  0xc7  0xc8   0xc9   0xcb
                     0xcc  0xcd  0xce  0xd1   0xd2   0xd3 ]
  float2s is FMOVs | FNEGs | FABSs | FSQRTs
  FTOs    is FiTOs | FsTOi
  FTOd    is FiTOd | FsTOd 
  FTOq    is FiTOq | FsTOq 
  FdTO    is FdTOi | FdTOs 
  FqTO    is FqTOs | FqTOi

  float3 is any of [ FADDs FADDd FADDq FSUBs FSUBd FSUBq  FMULs
                     FMULd FMULq FDIVs FDIVd FDIVq FsMULd FdMULq ],
    which is FPop1 & opf =
                   [ 0x41  0x42  0x43  0x45  0x46  0x47   0x49
                     0x4a  0x4b  0x4d  0x4e  0x4f  0x69   0x6e ]
  float3s is  FADDs | FSUBs | FMULs | FDIVs
  float3d is  FADDd | FSUBd | FMULd | FDIVd
  float3q is  FADDq | FSUBq | FMULq | FDIVq
patterns
 fcompares is any of      [ FCMPs FCMPEs ],
   which is FPop2 & opf = [ 0x51  0x55 ]
 fcompared is any of      [ FCMPd FCMPEd ],
   which is FPop2 & opf = [ 0x52  0x56 ]
 fcompareq is any of      [ FCMPq FCMPEq ],
   which is FPop2 & opf = [ 0x53  0x57 ]
patterns
  ibranch is any of [ BN BE  BLE BL  BLEU BCS BNEG BVS
                      BA BNE BG  BGE BGU  BCC BPOS BVC ],
    which is Bicc & cond = {0 to 15}

  pbranch is any of [ BPN BPE  BPLE BPL  BPLEU BPCS BPNEG BPVS
                      BPA BPNE BPG  BPGE BPGU  BPCC BPPOS BPVC ],
    which is Bpcc & cond = {0 to 15}

  fbranch is any of [ FBN FBNE FBLG FBUL FBL   FBUG FBG   FBU
                      FBA FBE  FBUE FBGE FBUGE FBLE FBULE FBO ],
    which is FBfcc & cond = {0 to 15}

  cbranch is any of [ CBN CB123 CB12 CB13 CB1   CB23 CB2   CB3
                      CBA CB0   CB03 CB02 CB023 CB01 CB013 CB012 ],
    which is CBccc & cond = {0 to 15}

  trap is any of    [ TN TE  TLE TL  TLEU TCS TNEG TVS
                      TA TNE TG  TGE TGU  TCC TPOS TVC ],
    which is Ticc & cond = {0 to 15
}
  branch is ibranch | fbranch | cbranch
constructors
  dispA     rs1 + simm13! : address_  is  i = 1 & rs1 & simm13
  absoluteA simm13!       : address_  is  i = 1 & rs1 = 0 & simm13
  indexA    rs1 + rs2     : address_  is  i = 0 & rs1 & rs2
  indirectA rs1           : address_  is  i = 0 & rs2 = 0 & rs1
constructors
  imode simm13! : reg_or_imm  is  i = 1 & simm13
  rmode rs2     : reg_or_imm  is  i = 0 & rs2
patterns 
  loadg  is LDSB  | LDSH  | LDUB  | LDUH  | LD  | LDSTUB  | SWAP.
  loada  is LDSBA | LDSHA | LDUBA | LDUHA | LDA | LDSTUBA | SWAPA
  storeg is STB   | STH  | ST 
  storea is STBA  | STHA | STA
constructors
  loadg  [address_], rd
  LDD    [address_], rd    # { rd = 2 * _ }
  LDF    [address_], fds
  LDDF   [address_], fdd    # { fd = 2 * _ }
  LDC    [address_], cd
  LDDC   [address_], cd    # { cd = 2 * _ }

  storeg rd, [address_]
  STD    rd, [address_]    # { rd = 2 * _ }
  STF    fds, [address_]
  STDF   fdd, [address_]    # { fd = 2 * _ }
  STC    cd, [address_]
  STDC   cd, [address_]    # { cd = 2 * _ }
constructors
  indexR    rs1 + rs2     : regaddr  is  i = 0 & rs1 & rs2
  indirectR rs1           : regaddr  is  i = 0 & rs2 = 0 & rs1

  loada  [regaddr]asi, rd
  LDDA   [regaddr]asi, rd # { rd = 2 * _ }
  storea rd, [regaddr]asi
  STDA   rd, [regaddr]asi # { rd = 2 * _ }
constructors
  LDFSR  [address_], "%fsr"
  LDCSR  [address_], "%csr"
  STFSR  "%fsr", [address_]
  STCSR  "%csr", [address_]
  STDFQ  "%fq",  [address_]
  STDCQ  "%cq",  [address_]
constructors
  RDY    "%y",   rd
  RDPSR  "%psr", rd
  RDWIM  "%wim", rd
  RDTBR  "%tbr", rd
  WRY    rs1, reg_or_imm, "%y"
  WRPSR  rs1, reg_or_imm, "%psr"
  WRWIM  rs1, reg_or_imm, "%wim"
  WRTBR  rs1, reg_or_imm, "%tbr"
constructors
  RDASR   "%asr"rs1, rd
  WRASR   rs1, reg_or_imm, "%asr"rd
  STBAR
patterns 
  logical is AND | ANDcc | ANDN | ANDNcc | OR | ORcc | ORN | ORNcc |
             XOR | XORcc | XNOR | XNORcc
  shift   is SLL | SRL   | SRA
  arith   is ADD | ADDcc | ADDX | ADDXcc | TADDcc | TADDccTV |
             SUB | SUBcc | SUBX | SUBXcc | TSUBcc | TSUBccTV |
             MULScc | UMUL | SMUL | UMULcc | SMULcc |
             UDIV | SDIV | UDIVcc | SDIVcc |
             SAVE | RESTORE
  alu     is logical | shift | arith

constructors
  alu rs1, reg_or_imm, rd
placeholder for instruction is UNIMP & imm22 = 0xbad
relocatable reloc
constructors
  branch^",a"       reloc { reloc = L + 4 * disp22! } is L: branch & a = 1 & disp22
  branch            reloc { reloc = L + 4 * disp22! } is L: branch & a = 0 & disp22
  pbranch^",a" cc01,reloc { reloc = L + 4 * disp19! } is L: pbranch & a=1 & cc01 & disp19
  pbranch      cc01,reloc { reloc = L + 4 * disp19! } is L: pbranch & a=0 & cc01 & disp19
constructors
  call__  reloc   { reloc = L + 4 * disp30! } is L: CALL & disp30
constructors
  float2s fs2s, fds 
  FSQRTd  fs2d, fdd # { fs2 = 2 * _, fd = 2 * _ }
  FSQRTq  fs2q, fdq # { fs2 = 4 * _, fd = 4 * _ }

  FTOs fs2s, fds
  FTOd fs2s, fdd  # { fd = 2 * _ }
  FTOq fs2s, fdq  # { fd = 4 * _ }
  FdTO fs2d, fds  # { fs2 = 2 * _ }
  FqTO fs2q, fds  # { fs2 = 4 * _ }
  FqTOd fs2q, fdd  # { fs2 = 4 * _, fd = 2 * _ }
  FdTOq fs2d, fdq  # { fs2 = 2 * _, fd = 4 * _ }

  float3s  fs1s, fs2s, fds
  float3d  fs1d, fs2d, fdd # { fs1 = 2 * _, fs2 = 2 * _, fd = 2 * _ }
  float3q  fs1q, fs2q, fdq # { fs1 = 4 * _, fs2 = 4 * _, fd = 4 * _ }
  FsMULd   fs1d, fs2d, fdd # { fd = 4 * _ }
  FdMULq   fs1q, fs2q, fdq # { fs1 = 2 * _, fs2 = 2 * _, fd = 4 * _ }

  fcompares fs1s, fs2s
  fcompared fs1d, fs2d # { fs1 = 2 * _, fs2 = 2 * _ }
  fcompareq fs1q, fs2q # { fs1 = 4 * _, fs2 = 4 * _ }
constructors
  NOP
  FLUSH  address_
  JMPL   address_, rd
  RETURN address_
  trap   address_
  UNIMP  imm22
constructors
#  encode_sethi "%hi("val")", rd                   is  SETHI & rd & imm22 = val@[10:31]
  sethi "%hi("val")", rd { val@[0:9] = 0 } is  SETHI & rd & imm22 = val@[10:31]
