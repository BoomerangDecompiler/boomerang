# instruction fields go [0..31] left-->right
bit 0 is most significant


fields of instruction (32)
    inst 00:31
      op 00:05

    sub_16 16:18    r_06   6:10  t_27 27:31  fmt_19 19:20    f_20 20:20
    class_21 21:22  r1_24 24:24  t_25 25:25    r_11 11:15    f_19 19:19
      uid_23 23:24   u_18 18:18  m_26 26:26 addr_19 19:19 addr_22 22:22
        a_18 18:18   x_11 11:15 s2_16 16:17    b_06 06:10  im5_11 11:15
      im4_11 11:14 im1_15 15:15

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

patterns
# Floating point loads and stores [Table 6-8]
  floads         is any of [ fldwx flddx fldws fldds],
                 which is uid_23 < 2 & addr_19 = [0 1] & [ Copr_w Copr_dw]
                    & addr_22 = 0
  fstores        is any of [ fstwx fstdx fstws fstds],
                 which is uid_23 < 2 & addr_19 = [0 1] & [ Copr_w Copr_dw]
                    & addr_22 = 1
  fld_st         is fldwx | flddx | fldws | fldds
                  | fstwx | fstdx | fstws | fstds

 # Floating point operations (Table 6-9)

  fp0c0e         is FPOP0C | FPOP0E

  [ fadd   fsub   fmpy   fdiv ]
                 is FPOP0C & class_21 = 3 & sub_16 = { 0 to 3}

  ins_flt_c3      is fadd   | fsub   | fmpy   | fdiv


constructors
# Floating point loads and stores
# Addressing modes
patterns
  ins_faddr_s                 is u_18 = 1 & m_26 = 0 & addr_19 = 0
  ins_faddr_m                 is u_18 = 0 & m_26 = 1 & addr_19 = 0
  ins_faddr_sm                is u_18 = 1 & m_26 = 1 & addr_19 = 0
  ins_faddr_x                 is u_18 = 0 & m_26 = 0 & addr_19 = 0
  ins_faddr_mb                is a_18 = 1 & m_26 = 0 & addr_19 = 1
  ins_faddr_ma                is a_18 = 0 & m_26 = 1 & addr_19 = 1
  ins_faddr_si                is            m_26 = 0 & addr_19 = 1
  ins_faddr_all  is  ins_faddr_s  | ins_faddr_m  | ins_faddr_sm | ins_faddr_x
                   | ins_faddr_mb | ins_faddr_ma | ins_faddr_si

constructors
  c_faddrs        : c_faddr is ins_faddr_all

  index_faddr (x, s, b) : faddr is x_11 = x   & s2_16 = s & b_06 = b
                                    & addr_22 = 0
#  sdisps_faddr(d!, s, b): faddr { d@[0:3 ] = im5_11@[1:4],
#                                  d@[4:31] = im5_11@[0]! }
  sdisps_faddr(d!, s, b) : faddr { d@[0:3 ] = im4_11,
                                   d@[4:31] = im1_15! }
                                is s2_16 = s & b_06 = b & addr_22 = 1 & im4_11
                                  & im1_15

  floads  ,c_faddr faddr, t_27
  fstores ,c_faddr r, faddr            is fstores & c_faddr & t_27 = r & faddr

# Floating point
# Class 3, opcode 0C
  flt_c3.C ,fmt r1,r2,t   : t_flt_c3
                          is ins_flt_c3 & r_06 = r1 & r_11 = r2 & t_27 = t
                             & fmt_19 = fmt & op = 0x0C
# Class 3, opcode 0E
  flt_c3.E ,fmt r1,r2,t   : t_flt_c3
                          {  r1@[0:4] = r_06, r1@[5] = r1_24, r1@[6:31] = 0,
                             r2@[0:4] = r_11, r2@[5] = f_19,  r2@[6:31] = 0,
                              t@[0:4] = t_27,  t@[5] = t_25,   t@[6:31] = 0 }
                           is ins_flt_c3 & f_20 = fmt
                             & r_06 & r1_24 & r_11 & f_19 & t_27 & t_25
                             & op = 0x0E


constructors
  flt_c3 t_flt_c3           is t_flt_c3 & ins_flt_c3

