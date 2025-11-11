#include "../include/AsanPass.h"
#include "../include/logger.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

PreservedAnalyses AsanPass::run(Module &M, ModuleAnalysisManager &AM) {

    errs() << "AsanPass: running on module: " << M.getSourceFileName() << "\n";

    std::string sourcefile = getBaseName(M.getSourceFileName());

    llvm::LLVMContext &Ctx = M.getContext();

    llvm::Type *Int8PtrTy = llvm::PointerType::getUnqual(llvm::Type::getInt8Ty(Ctx));

    llvm::FunctionType *LogFuncTy =
        llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx),
                                {Int8PtrTy, Int8PtrTy}, false);

    llvm::FunctionCallee LogFunc =
        M.getOrInsertFunction("__asan_log_violation", LogFuncTy);

    
    
    

    for (Function &F : M) {

        for (BasicBlock &BB : F) {

            for (Instruction &I : BB) {
                
                if (auto *callInst = dyn_cast<CallInst>(&I)) {
                    
                    if (Function *calledFunc = callInst->getCalledFunction()) {
                    
                        if (calledFunc->getName().starts_with("__asan_report_")) {
                                
                                // putting the call of log function    
                                // ---- Build log message ----
                                std::string logMsg = "ASAN violation in function: " + F.getName().str();
                                if (I.getDebugLoc()) {
                                    const DebugLoc &DL = I.getDebugLoc();
                                    logMsg += " at " + DL->getFilename().str() + ":" +
                                            std::to_string(DL->getLine());
                                }

                                // errs() << logMsg << "\n";

                                IRBuilder<> Bb(&I);
                                Value *msgPtr = Bb.CreateGlobalStringPtr(logMsg);
                                Value *sourcefilePtr = Bb.CreateGlobalStringPtr(sourcefile);
                                Bb.CreateCall(LogFunc, {msgPtr, sourcefilePtr});

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
