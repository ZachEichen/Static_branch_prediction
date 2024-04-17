#! /bin/bash 
# Usage: bash make_su bmission.sh <uniquename>

RUN_SCRIPT="../run_script.sh"

# Handle inputs 
# UNIQUENAME=$1
OUT_DIR=processed
DATASET_BASE_DIR=../snippets_ds/snippets

# Make the directory and copy over pass and readme
mkdir -p ../${OUT_DIR}
# mkdir -p ../${OUT_DIR}/src
# mkdir -p ../${OUT_DIR}/results
# cp ${PASS_FILE_PATH} ${OUT_DIR}/hw1pass.cpp
# cp README.md ${OUT_DIR}
q

ls ../snippets_ds/snippets

# Iterate over all benchmark<n> folders (this way you can add your own too)
for directory in "${DATASET_BASE_DIR}/"*/; do 
  # then get the test file names; 
  echo "directory: $directory"
  for file in "$directory"*.cpp; do
      # Get the file name without the extension
      filename=$(basename "$file")
      stem="${filename%.*}"
      echo ""
      echo "Running $directory; with file $filename"

      # run the test
      # there's probably a better way to do this than by cd _ <work> cd .. but idk what it is
      # cd $directory 
      # cd
      pwd
      echo ${pwd} ../${OUT_DIR}/results/${stem}.opcstats
      bash ${RUN_SCRIPT} "$directory" "$stem" 2>&1 | tee  ../${OUT_DIR}/results/${stem}.opcstats
      # cd .. 
  done
done

