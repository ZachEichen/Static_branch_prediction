#!/bin/bash

# Define path to the library
PATH2LIB="build/pass/LoopAnalysisPass.so" 

# Check if .so file exists, otherwise use .dylib
if [ -e build/pass/LoopAnalysisPass.so ] 
then
    echo "Using build/pass/LoopAnalysisPass.so"
else
    PATH2LIB="build/pass/LoopAnalysisPass.dylib"
    echo "Using build/pass/LoopAnalysisPass.dylib"
fi

PASS=loop

# Delete outputs from previous runs.
rm -f *_fplicm *.bc *_output *.ll

# Convert source code to LLVM bitcode.
# clang -emit-llvm -c ${1}.c -Xclang -disable-O0-optnone -o ${1}.bc
clang++ -emit-llvm -std=c++20 -c ${1}.cpp -Xclang -disable-O0-optnone -o ${1}.bc -Wno-deprecated-non-prototype

# Canonicalize natural loops.
opt -passes='loop-simplify' ${1}.bc -o ${1}.ls.bc

# Apply the custom pass
opt -load-pass-plugin="${PATH2LIB}" -passes="${PASS}" ${1}.ls.bc -o ${1}.modified.bc



# # Before optimization 
# opt -S ${1}.modified.bc -o ${1}.modified.ll


# Feed custom pass into O3
opt -O3 ${1}.modified.bc -o ${1}.optimized.bc


# # After optimization
# opt -O3 -S ${1}.modified.bc -o ${1}.optimized.ll

# Can create executable
# clang ${1}.optimized.bc -o ${1}.out
clang++ ${1}.optimized.bc -o ${1}.out

# Cleanup: Optionally retain or remove intermediate files.
rm -f *_fplicm *.bc *_output
rm -f tests/*_fplicm tests/*.bc tests/*_output
