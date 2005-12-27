#!/bin/bash
# functest.sh functional test script $Revision$	# 1.23.2.1
# Note: to test with data flow based type analysis, pass a parameter of -Td
#
# 02 Feb 05 - Mike: Conditional tests for no type analysis. So all tests should pass whether -Td is passed or not
# 06 Feb 05 - Mike: Pass the test-set parameter to testOne.sh
# 24 Dec 05 - Gerard: Support for more than one switch added
#
TESTONE=./testOne.sh
#TESTONE=sh testOne.sh	# For MinGW (configure me one day)
BOOMSW=$*
TYPEANALYSIS=""
# look for -Td to see if type analysis is enabled
while [ "$1" ]; do
	if [ "$1" = "-Td" ]; then TYPEANALYSIS="YES"; fi
	shift
done
echo Boomerang switches are $BOOMSW
if [ "$TYPEANALYSIS" ]
then
  echo Type analysis enabled
else
  echo "Note: Type analysis is disabled"
  echo "      To enable type analysis use the \"-Td\" switch"
fi
echo

rm -rf functest
mkdir functest
rm -rf functests.out

$TESTONE pentium hello		1 "$BOOMSW"
$TESTONE sparc   hello		1 "$BOOMSW"
$TESTONE ppc	 hello		1 "$BOOMSW"
$TESTONE pentium twoproc	1 "$BOOMSW"
$TESTONE sparc   twoproc	1 "$BOOMSW"
$TESTONE ppc	 twoproc	1 "$BOOMSW"
$TESTONE pentium fib		1 "$BOOMSW"
$TESTONE sparc   fib		1 "$BOOMSW"
$TESTONE pentium fibo-O4	1 "$BOOMSW" < test/source/fibo-O4.in1
$TESTONE sparc   fibo-O4	1 "$BOOMSW" < test/source/fibo-O4.in1
$TESTONE pentium fibo-O4	3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE sparc   fibo-O4	3 "$BOOMSW" < test/source/fibo-O4.in3
$TESTONE pentium global1	1 "$BOOMSW"
$TESTONE sparc   global1	1 "$BOOMSW"
$TESTONE pentium global2	1 "$BOOMSW"
$TESTONE sparc   global2	1 "$BOOMSW"
$TESTONE pentium global3	1 "$BOOMSW"
$TESTONE sparc   global3	1 "$BOOMSW"
if [ ! "$TYPEANALYSIS" ]
then
  echo Skipping switch tests, requires type analysis
  echo
else
$TESTONE pentium switch_gcc	5 "$BOOMSW" '2 3 4 5'
$TESTONE sparc   switch_gcc 5 "$BOOMSW" '2 3 4 5'
$TESTONE pentium switch_cc	4 "$BOOMSW" '2 3 4'
$TESTONE sparc   switch_cc	4 "$BOOMSW" '2 3 4'
$TESTONE pentium switch_gcc	1 "$BOOMSW"
$TESTONE sparc   switch_gcc	1 "$BOOMSW"
$TESTONE pentium switch_cc	1 "$BOOMSW"
$TESTONE sparc   switch_cc	1 "$BOOMSW"
$TESTONE pentium nestedswitch 4 "$BOOMSW" '2 3 4'
$TESTONE sparc   nestedswitch 4 "$BOOMSW" '2 3 4'
fi
if [ ! "$TYPEANALYSIS" ]
then
  echo Skipping stattest, now requires type analysis
  echo
else
  echo Some known failures from here down
$TESTONE pentium stattest	1 "$BOOMSW"
$TESTONE sparc   stattest	1 "$BOOMSW"
fi
$TESTONE pentium minmax		1 "$BOOMSW" '2 3 4'
$TESTONE sparc   minmax		1 "$BOOMSW" '2 3 4'
$TESTONE pentium minmax2	1 "$BOOMSW" 'two'
$TESTONE sparc   minmax2	1 "$BOOMSW" 'two'
$TESTONE pentium minmax3	1 "$BOOMSW" 'two'
if [ ! "$TYPEANALYSIS" ]
then
  echo Skipping printpi tests, requires type analysis
  echo
else
$TESTONE pentium printpi	1 "$BOOMSW"
$TESTONE sparc   printpi	1 "$BOOMSW"
fi
$TESTONE pentium uns		1 "$BOOMSW" '2 3'
$TESTONE sparc   uns		1 "$BOOMSW" '2 3'
$TESTONE pentium fromssa2	1 "$BOOMSW"
$TESTONE sparc   fromssa2	1 "$BOOMSW"
$TESTONE pentium sumarray-O4 1 "$BOOMSW"
if [ ! "$TYPEANALYSIS" ]
then
  echo Skipping sparc sumarray and recursion tests, require type analysis
  echo
else
$TESTONE sparc   sumarray-O4 1 "$BOOMSW"
$TESTONE pentium recursion	2 "$BOOMSW" 2
#$TESTONE sparc   recursion	2 "$BOOMSW" 2
fi
$TESTONE pentium bswap		1 "$BOOMSW"
$TESTONE pentium testset	1 -O '2 3 4'
$TESTONE sparc   andn		1 "$BOOMSW"

#$TESTONE pentium line1		1 "$BOOMSW" test/source/line1.c
#$TESTONE sparc   line1		1 "$BOOMSW" test/source/line1.c

echo Known failure:
$TESTONE sparc	elfhashtest	1 "$BOOMSW"

echo === Done ===
