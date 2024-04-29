# Learning Static branch probabilities

In this project we attempt to learn branch probabilites via deep learning-enabled code inspection. 



Note: The LLVM pass for training/inference is included in the 'Loop' directory.

For training, switch the output to errs() and only use the loop analysis, dataflow analysis, and ground_truth functions in the InstructionInfo constructor. Comment out the system_call, update_metadata, and dependence analysis functions.

For inference, switch the output to the output stream 'o' and only use loop analysis, dataflow analysis, system_call, and update_metadata. Comment out ground_truth and dependence analysis.
