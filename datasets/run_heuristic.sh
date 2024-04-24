#!/bin/bash

# Run with sh run.sh <target directory> <input file name without extension>

# Disable for no detail
# set -ex

# Define paths
PATH2LIB="/n/eecs583b/home/shprasad/Static_branch_prediction/Loop/build/pass/LoopAnalysisPass.so"
PASS=loop

# Directory and file setup
TARG_DIR=$1
BENCH="$TARG_DIR/$2.cpp"
FILENAME=$2

# Delete outputs from previous runs. Update this when you want to retain some files.
rm -f *_fplicm *.bc *_output *.ll *.in *.in.Z

# Convert source code to bitcode (IR).
clang++ -emit-llvm -std=c++20 -c $BENCH -Xclang -disable-O0-optnone -o "$FILENAME.bc" -Wno-deprecated-non-prototype

# Canonicalize natural loops (Ref: llvm.org/doxygen/LoopSimplify_8h_source.html)
opt -passes='loop-simplify' "$FILENAME.bc" -o "$FILENAME.ls.bc"

# Apply the LLVM pass directly on the simplified bitcode and append output to the CSV file
opt --disable-output -load-pass-plugin="$PATH2LIB" -passes="$PASS" "$FILENAME.ls.bc"

# Cleanup: Remove this if you want to retain the created files.
rm -f *_fplicm *.bc *_output *.ll
rm -f tests/*_fplicm tests/*.bc tests/*_output tests/*.ll
