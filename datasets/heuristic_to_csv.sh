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

# CSV header output
echo "raw_string,loop_depth,number_BB,number_exits,number_exit_blocks,num_successors,isexit,isbackedge,isdestinationinloop,isdestinationnestedloop,opcode_condition,prev_instr_1,prev_instr_2,operand_1.constant,operand_1.isfunctionarg,operand_1.isglobalvar,operand_1.isgeneralvar,operand_1.other,operand_2.constant,operand_2.isfunctionarg,operand_2.isglobalvar,operand_2.isgeneralvar,operand_2.other,operand_3.constant,operand_3.isfunctionarg,operand_3.isglobalvar,operand_3.isgeneralvar,operand_3.other,operand_4.constant,operand_4.isfunctionarg,operand_4.isglobalvar,operand_4.isgeneralvar,operand_4.other,branch_prob" > "$FILENAME.csv"

# Apply the LLVM pass directly on the simplified bitcode and append output to the CSV file
opt --disable-output -load-pass-plugin="$PATH2LIB" -passes="$PASS" "$FILENAME.ls.bc" >> "$FILENAME.csv" 2>&1

# Cleanup: Remove this if you want to retain the created files.
rm -f *_fplicm *.bc *_output *.ll
rm -f tests/*_fplicm tests/*.bc tests/*_output tests/*.ll
