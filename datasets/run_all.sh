#! /bin/bash 
# Usage: bash make_su bmission.sh <uniquename>

RUN_SCRIPT=run_lx.sh

# Handle inputs 
# UNIQUENAME=$1
# OUT_DIR=${UNIQUENAME}_hw1

echo -e "\nBuilding"
# Build
cd $BUILD_DIR
make 
cd .. 

# Make the directory and copy over pass and readme
mkdir -p ${OUT_DIR}
mkdir -p ${OUT_DIR}/src
mkdir -p ${OUT_DIR}/results
cp ${PASS_FILE_PATH} ${OUT_DIR}/hw1pass.cpp
cp README.md ${OUT_DIR}


# Iterate over all benchmark<n> folders (this way you can add your own too)
for directory in "benchmark"?; do 
  # then get the test file names; 
  for file in "$directory"/src/*.c; do
      # Get the file name without the extension
      filename=$(basename "$file")
      stem="${filename%.*}"
      echo ""
      echo "Running $directory; with file $filename"

      # run the test
      # there's probably a better way to do this than by cd _ <work> cd .. but idk what it is
      cd $directory 
      bash ../${RUN_SCRIPT} "$stem" 2>&1 | tee  ../${OUT_DIR}/results/${stem}.opcstats
      cd .. 
  done
done

# finally, Compress the file 
echo -e "\n  Compressing directory\n"
tar -cvzf ${UNIQUENAME}_hw1.tgz ${UNIQUENAME}_hw1

echo -e "\nSubmission has been created, but not submitted." 
echo -e " * You can find your file in ${UNIQUENAME}_hw1.tgz"
echo -e " * Please verify that the folder structure is correct (I am not very good at bash)"
echo -e " * You still need to submit your zip file."
echo -e " * To do this, you can call something like the following:  (as per the spec)"
echo    "      scp ${UNIQUENAME}_hw1.tgz ${UNIQUENAME}@eecs538a.eecs.umich.edu:/hw1_submissions/"

