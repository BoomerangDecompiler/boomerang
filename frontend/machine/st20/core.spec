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
	primary is any of [ ldlp _ ldnl ldc ldnlp _ ldl adc _ _ ajw eqc stl stnl],
			which is fc = {1 to 0xE}
	j		is fc = 0
	pfix	is fc = 2
	nfix	is fc = 6
	call	is fc = 9
	cj		is fc = 0xA


constructors
	primary	oper		is primary & bot = oper	# & fc != 2 # & fc != 6 & fc != 0xF

	j		oper		is j	& bot = oper
	call	oper		is call	& bot = oper
	cj		oper		is cj	& bot = oper

	

	testpranal			is all = 0x22 ; all = 0xFA
	saveh				is all = 0x23 ; all = 0xFE
	savel				is all = 0x23 ; all = 0xFD
	sthf				is all = 0x21 ; all = 0xF8
	sthb				is all = 0x25 ; all = 0xF0
	stlf				is all = 0x21 ; all = 0xFC
	stlb				is all = 0x21 ; all = 0xF7
	sttimer				is all = 0x25 ; all = 0xF4
	lddevid				is all = 0x21 ; all = 0x27 ; all = 0xFC
	ldmemstartval		is all = 0x27 ; all = 0xFE

	andq				is all = 0x24 ; all = 0xF6
	orq					is all = 0x24 ; all = 0xFB
	xor					is all = 0x23 ; all = 0xF3
	not					is all = 0x23 ; all = 0xF2
	shl					is all = 0x24 ; all = 0xF1
	shr					is all = 0x24 ; all = 0xF0

	add					is all = 0xF5
	sub					is all = 0xFC
	mul					is all = 0x25 ; all = 0xF3
	fmul				is all = 0x27 ; all = 0xF2
	div					is all = 0x22 ; all = 0xFC
	rem					is all = 0x21 ; all = 0xFF
	gt					is all = 0xF9
	gtu					is all = 0x25 ; all = 0xFF
	diff				is all = 0xF4
	sum					is all = 0x25 ; all = 0xF2
	prod				is all = 0xF8
	satadd				is all = 0x26 ; all = 0xF8
	satsub				is all = 0x26 ; all = 0xF9
	satmul				is all = 0x26 ; all = 0xFA

	ladd				is all = 0x21 ; all = 0xF6
	lsub				is all = 0x23 ; all = 0xF8
	lsum				is all = 0x23 ; all = 0xF7
	ldiff				is all = 0x24 ; all = 0xFF
	lmul				is all = 0x23 ; all = 0xF1
	ldiv				is all = 0x21 ; all = 0xFA
	lshl				is all = 0x23 ; all = 0xF6
	lshr				is all = 0x23 ; all = 0xF5
	norm				is all = 0x21 ; all = 0xF9
	slmul				is all = 0x26 ; all = 0xF4
	sulmul				is all = 0x26 ; all = 0xF5

	rev					is all = 0xF0
	xword				is all = 0x23 ; all = 0xFA
	cword				is all = 0x25 ; all = 0xF6
	xdble				is all = 0x21 ; all = 0xFD
	csngl				is all = 0x24 ; all = 0xFC
	mint				is all = 0x24 ; all = 0xF2
	dup					is all = 0x25 ; all = 0xFA
	pop					is all = 0x27 ; all = 0xF9
	reboot				is all = 0x68 ; all = 0xFD

	bsub				is all = 0xF2
	wsub				is all = 0xFA
	wsubdb				is all = 0x28 ; all = 0xF1
	bcnt				is all = 0x23 ; all = 0xF4
	wcnt				is all = 0x23 ; all = 0xFF
	lb					is all = 0xF1
	sb					is all = 0x23 ; all = 0xFB

	in					is all = 0xF7
	out					is all = 0xFB
	outword				is all = 0xFF
	outbyte				is all = 0xFE

	ret					is all = 0x22 ; all = 0xF0
	ldpi				is all = 0x21 ; all = 0xFB
	gcall				is all = 0xF6
	lend				is all = 0x22 ; all = 0xF1

	startp				is all = 0xFD
	endp				is all = 0xF3

	fptesterr			is all = 0x29 ; all = 0xFC

	nop					is all = 0x63 ; all = 0xF0


	

# Place these after all the seconary instruction constructors, because otherwise all the secondary instructions would
# match to pfix or nfix or the opr instructions
	pfix	oper		is pfix & bot = oper
	nfix	oper		is nfix & bot = oper
	
