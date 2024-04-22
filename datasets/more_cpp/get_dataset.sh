#! bin/bash 

echo "this script should be run from datasets/more_cpp"

echo "\nDownlaoding"
wget https://dax-cdn.cdn.appdomain.cloud/dax-project-codenet/1.0.0/Project_CodeNet_C++1000.tar.gz
echo "Unzipping"
tar -zxf Project_CodeNet_C++1000.tar.gz
rm Project_CodeNet_C++1000.tar.gz
