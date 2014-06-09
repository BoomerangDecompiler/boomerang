#!/usr/bin/env python
# Boomerang test functional test runner
# ARGV[0] test_executable
# ARGV[1] platform
# ARGV[2] test
# ARGV[3] test-set
# ARGV[4] options
# ARGV[5] parameters to the recompiled executable

import os
import subprocess
import shutil
import sys
from collections import defaultdict

TESTS_DIR="./tests"
TEST_INPUT=os.path.join(TESTS_DIR,"inputs")

print("Regression tester 0.0.1\n")
FAILED_COMMANDLINES=""
def perform_test(exepath,machine,test,args):
    test_file = os.path.join(TEST_INPUT,machine,test)
    output_path=os.path.join(TESTS_DIR,"outputs",machine)
    log_name = os.path.join(output_path,test)
    cmdline = ['-P',os.getcwd(),'-o',output_path] + args + [test_file]
    test_stdout = open(log_name+".stdout", "w")
    test_stderr = open(log_name+".stderr", "w")
    result = subprocess.call([exepath]+cmdline, stdout=test_stdout, stderr=test_stderr)
    test_stdout.close()
    test_stdout.close()
    sys.stdout.write('.' if result == 0 else '!')
    return [result == 0, ' '.join(cmdline)]

if os.path.isdir(os.path.join(TESTS_DIR,"outputs_prev")):
    shutil.rmtree(os.path.join(TESTS_DIR,"outputs_prev"))

if os.path.isdir(os.path.join(TESTS_DIR,"outputs")):
    shutil.move(os.path.join(TESTS_DIR,"outputs"),os.path.join(TESTS_DIR,"outputs_prev"))

#exit(1)
#sh -c "./boomerang -o functest $4 test/$1/$2 2>/dev/null >/dev/null"
crashes = defaultdict(list)
for f in os.listdir(TEST_INPUT):
        machine_dir = os.path.join(TEST_INPUT,f)
        if not os.path.isdir(machine_dir): continue
        machine = f
        for test in os.listdir(machine_dir):
            test_path = os.path.join(machine_dir,test)
            os.makedirs(os.path.join(TESTS_DIR,"outputs",machine,test))
            test_res = None
            if "hello.exe" in test:
                print("skipping hello.exe - it causes memory exhaustion")
                test_res = [False,"skipped windows/hello.exe"]
            else:
                test_res = perform_test(sys.argv[1],machine,test,sys.argv[2:])
                shutil.move(os.path.join(TESTS_DIR,"outputs",machine,"log"),os.path.join(TESTS_DIR,"outputs",machine,test+".log"))

            if not test_res[0]:
                crashes[machine].append([test,test_res[1]])

for machine, crash_list in crashes.iteritems():
    print("\nEncountered "+str(len(crash_list))+" program failures for "+machine)
    for test in crash_list:
        print("Decompiler failed on "+machine+"/"+test[0]+" - "+str(test[1]))

#Dir.open(TESTS_DIR+"/inputs").each() {|f|
#        next if f=="." or f==".."
#        FileUtils.mv(TESTS_DIR+"/inputs/"+f,TESTS_DIR+"/outputs/"+f) if f.end_with?(".b")
#}
#puts "**************************************\n"
