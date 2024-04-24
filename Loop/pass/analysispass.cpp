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
#include "llvm/Support/FileSystem.h"

//C++ stdlib
#include <vector>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <system_error>
#include <algorithm>
#include <fstream>

#include "features.hpp"

//Why does LLVM hate me?
std::error_code EIC;

const std::string PATH_TO_OUTPUT_CSV = "../output.csv";

llvm::raw_fd_ostream o = llvm::raw_fd_ostream(PATH_TO_OUTPUT_CSV, EIC);


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
            // initialize_dependenceanalysis(di, pdi, bpi); // dont call this for now
            initialize_dataflowanalysis();
            ground_truth(bpi);
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
                // errs() << "destination going to: " ;
                // p.first->print(errs()); errs() << "\n";
                if(strcmp(p.first->getOpcodeName(), "call"))continue;

                // errs() << "Function call at: " << "\n";
                // p.first->print(errs());
                // errs() << " with name " << (reinterpret_cast<CallInst*>(p.first)->getCalledFunction() ? reinterpret_cast<CallInst*>(p.first)->getCalledFunction()->getName() : "Indirect Call, ");
                // errs() << " with probabilty" << p.second << "\n";
                //auto fname = reinterpret_cast<CallInst*>(p.first)->getCalledFunction() ? reinterpret_cast<CallInst*>(p.first)->getCalledFunction()->getName().str() : "Indirect Call";
                _dependenceinfo.dependentFunctionCalls.push_back(reinterpret_cast<CallInst*>(p.first));
            }

            
            _dependenceinfo.mostfrequentFunction = _dependenceinfo.dependentFunctionCalls.empty() ? nullptr : _dependenceinfo.dependentFunctionCalls[0];

            // Get the most frequent function's attributes.
            auto f = _dependenceinfo.mostfrequentFunction;

            if(f) {
                auto attr = f->getCalledFunction()->getAttributes();

                 // Limit to 5? 

                size_t cnt = 1;

                for(auto a: attr){
                    _dependenceinfo.FFattributes.push_back(a.getAsString());
                    //errs() << "pushing attribute " << a.getAsString() << "\n";
                    if(cnt++ > 5) break;
                }
            }  

        }

        void initialize_loopanalysis(llvm::LoopAnalysis::Result& _li) {
             
            if(!_loop){
                //errs() << "This branch instruction does not have a loop associated with it.\n";
                _loopinfo = LoopFeatures{-1,-1,-1,-1,-1,false,false,false,false};
                o << "\"";
                _I->print(o); o << "\",";
                o << _loopinfo;
                o.flush();
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

            // errs() << "Instruction:" << "\n";
            o << "\"";
            _I->print(o); o << "\",";
            o << _loopinfo;
            o.flush();
        }

        // int encodeOperand(llvm::Value* operand) {
        //     if (auto* constInt = llvm::dyn_cast<llvm::ConstantInt>(operand)) {
        //         return constInt->getSExtValue();
        //     } else if (auto* constFP = llvm::dyn_cast<llvm::ConstantFP>(operand)) {
        //         return static_cast<int>(constFP->getValueAPF().convertToDouble()); 
        //     } else if (auto* arg = llvm::dyn_cast<llvm::Argument>(operand)) {
        //         return -2;  // function arguments
        //     } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(operand)) {
        //         return -3;  // global variables
        //     } else if (auto* inst = llvm::dyn_cast<llvm::Instruction>(operand)) {
        //         return -1;  // other instruction-generated operand
        //     }
        //     return 0;  // unhandled types
        // }

        void ground_truth(llvm::BranchProbabilityAnalysis::Result& bpi){
             
            

            BranchProbability BProb = bpi.getEdgeProbability(_I->getParent(), _I->getSuccessor(0));
            if (BProb.getDenominator() != 0 ){
                float prob = ((float)BProb.getNumerator()) / ((float)BProb.getDenominator());
                o << prob << "\n";
            } else {
                o << 0.0 << "\n";
            }
            o.flush();
        }
        

        void initialize_dataflowanalysis(){
            
             
            
            auto *BI = dyn_cast<BranchInst>(_I);
            auto V = BI->getCondition();

            int it = 0;
            int ito = 1;
            if (Instruction *Inst = dyn_cast<Instruction>(V)) {
                _dataflowinfo.opcodes[0] = ((int)Inst->getOpcode());
                for (Use &U : Inst->operands()) {
                    // if(!(visited.find(Inst) != visited.end())){
                    Value *Op = U.get();
                    if (Instruction *InstU = dyn_cast<Instruction>(U)){
                        _dataflowinfo.opcodes[ito] = ((int)InstU->getOpcode());
                        ito += 1;
                        // errs() << "Use case" << *InstU << "\n";
                        for (unsigned i = 0; i < InstU->getNumOperands(); ++i) {
                            if(it>3){
                                break;
                            }
                            llvm::Value *Operand = InstU->getOperand(i);
                            // F.operands[it] = (encodeOperand(Operand));
                            
                            // _dataflowinfo.operands[it] = (encodeOperand(Operand));
                            if (auto* constInt = llvm::dyn_cast<llvm::ConstantInt>(Operand)) {
                                _dataflowinfo.operands[it] = {constInt->getSExtValue(),0,0,0,0};
                            } else if (auto* constFP = llvm::dyn_cast<llvm::ConstantFP>(Operand)) {
                                _dataflowinfo.operands[it] = {static_cast<int>(constFP->getValueAPF().convertToDouble()),0,0,0,0};
                            } else if (auto* arg = llvm::dyn_cast<llvm::Argument>(Operand)) {
                                _dataflowinfo.operands[it] = {0,1,0,0,0};
                            } else if (auto* global = llvm::dyn_cast<llvm::GlobalVariable>(Operand)) {
                                _dataflowinfo.operands[it] = {0,0,1,0,0};
                            } else if (auto* inst = llvm::dyn_cast<llvm::Instruction>(Operand)) {
                                _dataflowinfo.operands[it] = {0,0,0,1,0};
                            }
                            else{
                                _dataflowinfo.operands[it] = {0,0,0,0,1};
                            }
                            it += 1;

                        }
                    }
                }
            }
                        
            for (const auto &Codes : _dataflowinfo.opcodes) {
                o << Codes << ", ";
            }
            for (int j = 0; j < _dataflowinfo.operands.size(); ++j) {
                for (int i = 0; i < _dataflowinfo.operands[j].size(); ++i) {
                    o << _dataflowinfo.operands[j][i];
                    // if (!(i == _dataflowinfo.operands[j].size() - 1 &&  j == _dataflowinfo.operands.size() - 1)) {
                        o << ", ";
                    // }
                }
            }
            o.flush();
            // errs() << "\n";           
        }

        LoopFeatures _loopinfo;
        DependenceFeatures _dependenceinfo;
        DataflowFeatures _dataflowinfo;
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

            auto isFileEmpty = []() {
                std::ifstream f(PATH_TO_OUTPUT_CSV, std::ifstream::ate | std::ifstream::binary);        
                bool ret = f.tellg() == 0;
                f.close();
                return ret;
            };

            if(isFileEmpty()){
                o << "raw_string, loop_depth, number_BB, number_exits, number_exit_blocks, num_successors, isexit, isbackedge, isdestinationinloop, isdestinationnestedloop, opcode_condition, prev_instr_1, prev_instr_2, operand_1.constant, operand_1.isfunctionarg, operand_1.isglobalvar, operand_1.isgeneralvar, operand_1.other, operand_2.constant, operand_2.isfunctionarg, operand_2.isglobalvar, operand_2.isgeneralvar, operand_2.other, operand_3.constant, operand_3.isfunctionarg, operand_3.isglobalvar, operand_3.isgeneralvar, operand_3.other, operand_4.constant, operand_4.isfunctionarg, operand_4.isglobalvar, operand_4.isgeneralvar, operand_4.other\n";
                o.flush();
            }

            
            
            // errs() << 
            for(BasicBlock& BB: F){
                for(Instruction& I: BB){
                    //only care about branching instructions.
                    // errs() << "Inst" << I << "\n";
                    if(strcmp(I.getOpcodeName(), "br")) continue;
                    if (auto *BI = dyn_cast<BranchInst>(&I)){
                        if(BI->isConditional()){

                            iinfos[&I] = InstructionInfo(&I, li, di, pdi, bpi);
                            branch_instructions.push_back(&I);
                        }
                    }
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