#include "../include/AsanPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;

PreservedAnalyses AsanPass::run(Module &M, ModuleAnalysisManager &AM) {
    errs() << "AsanPass: running on module: " << M.getName() << "\n";

    for (Function &F : M) {

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                
                if (auto *callInst = dyn_cast<CallInst>(&I)) {
                    
                    if (Function *calledFunc = callInst->getCalledFunction()) {
                    
                        if (calledFunc->getName().starts_with("__asan_report_")) {
                                
                                BasicBlock* pred = BB.getSinglePredecessor();
                                BranchInst *BI = dyn_cast<BranchInst>(pred->getTerminator());

                            
                                BasicBlock *succ = nullptr;
                                if (BI->getSuccessor(0) == &BB) {
                                    succ = BI->getSuccessor(1);
                                } else {
                                    succ = BI->getSuccessor(0);
                                }
                                        
                      
                                BasicBlock* newSafeBlock = succ->splitBasicBlock((*succ->getFirstInsertionPt()).getNextNode(),"safe_block");
                                
                                // now jump from predessor to to succ;
                        
                                Value* cond = BI->getCondition();

                                IRBuilder<> B(BI);
                                B.CreateCondBr(cond,&BB,succ);
                                BI->eraseFromParent();

                                // asan block transformation
                                
                                auto next = I.getNextNode();
                                
                                IRBuilder<> AsanB(next);
                                
                                AsanB.CreateBr(newSafeBlock);

                                next->eraseFromParent();
                                I.eraseFromParent();
                                
                                break;

                        }
                    }
                }
            }
        }
    }

    return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "AsanPass", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "Asan-Pass") {
                        MPM.addPass(AsanPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}
