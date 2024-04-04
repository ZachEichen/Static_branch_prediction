#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
// #include "llvm/Support/Format.h"

#include <iostream>

using namespace llvm;

namespace
{

  bool branch_is_biased(const Instruction &instr, llvm::BranchProbabilityAnalysis::Result &bpi)
  {
    unsigned int num_successors = instr.getNumSuccessors();

    // errs() << "new instr: " << instr.getOpcode() << " has num successors " << num_successors << "\n";
    for (unsigned int i = 0; i < num_successors; ++i){
      BranchProbability BProb = bpi.getEdgeProbability(instr.getParent(), instr.getSuccessor(i));
      if (BProb.getDenominator() != 0 ){
        float prob = ((float)BProb.getNumerator()) / ((float)BProb.getDenominator());
        if (prob > 0.8){
          return true; 
        }
      } else {
        return false; 
      }
    }
    return false; 
  }

  int get_block_freq(const BasicBlock &bb,llvm::BlockFrequencyAnalysis::Result &bfi){
    // TODO: maybe we don't need to cast this? 
    std::optional<uint64_t> val =  bfi.getBlockProfileCount(&bb); 
    // handle None case, and convert to integer. 
    return val? ((int) *val) : 0; 

    // getblock frequency 
    // get profile counts 
    // branch probability data documentation. 
  }

  struct HW1Pass : public PassInfoMixin<HW1Pass>
  {

    PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM)
    {
      // These variables contain the results of some analysis passes.
      // Go through the documentation to see how they can be used.
      llvm::BlockFrequencyAnalysis::Result &bfi = FAM.getResult<BlockFrequencyAnalysis>(F);
      llvm::BranchProbabilityAnalysis::Result &bpi = FAM.getResult<BranchProbabilityAnalysis>(F);

      // Add your code here
      // errs() << "function: " << F.getName().str();

      /*
        Branch: br, switch, indirectbr
        Integer ALU: add, sub, mul, udiv, sdiv, urem, shl, lshr, ashr, and, or, xor, icmp, srem
        Floating-point ALU: fadd, fsub, fmul, fdiv, frem, fcmp
        Memory: alloca, load, store, getelementptr, fence, atomiccmpxchg, atomicrmw
        Others: everything else
      */
      float total_instr = 0;
      float int_alu_instr = 0;
      float fp_alu_instr = 0;
      float memory_instr = 0;
      float biased_br = 0, unbiased_br = 0;
      float other_ct = 0;


      for (auto block_iter = F.begin(); block_iter != F.end(); ++block_iter)
      {
        int block_freq = get_block_freq(*block_iter,bfi); 
        // errs() << "new basic block with freq: " << block_freq  <<"\n";
        for (auto instr_iter = block_iter->begin(); instr_iter != block_iter->end(); ++instr_iter)
        {
          // errs() << "new instr: " << instr_iter->getOpcode() << "\n";
          total_instr += block_freq;
          switch (instr_iter->getOpcode())
          {
          // branches
          case Instruction::Br:
          case Instruction::Switch:
          case Instruction::IndirectBr:
            // TODO: add code here
            if (branch_is_biased(*instr_iter, bpi))
            {
              biased_br+= block_freq;
            }
            else
            {
              unbiased_br+= block_freq;
            }
            break;
          case Instruction::Add:
          case Instruction::Sub:
          case Instruction::Mul:
          case Instruction::UDiv:
          case Instruction::SDiv:
          case Instruction::URem:
          case Instruction::Shl:
          case Instruction::LShr:
          case Instruction::AShr:
          case Instruction::Or:
          case Instruction::Xor:
          case Instruction::ICmp:
          case Instruction::SRem:
            int_alu_instr+= block_freq;
            break;
          case Instruction::FAdd:
          case Instruction::FSub:
          case Instruction::FMul:
          case Instruction::FDiv:
          case Instruction::FCmp:
            fp_alu_instr+= block_freq;
            break;
          case Instruction::Alloca:
          case Instruction::Load:
          case Instruction::Store:
          case Instruction::GetElementPtr:
          case Instruction::Fence:
          case Instruction::AtomicCmpXchg:
          case Instruction::AtomicRMW:
            memory_instr+= block_freq;
            break;
          default:
            other_ct+= block_freq;
            break;
          }
        }
      }

      // Print FuncName, DynOpCount, %IALU, %FALU, %MEM, %Biased-Br, %Unbiased-Br, %Others
      errs() << F.getName().str()
            << format(", %f, ",(int)total_instr )
            << format("%.3f, ", int_alu_instr / total_instr)
            << format("%.3f, ", fp_alu_instr / total_instr)
            << format("%.3f, ", memory_instr / total_instr)
            << format("%.3f, ", biased_br / total_instr)
            << format("%.3f, ", unbiased_br / total_instr) 
            << format("%.3f\n",other_ct / total_instr);

      return PreservedAnalyses::all();
    }
  };
}

extern "C" ::llvm::PassPluginLibraryInfo LLVM_ATTRIBUTE_WEAK llvmGetPassPluginInfo()
{
  return {
      LLVM_PLUGIN_API_VERSION, "HW1Pass", "v0.1",
      [](PassBuilder &PB)
      {
        PB.registerPipelineParsingCallback(
            [](StringRef Name, FunctionPassManager &FPM,
               ArrayRef<PassBuilder::PipelineElement>)
            {
              if (Name == "hw1")
              {
                FPM.addPass(HW1Pass());
                return true;
              }
              return false;
            });
      }};
}