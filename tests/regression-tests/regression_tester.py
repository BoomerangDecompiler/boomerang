#!/usr/bin/env python3

import os
import shutil
import subprocess
import sys
import time

from collections import defaultdict


# ${CMAKE_BINARY_DIR}/tests/regression-tests
TESTS_DIR = os.getcwd()
TESTS_INPUT = os.path.abspath(os.path.join(TESTS_DIR, "../../out/share/boomerang/samples/"))

test_results = defaultdict();


'''
  Perform the actual test.
  Parameters:
    - exepath:     Path to the Boomerang executable.
    - test_file:   Path to the input binary file.
    - output_path: Path to the output directory.
    - args:        additional command line arguments.

  The output directory will be created if it does not exist.

  Returns:
    - a character indicatiing test result status (.=success, !=failure etc.)
    - the full commandline
    - the input file path
    - the throughput in B/s
  as a list.
'''
def perform_test(exepath, test_file_path, output_path, args):
    cmdline   = [exepath] + ['-P', os.path.dirname(exepath), '-o', output_path] + args + [test_file_path]
    input_file = os.sep.join(test_file_path.split(os.sep)[-1:]) # input file without the path

    try:
        os.makedirs(output_path)

        test_stdout = open(os.path.join(output_path, input_file + ".stdout"), "w")
        test_stderr = open(os.path.join(output_path, input_file + ".stderr"), "w")

        start_t = time.time()

        try:
            result = subprocess.call(cmdline, stdout=test_stdout, stderr=test_stderr, timeout=120)
            result = '.'
        except:
            result = '!'

        end_t = time.time()

        test_stdout.close()
        test_stdout.close()

        file_size = os.path.getsize(test_file_path)
        return [result, ' '.join(cmdline), test_file_path, float(file_size)/(end_t-start_t)]
    except:
        return ['d', ' '.join(cmdline), test_file_path, 0]



'''
  Test the decompiler with all files in a specific directory
  and all subdirectories.
'''
def test_all_inputs_in(dir_path):
    for f in os.listdir(dir_path):
        if os.path.isdir(os.path.join(dir_path, f)):
            # recurse into subdirectories
            test_all_inputs_in(os.path.join(dir_path, f))
        else:
            # test the actual file
            input_file = os.path.join(dir_path, f)

            # Find the path relative to TESTS_INPUT and preprend it with TESTS_DIR
            output_dir = os.path.relpath(input_file, TESTS_INPUT)
            output_dir = os.path.abspath(os.path.join(TESTS_DIR, "outputs", output_dir))

            test_results[input_file] = perform_test(sys.argv[1], input_file, output_dir, sys.argv[2:])
            sys.stdout.write(test_results[input_file][0]) # print status
            sys.stdout.flush()

def main():
    print("")
    print("Boomerang 0.4.0 Regression Tester")
    print("=================================")
    print("")

    # Backup previous output to outputs_prev
    old_path = os.path.join(TESTS_DIR, "outputs_prev")
    new_path = os.path.join(TESTS_DIR, "outputs")

    if os.path.isdir(old_path): shutil.rmtree(old_path)
    if os.path.isdir(new_path): shutil.move(new_path, old_path)

    sys.stdout.write("Testing ")
    test_all_inputs_in(TESTS_INPUT)
    print("\n")

    num_tests = len(test_results)
    num_failed = sum(1 for res in test_results.values() if res[0] != '.')

    # Works because items are compared from left to right
    (min_throughput, min_name) = min((res[3], os.path.relpath(res[2], TESTS_INPUT)) for res in test_results.values() if res[0] == '.')

    print("Summary:")
    print("========")
    print("Number of tests: " + str(num_tests))
    print("Failed tests:    " + str(num_failed))
    print("Min throughput:  " + str(int(min_throughput) / 1000) + " kB/s (" + min_name + ")")
    print("")
    print("Failures:")
    print("=========")

    some_failed = False
    for res in test_results.values():
        if (res[0] != '.'):
            some_failed = True
            sys.stdout.write(res[0] + " " + os.path.relpath(res[2], TESTS_INPUT) + "\n")

    # Everything OK?
    if not some_failed:
        print("None")

    print("")
    sys.stdout.flush()


if __name__ == "__main__":
    main()
