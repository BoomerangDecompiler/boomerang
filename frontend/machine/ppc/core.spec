# FILE:     core.spec
# OVERVIEW: This is the New Jersey Machine Code Toolkit core specification file
#           for the Power PC processor
#
# $Revision$

#### Copyright (c) 1995 Flavors Technology, Inc.
#### This file may be distributed freely as long as this notice remains.
#### Please send comments, corrections, or questions to: <e@flavors.com>
#### 14Jul95 e

# this file is to be processed by NJ Machine-Code Toolkit, version 0.1a
# in MPW: njmcte -byteorder X -foldemit -encoder "/ppc.out" "/ppc.spec"

# references to Tables and Sections in the comments below refer to
#   PowerPC Microprocessor Family: The Programming Environments
#     by IBM (aka MPRPPCFPE-01) & Motorola (aka MPCFPE/AD)

# I have all the PowerPC 32-bit instructions done and most of the 64 bit
# instructions; I may not do all the 64 bit instructions since the chip I'm
# targeting (PPC 604) doesn't support them -- the first chip that will, the
# PPC 620, isn't ready yet. I'm now working on the synthetic instructions.

# No testing has been done yet, but my visual inspection of the generators
# and the prototypes indicates that all is well.

# MVE: Periods (dot characters) are bad news for SSL files. So the letter "q" is used instead (means set flags).


fields of instruction (32)
 OPCD 26:31
 LI    2:25
 AA    1:1
 LK    0:0
 BD    2:15
 BO   21:25
 BO4  22:25		# Upper 4 bits of BO
 Bo   21:25
 S    21:25
 fS   21:25
 D    21:25
 fD   21:25
 crbD 21:25
 crfD 23:25
 crDz 21:22
 L    21:21
 Lz   22:22
 TO   21:25
 BI   16:20
 BIcr 18:20
 BIcc 16:17
 A    16:20
 fA   16:20
 crbA 16:20
 crfS 18:20
 crSz 16:17
 B    11:15
 fB   11:15
 crbB 11:15
 NB   11:15
 SH   11:15
 C     6:10
 fC    6:10
 MB    6:10
 ME    1:5
 d     0:15
 SIMM  0:15
 UIMM  0:15
 ds    2:15
 Xo0   0:1
 Xo1   1:10
 Xo2   2:10
 Xo4   2:4
 Xo5   1:5
 Xo9   1:9
 OE   10:10
 SR   16:19
 SRz  20:20
 Rc    0:0
 IMM  12:15
 IMMz 11:11
 sprH 11:15 # spr  11:20  # not used, 5 bit subfields are swapped
 sprL 16:20
 tbrH 11:15 # tbr  11:20  # not used, 5 bit subfields are swapped
 tbrL 16:20
 FM   17:24 # mtfsf
 FMz1 16:16 # mtfsf
 FMz2 25:25 # mtfsf
 CRM  12:19 # mtcrf
 CRz1 11:11 # mtcrf
 CRz2 20:20 # mtcrf
 mbe   5:5

fieldinfo [ S D A B C ] is
 [ guaranteed
   names [ r0  r1  r2  r3  r4  r5  r6  r7  r8  r9  r10 r11 r12 r13 r14 r15
           r16 r17 r18 r19 r20 r21 r22 r23 r24 r25 r26 r27 r28 r29 r30 r31 ] ]

fieldinfo [ fS fD fA fB fC ] is
 [ guaranteed
   names [ fr0  fr1  fr2  fr3  fr4  fr5  fr6  fr7
           fr8  fr9  fr10 fr11 fr12 fr13 fr14 fr15
           fr16 fr17 fr18 fr19 fr20 fr21 fr22 fr23
           fr24 fr25 fr26 fr27 fr28 fr29 fr30 fr31 ] ]

fieldinfo [ SIMM UIMM d ds ] is [ checked ]   # suppresses masking

fieldinfo [ SH MB ME NB SR ] is [ checked ]   # suppresses masking

fieldinfo [ IMM CRM FM L ]    is [ unchecked ] # masked only

fieldinfo [ BO BI crfD crfS ] is [ unchecked ] # masked only

fieldinfo [ crbA crbB crbD ]  is [ unchecked ] # masked only

fieldinfo TO is
 [ guaranteed
   sparse [ lgt =  1, llt = 2, eq = 4,
            lge =  5, lle = 6,
            lnl =  5, lng = 6,
             gt =  8,  ge = 12,  nl = 12,
             lt = 16,  le = 20,  ng = 20,
             ne = 24 ] ]

fieldinfo BIcr is [ guaranteed names [ cr0 cr1 cr2 cr3 cr4 cr5 cr6 cr7 ] ]

## suffix op fields

fieldinfo Rc is [ names [ "" "q" ] ]
fieldinfo OE is [ names [ "" "o" ] ]
fieldinfo LK is [ names [ "" "l" ] ]
fieldinfo AA is [ names [ "" "a" ] ]

## primary opcode

patterns
 [ _      _      tdi    twi    _      _      _      mulli
   subfic _      Cmpli  Cmpi   addic  addicq addi   addis
   bc     Sc     b      cr_dx  rlwimi rlwinm _      rlwnm
   ori    oris   xori   xoris  andiq  andisq rl_64  ab_dx
   lwz    lwzu   lbz    lbzu   stw    stwu   stb    stbu
   lhz    lhzu   lha    lhau   sth    sthu   lmw    stmw
   lfs    lfsu   lfd    lfdu   stfs   stfsu  stfd   stfdu
   _      _      ld_dx  s_dx   _      _      std_dx d_dx   ]
 is OPCD = { 0 to 63 }

### instruction classes & extended opcodes by instruction format

## I-Form

# no patterns needed

## B-Form

# no patterns needed

## SC-Form

sc is Sc & BO = 0 & BI = 0 & BD = 0 & LK = 0 & AA = 1

## D-Form

Ddad_ is lwz | lwzu | lbz | lbzu | lhz | lhzu | lha | lhau | lmw
Ddaf_ is lfs | lfsu | lfd | lfdu

Ddasi_ is mulli | subfic | addic | addicq | addi | addis

Dsad_ is stw | stwu | stb | stbu | sth | sthu | stmw
Dsaf_ is stfs | stfsu | stfd | stfdu

Dsaui_ is ori | oris | xori | xoris | andiq | andisq

cmpi  is Cmpi  & Lz = 0
cmpli is Cmpli & Lz = 0

Dtoas_ is tdi | twi

## DS-Form

DSld_ is any of [ ld ldu lwa ], which is ld_dx & Xo0 = {0 to 2}
DSst_ is any of [ std stdu ],  which is std_dx & Xo0 = {0 to 1}

## X-Form

Xdab_ is any of [ eciwx lbzux lbzx ldarx ldux  ldx lhaux lhax lhbrx
                  lhzux lhzx lswx lwarx lwaux lwax lwbrx lwzux lwzx ],
 which is Xo1 = [  310   119   87    84    53   21  375   343  790
                   311   279  533   20   373  341   534    55    23 ]
 & ab_dx
 & Rc = 0

# Floating point indexed loads
Xdaf_ is any of [  lfdux lfdx lfsux lfsx  ],
 which is Xo1 = [   631   599  567   535  ]
 & ab_dx
 & Rc = 0

lswi   is ab_dx         & Rc = 0 & Xo1 = 597

mfsrin is ab_dx & A = 0 & Rc = 0 & Xo1 = 659


Xd_ is any of  [ mfcr mfmsr ],
 which is Xo1 = [ 19    83  ]
 & ab_dx
 & A = 0
 & B = 0
 & Rc = 0

mfsr is ab_dx & B = 0 & Rc = 0 & SRz = 0 & Xo1 = 595

Xsabx_ is any of [ and andc eqv nand nor  or orc sld slw srad sraw srd srw xor ],
 which is Xo1 = [   28  60  284  476 124 444 412  27  24  794  792 539 536 316 ] & ab_dx

Xsab0_ is any of [ ecowx stbux stbx stdux stdx
                    sthbrx sthux sthx stswx stwbrx stwux stwx ],
 which is Xo1 = [   438   247   215  181   149
                      918   439   407  661    662   183   151 ]
 & ab_dx
 & Rc = 0

Xsab1_ is any of [ stdcxq stwcxq ],
 which is Xo1 = [   214    150   ]
 & ab_dx
 & Rc = 1

Xsab_ is Xsab0_ | Xsab1_

Xsaf_ is any of [ stfdux stfdx stfiwx stfsux stfsx ],
 which is Xo1 = [  759   727    983    695    663  ]
 & ab_dx
 & Rc = 0

stswi is ab_dx         & Rc = 0 & Xo1 = 725

Xsax_ is any of [ cntlzd cntlzw extsb extsh extsw ],
 which is Xo1 = [   58     26    954   922   986  ]
 & ab_dx
 & B = 0

mtsrin is ab_dx & A = 0           & Rc = 0 & Xo1 = 242

mtmsr  is ab_dx & A = 0   & B = 0 & Rc = 0 & Xo1 = 146

mtsr   is ab_dx & SRz = 0 & B = 0 & Rc = 0 & Xo1 = 210

srawi  is ab_dx & Xo1 = 824

Xcmp_ is any of [ cmp cmpl ],
 which is Xo1 = [  0   32  ]
 & ab_dx
 & Rc = 0 & Lz = 0

Xcab_ is any of [ fcmpo fcmpu ],
 which is Xo1 = [   32    0   ]
 & d_dx
 & Rc = 0 & L = 0 & Lz = 0

mcrfs is d_dx & B = 0 & crSz = 0 & crDz = 0 & Rc = 0 & Xo1 = 64

mcrxr is ab_dx & B = 0 & A = 0 & crDz = 0 & Rc = 0 & Xo1 = 512

mtfsfi is d_dx & IMMz = 0 & A = 0 & crDz = 0       & Xo1 = 134

Xto_ is any of [  td tw ],
 which is Xo1 = [ 68  4 ]
 & ab_dx
 & Rc = 0

Xdbx_ is any of [ fabs fcfid fctid fctidz fctiw fctiwz fmr fnabs fneg frsp ],
 which is Xo1 = [  264  846   814    815    14    15    72  136   40   12  ]
 & d_dx
 & A = 0

mffs is d_dx & A = 0 & B = 0 & Xo1 = 583

mtfsb0 is d_dx & A = 0 & B = 0 & Xo1 = 70

mtfsb1 is d_dx & A = 0 & B = 0 & Xo1 = 38

Xab_ is any of [ dcbf dcbi dcbst dcbt dcbtst dcbz icbi ],
 which is Xo1 = [ 86   470   54   278   246  1014  982 ]
 & ab_dx
 & D = 0
 & Rc = 0

Xb_ is any of [ slbie tlbie ],
 which is Xo1 = [ 434  306  ]
 & ab_dx
 & S = 0
 & A = 0
 & Rc = 0

X_ is any of  [ eieio slbia sync tlbia tlbsync ],
 which is Xo1 = [ 854  498   598  370    566   ]
 & ab_dx
 & S = 0
 & A = 0
 & B = 0
 & Rc = 0

## XL-Form

#XLb_ is any of [ bcctr bclr ],
# which is Xo1 = [ 528   16  ]
# & cr_dx & crbB = 0

XLc_ is any of [ crand crandc creqv crnand crnor cror crorc crxor ],
 which is Xo1 = [ 257    129   289    225    33   449  417   193  ]
 & cr_dx & LK = 0

mcrf  is cr_dx & LK = 0 & crDz = 0 & crSz = 0 & crbB = 0 & Xo1 =   0

isync is cr_dx & LK = 0 & crbD = 0 & crbA = 0 & crbB = 0 & Xo1 = 150
rfi   is cr_dx & LK = 0 & crbD = 0 & crbA = 0 & crbB = 0 & Xo1 =  50

## XFX-Form

ab_dx0 is ab_dx & Rc = 0

         [ mfspr mftb mtspr ]
 is Xo1 = [ 339   371  467  ]
 & ab_dx0

mtcrf is ab_dx0 & Xo1 = 144 & CRz2 = 0 & CRz1 = 0

## XFL-Form

mtfsf is  d_dx  & Xo1 = 711 & FMz2 = 0 & FMz1 = 0

## XS-Form (64 bit instruction)

sradi is  ab_dx  & Xo2 = 413 # split sh field

## XO-Form

XO_ is any of   [ add addc adde divd divdu divw divwu mulld mullw subf subfc subfe ],
 which is Xo9 = [ 266  10   138  489  457   491  459   233   235   40    8   136  ]
 & ab_dx

XOo_ is any of  [ mulhd mulhdu mulhw mulhwu ],
 which is Xo9 = [   73     9     75    11   ]
 & ab_dx
 & OE = 0

XOb_ is any of [ addme addze neg subfme subfze ],
 which is Xo9 = [ 234   202  104   232    200  ]
 & ab_dx
 & B = 0

# A-Form

A1c_ is any of [ fadds fdivs fsubs ],
which is Xo5 = [   21   18    20   ]
 & s_dx
 & C = 0

fmuls is Xo5 = 25 & s_dx & B = 0 # A1b_

A1ac_ is any of [ fres fsqrts ],
which is Xo5 = [   24    22   ]
 & s_dx
 & A = 0
 & C = 0

A1_ is any of [ fmadds fmsubs fnmadds fnmsubs ],
which is Xo5 = [  29     28      31      30   ]
 & s_dx

A2c_ is any of [ fadd fdiv fsub ],
which is Xo5 = [  21   18   20  ]
 & d_dx
 & C = 0

fmul is Xo5 = 25 & d_dx & B = 0 # A2b_

A2ac_ is any of [ frsqrte fsqrt ],
which is Xo5 =  [    26     22  ]
 & d_dx
 & A = 0
 & C = 0

A2_ is any of [ fmadd fmsub fnmadd fnmsub fsel ],
which is Xo5 = [  29    28    31     30    23  ]
 & d_dx

Ac_  is  A1c_  | A2c_
Ab_  is  fmul  | fmuls
Aac_ is  A1ac_ | A2ac_
A_   is  A1_   | A2_

## M-Form

M_ is rlwimi | rlwinm

## MD-Form  (64-bit instructions)

MD_ is any of [ rldic rldicl rldicr rldimi ],
 which is Xo4 = [ 2    0       1     3     ]
 & rl_64

## MDS-Form (64-bit instructions)

MDS_ is any of [ rldcl rldcr ],
 which is AA = [   0     1   ]
 & rl_64
 & Xo4 = 4

# #####

### bookkeeping

trap_e is twi & TO = 31 & A = 0 & SIMM = 0xbad

constructors  trap_e

placeholder   for instruction is trap_e

### the instructions

relocatable reloc

constructors

## I-Form

# very ugly code generated...
#  b^LK^AA reloc
#    when { AA = 0, reloc = $pc + LI * 4 } is b & LK & AA & LI
#    when { AA = 1, reloc =       LI * 4 } is b & LK & AA & LI

   b^LK     reloc { reloc = L + LI! * 4 } is L: b & LK & LI & AA = 0
   b^LK^"a" reloc { reloc =     LI! * 4 } is    b & LK & LI & AA = 1

## B-Form

# very ugly code generated...
#  bc^LK^AA reloc
#    when { AA = 0, reloc = $pc + BD * 4 } is bc & LK & AA & BO & BI & BD
#    when { AA = 1, reloc =       BD * 4 } is bc & LK & AA & BO & BI & BD

   bc^LK     BO,BI,reloc { reloc = L + BD * 4 }
                         is L: bc & LK & BO & BI & BD & AA = 0
   bc^LK^"a" BO,BI,reloc { reloc =     BD * 4 }
                         is    bc & LK & BO & BI & BD & AA = 1

## SC-Form

   sc ()

## D-Form

   Ddad_   D, d!(A)
   Ddaf_  fD, d!(A)
   Ddasi_  D, A, SIMM!
   Dsad_   S, d!(A)
   Dsaf_  fS, d!(A)
   Dsaui_  A, S, UIMM
   cmpi    crfD, L, A, SIMM!
   cmpli   crfD, L, A, UIMM
   Dtoas_  TO, A, SIMM!

## DS-Form

   DSld_ D,x(A) { x = ds! * 4 } is DSld_ & D & ds & A
   DSst_ S,x(A) { x = ds! * 4 } is DSst_ & S & ds & A

## X-Form

   Xdab_      D, A, B
   Xdaf_     fD, A, B
   lswi       D, A, NB
   mfsrin     D, B
   Xd_        D
   mfsr       D, SR
   Xsabx_^Rc  A, S, B
   Xsab_      S, A, B
   Xsaf_     fS, A, B
   stswi      S, A, NB
   Xsax_^Rc   A, S
   mtsrin     S, B
   mtmsr      S
   mtsr       SR, S
   srawi^Rc   A, S, SH
   Xcmp_      crfD, L, A, B
   Xcab_      crfD, fA, fB
   mcrfs      crfD, crfS
   mcrxr      crfD
   mtfsfi^Rc  crfD, IMM
   Xto_       TO, A, B
   Xdbx_^Rc   fD, fB
   mffs^Rc    fD
   mtfsb0^Rc  crbD
   mtfsb1^Rc  crbD
   Xab_       A, B
   Xb_        B
   X_         ()

## XL-Form

#   XLb_^LK  BO, BI			# bcctr, bclr: prefer bltctr to bcctr 12, 0
   XLc_  crbD, crbA, crbB
   mcrf  crfD, crfS
   isync ()
   rfi   ()

## XFX-Form

#  XFX_   # it would be nice if we could say:        spr < 0x400
   mfspr  D, spr { spr@[10:31] = 0 }
                 is mfspr & D & sprH = spr@[5:9] & sprL = spr@[0:4]
   mftb   D, tbr { tbr@[10:31] = 0 }
                 is mftb  & D & tbrL = tbr@[0:4] & tbrH = tbr@[5:9] 
   mtcrf  CRM, S
   mtspr  spr, S { spr@[10:31] = 0 }
                 is mtspr & S & sprH = spr@[5:9] & sprL = spr@[0:4]

## XFL-Form

   mtfsf^Rc  FM, fB

## XS-Form (64 bit instruction)

   sradi^Rc A,S,sh 
              is sradi & Rc & A & S & SH = sh@[0:3] & AA = sh@[4]

## XO-Form

   XO_^OE^Rc  D, A, B
   XOb_^OE^Rc D, A
   XOo_^Rc    D, A, B

# A-Form

   A_^Rc      fD, fA, fC, fB
   Ab_^Rc     fD, fA, fC
   Ac_^Rc     fD, fA, fB
   Aac_^Rc    fD, fB

## M-Form

   M_^Rc      A, S, SH, MB, ME
   rlwnm^Rc   A, S,  B, MB, ME

## MD-Form  (64-bit instructions)

   MD_^Rc A,S,sh,m is MD_  & Rc & A & S & SH = sh@[0:3] & AA  = sh@[4] 
                                        & MB =  m@[0:3] & mbe =  m@[4]

## MDS-Form (64-bit instructions)

   MDS_^Rc A,S,B,m is MDS_ & Rc & A & S & B 
                                        & MB =  m@[0:3] & mbe =  m@[4]

### discard 64 bit operations

discard tdi DSld_ DSst_ # ld ldu lwa std stdu
discard cntlzd^Rc extsw^Rc fcfid^Rc fctid^Rc fctidz^Rc
discard ldarx ldux ldx lwaux lwax  slbia slbie
discard sld^Rc srad^Rc srd^Rc stdcxq stdux stdx td
discard divd^OE^Rc divdu^OE^Rc mulhd^Rc mulhdu^Rc mulld^OE^Rc
discard sradi^Rc MD_^Rc MDS_^Rc

### simplified mnemonics

constructors

## Section F.2

   subi   D,A,v   is addi(   D, A, -v )
   subis  D,A,v   is addis(  D, A, -v )
   subic  D,A,v   is addic(  D, A, -v )
   subicq D,A,v   is addicq( D, A, -v )

   sub   D,A,B is subf(   D, B, A )
   subq  D,A,B is subfq(  D, B, A )
   subo  D,A,B is subfo(  D, B, A )
   suboq D,A,B is subfoq( D, B, A )

   subc   D,A,B is subfc(   D, B, A )
   subcq  D,A,B is subfcq(  D, B, A )
   subco  D,A,B is subfco(  D, B, A )
   subcoq D,A,B is subfcoq( D, B, A )

## Section F.3

# Table F-2

   cmpdi  crfD, A, SIMM! is cmpi(  crfD, 1, A, SIMM! )
   cmpd   crfD, A, B     is cmp(   crfD, 1, A, B )
   cmpldi crfD, A, UIMM  is cmpli( crfD, 1, A, UIMM )
   cmpld  crfD, A, B     is cmpl(  crfD, 1, A, B )

# Table F-3

   cmpwi  crfD, A, SIMM! is cmpi(  crfD, 0, A, SIMM! )
   cmpw   crfD, A, B     is cmp(   crfD, 0, A, B )
   cmplwi crfD, A, UIMM  is cmpli( crfD, 0, A, UIMM )
   cmplw  crfD, A, B     is cmpl(  crfD, 0, A, B )

## Section F.4

# Table F-4 skipped, 64 bit ops

# Table F-5

   extlwi A,S,n,p    is rlwinm( A,S,p,     0,   n- 1   )
   extrwi A,S,n,p    is rlwinm( A,S,p+n,   32-n,31    ) 
   inslwi A,S,n,p    is rlwimi( A,S,32-p,  p,   p+n- 1 )
   insrwi A,S,n,p    is rlwimi( A,S,32-p-n,p,   p+n- 1 )
   rotlwi A,S,n      is rlwinm( A,S,n,     0,   31    )
   rotrwi A,S,n      is rlwinm( A,S,32-n,  0,   31    )
   rotlw  A,S,B      is rlwnm(  A,S,B,     0,   31    )  
   slwi   A,S,n      is rlwinm( A,S,n,     0,   31-n  )
   srwi   A,S,n      is rlwinm( A,S,32-n,  n,   31    )
   clrlwi A,S,n      is rlwinm( A,S,0,     n,   31    )  
   clrrwi A,S,n      is rlwinm( A,S,0,     0,   31-n  )
   clrlslwi A,S,p,n  is rlwinm( A,S,n,     p-n, 31-n  )

# Table F-6

## branches

fieldinfo BIcc is [ sparse [ lt = 0, gt = 1, eq = 2, so = 3, un = 3 ] ]		# un = unordered?

# BO[4] ( 1) branch prediction
# BO[3] ( 2) zero/nz
# BO[2] ( 4) inhibit ctr op
# BO[1] ( 8) t/f
# BO[0] (16) inhibit cc test

fieldinfo BO is [ sparse [ dnzf = 0, dzf =  2,  f =  4,
                           dnzt = 8, dzt = 10,  t = 12,
                           dnz = 16, dz =  18, "" = 20
                       ] ] # "" is branch always

fieldinfo Bo is [ sparse [ dnzf = 0, dzf =  2,  f =  4,
                           dnzt = 8, dzt = 10,  t = 12,
                           dnz = 16, dz =  18 ] ]


patterns
# MVE: These seem to cause error messages
# blt is BO = t & BIcc = lt
# ble is BO = f & BIcc = gt
# beq is BO = t & BIcc = eq
# bge is BO = f & BIcc = lt
# bgt is BO = t & BIcc = gt
# bnl is BO = f & BIcc = lt
# bne is BO = f & BIcc = eq
# bng is BO = f & BIcc = gt
# bso is BO = t & BIcc = so
# bns is BO = f & BIcc = so
# bun is BO = t & BIcc = un
# bnu is BO = f & BIcc = un

# Note that the LSB of BO is the y bit, which is the + or - branch prediction hint.
# We want to ignore that, hence the use of BO4 (upper 4 bits of BO)
ifcctrue  is BO4 = 6
ifccfalse is BO4 = 2

 blt is ifcctrue  & BIcc = 0		# t, lt
 ble is ifccfalse & BIcc = 1		# f, gt
 beq is ifcctrue  & BIcc = 2		# t, eq
 bge is ifccfalse & BIcc = 0		# f, lt
 bgt is ifcctrue  & BIcc = 1		# t, gt
 bnl is ifccfalse & BIcc = 0		# f, lt
 bne is ifccfalse & BIcc = 2		# f, eq
 bng is ifccfalse & BIcc = 1		# f, gt
 bso is ifcctrue  & BIcc = 3		# t, so
 bns is ifccfalse & BIcc = 3		# f, so
 bun is ifcctrue  & BIcc = 3		# t, un		Note: branch if UNordered (not unconditional)
 bnu is ifcctrue  & BIcc = 3		# f, un		Note: branch if not unordered
 bal is BO = 20     				# always, any (no branch prediction bit here)

bcc_ is blt | ble | beq | bge | bgt | bnl | bne | bng | bso | bns | bun | bnu | bal

constructors

# Table F-6

# see "I wish I could say" below
#
# "b"^Bo^LK^AA   BIcr    is bc & AA & LK & BIcr & Bo

#  "b"^BO^"lr"^LK BIcr    is bclr    & LK & BIcr & BO		# See bcc_^"lr"... below

# Table F-8

# I wish I could say:
# bcc_     BIcr, reloc { BI = BIcr * 4 + BIcc } is bc ( BO, BI, reloc )
# bcc_^"a" BIcr, reloc { BI = BIcr * 4 + BIcc } is bca( BO, BI, reloc )
# etc., and that this would make:
# void bXX(unsigned BIcr, RAddr reloc) { bc( t/f, ( BIcr * 4 + XX ), reloc ); }
# but it doesn't work, and this makes way too much code...

bcc_^LK     BIcr, reloc { reloc = L + BD * 4 } is L: bc & LK & BIcr & bcc_ & BD & AA = 0
bcc_^LK^"a" BIcr, reloc { reloc =     BD * 4 } is    bc & LK & BIcr & bcc_ & BD & AA = 1

#bcc_^"lr"^LK  BIcr     is bclr    & LK & BIcr & bcc_
#bcc_^"ctr"^LK BIcr     is bcctr   & LK & BIcr & bcc_

bcc_^"lr"^LK  BIcr     is Xo1 =  16 & cr_dx & crbB = 0   & LK & BIcr & bcc_
bcc_^"ctr"^LK BIcr     is Xo1 = 528 & cr_dx & crbB = 0   & LK & BIcr & bcc_
#XLb_ is any of [ bcctr bclr ],
# which is Xo1 = [ 528   16  ]
# & cr_dx & crbB = 0

