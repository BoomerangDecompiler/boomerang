# instruction fields go [0..31] left-->right
#bit 0 is most significant


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
 addr_19 19:19   cc_20 20:21 ext4_22 22:25    m_26 26:26  im5_27 27:31

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
   cr_06 06:10   ct_06 27:31 im13_06 06:18
 im10_06 06:15 ext5_11 11:15   s3_16 16:18
    s_18 18:18 ext8_19 19:26  ext_17 17:17
 
# branch on bit instructions
    p_06 06:10    c_16 16:16     e_17 17:17    d_18 18:18

# Floating point instructions
  sub_16 16:18  fmt_19 19:20 class_21 21:22    x_23 23:23
   r1_24 24:24    t_25 25:25     n_26 26:26    sub_14 14:16
   df_17 17:18   sf_19 19:20     f_20 20:20

# Coprocessor (including floating point) Loads and Stores
 addr_22 22:22  uid_23 23:25


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
[ cr_06 ct_06 ]
  is [ names [
   "%cr0"  "%cr1"  "%cr2"  "%cr3"  "%cr4"  "%cr5"  "%cr6"  "%cr7"  "%cr8"
   "%cr9"  "%cr10" "%cr11" "%cr12" "%cr13" "%cr14" "%cr15" "%cr16" "%cr17"
   "%cr18" "%cr19" "%cr20" "%cr21" "%cr22" "%cr23" "%cr24" "%cr25" "%cr26"
   "%cr27" "%cr28" "%cr29" "%cr30" "%cr31" ] ]


# Table C-1
patterns
[
  SystemOp   LDBl       CMPBt      BBsar
  _          LDHl       CMPIBt     BB
  ArithLog   LDWl       CMPBf      MOVB
  IndexMem   LDWlm      CMPIBf     MOVIB
  _          Load_dw    CMPICLR    _
  _          _          subi       _
  _          _          _          _
  _          Load_w     CMPBdwt    _
  LDIL       STBl       ADDBt      BE
  Copr_w     STHl       ADDIBt     BE.l
  ADDIL      STWl       ADDBf      UncondBr
  Copr_dw    STWlm      ADDIBf     CMPIBdw
  FPOP0C     Store_dw   addi.t     _
  LDO        _          addi       _
  FPOP0E     _          _          _
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
  COMCLR
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
  | COMCLR
  | UADDCM | UADDCMT
  | ADD.l | SHL1ADD.l | SHL2ADD.l | SHL3ADD.l
  | DCOR | IDCOR
  | HADD | HADD.s | HADD.u | HSUB | HSUB.s | HSUB.u
  | HAVG
  | HSHL1ADD | HSHL2ADD | HSHL3ADD | HSHR1ADD | HSHR2ADD | HSHR3ADD
  
  [ ADDI    ADDI.v   ]  is addi & ext_20 = { 0 to 1 }
  [ ADDI.t  ADDI.t.v ] is addi.t & ext_20 = { 0 to 1 }
  [ SUBI    SUBI.v   ]  is subi & ext_20 = { 0 to 1 }

  arith_imm is ADDI | ADDI.v | ADDI.t | ADDI.t.v | SUBI | SUBI.v
  
  [ arith_w arith_dw ] is arith & d_26 = {0 to 1}

  ins_arith_w    is arith_w | arith_imm
  ins_arith_dw   is arith_dw
  ins_arith_none is ADDIL
  
  
# Coprocessor (including floating point) Loads and Stores (Table C-14)

  cldw           is uid_23 > 1 & Copr_w  & addr_22 = 0
  cldd           is uid_23 > 1 & Copr_dw & addr_22 = 0
  cstw           is uid_23 > 1 & Copr_w  & addr_22 = 1
  cstd           is uid_23 > 1 & Copr_dw & addr_22 = 1

# Floating point loads and stores are just coprocessor loads and stores with
# the uid = 0 or 1 (page 8-1)
  fldw           is uid_23 < 2 & Copr_w  & addr_22 = 0
  fldd           is uid_23 < 2 & Copr_dw & addr_22 = 0
  fstw           is uid_23 < 2 & Copr_w  & addr_22 = 1
  fstd           is uid_23 < 2 & Copr_dw & addr_22 = 1

  copr_loads     is cldw | cldd | fldw | fldd
  copr_stores    is cstw | cstd | fstw | fstd

  index_copr_loads  is copr_loads  & addr_19 = 0
  sdisp_copr_loads  is copr_loads  & addr_19 = 1
  index_copr_stores is copr_stores & addr_19 = 0
  sdisp_copr_stores is copr_stores & addr_19 = 1

# Indexed & Short Displacement Loads/Stores (Table C-6)
[
  LDB     LDH      LDW      LDD
  LDD.a   LDCD     LDW.a    LDCW
  STB     STH      STW      STD
  STBY    STDBY    STW.a    STD.a
] is IndexMem & ext4_22 = {0 to 15}

  load is LDB | LDH | LDW | LDD | LDCD | LDCW
  loadabs is LDD.a | LDW.a

  loads is load | loadabs

  index_load is load & addr_19 = 0
  index_loadabs is loadabs & addr_19 = 0
  index_loads is index_load | index_loadabs

  sdisp_load is load & addr_19 = 1
  sdisp_loadabs is loadabs & addr_19 = 1
  sdisp_loads is sdisp_load | sdisp_loadabs
  
  store is STB | STH | STW | STD
  storeabs is STW.a | STD.a
  storebytes is STBY | STDBY

  stores is store | storeabs | storebytes
    
  sdisp_store is store & addr_19 = 1
  sdisp_storeabs is storeabs & addr_19 = 1
  sdisp_storebytes is storebytes & addr_19 = 1
  
  sdisp_storeless is sdisp_store | sdisp_storeabs
  sdisp_storenabs is sdisp_store | sdisp_storebytes
  sdisp_stores    is sdisp_storeless | sdisp_storebytes

  indices is index_load
  indicesabs is index_loadabs

  sdisps is sdisp_store | sdisp_load
  sdispsabs is sdisp_storeabs | sdisp_loadabs    
  sdispsbytes is sdisp_storebytes
  
  LDDl is Load_dw & ext_30 = 0
  STDl is Store_dw & ext_30 = 0
  opdl is LDDl | STDl

  LDWlm2i  is Load_w  & ext_29 = 1        # If bit 29 is clear, these are
  STWlm2i  is Store_w & ext_29 = 1        # floating point instructions!

    # For long displacement loads and stores, whether it is modify after or
    # modify before depends on the sign of the displacement!
  LDWlma   is LDWlm & i_31 = 0
  LDWlmb   is LDWlm & i_31 = 1

  STWlma   is STWlm & i_31 = 0
  STWlmb   is STWlm & i_31 = 1

  ld_bhw_l is LDBl | LDHl | LDWl
  ld_l     is ld_bhw_l | LDWlm2i | LDWlm | LDO
  loads_l  is ld_l | LDDl
 
# We don't want STWlm to be part of st_bhw_l because it then gets the wrong
# addressing mode
  st_bhw_l is STBl | STHl | STWl
  st_l     is st_bhw_l | STWlm2i | STWlm
  stores_l is st_l | STDl
  
  ldisp_12  is LDDl | STDl
  ldisp_16  is ld_bhw_l | st_bhw_l
  ldisp_16ma is LDWlma | STWlma
  ldisp_16mb is LDWlmb | STWlmb
  ldisp_16a is LDWlm2i | STWlm2i

  [ ins_def_il ins_m  ins_s  ins_sm ]
    is index_loads & u_18 = {0 to 1} & m_26 = {0 to 1}

  all_ldst  is loads | stores
  
  ins_c_il  is ins_m | ins_s | ins_sm
  ins_c_ils is ins_def_il | ins_c_il

  ins_def_d   is sdisp_loads & m_26 = 0
               | sdisp_storeless & m_26 = 0
               | opdl & m_28 = 0
               | ld_l
               | st_l
  ins_ma      is sdisp_loads & m_26 = 1 & a_18 = 0 & im5_11 != 0
               | sdisp_storeless & m_26 = 1 & a_18 = 0 & im5_27 != 0
               | LDDl & m_28 = 1 & a_29 = 0
  ins_mb      is sdisp_loads & m_26 = 1 & a_18 = 1
               | sdisp_storeless & m_26 = 1 & a_18 = 1
               | LDDl & m_28 = 1 & a_29 = 1
  ins_o       is sdisp_loads & m_26 = 1 & a_18 = 0 & im5_11 = 0
               | sdisp_storeless & m_26 = 1 & a_18 = 0 & im5_27 = 0
               | LDDl & m_28 = 1 & a_29 = 0 & im10_18 = 0

  ins_b       is sdisp_storebytes & a_18 = 0 & m_26 = 0
  ins_bm      is sdisp_storebytes & a_18 = 0 & m_26 = 1
  ins_e       is sdisp_storebytes & a_18 = 1 & m_26 = 0
  ins_em      is sdisp_storebytes & a_18 = 1 & m_26 = 1

  ins_c_d     is ins_ma | ins_mb | ins_o
  ins_c_by    is ins_bm | ins_e | ins_em | ins_b
  ins_c_disps is ins_def_d | ins_c_d | ins_c_by
  ins_c_copr  is Copr_w | Copr_dw

  ins_c_addrs is ins_c_disps | ins_c_ils 


### UncondBr ######## (Table C-13)
[
    B.l   B.g   B.p   B.ll
] is UncondBr & ext3_16 = [ 0 1 4 5 ]

  BLR      is UncondBr & ext3_16 = 2 & ve_19 = 0
  BV       is UncondBr & ext3_16 = 6 & ve_19 = 0
  BVE      is UncondBr & ext3_16 = 6 & ve_19 = 1
  BVE.l    is UncondBr & ext3_16 = 7 & ve_19 = 1

  ubranch is B.l | B.g | B.p
  
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

  cmpb_w is CMPBt | CMPBf
  cmpb_dw is CMPBdwt | CMPBdwf
  cmpb_all is cmpb_w | cmpb_dw
  cmpib_w is CMPIBt | CMPIBf
  cmpib_all is cmpib_w | CMPIBdw
    
  cmp_w is cmpb_w | cmpib_w
  cmp_dw is cmpb_dw | CMPIBdw
  
  bve is BVE | BVE.l
  
  bb_all is BB | BBsar
  be_all is BE | BE.l
  bea_17 is be_all
  
  ins_bb_w is BB & d_18 = 0
  ins_bb_dw is BB & d_18 = 1

  ins_bbs_w  is bb_all & d_18 = 0
  ins_bbs_dw is bb_all & d_18 = 1
  
  nullifiable_br is UncondBr | cmpb_all | cmpib_all | bb_all | be_all
  
  ins_br_nnull   is nullifiable_br & n_30 = 0
  ins_br_null    is nullifiable_br & n_30 = 1

# Floating point operations (Table 8-8)

  fp0c0e         is FPOP0C | FPOP0E

  [ fid   _   fcpy fabs fsqrt frnd fneg fnegabs ]
                 is fp0c0e & class_21 = 0 & sub_16 = {0 to 7}

  flt_c0         is fid | fcpy | fabs | fsqrt | frnd | fneg | fnegabs

  [ fcnvff  fcnvxf  fcnvfx  fcnvfxt]
                 is fp0c0e & class_21 = 1 & sub_14 = {0 to 3}

  flt_c1         is fcnvff | fcnvxf | fcnvxf | fcnvfxt

  fcmp           is fp0c0e              & class_21 = 2
  flt_c2_0c      is fcmp & FPOP0C
  flt_c2_0e      is fcmp & FPOP0E

  [ fadd fsub fmpy fdiv ] is fp0c0e & class_21 = 3 & sub_16 = { 0 to 3}
  

constructors
 
# heh .. instruction completers constructed to specific completers


# Instructions!

# Floating point
# Class 0, opcode 0C
  flt_c0 ,fmt r,t          { op = 0x0C }
                           is flt_c0 & r_06=r & t_27=t & fmt_19 = fmt

# Class 0, opcode 0E
  flt_c0 ,f r,t            { op = 0x0E,
                             r@[0:4] = r_06, r@[5] = r1_24, r@[6:31] = 0,
                             t@[0:4] = t_27, t@[5] = t_25,  t@[6:31] = 0 } 
                           is flt_c0 & f_20 = f & r_06 & r1_24 & t_27 & t_25

# Class 1 (fcnv), opcode 0C
# Note: sub_14 (not sub_16) determines the type of conversion performed
  flt_c1  ,sf,df r,t       {op = 0x0C }
                           is flt_c1 & r_06=r & t_27=t & df_17 = df & sf_19 = sf
# Class 1 (fcnv), opcode 0E
  flt_c1  ,sf,df r,t       { op = 0x0E,
                                 r@[0:4] = r_06, r@[5] = r1_24, r@[6:31] = 0,
                                 t@[0:4] = t_27, t@[5] =  t_25, t@[6:31] = 0 }
                            is flt_c1 & df_17 = df & sf_19 = sf
                               & r_06 & r1_24 & t_27 & t_25

# Class 2, opcode 0C
  flt_c2_0c ,fmt,c r1,r2   is FPOP0C & class_21 = 2 & r_06 = r1 & r_11 = r2
                              & t_27 = c & fmt_19 = fmt
# Class 2, opcode 0E
  flt_c2_0e ,f_20,c r1,r2  { r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0 }
                           is FPOP0E & class_21 = 2 & t_27 = c & f_20
                             & r_06 & r1_24 & r_11 & f_19

# Class 3, opcode 0C
  flt_c3_0c ,fmt r1,r2,t   is FPOP0C & class_21 = 3 & r_06 = r1 & r_11 = r2
                              & t_27 = t & fmt_19 = fmt
# Class 3, opcode 0E
  flt_c3_0e ,fmt r1,r2,t   { r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0,
                               t@[0:4] = t_27,  t@[5] = t_25,   t@[6:31] = 0 }
                           is FPOP0E & class_21 = 3 & f_20 = fmt
                             & r_06 & r1_24 & r_11 & f_19 & t_27 & t_25



