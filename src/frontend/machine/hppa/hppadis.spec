#
# Copyright (C) 2001, The University of Queensland
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# File: hppadis.spec
# Desc: toolkit details for a PA-RISC disassembler (copied from Sparc spec)

# interface to NJ 
address type is "DWord"
address to integer using "%a"
#address to integer using "%a - instr + pc"
address add using "%a+%o"
fetch 32 using "getDword(%a)"
