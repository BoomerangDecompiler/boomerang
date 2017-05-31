#
# Copyright (C) 1996,1999, The University of Queensland
# Copyright (C) 1996, Norman Ramsey???
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# File: sparcdis.spec
# Desc: toolkit details for a Sparc disassembler

# Created by Cristina - 96 
# 5 Mar 96 - Norman
#	added patterns
# 3 Mar 99 - Doug
#   changed 'fetch' and 'address' specifications so that they conform to the
#   ones hard-coded into the SML version of the toolkit (which currently doesn't
#   actually use this part of a SLED spec) 

patterns 
  load_greg is loadg | LDD 
  load_freg is LDF | LDDF
  load_creg is LDC | LDDC
  load_asi  is loada | LDDA

  sto_greg is storeg | STD 
  sto_freg is STF | STDF
  sto_creg is STC | STDC
  sto_asi  is storea | STDA

float_cmp  is fcompares | fcompared | fcompareq 

# interface to NJ 
address type is "DWord"
address to integer using "%a"
#address to integer using "%a - instr + pc"
address add using "%a+%o"
fetch 32 using "getDword(%a)"
#fetch 16 using "getWord"
#fetch 8  using "getByte"
