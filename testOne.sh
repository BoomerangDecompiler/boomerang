#!/bin/bash
# Call with test platform, test-program, arguments
# e.g. "./testOne.sh pentium hello"
# or   "./testOne.sh sparc fibo 10" 
echo $*
rm -f functest/code
./boomerang -o functest test/$1/$2 2>/dev/null >/dev/null
ret=$?
if [[ ret -ge 128 ]]; then
    echo Boomerang failed with signal $((ret-128)) > functest.res
else
    if [[ ! -f functest/code ]]; then
        echo $1 $2: No boomerang output! > functest.res
    else
        cp functest/code functest.c
        # if test/$1/$2.sed exists, use it to make "known error" corrections
        # to the source code
        if [[ -f test/$1/$2.sed ]]; then
            echo Warning... $1/$2.sed used
            sed -f test/$1/$2.sed functest.c > functest.tmp
            ret=$?
            if [[ ret -ne 0 ]]; then
                echo test/$1/$2.sed failed! > functest.res
                echo
                exit 10
            fi
            mv functest.tmp functest.c
        fi
        gcc -o functest.exe functest.c
        if [[ $? != 0 ]]; then
            echo $1 $2: Compile failed > functest.res
        else
            rm -f functest.out
            ./functest.exe $3 $4 $5 $6 $7 $8 $9 > functest.out
            ret=$?
            if [[ ret -ge 128 ]]; then
                echo $1 $2: Execution terminated with signal $((ret-128)) >\
                    functest.res
            else
                if [[ ret -ne 0 ]]; then
                    echo Warning! return code from execute was $((ret))
                fi
                diff -c test/source/$2.out functest.out
                if [[ $? != 0 ]]; then
                    echo $1 $2: failed diff > functest.res
                else
                    echo $1 $2: Passed > functest.res
                fi
            fi
        fi
    fi
fi
cat functest.res
cat functest.res >> functests.out
echo
