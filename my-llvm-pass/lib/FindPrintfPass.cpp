#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace{

    /*  
      Goal: Differentiating and Analyzing Instructions.
      a. dyn_cast<InstructionType>(Value): ->  
         If the value is the correct type, it returns a typed pointer to it; otherwise, it returns nullptr. 
         for eg. we analyze printf call
    */

    class FindPrintfPass: public PassInfoMixin<FindPrintfPass>{
        public:
            PreservedAnalyses run(Function &F, FunctionAnalysisManager& AM){
                for(BasicBlock &BB: F){
                    for(Instruction &I: BB){
                    if(auto *CI = dyn_cast<CallInst>(&I)){
                        Function *func = CI->getCalledFunction();

                        if(func && func->getName()=="printf"){
                        errs()<<"Hey I got printf function\n";
                        errs()<<"arguments are: \n";
                        for(int i=0; i<func->arg_size(); i++){
                            errs()<<i<<" : "<<*CI->getArgOperand(i)<<"\n";
                        }
                        }
                    }
                    }
                }
                return PreservedAnalyses::all();
            }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK::llvm:: PassPluginLibraryInfo llvmGetPassPluginInfo(){
    return {
        LLVM_PLUGIN_API_VERSION,
        "callInstructionPass",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB){
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM, ArrayRef<PassBuilder::PipelineElement>){
                    if(Name == "my-printf-pass"){
                        FPM.addPass(FindPrintfPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}