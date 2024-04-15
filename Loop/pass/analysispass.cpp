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
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Support/BranchProbability.h"

//C++ stdlib
#include <vector>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <algorithm>

#include "features.hpp"


using std::unordered_map;
using std::vector;
using std::pair;
using namespace llvm;

namespace {

    class InstructionInfo {
    public:
        InstructionInfo() {}
        InstructionInfo(Instruction* i, llvm::LoopAnalysis::Result &li, llvm::DependenceAnalysis::Result &di, llvm::PostDominatorTreeAnalysis::Result & pdi, llvm::BranchProbabilityAnalysis::Result& bpi){
            _parent = i->getParent();
            _loop = li.getLoopFor(_parent);
            _I = i;

            initialize_loopanalysis(li);
            initialize_dependenceanalysis(di, pdi, bpi);
            // Add Data Analysis and Control Analysis here
        }

        void initialize_dependenceanalysis(llvm::DependenceAnalysis::Result & _di, llvm::PostDominatorTree& _pdt, llvm::BranchProbabilityAnalysis::Result& bpi){
            
            
            // Find destinations of branch instruction.

            vector<pair<Instruction*, float>> cdi;
            for(int i=0; i<_loopinfo.num_successors; i++){
               
                BasicBlock* successor = _I->getSuccessor(i);
                for(Instruction& istr : *successor){
                    if(_pdt.dominates(_I, &istr)){
                        auto p = bpi.getEdgeProbability(_parent, i);
                        float prob = float(p.getNumerator()) / float(p.getDenominator());
                        cdi.push_back({&istr, prob});
                    }
                }
            }

            //sort by branch probability

            std::sort(cdi.begin(), cdi.end(), [](pair<Instruction*, float> a, pair<Instruction*, float> b){return a.second > b.second;});

            // If they are function calls...

            for(auto p : cdi){
                errs() << "destination going to: " ;
                p.first->print(errs()); errs() << "\n";
                if(strcmp(p.first->getOpcodeName(), "call"))continue;

                errs() << "Function call at: " << "\n";
                p.first->print(errs());
                errs() << " with name " << (reinterpret_cast<CallInst*>(p.first)->getCalledFunction() ? reinterpret_cast<CallInst*>(p.first)->getCalledFunction()->getName() : "Indirect Call, ");
                errs() << " with probabilty" << p.second << "\n";
                auto fname = reinterpret_cast<CallInst*>(p.first)->getCalledFunction() ? reinterpret_cast<CallInst*>(p.first)->getCalledFunction()->getName().str() : "Indirect Call";
                _dependenceinfo.dependentFunctionCalls.push_back(fname);

                errs() << "This function has the following attributes:\n";

                auto attr = reinterpret_cast<CallInst*>(p.first)->getCalledFunction()->getAttributes();

                for(auto a : attr){
                    errs() << a.getAsString() << "\n";
                }

                errs() << "end attribute\n";

            }

            

        }

        void initialize_loopanalysis(llvm::LoopAnalysis::Result& _li) {
            if(!_loop){
                errs() << "This branch instruction does not have a loop associated with it.\n";
                _loopinfo = LoopFeatures{-1,-1,-1,-1,-1,false,false,false,false};
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

            errs() << "Instruction:" << "\n";
            _I->print(errs()); errs() << "\n";
            //errs() << "\n" << _loopinfo << "\n\n";
        }

        

        LoopFeatures _loopinfo;
        DependenceFeatures _dependenceinfo;
        llvm::Loop* _loop;
        Instruction* _I;
        BasicBlock* _parent;

    };

    struct LoopAnalysisPass: public PassInfoMixin<LoopAnalysisPass> {

        PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM) {
            
            
            llvm::BlockFrequencyAnalysis::Result &bfi = FAM.getResult<BlockFrequencyAnalysis>(F); 
            llvm::BranchProbabilityAnalysis::Result &bpi = FAM.getResult<BranchProbabilityAnalysis>(F);
            llvm::LoopAnalysis::Result &li = FAM.getResult<LoopAnalysis>(F);
            llvm::DependenceAnalysis::Result &di = FAM.getResult<DependenceAnalysis>(F);
            llvm::PostDominatorTreeAnalysis::Result & pdi = FAM.getResult<PostDominatorTreeAnalysis>(F);
            

            unordered_map<Instruction*, InstructionInfo> iinfos;
            vector<Instruction*> branch_instructions;
            
            for(BasicBlock& BB: F){
                for(Instruction& I: BB){
                    //only care about branching instructions.
                    if(strcmp(I.getOpcodeName(), "br")) continue;
                    iinfos[&I] = InstructionInfo(&I, li, di, pdi, bpi);
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