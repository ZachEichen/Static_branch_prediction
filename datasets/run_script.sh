#!/bin/bash
# Run script for Homework 1 EECS 583 Winter 2024
# Place this script inside the benchmark directory. e.g. benchmark1/run.sh
# Usage: sh run.sh <benchmark_name>
# where <benchmark_name> = simple OR anagram OR compress

# ACTION REQUIRED: Ensure that the path to the library and pass name are correct.
PATH2LIB="/n/eecs583a/home/zeichen/homework/W24_EECS583_HW1/build_lx/hw1pass/HW1Pass.so"
PASS='hw1'

TARG_DIR=${1}
BENCH=$TARG_DIR/${2}.cpp
FILENAME=${2}


# Delete outputs from previous runs. Update this if you want to retain some files across runs.
rm -f default.profraw *_prof *_fplicm *.bc *.profdata *_output *.ll *.in *.in.Z

# Convert source code to bitcode (IR).
clang++ -emit-llvm -std=c++20 -c ${BENCH} -Xclang -disable-O0-optnone -o "$FILENAME.bc" -Wno-deprecated-non-prototype

# Instrument profiler passes. Generates profile data.
opt -passes='pgo-instr-gen,instrprof' $FILENAME.bc -o $FILENAME.prof.bc

# Generate binary executable with profiler embedded
clang++ -std=c++20 -fprofile-instr-generate $FILENAME.prof.bc -o "$FILENAME"_prof

# When we run the profiler embedded executable, it generates a default.profraw file that contains the profile data.
#Checking if a file exists
if test -e "$TARG_DIR/$FILENAME.txt"; then
    ./"$FILENAME"_prof < "$TARG_DIR/$FILENAME.txt"  > /dev/null 2>&1
else 
    ./"$FILENAME"_prof   > /dev/null 2>&1
fi 
# Converting it to LLVM form. This step can also be used to combine multiple profraw files,
# in case you want to include different profile runs together.
llvm-profdata merge -o $FILENAME.profdata default.profraw

# The "Profile Guided Optimization Instrumentation-Use" pass attaches the profile data to the bc file.
opt -passes="pgo-instr-use" -o $FILENAME.profdata.bc -pgo-test-profile-file=$FILENAME.profdata < $FILENAME.bc

# Uncomment this and disable the cleanup if you want to "see" the instumented IR.
llvm-dis $FILENAME.profdata.bc -o $FILENAME.prof.ll

# Runs your pass on the instrumented code.
opt --disable-output -load-pass-plugin="${PATH2LIB}" -passes="${PASS}" $FILENAME.profdata.bc

# Cleanup: Remove this if you want to retain the created files.
rm -f *.in *.in.Z default.profraw *_prof *_fplicm *.bc *.profdata *_output *.ll 
