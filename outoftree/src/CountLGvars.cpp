#include "../include/countlgvars.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

PreservedAnalyses CountLGvars::run(Module &M,ModuleAnalysisManager &AM){
    for(Function &F:M){
        if(!F.isDeclaration()){
            errs()<<"Function : "<<F.getName()<<"\n";
            int count = 0;
            for(BasicBlock& BB : F){
                for(Instruction& I : BB){
                    if(DILocation* Loc = I.getDebugLoc()){
                        errs()<<"Line "<<Loc->getLine()<<" : ";
                    }
                    
                
                    if(isa<BinaryOperator>(&I)){
                        auto *BO = cast<BinaryOperator>(&I);
                        errs()<<"Arithmetic Operation "<<BO->getOpcodeName()<<"\n";
                    }

                    if(auto *cmp = dyn_cast<ICmpInst>(&I)){
                        errs()<<"Comparison : "<<cmp->getPredicateName(cmp->getPredicate())<<"\n";
                    }
                    if(auto *br = dyn_cast<BranchInst>(&I)){
                        if(br->isConditional()){
                            errs()<<"Branch Instruction"<<"\n";
                        }
                    }

                    if(isa<AllocaInst>(&I)){
                        errs() << "Local Variable allocation\n";
                        count++;
                    }
                    if(isa<LoadInst>(&I)){
                        errs() << "Loading from memory\n";
                    }
                    if(isa<StoreInst>(&I)){
                        errs() << "Storing to memory\n";
                    }

                    if(isa<GetElementPtrInst>(&I)){
                        errs() << "Pointer arithmetic\n";
                    }

                    if(auto *call = dyn_cast<CallBase>(&I)){
                        if(call->getCalledFunction()){
                            errs()<<"Function call : "<<call->getCalledFunction()->getName()<<"\n";
                        }
                    }

                }
                errs()<<"local variables count in Function : "<<count<<"\n";
            }
        }
    }

    errs()<<"Global Variables"<<"\n";

    for(GlobalVariable &GV : M.globals()){
        errs()<<"Global Variable: "<<GV.getName()<<"\n";
    }

    return PreservedAnalyses::all();
}

extern "C" LLVM_ATTRIBUTE_WEAK::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo(){
    return {
        LLVM_PLUGIN_API_VERSION,
        "HelloWorldPass",
        "v0.1",
        [](PassBuilder &PB){
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager& MPM,
                ArrayRef<PassBuilder::PipelineElement>){
                    if(Name == "Instruction-Pass"){
                        MPM.addPass(CountLGvars());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}



// array index of bound
// pointer arithmetic
// llvm