#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace{
    /*
        Goal: to write general pass which only count the number of instruction in each function
    */
    class InstructorCounter: public PassInfoMixin<InstructorCounter>{
        public:
            PreservedAnalyses run(Function &F, FunctionAnalysisManager &FM){
                int counter = 0;
                for(auto &BB: F){
                    errs()<<"Basic block started\n";
                    for(Instruction &I: BB){
                        counter++;
                    }
                }
                errs()<<"Function: "<<F.getName()<<" total instruction: "<<counter<<"\n";
                return PreservedAnalyses::all();
            }
    };

}

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo(){
    return {
        LLVM_PLUGIN_API_VERSION,
        "MyInstructorCounter",
        LLVM_VERSION_STRING,
        [](PassBuilder &PB){
            
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "my-instructor-counter") {
                        FPM.addPass(InstructorCounter());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}