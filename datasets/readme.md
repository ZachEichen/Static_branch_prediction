

# Setting up the directory  

run  `git submodule init && git submodule update` to initialize and download the `snippets` dataset from the appropriate github module. 

navigate to the `more_cpp` directory, and run `get_dataset.sh` 

Initialize the directory 

# Using the scripts 

This directory is meant to be used in a similar way to the scripts used for the homeworks: 

Setup: 
* The `run_all.sh` script should not need modification, but you should ensure that `DATASET_BASE_DIR` and `OUT_DIR` are set properly. 
* `run_script.sh` will require a little more setup as follows: 
* You need to set `PATH2LIB` to the path to your compiled pass executable like in the homeworks
* You need to set `PASS` to the name of your pass, again, as done in the homeworks. 

## Running the scripts: 

1. `run_script.sh` evaluates a single program, instrumenting, running, and producing output. It takes two arguments:

    1.1. The path to the folder containing the `.cpp` file, and any relevant input files 

    1.2. The name of the `.cpp` file, and optional input file minus file extension.  If your file is `test.cpp` and has input `test.txt`, you should pass `test`

    1.3. If input files are given, they are assumed to have the form `filename.txt`. Only one is permitted per file at this point 
1. `run_all.sh` runs the Analysis on all files in subdirectories of `DATASET_BASE_DIR`, by default, the snippets dataset. 
1. `run_more_cpp.sh` runs the analysis on a subset of the (very big) IBM cpp snippet file. It should be noted that this dataset has a bunch of very simple sripts which should be ignored. 

    3.1. **Because of its size, you should run this in screen as follows**
    ```bash 
    cd processed
    screen bash run_more_cpp.sh
    ```


Once you've compiled your pass, make a `processed` directory, cd into that, and run `bash ../run_all.sh` to run all the passes. 

Note this creates a good amount of scratch files, so it is recomended to do this in a scratch directory. 
