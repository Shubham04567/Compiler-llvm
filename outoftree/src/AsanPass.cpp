#include "../include/AsanPass.h"
#include "../include/logger.h"
#include "llvm/Passes/PassBuilder.h"
#include <string>

using namespace llvm;

PreservedAnalyses AsanPass::run(Module &M, ModuleAnalysisManager &AM) {
    // errs() << "AsanPass: running on module: " << M.getSourceFileName() << "\n";
    std::string sourcefile = getBaseName(M.getSourceFileName());

    llvm::FunctionCallee LogFunc =
        PassUtils::declareIllegalAccessLogger(&M);

    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        BasicBlock *safeExit = PassUtils::createCanonicalSafeExit(F);

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {

                if (auto *callInst = dyn_cast<CallInst>(&I)) {

                    if (Function *calledFunc = callInst->getCalledFunction()) {

                        if (calledFunc->getName().starts_with("__asan_report_")) {

                            

                            // --- Start of instrumentation ---
                            // 1) Build log message
                            std::string logMsg = PassUtils::createLogMsg(&I);
                            unsigned faultLine = I.getDebugLoc().getLine();

                            // 2) Identify asanBlock (block that contains the call inst)
                            BasicBlock *asanBlock = I.getParent();
                            BasicBlock *pred = asanBlock->getSinglePredecessor();
                            if (!pred) {
                                // errs() << "AsanPass: skipping site - no single predecessor\n";
                                continue;
                            }

                            BranchInst *predTerm = dyn_cast<BranchInst>(pred->getTerminator());
                            if (!predTerm || predTerm->isUnconditional()) {
                                // errs() << "AsanPass: skipping site - predecessor terminator not conditional\n";
                                continue;
                            }

                            // 3) Find the normal successor (the block taken when no fault)
                            BasicBlock *succ = (predTerm->getSuccessor(0) == asanBlock) ? predTerm->getSuccessor(1)
                                                                                        : predTerm->getSuccessor(0);
                            if (!succ) continue;

                            Instruction *splitPoint = PassUtils::findNextSafeInstruction(succ->getFirstNonPHI());

                            // 4) Determine safe continuation
                            BasicBlock *safeTarget = nullptr;

                            if (!splitPoint) {
                                safeTarget = PassUtils::findNextSafeBlock(succ,faultLine);
                            } else {
                                // We found a split point inside `succ` itself => split there to create safeTarget
                                safeTarget = succ->splitBasicBlock(splitPoint, succ->getName() + ".safe");
                            }

                            // 5) Check if we found a target
                            if (!safeTarget) {
                                safeTarget = safeExit;
                            }

                            // 6) Update PHI nodes in safeTarget (or ipd) for new incoming edge from asanBlock
                            // If safeTarget == ipd and we didn't split, we still must add incoming values to ipd's PHIs.
                            for (PHINode &PN : safeTarget->phis()) {
                                PN.addIncoming(UndefValue::get(PN.getType()), asanBlock);
                            }

                            // 7) Insert logger call, erase report, and replace terminator with branch to safeTarget
                            CallInst *reportCI = dyn_cast<CallInst>(&I); // I is the __asan_report_* call

                            // Insert logger call before the report call
                            IRBuilder<> B(reportCI);
                            Value *msgPtr = B.CreateGlobalStringPtr(logMsg);
                            Value *srcPtr = B.CreateGlobalStringPtr(sourcefile);
                            B.CreateCall(LogFunc, {msgPtr, srcPtr});

                            // Erase report call
                            reportCI->eraseFromParent();

                            // Replace terminator (probably 'unreachable') with branch to the safe continuation
                            Instruction *asanTerm = asanBlock->getTerminator();
                            if (asanTerm) {
                                asanTerm->eraseFromParent();
                            }

                            // Set builder to end of asanBlock (in case it had no terminator)
                            B.SetInsertPoint(asanBlock);
                            B.CreateBr(safeTarget);

                            // errs() << "AsanPass: Patched violation site. Will log and continue to block: " << safeTarget->getName() << "\n";

                            // We modified this block, so we're done with it.
                            // Break from the inner loop (over Instructions)
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
    return PassUtils::buildPassPluginInfo<AsanPass>("AsanPass","v0.1");
}
