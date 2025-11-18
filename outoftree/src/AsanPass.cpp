/**
 * @file AsanPass.cpp
 * @brief Implementation of the AsanPass LLVM module pass.
 *
 * This pass searches for calls to AddressSanitizer (ASan) report functions
 * (e.g., `__asan_report_load8`) and replaces them. Instead of letting the
 * ASan report function terminate the program, this pass inserts a call to a
 * custom logging function and then redirects control flow to a "safe"
 * continuation point, effectively allowing the program to continue executing
 * after an ASan-detected error.
 */

#include "../include/AsanPass.h"
#include "../include/logger.h"

// LLVM Headers
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

// Standard Library Headers
#include <string>

using namespace llvm;

/**
 * @class AsanPass
 * @brief An LLVM ModulePass to intercept and neutralize ASan error reports.
 *
 * This pass iterates over all instructions in a module. When it finds a call
 * to an `__asan_report_*` function, it performs a complex transformation
 * of the Control Flow Graph (CFG):
 * 1. It inserts a call to a custom logger (`LogFunc`) just before the ASan call.
 * 2. It removes the `__asan_report_*` call.
 * 3. It identifies the "safe" execution path (i.e., the code that would have
 * run if no ASan error had occurred).
 * 4. It patches the basic block containing the ASan call to branch
 * unconditionally to this safe continuation point.
 * 5. It updates any PHI nodes in the safe continuation block to handle the
 * new incoming edge.
 */

/**
 * @brief Main entry point for the pass.
 * @param M The Module being processed.
 * @param AM The ModuleAnalysisManager, used to query analyses.
 * @return PreservedAnalyses::none() as the CFG is modified.
 */
PreservedAnalyses AsanPass::run(Module &M, ModuleAnalysisManager &AM) {
    // errs() << "AsanPass: running on module: " << M.getSourceFileName() << "\n";
    std::string sourcefile = getBaseName(M.getSourceFileName());

    // Declare the external logging function we will be calling.
    llvm::FunctionCallee LogFunc =
        PassUtils::declareIllegalAccessLogger(&M);

    // Iterate over all functions, basic blocks, and instructions.
    for (Function &F : M) {
        if (F.isDeclaration()) continue; // Skip external function declarations

        for (BasicBlock &BB : F) {

            for (Instruction &I : BB) {
                // Check if the instruction is a call instruction
                if (auto *callInst = dyn_cast<CallInst>(&I)) {
                    Function *calledFunc = callInst->getCalledFunction();
                    if (!calledFunc) continue;

                    // Check if it's an ASan report call
                    if (calledFunc->getName().starts_with("__asan_report_")) {
                        // --- Start of Instrumentation ---
                        // We found a call to an ASan error report.
                        // We will now transform the CFG to log this event
                        // and continue execution instead of crashing.

                        // 1) Build log message and get debug info
                        std::string logMsg = PassUtils::createLogMsg(&I);
                        unsigned faultLine = I.getDebugLoc().getLine();

                        // 2) Identify the key basic blocks.
                        // This pass relies on a specific CFG structure created by ASan:
                        //
                        //   [ PredecessorBlock ]
                        //   |       |
                        // (fault) (no-fault)
                        //   |       |
                        //   |       v
                        //   |   [ SuccessorBlock (safe) ]
                        //   v
                        // [ AsanBlock ]
                        //   (contains __asan_report_* call)
                        //
                        BasicBlock *asanBlock = I.getParent();
                        BasicBlock *pred = asanBlock->getSinglePredecessor();

                        // This logic *requires* the asan block to have a single
                        // predecessor (the block with the conditional branch).
                        if (!pred) {
                            // errs() << "AsanPass: skipping site - no single predecessor\n";
                            continue;
                        }

                        BranchInst *predTerm = dyn_cast<BranchInst>(pred->getTerminator());

                        // The predecessor must end with a conditional branch.
                        if (!predTerm || predTerm->isUnconditional()) {
                            // errs() << "AsanPass: skipping site - predecessor terminator not conditional\n";
                            continue;
                        }

                        // 3) Find the "safe" successor block (the one taken when no fault).
                        // It's the *other* successor of the predecessor's branch.
                        BasicBlock *succ = (predTerm->getSuccessor(0) == asanBlock)
                                                ? predTerm->getSuccessor(1)
                                                : predTerm->getSuccessor(0);
                        if (!succ) continue;

                        // errs()<<"succ\n"<<*succ<<"\n";
                        // 4) Determine the "safe continuation" point.
                        // We want to resume execution *after* the potentially
                        // faulting instruction, which is in the `succ` block.
                        Instruction *splitPoint = PassUtils::findNextSafeInstruction(succ->getFirstNonPHI());

                        BasicBlock *safeTarget = nullptr;
                        if (!splitPoint) {
                            // If no "safe" instruction is found inside `succ`,
                            // try to find the next valid block to jump to.
                            safeTarget = PassUtils::findNextSafeBlock(succ, faultLine);
                        } else {
                            // A safe instruction was found. Split `succ` at that
                            // point and make the new block our target.
                            safeTarget = succ->splitBasicBlock(splitPoint, succ->getName() + ".safe");
                        }

                        if (!safeTarget) {
                            // errs() << "AsanPass: skipping site - could not find safe target\n";
                            continue;
                        }

                        // errs()<<"safe:\n"<<*safeTarget<<"\n";
                        // errs()<<"split case : \n"<<*succ<<"\n";

                        // 5) Update PHI nodes in the safe target block.
                        // Since `asanBlock` will now branch to `safeTarget`,
                        // we must add an incoming value to any PHI nodes
                        // in `safeTarget` to account for this new edge.
                        // We use `UndefValue` as this is an exceptional path.
                        // for (PHINode &PN : safeTarget->phis()) {
                        //     PN.addIncoming(UndefValue::get(PN.getType()), asanBlock);
                        // }

                        // 6) Modify the `asanBlock`:
                        //    - Insert the logger call
                        //    - Erase the ASan report call
                        //    - Erase the old terminator (likely 'unreachable')
                        //    - Add a new branch to the `safeTarget`
                        CallInst *reportCI = dyn_cast<CallInst>(&I); // I is the __asan_report_* call

                        // Insert logger call *before* the report call
                        IRBuilder<> B(reportCI);
                        Value *msgPtr = B.CreateGlobalStringPtr(logMsg);
                        Value *srcPtr = B.CreateGlobalStringPtr(sourcefile);
                        B.CreateCall(LogFunc, {msgPtr, srcPtr});

                        // Erase the report call
                        reportCI->eraseFromParent();

                        // Erase the original terminator (e.g., 'unreachable')
                        Instruction *asanTerm = asanBlock->getTerminator();
                        if (asanTerm) {
                            asanTerm->eraseFromParent();
                        }

                        // Insert new branch to the safe continuation block
                        B.SetInsertPoint(asanBlock);
                        B.CreateBr(safeTarget);

                        // errs() << "AsanPass: Patched violation site. Will log and continue to block: " << safeTarget->getName() << "\n";

                        // We have fundamentally modified this basic block.
                        // Continuing to iterate over its instructions is unsafe.
                        // Break from the inner loop (over Instructions).
                        // errs()<<"asan : "<<BB<<"\n";
                        // errs()<<"pred : "<<*pred<<"\n";
                        break;
                    }
                }
            }
        }
    }

        // instrument module
    for (Function &F : M) {
        if (F.isDeclaration()) continue;

        // Now dominance fix pass
        FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
        BasicBlock *SafeExit = PassUtils::createCanonicalSafeExit(F);

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if(isa<PHINode>(I)) continue;
                
                PassUtils::fixDominanceForInstruction(&I, DT, SafeExit);
            }
        }
    }

    // We modified the module, so we don't preserve any analyses.
    return PreservedAnalyses::none();
}


// --- End of AsanPass Implementation ---

/**
 * @brief Plugin registration function.
 *
 * This function is the entry point for the LLVM pass plugin. It
 * registers the AsanPass with the pass manager, allowing it to be
 * loaded and used (e.g., via `opt -load ... -passes=AsanPass`).
 * @return PassPluginLibraryInfo
 */
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<AsanPass>("AsanPass", "v0.1");
}