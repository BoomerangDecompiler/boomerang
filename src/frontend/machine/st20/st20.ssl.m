dnl m4 processed
changequote([[, ]])dnl
#
# RTL description for ST20
#

MAXSIGN32   := 2**31 - 1;    # all bits except sign bit are set
SIGN_32_NEG := -2**31;
INF := 0x7F800000;

INTEGER
[ %Areg, %Breg, %Creg ][32] -> 0..2,
%sp[32] -> 3,
%pc[32] -> -1,
[ %ZF, %CF ][1] -> -1 ;

define([[PUSH]], [[
	*32* %Creg := %Breg
	*32* %Breg := %Areg
	*32* %Areg := $1]])

define([[POPALL]], [[
	*32* %Areg := %Breg
	*32* %Breg := %Creg]])
dnl#	*32* %Creg := -1
dnl	]])

#A was replaced so dont overwrite it
define([[POP1]], [[
	*32* %Breg := %Creg]])
dnl#	*32* %Creg := -1
dnl	]])
define([[POP2]], [[]])

#OP2(op) {
#	*32* %Areg := %Breg op %Areg
#	POP1()
#};

#OP2C(op) {
#	*32* %Areg := [%Breg op %Areg?1:0];
#	POP1()
#};

adc        val      # add constant
	*32* %Areg := %Areg + val;
add              # add
	*32* %Areg := %Breg + %Areg
	POP1();
addc             # add with carry
	_ ;
ajw        val      # adjust work space
	*32* %sp := %sp + (4*val);
andq		 # and
	*32* %Areg := %Breg & %Areg
	POP1();
arot             # anti-rotate stack
	_ ;
ashr             # arithmetic shift right
	#OP2(">>");
	*32* %Areg := %Breg >>A %Areg
	POP1();
biquad           # biquad IIR filter step
	_ ;
bitld            # load bit
	_ ;
bitmask          # create bit mask
	_ ;
bitst            # store bit
	_ ;
breakpoint       # breakpoint
	_ ;
cj         val      # conditional jump
	*32* tmp := %Areg
	*32* %ZF := [tmp = 0?1:0]
	*32* %Areg := [tmp = 0?%Areg:%Breg]
	*32* %Breg := [tmp = 0?%Breg:%Creg]
	*32* %pc := [tmp = 0?%pc+val:%pc];
cj1        val      # conditional jump
	*32* tmp := %Areg
	*32* %ZF := [tmp = 0?1:0]
	*32* %Areg := [tmp = 0?%Areg:%Breg]
	*32* %Breg := [tmp = 0?%Breg:%Creg];
dequeue          # dequeue a process
	_ ;
divstep          # divide step
	_ ;
dup              # duplicate
	PUSH(%Areg);
ecall            # exception call
	_ ;
enqueue          # enqueue a process
	_ ;
eqc        val      # equals constant
	*32* %Areg := [%Areg = val?1:0];
eret             # exception return
	_ ;
fcall      val   # function call
	_ ;
gajw             # general adjust workspace
	*32* tmp := %sp
	*32* %sp := %Areg
	*32* %Areg := tmp;
gt               # greater than
	#OP2C(">");
	*32* %Areg := [%Breg > %Areg?1:0]
	POP1();
gtu              # greater than unsigned
	#OP2C(">");
	*32* %Areg := [%Breg >u %Areg?1:0]
	POP1();
io               # input/output
	_ ;
j          val   # jump
	*32* %pc := %pc + val;
#	*32* %pc := val;
jab              # jump absolute
	_ ;
lbinc            # load byte and increment
	_ ;
ldc        val      # load constant
	PUSH(val);
ldl        val      # load local
	PUSH(m[%sp + (val*4)]{32});
ldlp       val      # load local pointer
	PUSH(%sp + (val*4));
ldnl       val      # load non-local
	*32* %Areg := m[%Areg + (val * 4)]{32};
ldnlp      val      # load non-local pointer
	*32* %Areg := %Areg + (val * 4);
ldpi             # load pointer to instruction
	*32* %Areg := %pc + (%Areg * 4);
ldprodid         # load product identity
	_ ;
ldtdesc          # load task descriptor
	_ ;
lsinc            # load sixteen and increment
	_ ;
lsxinc           # load sixteen sign extended and increment
	_ ;
lwinc            # load word and increment
	_ ;
mac              # multiply accumulate
	_ ;
mul              # multiply
	#OP2("*");
	*32* %Areg := %Breg * %Areg
	POP1();
nfix       val      # negative prefix
	_ ;
nop              # no operation
	_ ;
not              # bitwise not
	*32* %Areg := ~%Areg;
opr        val      # operate
	_ ;
orq               # or
	#OP2("|");
	*32* %Areg := %Breg | %Areg
	POP1();
order            # order
	_ ;
orderu           # unsigned order
	_ ;
pfix       val      # prefix
	_ ;
rev              # reverse
	*32* tmp := %Areg
	*32* %Areg := %Breg
	*32* %Areg := tmp;
rmw              # read modify write
	_ ;
rot              # rotate stack
	_ ;
run              # run process
	_ ;
saturate         # saturate
	_ ;
sbinc            # store byte and increment
	_ ;
shl              # shift left
	#OP2("<<");
	*32* %Areg := %Breg << %Areg
	POP1();
shr              # shift right
	#OP2(">>");
	*32* %Areg := %Breg >> %Areg
	POP1();
signal           # signal
	_ ;
smacinit         # initialize short multiply accumulate loop
	_ ;
smacloop         # short multiply accumulate loop
	_ ;
smul             # short multiply
	_ ;
ssinc            # store sixteen and increment
	_ ;
statusclr        # clear bits in status register
	_ ;
statusset        # set bits in status register
	_ ;
statustst        # test status register
	_ ;
stl        val      # store local
	*32* m[%sp + (val * 4)] := %Areg
	POPALL();
stnl       val      # store non-local
	*32* m[%Areg + (val * 4) ] := %Breg
	POPALL()
	POPALL();
stop             # stop process
	_ ;
sub              # subtract
	#OP2("-");
	*32* %Areg := %Breg - %Areg
	POP1();
subc             # subtract with carry
	_ ;
swap32           # byte swap 32
	_ ;
swinc            # store word and increment
	_ ;
timeslice        # timeslice
	_ ;
umac             # unsigned multiply accumulate
	_ ;
unsign           # unsign argument
	_ ;
wait             # wait
	_ ;
wsub             # word subscript
	#OP2("*4 +");
	*32* %Areg := (%Breg * 4) + %Areg
	POP1();
xbword           # sign extend byte to word
	*32* %Areg := sgnex(8,32,%Areg{8});
xor              # exclusive or
	#OP2("^");
	*32* %Areg := %Breg ^ %Areg
	POP1();

xsword           # sign extend sixteen to word
	*32* %Areg := sgnex(16, 32, %Areg{16});

# C2-C4 instructions

altq                 # alt start
	_ ;
altend              # alt end
	_ ;
altwt               # alt wait
	_ ;
bcnt                # byte count
	*32* %Areg := %Areg * 32;
bitcnt              # count bits set in word
	_ ;
bitrevnbits         # reverse bottom n bits in word
	_ ;
bitrevword          # reverse bits in word
	_ ;
bsub                # byte subscript
	#OP2("+");
	*32* %Areg := %Breg + %Areg
	POP1();
call    val         # call
	*32* %sp := %sp - 16
	*32* m[%sp] := %pc
	*32* m[%sp+4] := %Areg
	*32* m[%sp+8] := %Breg
	*32* m[%sp+12] := %Creg
#	*32* %Areg := %pc
#	*32* %Breg := -1
#	*32* %Creg := -1
	*32* %pc := %pc + val;
#	*32* %pc := val;
call1   val         # call
	*32* %sp := %sp - 16
	*32* m[%sp] := %pc
	*32* m[%sp+4] := %Areg
	*32* m[%sp+8] := %Breg
	*32* m[%sp+12] := %Creg;
#	*32* %Areg := %pc
#	*32* %Breg := -1
#	*32* %Creg := -1
#	*32* %pc := %pc + val;
#	*32* %pc := val;
call2   val         # call
	*32* %sp := %sp - 16
	*32* m[%sp] := %pc
	*32* m[%sp+4] := %Areg
	*32* m[%sp+8] := %Breg
	*32* m[%sp+12] := %Creg
	*32* %sp := %sp + 16;
#	*32* %Areg := %pc
#	*32* %Breg := -1
#	*32* %Creg := -1
#	*32* %pc := %pc + val;
#	*32* %pc := val;
causeerror          # cause error
	_ ;
cb                  # check byte
	_ ;
cbu                 # check byte unsigned
	_ ;
ccnt1               # check count from 1
	_ ;
cflerr              # check floating point error
	_ ;
cir                 # check in range
	_ ;
ciru                # check in range unsigned
	_ ;
clockdis            # clock disable
	_ ;
clockenb            # clock enable
	_ ;
clrhalterr          # clear halt-on error flag
	_ ;
crcbyte             # calculate CRC on byte
	_ ;
crcword             # calculate CRC on word
	_ ;
cs                  # check sixteen
	_ ;
csngl               # check single
	_ ;
csu                 # check sixteen unsigned
	_ ;
csub0               # check subscript from 0
	_ ;
cword               # check word
	_ ;
devlb               # device load byte
	*32* %Areg := zfill(8,32,m[%Areg]{8});
devls               # device load sixteen
	*32* %Areg := zfill(16,32,m[%Areg]{16});
devlw               # device load word
	*32* %Areg := m[%Areg]{32};
devmove             # device move
	_ ;
devsb               # device store byte
	*8* m[%Areg] := %Breg
	POPALL()
	POPALL();
devss               # device store sixteen
	*16* m[%Areg] := %Breg
	POPALL()
	POPALL();
devsw               # device store word
	*32* m[%Areg] := %Breg
	POPALL()
	POPALL();
diff                # difference
	#OP2("-");
	*32* %Areg := %Breg - %Areg
	POP1();
disc                # disable channel
	_ ;
diss                # disable skip
	_ ;
dist                # disable timer
	_ ;
div                 # divide
	#OP2("/");
	*32* %Areg := %Breg / %Areg
	POP1();
enbc                # enable channel
	_ ;
enbs                # enable skip
	_ ;
enbt                # enable timer
	_ ;
endp                # end process
	_ ;
fmul                # fractional multiply
	_ ;
fptesterr           # test for FPU error
	_ ;
gcall               # general call
	*32* tmp := %pc
	*32* %pc := %Areg
	*32* %Areg := tmp;
gintdis             # general interrupt disable
	_ ;
gintenb             # general interrupt enable
	_ ;
in                  # input message
	_ ;
insertqueue         # insert at front of scheduler queue
	_ ;
intdis              # (localised) interrupt disable
	_ ;
intenb              # (localised) interrupt enable
	_ ;
iret                # interrupt return
	*32* %Creg := m[%sp+20]{32}
	*32* %Breg := m[%sp+16]{32}
	*32* %Areg := m[%sp+12]{32}
	*32* %pc := m[%sp+8]{32}
	*32* %sp := m[%sp+4]{32};
	# no status reg yet
ladd                # long add
	*32* %Areg := %Areg + %Breg + zfill(1,32,%Creg@[0:0])
	POP2();
lb                  # load byte
	*32* %Areg := m[%Areg]{8};
lbx                 # load byte and sign extend
	*32* %Areg := m[%Areg]{8};
ldclock             # load clock
	_ ;
lddevid             # load device identity
	_ ;
ldiff               # long diff
	*33* tmp := %Breg - %Areg - zfill(1,32,%Creg@[0:0])
	*32* %Areg := tmp@[0:31]
	*32* %Breg := tmp@[32:32]
	*32* %Creg := -1;
ldinf               # load infinity
	PUSH(INF);
ldiv                # long divide
	*64* tmp@[0:31] := %Breg
	*64* tmp@[32:63] := %Creg
	*32* %Breg := tmp % %Areg
	*32* %Areg := tmp / %Areg
	*32* %Creg := -1;
ldmemstartval       # load value of MemStart address
	_ ;
ldpri               # load current priority
	_ ;
ldshadow            # load shadow registers
	_ ;
ldtimer             # load timer
	PUSH(0);
ldtraph             # load trap handler
	_ ;
ldtrapped           # load trapped process status
	_ ;
lend                # loop end
	_ ;
lmul                # long multiply
	*64* tmp := %Breg * %Areg
	*32* %Areg := tmp@[0:31]
	*32* %Breg := tmp@[32:63]
	*32* %Creg := -1;
ls                  # load sixteen
	*32* %Areg := m[%Areg]{16};
lshl                # long shift left
	#OP2("<<");
	*32* %Areg := %Breg >> %Areg
	POP1();
lshr                # long shift right
	#OP2(">>");
	*32* %Areg := %Breg >> %Areg
	POP1();
lsub                # long subtract
	_ ;
lsum                # long sum
	_ ;
lsx                 # load sixteen and sign extend
	*32* %Areg := m[%Areg]{16};
mint                # minimum integer
	PUSH(SIGN_32_NEG);
move                # move message
	_ ;		#special function
move2dall           # 2D block copy
	_ ;
move2dinit          # initialize data for 2D block move
	_ ;
move2dnonzero       # 2D block copy non-zero bytes
	_ ;
move2dzero          # 2D block copy zero bytes
	_ ;
norm                # normalize
	_ ;
out                 # output message
	_ ;
outbyte             # output byte
	_ ;
outword             # output word
	_ ;
pop                 # pop processor stack
	POPALL();
postnormsn          # post-normalize correction of single length fp number
	_ ;
prod                # product
	#OP2("*");
	*32* %Areg := %Breg * %Areg
	POP1();
reboot              # reboot
	_ ;
rem                 # remainder
	#OP2("%");
	*32* %Areg := %Breg % %Areg
	POP1();
resetch             # reset channel
	_ ;
restart             # restart
	_ ;
ret                 # return
	*32* %pc := m[%sp]{32}
	*32* %sp := %sp + 16;
roundsn             # round single length floating point number
	_ ;
runp                # run process
	_ ;
satadd              # saturating add
	_ ;
satmul              # saturating multiply
	_ ;
satsub              # saturating subtract
	_ ;
saveh               # save high priority queue registers
	_ ;
savel               # save low priority queue registers
	_ ;
sb                  # store byte
	*8* m[%Areg{32}] := %Breg@[0:7]
	POP2();
seterr              # set error flags
	_ ;
sethalterr          # set halt-on error flag
	_ ;
settimeslice        # set timeslicing status
	_ ;
slmul               # signed long multiply
	_ ;
ss                  # store sixteen
	*16* m[%Areg{32}] := %Breg@[0:15]
	POP2();
ssub                # sixteen subscript
	#OP2("*2 +");
	*32* %Areg := (%Breg *2) + %Areg
	POP1();
startp              # start process
	_ ;
stclock             # store clock register
	_ ;
sthb                # store high priority back pointer
	_ ;
sthf                # store high priority front pointer
	_ ;
stlb                # store low priority back pointer
	_ ;
stlf                # store low priority front pointer
	_ ;
stoperr             # stop on error
	_ ;
stopp               # stop process
	_ ;
stshadow            # store shadow registers
	_ ;
sttimer             # store timer
	_ ;
sttraph             # store trap handler
	_ ;
sttrapped           # store trapped process
	_ ;
sulmul              # signed timer unsigned long multiply
	_ ;
sum                 # sum
	_ ;
swapqueue           # swap scheduler queue
	_ ;
swaptimer           # swap timer queue
	_ ;
talt                # timer alt start
	_ ;
taltwt              # timer alt wait
	_ ;
testerr             # test error flag
	_ ;
testhalterr         # test halt-on error flag
	_ ;
testpranal          # test processor analysing
	_ ;
tin                 # timer input
	_ ;
trapdis             # trap disable
	_ ;
trapenb             # trap enable
	_ ;
tret                # trap return
	_ ;
unpacksn            # unpack single length fp number
	_ ;
wcnt                # word count
	_ ;
wsubdb              # form double word subscript
	#OP2("*8 +");
	*32* %Areg := (%Breg *8) + %Areg
	POP1();
xdble               # extend to double
	*32* %Creg := %Breg
	*32* %Breg := [%Areg@[31:31] = 0?0:-1];
xword               # extend word
	_ ;
