#
# Copyright (C) 2000-2001, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

#===============================================================================
# FILE:        hppa.spec
# OVERVIEW:    New Jersey Machine Code Toolkit specification file for the
#              HP Pa/risc architecture (basically version 1.1)
#===============================================================================

# $Revision$
#  April 01 - Simon: Initial revision
# 01 May 01 - Simon: Fixed a problem with arith_imm's (bit numbers wrong?)
# 04 May 01 - Mike: Merged two .spec files
# 07 May 01 - Mike: Split [LD|ST]Wlm into lma and lmb depending on sign of im14
# 08 May 01 - Mike: New constructor for [LD|ST]Wlm (ldispa16m[ab]_addr
# 08 May 01 - Cristina: Cross-referenced table letters from Appendix C, 
#				PA-RISC 2.0 Architecture book
# 18 Jun 01 - Mike: Added Floating Point instuctions
# 26 Jun 01 - Mike: Changed some mnemonics from 2.0 to 1.1 form
# 27 Jun 01 - Mike: Major rewrite of integer loads and stores, in preparation
#               for "lambdas" in SSL file
# 19 Jul 01 - Simon: Reorganised loads and stores (again) and grouped addresses
#               for both.
# 23 Jul 01 - Simon: COMPICLR, addr_ldisp_17_old
# 25 Jul 01 - Simon: Added shift, extract, deposit
# 27 Jul 01 - Mike: Added XMPYU pattern and constructor; x_23 for flt_c3.E
# 07 Aug 01 - Simon: Changed iloads and istores to take more parameters, so
#               that the addressing mode details are explicit in the SSL now
# 10 Aug 01 - Simon: fixed glitch in the bb_all constructor


# The convention for bit fields is aann_mm, where aa briefly describes the
# field, nn is the field length in bits, and mm is the starting bit number
# (in HP numbering, i.e. 0 at the left)
# Example: In the HP book, the im11 field of ADDI extends from bits 21 to 31.
# The sign bit is separated (always in bit 31), so we name the field from bits
# 21 to 30 as im10_21 (10 bits starting at 21)

# instruction fields go [0..31] left-->right
bit 0 is most significant


fields of instruction (32)
    inst 00:31
      op 00:05
# piecewise opcode
#   op_00 00:00  op_01 01:01  op_02 02:02  op_03 03:03  op_04 04:04  op_05 05:05

    b_06 06:10

# arithmetic/logical instructions (arith/log)
    r_06 06:10    r_11 11:15    c3_16 16:18    f_19 19:19
 ext6_20 20:25    d_26 26:26     t_27 27:31 im21_11 11:31
  im2_16 16:17  im2_18 18:19  im11_20 20:30

# indexed and short displacement load/store instructions (IndexMem)
#   r_11 11:15   t_27 27:31
    x_11 11:15  im5_11 11:15   s2_16 16:17    u_18 18:18    a_18 18:18
 addr_19 19:19   cc_20 20:21 ext4_22 22:25    m_26 26:26  im4_27 27:30

# unconditional branch instructions
#   x_11 11:15  w11_19 19:29
    t_06 06:10   w5_06 06:10   w5_11 11:15 ext3_16 16:18   ve_19 19:19  w10_19 19:28
    w_29 29:29   rv_20 20:29    n_30 30:30    w_31 31:31   rv_31 31:31    p_31 31:31

# arithmetic immediate instructions
#   r_06 06:10   c3_16 16:18    f_19 19:19
    t_11 11:15  ext_20 20:20 im10_21 21:30
 
# long displacement load/store instructions (load_dw/store_dw/load_w/store_w)
#   t_11 11:15   s2_16 16:17
 im10_18 18:27 im13_18 18:30 im11_18 18:28
    m_28 28:28
    a_29 29:29  ext_30 30:30    i_31 31:31
  ext_29 29:29

# system operation instructions
#   r_11 11:15 ext5_06 06:10    t_27 27:31  im5_27 27:31   s2_16 16:18
   cr_06 06:10               im13_06 06:18
 im10_06 06:15 ext5_11 11:15   s3_16 16:18
    s_18 18:18 ext8_19 19:26  ext_17 17:17
 
# branch on bit instructions
    p_06 06:10    c_16 16:16     e_17 17:17    d_18 18:18

# floating point instructions
  sub_16 16:18  fmt_19 19:20 class_21 21:22    x_23 23:23
   r1_24 24:24    t_25 25:25     n_26 26:26  sub_14 14:16
   df_17 17:18   sf_19 19:20     f_20 20:20    r_27 27:31   r_25 25:25

# Coprocessor (including floating point) Loads and Stores
 addr_22 22:22 uid2_23 23:24   im4_11 11:14 im1_15 15:15

# Misc (e.g. BREAK)
  im5_27 27:31

# Shift, Extract, Deposit instructions
   cl_19 19:19    p_20 20:20    se_21 21:21   cl_23 23:23 pos5_22 22:26 
clen5_27 27:31   c1_16 16:16    c2_17 17:18

fieldinfo
# five bit register names
[ b_06 x_11 r_06 r_11 t_06 t_11 t_27 ]
  is [ names [
   "%r0" "%r1" "%r2" "%r3" "%r4" "%r5" "%r6" "%r7" "%r8"
   "%r9" "%r10" "%r11" "%r12" "%r13" "%r14" "%r15" "%r16" "%r17"
   "%r18" "%r19"  "%r20" "%r21" "%r22" "%r23" "%r24" "%r25" "%r26"
   "%r27" "%r28" "%r29"  "%r30" "%r31" ] ]

# two bit space register names
[ s2_16 ]
  is [ names [ "%s0" "%s1" "%s2" "%s3" ] ]

# three bit space register names
[ s3_16 ]
  is [ names [ "%s0" "%s1" "%s2" "%s3" "%s4" "%s5" "%s6" "%s7" ] ]

# five bit control register names
[ cr_06 ]
  is [ names [
   "%cr0"  "%cr1"  "%cr2"  "%cr3"  "%cr4"  "%cr5"  "%cr6"  "%cr7"  "%cr8"
   "%cr9"  "%cr10" "%cr11" "%cr12" "%cr13" "%cr14" "%cr15" "%cr16" "%cr17"
   "%cr18" "%cr19" "%cr20" "%cr21" "%cr22" "%cr23" "%cr24" "%cr25" "%cr26"
   "%cr27" "%cr28" "%cr29" "%cr30" "%cr31" ] ]


# Table C-1
patterns
[
#  0          1          2          3
  SystemOp   LDB        CMPBT      BVB          # 0
  _          LDH        CMPIBT     BB
  ArithLog   LDW        CMPBF      MOVB
  IndexMem   LDWM       CMPIBF     MOVIB
  _          Load_dw    CMPICLR    Sh_Ex_Dep4   # 4
  _          _          subi       Sh_Ex_Dep5
  _          FLDWM      _          Sh_Ex_Dep6
  _          Load_w     CMPBdwt    _
  LDIL       STB        ADDBT      BE           # 8
  Copr_w     STH        ADDIBT     BLE
  ADDIL      STW        ADDBF      UncondBr
  Copr_dw    STWM       ADDIBF     CMPIBdw
  FPOP0C     Store_dw   addi.t     Sh_Ex_DepC   # C
  LDO        _          addi       Sh_Ex_DepD
  FPOP0E     FSTWM      _          _
  _          Store_w    CMPBdwf    _
] is  op = { 0 to 63 columns 4 }


### ArithLog ######### (Table C-5)
### ArithImm ######### (Table C-9)
### IndexMem #########
### Load_dw ########## (Table C-7)
### Store_dw ######### (Table C-7)
### CompareBranch ####

patterns

[
  ADD       ADD.v     ADD.c       ADD.c.v   SHL1ADD
  SHL1ADD.v SHL2ADD   SHL2ADD.v   SHL3ADD   SHL3ADD.v
  SUB       SUB.v     SUB.t       SUB.t.v   SUB.b     SUB.b.v
  DS        ANDCM     AND         OR        XOR       UXOR
  CMPCLR
  UADDCM    UADDCMT
  ADD.l     SHL1ADD.l  SHL2ADD.l    SHL3ADD.l
  DCOR      IDCOR
  HADD      HADD.s    HADD.u      HSUB      HSUB.s    HSUB.u
  HAVG
  HSHL1ADD  HSHL2ADD  HSHL3ADD    HSHR1ADD  HSHR2ADD  HSHR3ADD
] is ArithLog & ext6_20 = [
  0x18     0x38     0x1C     0x3C     0x19
  0x39     0x1A     0x3A     0x1B     0x3B
  0x10     0x30     0x13     0x33     0x14     0x34
  0x11     0x00     0x08     0x09     0x0A     0x0E
  0x22
  0x26     0x27
  0x28     0x29     0x2A     0x2B
  0x2E     0x2F
  0x0F     0x0D     0x0C     0x07     0x05     0x04
  0x0B
  0x1D     0x1E     0x1F     0x15     0x16     0x17
]

arith is ADD | ADD.v | ADD.c | ADD.c.v | SHL1ADD
  | SHL1ADD.v | SHL2ADD | SHL2ADD.v | SHL3ADD | SHL3ADD.v
  | SUB | SUB.v | SUB.t | SUB.t.v | SUB.b | SUB.b.v
  | DS | ANDCM | AND | OR | XOR | UXOR
  | CMPCLR
  | UADDCM | UADDCMT
  | ADD.l | SHL1ADD.l | SHL2ADD.l | SHL3ADD.l
  | DCOR | IDCOR
  | HADD | HADD.s | HADD.u | HSUB | HSUB.s | HSUB.u
  | HAVG
  | HSHL1ADD | HSHL2ADD | HSHL3ADD | HSHR1ADD | HSHR2ADD | HSHR3ADD
  
  [ ADDI    ADDI.v   ]  is addi & ext_20 = { 0 to 1 }
  [ ADDI.t  ADDI.t.v ]  is addi.t & ext_20 = { 0 to 1 }
  [ SUBI    SUBI.v   ]  is subi & ext_20 = { 0 to 1 }

  arith_imm is ADDI | ADDI.v | ADDI.t | ADDI.t.v | SUBI | SUBI.v | CMPICLR
 
  [ arith_w arith_dw ] is arith & d_26 = {0 to 1}

  NOP            is OR & r_06 = 0 & r_11 = 0 & t_27 = 0
  COPY           is OR & r_06 = 0 & r_11 != 0
  
  

#   #   #   #   #   #   #   #   #   #   #   #   #   #   #
#                                                       #
#   I n t e g e r    l o a d s   a n d   s t o r e s    #
#                                                       #
#   #   #   #   #   #   #   #   #   #   #   #   #   #   #




# Indexed and Short Displacement Load/Store instructions
# Table D-6 of 1.1 manual
# BASIC INSTRUCTIONS for loads/stores

# some integer load/stores
iloads_x is any of [ LDBX LDHX LDWX LDDX LDDAX LDWAX ],
   which is IndexMem & addr_19 = 0 & ext4_22 = [ 0x0 0x1 0x2 0x3 0x4 0x6 ]
iloads_s is any of [ LDBS LDHS LDWS LDDS LDDAS LDWAS ],
   which is IndexMem & addr_19 = 1 & ext4_22 = [ 0x0 0x1 0x2 0x3 0x4 0x6 ]

iloadclears_x is any of [ LDCDX LDCWX ],
   which is IndexMem & addr_19 = 0 & ext4_22 = [ 0x5 0x7 ]
iloadclears_s is any of [ LDCDS LDCWS ],
   which is IndexMem & addr_19 = 1 & ext4_22 = [ 0x5 0x7 ]

istores_s is any of [ STBS STHS STWS STDS STWAS STWDS ],
   which is IndexMem & addr_19 = 1 & ext4_22 = [ 0x8 0x9 0xA 0xB 0xE 0xF ]
istorebytes_s is any of [ STBYS STDBYS ],
   which is IndexMem & addr_19 = 1 & ext4_22 = [ 0xC 0xD ] # these are special ..

ldisp_iloads is   LDB | LDH | LDW | LDWM
ldisp_istores is  STB | STH | STW | STWM

# some floating point load/stores
floads_x is any of [ FLDWX FLDDX ],
   which is uid2_23 = 0 & addr_19 = 0 & addr_22 = 0 & [ Copr_w Copr_dw ]
floads_s is any of [ FLDWS FLDDS ],
   which is uid2_23 = 0 & addr_19 = 1 & addr_22 = 0 & [ Copr_w Copr_dw ]

fstores_x is any of [ FSTWX FSTDX ],
   which is uid2_23 = 0 & addr_19 = 0 & addr_22 = 1 & [ Copr_w Copr_dw ]
fstores_s is any of [ FSTWS FSTDS ],
   which is uid2_23 = 0 & addr_19 = 1 & addr_22 = 1 & [ Copr_w Copr_dw ]

# ignore the op == 0x1? floating point loads/stores - they exist in pa-risc 2.x only



# here are some patterns for the address typed constructors that follow
# ADDRESS KIND PATTERNS for loads/stores
# 

  # first take care of the integer loads/stores
  
  index_iloads is iloads_x | iloadclears_x
# index_istores is .. no indexed stores -_- stupid pa-risc
  sdisp_iloads is iloads_s | iloadclears_s
  sdisp_istores is istores_s | istorebytes_s
# ldisp_iloads is ldisp_iloads
# ldisp_istores is ldisp_istores

  # then take care of the float loads/stores

  index_floads is floads_x
  index_fstores is fstores_x
  sdisp_floads is floads_s
  sdisp_fstores is fstores_s


  # now group the integer index addresses with the float index addresses
  # and group the integer sdisp addresses with the float sdisp addresses

  index_loads is index_iloads | index_floads
  index_stores is index_fstores # no indexed integer stores -> see index_istores above
  
index_addrs is index_loads | index_stores

  sdisp_loads is sdisp_iloads | sdisp_floads
  sdisp_stores is sdisp_istores | sdisp_fstores

  sdisp_addrs is sdisp_loads | sdisp_stores

sdisp_addrs_im_r is sdisp_iloads | sdisp_floads | sdisp_fstores
sdisp_addrs_r_im is sdisp_istores

  ldisp_loads is ldisp_iloads
  ldisp_stores is ldisp_istores
  
  ldisp_addr_16_old is ldisp_iloads | ldisp_istores # | ldisp_floads | ldisp_fstores
ldisp_addrs_16_old is ldisp_addr_16_old & op >= 0x00

  ldisp_addr_17_old is BE | BLE
ldisp_addrs_17_old is ldisp_addr_17_old & op >= 0x00

# the op >= 0x00 is necessary, even though it's redundant
# it ensures that the typed constructor that produces the addresses for it
# doesn't name these constructors with the names of the instructions in
# ldisp_addrs_17_old (ie. LDW, LDWM); if it didn't prevent this, we couldn't
# declare constructors with these names later (for the actual LDW, LDWM etc.
# instructions' constructors). I'm exploiting a glitch in the toolkit to do this,
# but, it's completely necessary - simon


# for your own personal entertainment
# CONSTRUCTOR PATTERNS for the loads/stores

iloads is index_iloads | sdisp_iloads
istores is sdisp_istores

fwloads is FLDWX | FLDWS
fwstores is FSTWX | FSTWS
fdloads is FLDDX | FLDDS
fdstores is FSTDX | FSTDS

#floads is index_floads | sdisp_floads
#fstores is index_fstores | sdisp_fstores

iloads_ldisp is ldisp_iloads
istores_ldisp is ldisp_istores




# and now, some completers for different sorts of addresses
# COMPLETER PATTERNS


# for instructions with displacement addresses (but not LDO ;)
# actually, I'll restrict that to short displacement address completers;
# as much as I'd love to be able to generalise by including the long
# displacement ones here too (ma, mb eg. for opcodes 0x16 and 0x17) that would
# mean .. | LDWM | LDW | .. which would prevent me from using LDWM and LDW later.
# looks like they'll have to do their modification separately.

  sdisp_addrs_notbytes is istores_s | sdisp_loads | sdisp_floads | sdisp_fstores
  sdisp_addrs_bytes is istorebytes_s

pc_s_addr_mb    is sdisp_addrs_notbytes & a_18 = 1 & m_26 = 1
pc_s_addr_ma    is sdisp_addrs_notbytes & a_18 = 0 & m_26 = 1
pc_s_addr_none  is sdisp_addrs_notbytes & m_26 = 0

# don't forget that STBYS, STDBYS have different completers
pc_y_addr_m     is sdisp_addrs_bytes & m_26 = 1 & a_18 = 0
pc_y_addr_e     is sdisp_addrs_bytes & a_18 = 1 & m_26 = 0
pc_y_addr_me    is sdisp_addrs_bytes & m_26 = 1 & a_18 = 1
pc_y_addr_none  is sdisp_addrs_bytes & m_26 = 0 & a_18 = 0

# okay, it turns out that the ,s completer for indexed address has different
# shift amounts for different instructions. let's just handle that in this file
# so that decoder*.m needn't
#
# so, after all that, for instructions with indexed addresses:

pc_x_addr_m is index_addrs & m_26 = 1
pc_x_addr_notm is index_addrs & m_26 = 0
  
  index_align_byte is LDBX
  index_align_hwrd is LDHX
  index_align_word is LDWX | LDWAX | FLDWX | FSTWX
  index_align_dwrd is LDDX | LDDAX | FLDDX | FSTDX | LDCWX | LDCDX

  index_addr_shift is index_addrs & u_18 = 1 
  index_addr_notshift is index_addrs & u_18 = 0

pc_x_addr_s_byte is index_addr_shift & index_align_byte # shifts 0 bits
pc_x_addr_s_hwrd is index_addr_shift & index_align_hwrd # shifts 1 bit
pc_x_addr_s_word is index_addr_shift & index_align_word # shifts 2 bits
pc_x_addr_s_dwrd is index_addr_shift & index_align_dwrd # shifts 3 bits
pc_x_addr_nots is index_addr_notshift

pc_l_addr_none is ldisp_addrs_16_old & op >= 0x00

### Sh_Ex_Dep ####### (Table C-10/-11 of 2.0 manual)

[ VSHD      SHD     ]   is Sh_Ex_Dep4 & cl_19 = 0 & p_20 = [ 0 1 ] & se_21 = 0
[ VEXTRU    EXTRU   ]   is Sh_Ex_Dep4 & cl_19 = 1 & p_20 = [ 0 1 ] & se_21 = 0
[ VEXTRS    EXTRS   ]   is Sh_Ex_Dep4 & cl_19 = 1 & p_20 = [ 0 1 ] & se_21 = 1
[ VDEP      DEP     ]   is Sh_Ex_Dep5 & cl_19 = 0 & p_20 = [ 0 1 ] & se_21 = 1
[ VDEPI     DEPI    ]   is Sh_Ex_Dep5 & cl_19 = 1 & p_20 = [ 0 1 ] & se_21 = 1
[ ZVDEP     ZDEP    ]   is Sh_Ex_Dep5 & cl_19 = 0 & p_20 = [ 0 1 ] & se_21 = 0
[ ZVDEPI    ZDEPI   ]   is Sh_Ex_Dep5 & cl_19 = 1 & p_20 = [ 0 1 ] & se_21 = 0

ext_var  is VEXTRU | VEXTRS
dep_var  is VDEP   | ZVDEP
ext_fix  is EXTRU  | EXTRS
dep_fix  is DEP    | ZDEP
dep_ivar is VDEPI  | ZVDEPI
dep_ifix is DEPI   | ZDEPI

sep_all is VSHD | SHD | ext_var | dep_var | ext_fix | dep_fix | dep_ivar | dep_ifix

### UncondBr ######## (Table D-9 of 1.1 manual)
[
    BL   GATE   BL.PUSH   BL.LONG
] is UncondBr & ext3_16 = [ 0 1 4 5 ]

  BLR      is UncondBr & ext3_16 = 2 & ve_19 = 0
  BV       is UncondBr & ext3_16 = 6 & ve_19 = 0
  BVE      is UncondBr & ext3_16 = 6 & ve_19 = 1
  BVE.l    is UncondBr & ext3_16 = 7 & ve_19 = 1

  ubranch is BL | GATE | BL.PUSH
  
### SystemOp ######## (Table C-2)
[
  BREAK     sync      RFI       RFI.r
  SSM       RSM       MTSM      LDSID
  MTSP      MFSP      MFIA      MTCTL
  MTSARCM   mfctl
] is SystemOp & ext8_19 =
[
  0x00      0x20      0x60      0x65
  0x6B      0x73      0xC3      0x85
  0xC1      0x25      0xA5      0xC2
  0xC6      0x45
]

  rfis is RFI | RFI.r
[ SYNC   SYNCDMA ] is sync & ext5_11 = [ 0  10 ]
  syncs is SYNC | SYNCDMA
[ MFCTL MFCTL.w ] is mfctl & ext_17 = [ 0  1 ]
  mfctls is MFCTL | MFCTL.w

  sysop_i_t    is SSM | RSM
  sysop_simple is rfis | syncs
  sysop_r      is MTSM | MTSARCM
  sysop_cr_t   is mfctls

  addb_all is ADDBT | ADDBF
  addib_all is ADDIBT | ADDIBF
  addbr_all is addb_all | addib_all

  cmpb_w is CMPBT | CMPBF
  cmpb_dw is CMPBdwt | CMPBdwf
  cmpb_all is cmpb_w | cmpb_dw
  
  cmpib_w is CMPIBT | CMPIBF
  cmpib_dw is CMPIBdw
  cmpib_all is cmpib_w | cmpib_dw
  
  cmp_w is cmpb_w | cmpib_w
  cmp_dw is cmpb_dw
  cmpi_dw is CMPIBdw
  
  bve is BVE | BVE.l
  
  bb_all is BB | BVB
  be_all is BE | BLE
  #bea_17 is be_all # *was* for ldisp17's, but don't need to declare at this time
  
  ins_bb_w is BB & d_18 = 0
  ins_bb_dw is BB & d_18 = 1

  ins_bbs_w  is bb_all & d_18 = 0
  ins_bbs_dw is bb_all & d_18 = 1
  
  ins_arith_w    is arith_w | arith_imm | addbr_all
  ins_arith_dw   is arith_dw
  ins_arith_none is ADDIL

  # must add ADDB/ADDIB/MOVB/MOVIB groups to here when they're implemented
  nullifiable_br is UncondBr    | cmpb_all | cmpib_all
                                | bb_all | be_all 
                                | addb_all | addib_all
                                | MOVB | MOVIB
  
  ins_br_nnull   is nullifiable_br & n_30 = 0
  ins_br_null    is nullifiable_br & n_30 = 1

  ins_br_nonneg is CMPBT | CMPBdwt | CMPIBT
                 | arith & f_19 = 0
                 | arith_imm & f_19 = 0
                 | ADDBT | ADDIBT
                 | MOVB & c1_16 = 0
                 | MOVIB & c1_16 = 0
                 | cmpi_dw & c1_16 = 0
                 | sep_all & c1_16 = 0

  ins_br_neg    is CMPBF | CMPBdwf | CMPIBF
                 | arith & f_19 = 1
                 | arith_imm & f_19 = 1
                 | ADDBF | ADDIBF
                 | MOVB & c1_16 = 1
                 | MOVIB & c1_16 = 1
                 | cmpi_dw & c3_16 = 1
                 | sep_all & c3_16 = 1
                 
  
# Floating point operations (Table 6-9)

  fp0c0e         is FPOP0C | FPOP0E

  # Class 0
  flt_c0        is any of
                  [ FID       _           FCPY        FABS
                    FSQRT     FRND        FNEG        FNEGABS ],
                which is FPOP0C & class_21 = 0 & sub_16 = {0 to 7}
  flt_c0.E       is any of
                  [ _         _           FCPY.E      FABS.E
                    FSQRT.E   FRND.E      FNEG.E      FNEGABS.E ],
                which is FPOP0E & class_21 = 0 & sub_16 = {0 to 7}
  flt_c0_all     is flt_c0 | flt_c0.E

  # Class 1
  flt_c1        is any of
                 [ FCNVFF     FCNVXF     FCNVFX     FCNVFXT ],
                which is FPOP0C & class_21 = 1 & sub_14 = {0 to 3}
  flt_c1.E      is any of
                 [ FCNVFF.E   FCNVXF.E   FCNVFX.E   FCNVFXT.E ],
                which is FPOP0E & class_21 = 1 & sub_14 = {0 to 3}
  flt_c1_all    is flt_c1 | flt_c1.E

  # Class 2
  flt_c2         is any of [FCMP   FTEST   ],
                    which is FPOP0C & class_21 = 2 & sub_16 = {0 to 1}
  flt_c2.E       is any of [FCMP.E FTEST.E ],
                    which is FPOP0E & class_21 = 2 & sub_16 = {0 to 1}
  flt_c2_all     is flt_c2 | flt_c2.E

  # Class 3
  flt_c3         is any of
                  [ FADD     FSUB     FMPY     FDIV ],
                    which is FPOP0C & class_21 = 3 & sub_16 = {0 to 3}
  flt_c3.E       is any of
                  [ FADD.E   FSUB.E   FMPY.E   FDIV.E ],
                    which is FPOP0E & class_21 = 3 & sub_16 = {0 to 3}
                    & x_23 = 0
  flt_c3_all     is flt_c3 | flt_c3.E

  # A special class 3 instruction
  XMPYU          is FPOP0E & class_21 = 3 & sub_16 = 2 & x_23 = 1



# do not delete these! when I get the time I'll implement condition
# code constructors based on them so that the emulator isn't broken -.s
ins_cond3_16 is cmp_w | cmp_dw | ins_arith_w | ins_arith_dw

ins_sep is MOVB | MOVIB | sep_all

ins_c0 is ins_cond3_16 & c3_16 = 0     # never
          | ins_sep & c2_17 = 0
ins_c1 is ins_cond3_16 & c3_16 = 1     # op1 = op2
          | ins_sep & c2_17 = 1
          | cmpib_all & c2_17 = 1
ins_c2 is ins_cond3_16 & c3_16 = 2     # op1 < op2
          | ins_sep & c2_17 = 2
          | cmpib_all & c2_17 = 2
ins_c3 is ins_cond3_16 & c3_16 = 3     # op1 <= op2
          | ins_sep & c2_17 = 3
          | cmpib_all & c2_17 = 3
ins_c4 is ins_cond3_16 & c3_16 = 4     # op1 << op2
          | cmpib_all & c2_17 = 0
ins_c5 is ins_cond3_16 & c3_16 = 5     # op1 <<= op2
ins_c6 is ins_cond3_16 & c3_16 = 6
ins_c7 is ins_cond3_16 & c3_16 = 7

ins_c_no    is ins_c0 & ins_br_nonneg
ins_c_yes   is ins_c0 & ins_br_neg
ins_c_eq    is ins_c1 & ins_br_nonneg
ins_c_ne    is ins_c1 & ins_br_neg
ins_c_l     is ins_c2 & ins_br_nonneg
ins_c_ge    is ins_c2 & ins_br_neg
ins_c_le    is ins_c3 & ins_br_nonneg
ins_c_g     is ins_c3 & ins_br_neg
ins_c_ul    is ins_c4 & ins_br_nonneg
ins_c_uge   is ins_c4 & ins_br_neg
ins_c_ule   is ins_c5 & ins_br_nonneg
ins_c_ug    is ins_c5 & ins_br_neg
ins_c_sv    is ins_c6 & ins_br_nonneg
ins_c_nsv   is ins_c6 & ins_br_neg
ins_c_od    is ins_c7 & ins_br_nonneg
ins_c_ev    is ins_c7 & ins_br_neg

constructors
 
                #   #   #   #   #   #   #   #   #   #
                #                                   #
                #  A d d r e s s i n g   m o d e s  #
                #                                   #
                #   #   #   #   #   #   #   #   #   #

  c_c_nonneg              : c_c_n     is ins_br_nonneg
  c_c_neg                 : c_c_n     is ins_br_neg

  c_cmpb_w   c3_16, c_c_n : c_c       is cmp_w & c3_16 & c_c_n
  c_cmpb_dw  c3_16, c_c_n : c_c       is cmp_dw & c3_16 & c_c_n
  c_cmpib_dw cond         : c_c {
                            cond@[02:31] = c1_16,
                            cond@[00:01] = c2_17
                            } is cmpi_dw & c1_16 & c2_17

  c_bbs_w     c_16        : c_c       is ins_bbs_w & c_16
  c_bbs_dw    c_16        : c_c       is ins_bbs_dw & c_16

  c_arith_w  c3_16, c_c_n : c_c       is ins_arith_w & c3_16 & c_c_n
  c_arith_dw c3_16, c_c_n : c_c       is ins_arith_dw & c3_16 & c_c_n
  c_arith_none            : c_c       is ins_arith_none

  c_sep      cond         : c_c {
                            cond@[02:31] = c1_16,
                            cond@[00:01] = c2_17
                            } is ins_sep & c1_16 & c2_17

  c_br_nnull              : c_null    is ins_br_nnull
  c_br_null               : c_null    is ins_br_null

  c_mfctl_w               : c_wcr     is MFCTL.w
  c_mfctl r_06            : c_wcr     is MFCTL & r_06

  c_bitpos_w p_06   : c_bit     is ins_bb_w & p_06
# c_bitpos_dw p_06  : c_bit     is ins_bb_dw & p_06 # no doubleword for PA-RISC 1.1!
  c_bitsar          : c_bit     is BVB

# c_addr: completers for integer loads and stores
# Declare constructors for the patterns pc_?_addr_*, all of type c_addr
  c_s_addr_mb         : c_addr           is pc_s_addr_mb
  c_s_addr_ma         : c_addr           is pc_s_addr_ma
  c_s_addr_notm       : c_addr           is pc_s_addr_none
  
  c_x_addr_m          : c_addr           is pc_x_addr_m
  c_x_addr_notm       : c_addr           is pc_x_addr_notm
  
  c_y_addr_e          : c_addr           is pc_y_addr_e
  c_y_addr_m          : c_addr           is pc_y_addr_m
  c_y_addr_me         : c_addr           is pc_y_addr_me
  c_y_addr_none       : c_addr           is pc_y_addr_none

  c_l_addr_none       : c_addr           is pc_l_addr_none
  

  x_addr_nots   x  : c_xd    is pc_x_addr_nots & x_11 = x
  x_addr_s_byte x  : c_xd    is pc_x_addr_s_byte & x_11 = x
  x_addr_s_hwrd x  : c_xd    is pc_x_addr_s_hwrd & x_11 = x
  x_addr_s_word x  : c_xd    is pc_x_addr_s_word & x_11 = x
  x_addr_s_dwrd x  : c_xd    is pc_x_addr_s_dwrd & x_11 = x


#   #   #   #   #   #   #   #   #   #   #   #   #   #   #
#                                                       #
# A d d r e s s e s    f o r    L o a d s / S t o r e s #
#         (is this hard to read or what?)               #
#   #   #   #   #   #   #   #   #   #   #   #   #   #   #

# short displacement addresses.
# note: integer loads and stores have fields swapped but floats don't
  s_addr_im_r im5!          : c_xd { im5@[0:3] = im4_11,
                                        im5@[4:31] = im1_15! }
                                is sdisp_addrs_im_r & im4_11 & im1_15

  s_addr_r_im im5!          : c_xd { im5@[0:3] = im4_27,
                                        im5@[4:31] = i_31! }
                                is sdisp_addrs_r_im & im4_27 & i_31

# long displacement addresses.
# note: pa-risc 1.x has only two sets of these; 2.x seems to be all over the place -_-;;
  l_addr_16_old ldisp!      : c_xd { ldisp@[13:31] = i_31!,
                                         ldisp@[0:12] = im13_18 }
                                is ldisp_addrs_16_old & im13_18 & i_31
 
  l_addr_17_old ldisp!      : c_xd { ldisp@[16:31] = w_31!,
                                         ldisp@[0:9] = w10_19,
                                         ldisp@[10:10] = w_29,
                                         ldisp@[11:15] = w5_11 }
                                is ldisp_addrs_17_old & w_31 & w10_19 & w_29 & w5_11


                #   #   #   #   #   #   #   #   #
                #                               #
                #   I n s t r u c t i o n s !   #
                #                               #
                #   #   #   #   #   #   #   #   #

  NOP
  COPY      r_11, t_27

  arith      c_c, r_11, r_06, t_27
  arith_imm  c_c, imm11!, r_06, t_11 {
                        imm11@[10:31] = i_31!,
                        imm11@[0:9] = im10_21
                } is arith_imm & c_c & i_31 & im10_21 & r_06 & t_11

  ADDIL      imm21!, r_06, "%r1" {
                        imm21@[31]!   =    i_31,
                        imm21@[20:30] = im11_20,
                        imm21@[18:19] =  im2_16,
                        imm21@[13:17] =  im5_11,
                        imm21@[11:12] =  im2_18,
                        imm21@[00:10] = 0
                } is ADDIL & r_06 & i_31 & im11_20 & im2_16 & im5_11 & im2_18

#   #   #   #   #   #   #   #   #   #   #   #   #   #   #
#                                                       #
#    I i N n S s T t R r U u C c T t I i O o N s S      #
#                                                       #
#   #   #   #   #   #   #   #   #   #   #   #   #   #   #

# All loads and stores, except long displacement

  iloads    c_addr c_xd(s2_16, b_06),t_27
  istores   c_addr r_11, c_xd(s2_16, b_06)
  
  fdloads    c_addr c_xd(s2_16, b_06),t_27
  fdstores   c_addr r_27,c_xd(s2_16, b_06)
  
  fwloads    c_addr c_xd(s, b),treg {
                   treg@[0:4] = t_27, treg@[5:5] = t_25,
                   treg@[6:31] = 0
                } is fwloads & c_addr & c_xd & s2_16 = s & b_06 = b & t_27 & t_25
  fwstores   c_addr rreg, c_xd(s, b) {
                    rreg@[0:4] = r_27, rreg@[5:5] = r_25,
                    rreg@[6:31] = 0
                } is fwstores & c_addr & c_xd & s2_16 = s & b_06 = b & r_27 & r_25
  
  iloads_ldisp c_addr c_xd(s2_16, b_06),t_11
  istores_ldisp c_addr r_11,c_xd(s2_16, b_06)
  
  LDO ldisp!(b),t      { ldisp@[13:31] = i_31!, ldisp@[0:12] = im13_18 } is LDO &
                            b_06 = b & t_11 = t & im13_18 & i_31

  LDIL      imm21!, t_06 {
                       imm21@[31]!   =    i_31,
                       imm21@[20:30] = im11_20,
                       imm21@[18:19] =  im2_16,
                       imm21@[13:17] =  im5_11,
                       imm21@[11:12] =  im2_18,
                       imm21@[00:10] =       0
                } is LDIL & t_06 & i_31 & im11_20 & im2_16 & im5_11 & im2_18

  VSHD        r1, r2, t, c_c      is VSHD 
                    & r_11 = r1 & r_06 = r2 & t_27 = t & c_c
  SHD         r1, r2, p, t, c_c   {
                                    p@[0:31] = 31 - pos5_22
                                  } is SHD
                    & r_11 = r1 & r_06 = r2 & pos5_22 & t_27 = t & c_c
  ext_var     r, len, t, c_c      {
                                    len = 32 - clen5_27
                                  } is ext_var
                    & r_06 = r & clen5_27 & t_11 = t & c_c
  ext_fix     r, p, len, t, c_c   {
                                    len = 32 - clen5_27
                                  } is ext_fix
                    & r_06 = r & pos5_22 = p & clen5_27 & t_11 = t & c_c
  dep_var     r, len, t, c_c      {
                                    len = 32 - clen5_27
                                  } is dep_var
                    & r_11 = r & clen5_27 & t_06 = t & c_c
  dep_fix     r, p, len, t, c_c   {
                                    p@[0:31] = 31 - pos5_22,
                                    len= 32 - clen5_27
                                  }is dep_fix
                    & r_11 = r & pos5_22 & clen5_27 & t_06 = t & c_c
  dep_ivar    i!, len, t, c_c      {
                                    i@[4:31]! = im1_15!,
                                    i@[0:3] = im4_11,
                                    len = 32 - clen5_27
                                  } is dep_ivar
                    & im4_11 & im1_15 & clen5_27 & t_06 = t & c_c
  dep_ifix    i!, p, len, t, c_c   {
                                    p@[0:31] = 31 - pos5_22,
                                    i@[4:31]! = im1_15!,
                                    i@[0:3] = im4_11,
                                    len = 32 - clen5_27
                                  } is dep_ifix
                    & im4_11 & im1_15 & pos5_22 & clen5_27 & t_06 = t & c_c

  ubranch c_null ubr_target!,t_06 {
  		 ubr_target = offset + 8,
                 offset@[18:31] = w_31!,
                 offset@[13:17] = w5_11,
                 offset@[12] = w_29,
                 offset@[2:11] = w10_19,
                 offset@[0:1] = 0
              } is ubranch & c_null & t_06 & w_31 & w5_11 & w10_19 & w_29

  BL.LONG c_null ubr_target!,2 {
                 ubr_target = offset + 8,
                 offset@[23:31] = w_31!,
                 offset@[18:22] = w5_06,
                 offset@[13:17] = w5_11,
                 offset@[12] = w_29,
                 offset@[2:11] = w10_19,
                 offset@[0:1] = 0
              } is BL.LONG & c_null & w_31 & w5_06 & w5_11 & w10_19 & w_29

  BLR c_null x_11,t_06
  BV c_null x_11(b_06)
  bve p_31 c_null (b_06)

  BREAK        im5_27,im13_06
  sysop_i_t    im10_06,t_27
  sysop_simple
  sysop_r      r_11
  sysop_cr_t   c_wcr,t_27
  MTCTL        r_11,cr_06
  MFIA         t_27
  LDSID        (s2_16,b_06),t_27
  MTSP         r_11,sr! {
                    sr@[3:31] = 0,
                    sr@[2:2] = s_18,
                    sr@[0:1] = s2_16
             } is MTSP & r_11 & s_18 & s2_16

  MFSP         sr!,t_27 {
                    sr@[3:31] = 0,
                    sr@[2:2] = s_18,
                    sr@[0:1] = s2_16
             } is MFSP & t_27 & s_18 & s2_16


  addb_all c_c, c_null  r_11, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0
                         } is addb_all & c_c & c_null & r_11 & r_06
                                            & w_31 & w_29 & w10_19

  addib_all c_c, c_null im5!, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0,
                            im5@[0:3]       = im4_11,
                            im5@[4:31]!     = im1_15!
                         } is addib_all & c_c & c_null & im4_11
                            & im1_15 & r_06 & w_31 & w_29 & w10_19
  
  MOVB c_c, c_null r_11, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0
                         } is MOVB & c_c & c_null & r_11 & r_06
                                            & w_31 & w_29 & w10_19
  
  MOVIB c_c, c_null im5, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0,
                            im5@[0:3]       = im4_11,
                            im5@[4:31]!     = im1_15!
                         } is MOVIB & c_c & c_null & im4_11
                            & im1_15 & r_06 & w_31 & w_29 & w10_19
  
  cmpb_all c_c, c_null  r_11, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0
                         } is cmpb_all & c_c & c_null & r_11 & r_06
                                             & w_31 & w_29 & w10_19
  
  cmpib_all c_c, c_null  im5!, r_06, target {
                            target@[13:31]! = w_31!,
                            target@[12]     = w_29,
                            target@[2:11]   = w10_19,
                            target@[0:1]    = 0,
                            im5@[0:3]       = im4_11,
                            im5@[4:31]!     = im1_15!
                         } is cmpib_all & c_c & c_null & im4_11 & im1_15 & r_06
                                             & w_31 & w_29 & w10_19
  
  
  bb_all c_c, c_null   r_11,c_bit,target {
                            target@[13:31]! = w_31!,
                            target@[12] = w_29,
                            target@[2:11] = w10_19,
                            target@[0:1] = 0
                         } is bb_all & c_c & c_null & r_11 & c_bit
                                             & w_31 & w_29 & w10_19

# Floating point
# Class 0, opcode 0C
  flt_c0 ,fmt r,t          { op = 0x0C }
                           is flt_c0 & r_06=r & t_27=t & fmt_19 = fmt

# Class 0, opcode 0E
  flt_c0.E ,f r,t          { op = 0x0E,
                             r@[0:4] = r_06, r@[5] = r1_24, r@[6:31] = 0,
                             t@[0:4] = t_27, t@[5] = t_25,  t@[6:31] = 0 } 
                           is flt_c0 & f_20 = f & r_06 & r1_24 & t_27 & t_25

# Class 1 (fcnvxx), opcode 0C
# Note: sub_14 (not sub_16) determines the type of conversion performed
  flt_c1  ,sf,df r,t       {op = 0x0C }
                           is flt_c1 & r_06=r & t_27=t & df_17 = df & sf_19 = sf
# Class 1 (fcnvxx), opcode 0E
  flt_c1.E  ,sf,df r,t     { op = 0x0E,
                                 r@[0:4] = r_06, r@[5] = r1_24, r@[6:31] = 0,
                                 t@[0:4] = t_27, t@[5] =  t_25, t@[6:31] = 0 }
                            is flt_c1.E & df_17 = df & sf_19 = sf
                               & r_06 & r1_24 & t_27 & t_25

# Class 2, opcode 0C
  flt_c2 ,fmt,c r1,r2      { op = 0x0C }
                           is flt_c2 & r_06 = r1 & r_11 = r2
                             & t_27 = c & fmt_19 = fmt
# Class 2, opcode 0E
  flt_c2.E ,f_20,c r1,r2   { op = 0x0E,
                             r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0 }
                           is flt_c2.E & t_27 = c & f_20 & r_06 & r1_24 & r_11
                             & f_19

# Class 3, opcode 0C
  flt_c3 ,fmt r1,r2,t      is flt_c3 & r_06 = r1 & r_11 = r2 & t_27 = t
                             & fmt_19 = fmt
# Class 3, opcode 0E (x=0)
  flt_c3.E ,fmt r1,r2,t   {  r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0,
                              t@[0:4] = t_27,  t@[5] = t_25,   t@[6:31] = 0 }
                           is flt_c3.E & f_20 = fmt
                             & r_06 & r1_24 & r_11 & f_19 & t_27 & t_25

# Class 3, opcode 0E, x=1
  XMPYU  r1, r2, t        {  r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0,
                              t@[0:4] = t_27,  t@[5] = t_25,   t@[6:31] = 0 }
                            is XMPYU & r_06 & r1_24 & r_11 & f_19 & t_27 & t_25

