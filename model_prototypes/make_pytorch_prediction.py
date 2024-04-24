import argparse
import pandas as pd
import torch.nn as nn
import torch
import torch.nn.functional as F

class LinearRegressionFFN(nn.Module):
    def __init__(self, input_dim, hidden_dim=50):
        super(LinearRegressionFFN, self).__init__()
        self.linear1 = nn.Linear(input_dim, hidden_dim)
        self.linear2 = nn.Linear(hidden_dim, 1)

    def forward(self, input):
        hidden_output = F.softmax(self.linear1(input), dim=-1)
        return torch.sigmoid(self.linear2(hidden_output))

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

    training_data = [torch.Tensor(branch_features.iloc[i].tolist()) for i in range(len(branch_features))]

    model = LinearRegressionFFN(len(branch_features.columns))
    model.load_state_dict(torch.load("pytorch/pytorch_branch_model.pt"))
    model.eval()

    with open(args.output_file, 'w') as output_writer:
        for data in training_data:
            output_writer.write(str(float(model(data))))



if(__name__ == "__main__"):
    main()

    