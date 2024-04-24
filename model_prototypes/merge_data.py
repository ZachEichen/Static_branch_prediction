'''
This file is a Python script that merges all the data extracted from each of the 44 CSV files
that the dataset consists of from the feature extraction pass into 1 collection.
This file then splits the data into training, validation, and test sets. 
'''
import argparse
import os
import csv
import math
import pandas as pd
from sklearn.utils import shuffle

def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Script for merging feature extraction CSV files"
    )
    parser.add_argument(
        "-f",
        "--folder",
        dest="folder",
        help="The folder containing all CSV files to be merged",
        required=True)
    args = parser.parse_args(argv)
    
    csv_path = "model_data/branch.csv"
    train_path = "model_data/branch_train.csv"
    val_path = "model_data/branch_val.csv"
    test_path = "model_data/branch_test.csv"

    field_names = ["raw_string", "loop_depth", "number_BB", "number_exits", "number_exit_blocks", "num_successors", "isexit", "isbackedge", "isdestinationinloop", "isdestinationnestedloop", "opcode_condition", "prev_instr_1", "prev_instr_2", "operand_1.constant", "operand_1.isfunctionarg", "operand_1.isglobalvar", "operand_1.isgeneralvar", "operand_1.other", "operand_2.constant", "operand_2.isfunctionarg", "operand_2.isglobalvar", "operand_2.isgeneralvar", "operand_2.other", "operand_3.constant", "operand_3.isfunctionarg", "operand_3.isglobalvar", "operand_3.isgeneralvar", "operand_3.other", "operand_4.constant", "operand_4.isfunctionarg", "operand_4.isglobalvar", "operand_4.isgeneralvar", "operand_4.other", "branch_prob"]
    with open(csv_path, 'w', newline='', encoding='utf-8') as csvfile, open(train_path, 'w', newline='', encoding='utf-8') as trainfile, open(val_path, 'w', newline='', encoding='utf-8') as valfile, open(test_path, 'w', newline='', encoding='utf-8') as testfile:
        writer = csv.DictWriter(csvfile, fieldnames=field_names)
        writer.writeheader()

        train_writer = csv.DictWriter(trainfile, fieldnames=field_names)
        train_writer.writeheader()
        
        val_writer = csv.DictWriter(valfile, fieldnames=field_names)
        val_writer.writeheader()
        
        test_writer = csv.DictWriter(testfile, fieldnames=field_names)
        test_writer.writeheader()
    i = 0
    for filename in os.listdir(args.folder):
        path = os.path.join(args.folder, filename)
        i += 1
        if os.path.isfile(path):
            try:
                input_file = pd.read_csv(path)
                input_file.to_csv(csv_path, mode="a", index=False, header=False)

                input_file = input_file.sample(frac=1, random_state=7).reset_index(drop=True)

                if i > 37:
                    input_file.to_csv(test_path, mode="a", index=False, header=False)
                else:
                    train_sample = input_file.loc[:math.ceil(len(input_file) * .8)]
                    val_sample = input_file.loc[(math.ceil(len(input_file) * .8)+1):]

                    train_sample.to_csv(train_path, mode="a", index=False, header=False)
                    val_sample.to_csv(val_path, mode="a", index=False, header=False)
            except:
                pass

if __name__ == "__main__":
    main()