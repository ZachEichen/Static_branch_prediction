#ifndef _FEATURES_

#include "llvm/Support/raw_ostream.h"

struct LoopFeatures {
    LoopFeatures()  {}

    // Numerical Features
    int loop_depth;
    int number_BB;
    int number_exits; // These are exit edges
    int number_exit_blocks; // These are exit blocks

    // Add Features Not in Paper
    int num_successors;
    

    // Categorical Features
    bool isexit; 
    bool isbackedge;
    bool isdestinationinloop;
    bool isdestinationnestedloop;

    friend llvm::raw_ostream &operator << (llvm::raw_ostream &os, const LoopFeatures &lf){

        os << "Loop Depth: " << lf.loop_depth << "\n";
        os << "Number of BBs: " << lf.number_BB << "\n";
        os << "Number of Exits: " << lf.number_exits << "\n";
        os << "NUmber of Exit Blocks: " << lf.number_exit_blocks << "\n";
        os << "NUmber of Successors: " << lf.num_successors << "\n";


        os << "Is Exit?: " << (lf.isexit? " Yes " : " No ") << "\n";
        os << "Is Back Edge?: " << (lf.isbackedge? " Yes " : " No ") << "\n";
        os << "Is the Destination In Loop?: " << (lf.isdestinationinloop? " Yes " : " No ") << "\n";
        os << "Is the Destination In Nested Loop?: " << (lf.isdestinationnestedloop? " Yes " : " No ") << "\n";

        return os;
    }

};

#endif