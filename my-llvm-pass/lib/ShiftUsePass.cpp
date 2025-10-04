#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace{
    // understand all value hirarchy : https://llvm.org/doxygen/classllvm_1_1Value.html

    /*
    Goal: use of IR builder 
    replace multiplication by 2 with shift operator
    */

    class ShiftUsePass: public PassInfoMixin<ShiftUsePass>{
        public:

        PreservedAnalyses run(Function &F, FunctionAnalysisManager& FM){
            std::vector<Instruction*> InstructionsToDelete;

            for(BasicBlock &BB: F){
                for(Instruction &I: BB){
                    if(auto *BO = dyn_cast<BinaryOperator>(&I)){
                        if(BO->getOpcode() == Instruction::Mul){
                            errs()<<"Multiplication found\n";
                            Value* op1 = BO->getOperand(0);
                            Value* op2 = BO->getOperand(1);

                            ConstantInt* Cint = nullptr;
                            Value* otherOp = nullptr;

                            if(Cint = dyn_cast<ConstantInt>(op1)){
                                otherOp = op2;
                            }
                            else if(Cint = dyn_cast<ConstantInt>(op2)){
                                otherOp = op1;
                            }

                            if(Cint && Cint->getZExtValue() == 2){
                                errs()<<"Multiplication by 2 found\n";
                                IRBuilder<> Builder(&I);

                                Value* newShift = Builder.CreateShl(otherOp,1,"mul_by_2_shl");

                                I.replaceAllUsesWith(newShift);
                                InstructionsToDelete.push_back(&I);
                            }
                        }
                    }
                }
            }
            for (Instruction *I : InstructionsToDelete) {
                I->eraseFromParent();
            }
            return InstructionsToDelete.empty() ? PreservedAnalyses::all() : PreservedAnalyses::none();
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK::llvm:: PassPluginLibraryInfo llvmGetPassPluginInfo(){
    return {
        LLVM_PLUGIN_API_VERSION,
        "MulOptPass",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB){
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>){
                    if(Name == "mul-opt-pass"){
                        FPM.addPass(ShiftUsePass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}