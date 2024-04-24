#!/bin/bash

# Delete outputs from previous runs.
rm -f *_fplicm *.bc *_output *.ll

# Convert source code to LLVM bitcode.
# clang -emit-llvm -c ${1}.c -Xclang -disable-O0-optnone -o ${1}.bc
clang++ -emit-llvm -std=c++20 -c ${1}.cpp -Xclang -disable-O0-optnone -o ${1}.bc -Wno-deprecated-non-prototype

# Canonicalize natural loops.
opt -passes='loop-simplify' ${1}.bc -o ${1}.ls.bc

# O3 optimization
opt -O3 ${1}.ls.bc -o ${1}.optimized.bc

# Can create executable
# clang ${1}.optimized.bc -o ${1}.out
clang++ ${1}.optimized.bc -o ${1}.out

# Cleanup: Optionally retain or remove intermediate files.
rm -f *_fplicm *.bc *_output *.ll
rm -f tests/*_fplicm tests/*.bc tests/*_output tests/*.ll
