#
# Copyright (C) 2005, Mike Van Emmerik
#
# See the file "LICENSE.TERMS" for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL
# WARRANTIES.
#

# 10 Mar 05 - Mike: Created

fields of opcodet (8) fc 4:7 bot 0:3
	op1 0:3
	op2 0:3
	op3 0:3
	all 0:7
                     

patterns
	primary is any of [ 		ldlp 	_ 		ldnl 		ldc 	ldnlp 	_ 		ldl
						adc 	_		_		ajw			eqc		stl		stnl		],
			which is fc = {1 to 0xE}
	j		is fc = 0
	pfix	is fc = 2
	nfix	is fc = 6
	call	is fc = 9
	cj		is fc = 0xA
	opr		is fc = 0xF


constructors
	primary	oper		is primary & bot = oper

	j		oper		is j	& bot = oper
	pfix	oper		is pfix & bot = oper
	nfix	oper		is nfix & bot = oper
	call	oper		is call	& bot = oper
	cj		oper		is cj	& bot = oper
	opr		oper		is opr	& bot = oper

	
