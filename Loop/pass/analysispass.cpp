#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopIterator.h"
#include "llvm/Analysis/LoopPass.h"
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


//C++ stdlib
#include <vector>
#include <iostream>
#include <unordered_map>
#include <utility>

#include "features.hpp"


using std::unordered_map;
using std::vector;
using std::pair;
using namespace llvm;

namespace {

    class InstructionInfo {
    public:
        InstructionInfo() {}
        InstructionInfo(Instruction* i, llvm::LoopAnalysis::Result &li){
            _loop = li.getLoopFor(i->getParent());
            _I = i;

            initialize_loopanalysis(li);

            // Add Data Analysis and Control Analysis here
        }

        void initialize_loopanalysis(llvm::LoopAnalysis::Result& _li) {
            if(!_loop){
                errs() << "This branch instruction does not have a loop associated with it.\n";
                return;
            }

            _loopinfo.loop_depth = _loop -> getLoopDepth();
            _loopinfo.number_BB = _loop -> getBlocksVector().size();

            _loopinfo.num_successors = _I->getNumSuccessors();

            // Analyze num of exit edge/block & cat.

            llvm::SmallVector<pair<BasicBlock*, BasicBlock*>> exitEdges;
            llvm::SmallVector<BasicBlock*> exitBlocks;

            _loop->getExitEdges(exitEdges);
            _loop->getExitBlocks(exitBlocks);

            _loopinfo.number_exits = exitEdges.size();
            _loopinfo.number_exit_blocks = exitBlocks.size();

            // Analyze instruction, not loop
            llvm::SmallVector<BasicBlock*> s;
            for(size_t i=0; i<_loopinfo.num_successors; i++){
                s.push_back(_I->getSuccessor(i));
            }

            BasicBlock* parent = _I->getParent();

            _loopinfo.isexit = false;

            for(auto e : exitEdges){
                if(e.first != parent) continue;

                for(size_t i=0; i<_loopinfo.num_successors; i++){
                    if(e.second == s[i]) _loopinfo.isexit = true;
                }
            }

            // Check categorical

            auto header = _loop->getHeader();

            auto binl = _loop->getBlocksVector();
            auto isinloop = [&binl] (auto b) {
                for(auto bb: binl){
                    if(bb == b) return true;
                }
                return false;
            };
            auto isininnerloop = [&binl, &_li] (auto b, int depth){
                for(auto bb: binl){
                    if(bb == b && _li.getLoopFor(b)->getLoopDepth() > depth) return true;
                }
                return false;
            };

            _loopinfo.isbackedge = false;
            _loopinfo.isdestinationinloop = false;
            _loopinfo.isdestinationnestedloop = false;

            for(size_t i=0; i<_loopinfo.num_successors; i++){
                if(s[i] == header) _loopinfo.isbackedge = true;
                if(isinloop(s[i])) _loopinfo.isdestinationinloop = true;
                if(isininnerloop(s[i], _loopinfo.loop_depth)) _loopinfo.isdestinationnestedloop = true;
                
            }

            errs() << "Instruction: " << "\n";
            _I->print(errs());
            errs() << "\n" << _loopinfo << "\n\n";
        }

        

        LoopFeatures _loopinfo;
        llvm::Loop* _loop;
        Instruction* _I;

    };

    struct LoopAnalysisPass: public PassInfoMixin<LoopAnalysisPass> {

        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            
            
            llvm::BlockFrequencyAnalysis::Result &bfi = FAM.getResult<BlockFrequencyAnalysis>(F); 
            llvm::BranchProbabilityAnalysis::Result &bpi = FAM.getResult<BranchProbabilityAnalysis>(F);
            llvm::LoopAnalysis::Result &li = FAM.getResult<LoopAnalysis>(F);

            unordered_map<Instruction*, InstructionInfo> iinfos;
            vector<Instruction*> branch_instructions;
            
            for(BasicBlock& BB: F){
                for(Instruction& I: BB){
                    //only care about branching instructions.
                    if(strcmp(I.getOpcodeName(), "br")) continue;
                    iinfos[&I] = InstructionInfo(&I, li);
                    branch_instructions.push_back(&I);
                }
            }
            return PreservedAnalyses::all();
        }

    };    
}

// Make Pass accessible to cmd

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo() {
  return {
    LLVM_PLUGIN_API_VERSION, "LoopAnalysisPass", "v0.1",
    [](PassBuilder &PB) {
      PB.registerPipelineParsingCallback(
        [](StringRef Name, FunctionPassManager &FPM,
        ArrayRef<PassBuilder::PipelineElement>) {
            if(Name == "loop"){
                FPM.addPass(LoopAnalysisPass());
                return true;
            }
            return false;
        }
      );
    }
  };
}