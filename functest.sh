#!/bin/bash
# functest.sh functional test script $Revision$	# 1.23.2.1
# Note: to test with data flow based type analysis, pass a parameter of -Td
#
# 02 Feb 05 - Mike: Conditional tests for no type analysis. So all tests should pass whether -Td is passed or not
# 06 Feb 05 - Mike: Pass the test-set parameter to testOne.sh
# 24 Dec 05 - Gerard: Support for more than one switch added
#

# On MSYS we need to invoke sh ourselves
KERNEL=`uname -s`
TESTONE="./testOne.sh"				# For most systems
if [ ${KERNEL:0:4} = "MSYS" ]		# Older MinGW
	then TESTONE="sh testOne.sh"
fi
if [ ${KERNEL:0:5} = "MINGW" ]		# More modern MinGW
	then TESTONE="sh testOne.sh"
fi

# Store the command line switches in BOOMSW
BOOMSW=$*

# Look for -Td to see if type analysis is enabled
# If it isn't, we will skip some tests
TYPEANALYSIS=""
while [ "$1" ]; do
	if [ "$1" = "-Td" ]; then TYPEANALYSIS="YES"; fi
	shift
done
if [ "$TYPEANALYSIS" ]
then
	echo Type analysis enabled
else
	echo "Note: Type analysis is disabled"
	echo "      To enable type analysis use the \"-Td\" switch"
fi

# Clean up
rm -rf functest
mkdir functest
rm -rf functests.out

# Run the tests
echo
echo === These should be OK ===
$TESTONE pentium hello			1 "$BOOMSW"
$TESTONE sparc   hello			1 "$BOOMSW"
$TESTONE ppc	 hello			1 "$BOOMSW"
$TESTONE pentium twoproc		1 "$BOOMSW"
$TESTONE sparc   twoproc		1 "$BOOMSW"
$TESTONE ppc	 twoproc		1 "$BOOMSW"
$TESTONE pentium param1			1 "$BOOMSW"
$TESTONE sparc   param1			4 "$BOOMSW" '2 3 4'
$TESTONE pentium restoredparam	3 "$BOOMSW" '2 3'
$TESTONE pentium fib			1 "$BOOMSW"
$TESTONE sparc   fib			1 "$BOOMSW"
$TESTONE pentium fibo-O4		1 "$BOOMSW" < test/source/fibo-O4.in1
$TESTONE sparc   fibo-O4		1 "$BOOMSW" < test/source/fibo-O4.in1
$TESTONE pentium fibo-O4		3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE sparc   fibo-O4		3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE pentium fibo3			3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE sparc   fibo3			3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE pentium fibo4			3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE pentium global1		1 "$BOOMSW"
$TESTONE sparc   global1		1 "$BOOMSW"
$TESTONE pentium global2		1 "$BOOMSW"
$TESTONE sparc   global2		1 "$BOOMSW"
$TESTONE pentium global3		1 "$BOOMSW"
$TESTONE sparc   global3		1 "$BOOMSW"

$TESTONE pentium minmax			1 "$BOOMSW" '2 3 4'
$TESTONE sparc   minmax			1 "$BOOMSW" '2 3 4'
$TESTONE pentium minmax2		1 "$BOOMSW" 'two'
$TESTONE sparc   minmax2		1 "$BOOMSW" 'two'
$TESTONE pentium minmax3		1 "$BOOMSW" 'two'
$TESTONE pentium uns			1 "$BOOMSW" '2 3'
$TESTONE sparc   uns			1 "$BOOMSW" '2 3'
$TESTONE pentium fromssa2		1 "$BOOMSW"
$TESTONE sparc   fromssa2		1 "$BOOMSW"
$TESTONE pentium bswap			1 "$BOOMSW"
$TESTONE pentium testset		1 "$BOOMSW" '2 3 4'
$TESTONE sparc   andn			1 "$BOOMSW"
$TESTONE pentium callchain		1 "$BOOMSW"
$TESTONE sparc	 callchain		1 "$BOOMSW"
$TESTONE pentium short1			1 "$BOOMSW"
$TESTONE sparc short1			1 "$BOOMSW"
$TESTONE pentium short2			1 "$BOOMSW"
$TESTONE sparc short2			1 "$BOOMSW"
$TESTONE pentium sumarray-O4	1 "$BOOMSW"


if [ ! "$TYPEANALYSIS" ]
then
	echo
	echo === Skipping tests which require type analysis ===
else
	$TESTONE sparc   sumarray-O4	1 "$BOOMSW"
	$TESTONE pentium paramchain		1 "$BOOMSW"
	$TESTONE sparc	 paramchain		1 "$BOOMSW"

	#$TESTONE pentium line1			1 "$BOOMSW" test/source/line1.c
	#$TESTONE sparc   line1			1 "$BOOMSW" test/source/line1.c

	echo
	echo === Switch tests ===
	$TESTONE pentium switch_gcc		5 "$BOOMSW" '2 3 4 5'
	$TESTONE sparc   switch_gcc		5 "$BOOMSW" '2 3 4 5'
	$TESTONE pentium switch_cc		4 "$BOOMSW" '2 3 4'
	$TESTONE sparc   switch_cc		4 "$BOOMSW" '2 3 4'
	$TESTONE pentium switch_gcc		1 "$BOOMSW"
	$TESTONE sparc   switch_gcc		1 "$BOOMSW"
	$TESTONE pentium switch_cc		1 "$BOOMSW"
	$TESTONE sparc   switch_cc		1 "$BOOMSW"
	$TESTONE pentium nestedswitch	4 "$BOOMSW" '2 3 4'
	$TESTONE sparc   nestedswitch	4 "$BOOMSW" '2 3 4'
	
	echo
	echo === Double handling tests ===
	$TESTONE pentium printpi		1 "$BOOMSW"
	$TESTONE sparc   printpi		1 "$BOOMSW"

	echo
	echo === Pentium floating point handling ===
	# Note: these have a .sed file, for 2 known problems: 1) "%f" in scanf means float, not double
	# 2) Don't handle two halves of a double properly for printf
	$TESTONE pentium fbranch		4 $BOOMSW < test/source/fbranch.in4
	$TESTONE pentium fbranch		5 $BOOMSW < test/source/fbranch.in5
	$TESTONE pentium fbranch		6 $BOOMSW < test/source/fbranch.in6
fi

echo
echo === Sometimes issues with the GC ===
$TESTONE sparc   global2		1 "$BOOMSW"
$TESTONE sparc   global3		1 "$BOOMSW"

echo
echo === Intermittent failure due to incorrect switch analysis ===
$TESTONE pentium recursion		2 "$BOOMSW" 2

echo
echo === Known faillures ===
$TESTONE pentium stattest		1 "$BOOMSW"		# TA does not handle structs properly yet
$TESTONE sparc   stattest		1 "$BOOMSW"		# ditto
$TESTONE sparc	elfhashtest		1 "$BOOMSW"		# Not sure why fails
# Specials for Mike
$TESTONE sparc   fibo4			3 "$BOOMSW" < test/source/fibo-O4.in3	# Some problem with bypassing m[...]
$TESTONE sparc   recursion		2 "$BOOMSW" 2	# Not sure why fails
$TESTONE pentium recursion2		1 "$BOOMSW"

echo
echo === Done ===
echo
