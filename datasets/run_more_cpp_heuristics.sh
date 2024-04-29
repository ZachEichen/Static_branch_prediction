#!/usr/bin/env bash
# Usage: bash make_su bmission.sh <uniquename>

RUN_SCRIPT="../run_heuristic.sh"

# Handle inputs 
OUT_DIR=processed/more_cpp_heuristic
DATASET_BASE_DIR=../more_cpp/Project_CodeNet_C++1000
MAX_PER_PROBLEM=30 # set to -1 if you want to get all problems 


# Make the directory and copy over pass and readme
mkdir -p ../${OUT_DIR}
# mkdir -p ../${OUT_DIR}/src
mkdir -p ../${OUT_DIR}/results

# ls $DATASET_BASE_DIR
mkdir -p temp
# Iterate over all benchmark<n> folders (this way you can add your own too)
for directory in "${DATASET_BASE_DIR}/"p0*/; do 
  # then get the test file names; 
  echo "directory: $directory"
  dirbase=$(basename "$directory")
  for file in $(ls "$directory"*.cpp | head -$MAX_PER_PROBLEM); do
      # Get the file name without the extension
      (
      filename=$(basename "$file")
      stem="${filename%.*}"
    #   echo ""

      # run the test
      # there's probably a better way to do this than by cd _ <work> cd .. but idk what it is
      # cd $directory 
      # cd
    #   pwd
      # echo ${pwd} ../${OUT_DIR}/results/${dirbase}_${stem}.opcstats

      bash ${RUN_SCRIPT} "$directory" "$stem" 2>&1 | tee  ../${OUT_DIR}/results/${dirbase}_${stem}.opcstats
    #   echo "Running $directory; with file $filename"
    #   echo "basename: $dirbase"

      ) &

      # cd .. 
  done
  wait
  printf "\n\n\n\n ~~~~~ new dir ~~~~~\n\n\n" 
done

# refresh processed_z directory 
# rm -r /Dataset/group22/processed_3
cp -r . /Dataset/group22/processed_3
chmod -R 777 /Dataset/group22/processed_3