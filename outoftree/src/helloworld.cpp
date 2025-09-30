#include "../include/helloworld.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Passes/PassPlugin.h"
#include <llvm/Passes/PassBuilder.h>

using namespace llvm;

PreservedAnalyses HelloWorldPass::run(Module &M,ModuleAnalysisManager &AM){
    for(Function &F : M){
        if(!F.isDeclaration()){
            errs()<<"Function Found : "<<F.getName()<<"\n";
        }
    }
    return PreservedAnalyses::all();
}


llvm::PassPluginLibraryInfo  getHelloWorldInfo(){
    return{
        LLVM_PLUGIN_API_VERSION,
        "HelloWorldPass",
        "v0.1",
        [](PassBuilder &PB){
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager& MPM,
                ArrayRef<PassBuilder::PipelineElement>){
                    if(Name == "hello-world"){
                        MPM.addPass(HelloWorldPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(){
    return getHelloWorldInfo();
}

