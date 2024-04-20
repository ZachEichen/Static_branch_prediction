#!/bin/bash

# run with sh run.sh <input file name>

# disbale for no detail
# set -ex

# add support for .dylib

PATH2LIB="build/pass/LoopAnalysisPass.so" 

if [ -e build/pass/LoopAnalysisPass.so ] 
then
    PATH2LIB="build/pass/LoopAnalysisPass.so" 
else
    PATH2LIB="build/pass/LoopAnalysisPass.dylib"
    
fi

PASS=loop

# Delete outputs from previous runs. Update this when you want to retain some files.
rm -f default.profraw *_prof *_fplicm *.bc *.profdata *_output *.ll

# Convert source code to bitcode (IR).
clang -emit-llvm -c ${1}.c -Xclang -disable-O0-optnone -o ${1}.bc

# Canonicalize natural loops (Ref: llvm.org/doxygen/LoopSimplify_8h_source.html)
opt -passes='loop-simplify' ${1}.bc -o ${1}.ls.bc

# Instrument profiler passes.
opt -passes='pgo-instr-gen,instrprof' ${1}.ls.bc -o ${1}.ls.prof.bc

# Generate binary executable with profiler embedded
clang -fprofile-instr-generate ${1}.ls.prof.bc -o ${1}_prof

# run to generate .proraw file
./${1}_prof > /dev/null

# Converting it to LLVM form. This step can also be used to combine multiple profraw files,
# in case you want to include different profile runs together.
llvm-profdata merge -o ${1}.profdata default.profraw

# The "Profile Guided Optimization Use" pass attaches the profile data to the bc file.
opt -passes="pgo-instr-use" -o ${1}.profdata.bc -pgo-test-profile-file=${1}.profdata < ${1}.ls.prof.bc > /dev/null

# CSV Format.

echo "raw_string, loop_depth, number_BB, number_exits, number_exit_blocks, num_successors, isexit, isbackedge, isdestinationinloop, isdestinationnestedloop, opcode_condition, prev_instr_1, prev_instr_2, operand_1.constant, operand_1.isfunctionarg, operand_1.isglobalvar, operand_1.isgeneralvar, operand_1.other, operand_2.constant, operand_2.isfunctionarg, operand_2.isglobalvar, operand_2.isgeneralvar, operand_2.other, operand_3.constant, operand_3.isfunctionarg, operand_3.isglobalvar, operand_3.isgeneralvar, operand_3.other, operand_4.constant, operand_4.isfunctionarg, operand_4.isglobalvar, operand_4.isgeneralvar, operand_4.other" &> ${1}.csv

# We now use the profile augmented bc file as input to your pass.
opt --disable-output -load-pass-plugin="${PATH2LIB}" -passes="${PASS}" ${1}.profdata.bc >> ${1}.csv 2>&1

# Cleanup: Remove this if you want to retain the created files. And you do need to.
rm -f default.profraw *_prof *_fplicm *.bc *.profdata *_output *.ll
rm -f tests/default.profraw tests/*_prof tests/*_fplicm tests/*.bc tests/*.profdata tests/*_output tests/*.ll
