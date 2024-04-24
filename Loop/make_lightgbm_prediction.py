import argparse
import pandas as pd
import lightgbm as lgb

def main(argv=None):
    parser = argparse.ArgumentParser(
        description="Script for merging feature extraction CSV files"
    )
    parser.add_argument(
        "-i",
        "--input_file",
        dest="input_file",
        help="The path to the input file containing the branch's features to make a prediction based on",
        required=True)
    parser.add_argument(
        "-o",
        "--output_file",
        dest="output_file",
        help="The path to the output file containing the model's prediction",
        required=True)
    args = parser.parse_args(argv)

    branch_features = pd.read_csv(args.input_file)

    raw_string = "raw_string"

    branch_features = branch_features.loc[:, ~branch_features.columns.isin([raw_string])]

    training_data = [branch_features.iloc[i].tolist() for i in range(len(branch_features))]

    model = lgb.Booster(model_file="../model_prototypes/lightgbm/lightgbm_branch_model.txt")

    with open(args.output_file, 'w') as output_writer:
            predictions = model.predict(training_data)
            for prediction in predictions:
                output_writer.write(str(prediction))
                output_writer.write("\n")



if(__name__ == "__main__"):
    main()

    