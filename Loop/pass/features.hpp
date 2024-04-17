#ifndef _FEATURES_

// TODO: Clean this up maybe?
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Analysis/LoopNestAnalysis.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar/LoopPassManager.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/IR/CFG.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/IteratedDominanceFrontier.h"

#include <string>
#include <vector>

struct LoopFeatures {
    LoopFeatures()  {}
    LoopFeatures(int a, int b, int c, int d, int e, bool f, bool g, bool h, bool i) : loop_depth(a), number_BB(b), number_exits(c), number_exit_blocks(d), num_successors(e), isexit(f), isbackedge(g), isdestinationinloop(h), isdestinationnestedloop(i) {}
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

    //use for debug
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

struct DependenceFeatures{
    
    std::vector<std::string> dependentFunctionCalls;

};

#endif