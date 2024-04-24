#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>

int main()
{
    // write all features to csv file

    // make System call to run python script
    // giving system command and storing return value
    int returnCode = system("python3 make_prediction.py -i test.csv -o test.txt");

    // checking if the command was executed successfully
    if (returnCode == 0)
    {
        printf("Command executed successfully.");
    }
    else
    {
        printf("Command execution failed or returned "
               "non-zero: %d",
               returnCode);
    }

    std::string output_filename = "test.txt";

    std::ifstream predictionsFile(output_filename);

    if (!predictionsFile.is_open())
        throw std::runtime_error("Could not open file");

    double branch_prob;

    std::string line;

    while(getline(predictionsFile, line)){
        std::stringstream ss(line);
        while(ss >> branch_prob){
            std::string junk;
            getline(ss, junk, ',');
        }
    }

    // Can put probability back in
    std::cout << branch_prob << std::endl;


    return 0;
}
