/**
 * @file Memcpymv.cpp
 * @brief Implementation of the Memcpymv instrumentation pass.
 *
 * This pass searches for calls to `asan_memcpymv` and instruments them
 * with a runtime null-check on the source and destination pointers.
 *
 * If either pointer is null, control flow is diverted to a custom
 * logging block, which logs the error and then branches to a "safe"
 * continuation point, effectively bypassing the potentially-crashing
 * `asan_memcpymv` call.
 */

#include "../include/Memcpymv.h"
#include "../include/PassUtils.h" // For PassUtils helper functions
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

/**
 * @brief Instruments a single `asan_memcpymv` call.
 *
 * This function modifies the CFG to wrap the `asan_memcpymv` call in a
 * null-check for its source and destination arguments.
 *
 * Before:
 * [ origBB: ... ; %call = call @asan_memcpymv(...) ; ... ]
 *
 * After:
 * [ origBB: ... ]
 * |
 * |  %destNull = icmp eq %dest, null
 * |  %srcNull = icmp eq %src, null
 * |  %anyNull = or %destNull, %srcNull
 * |  br i1 %anyNull, label %asanAbortBlock, label %afterMemcpymvBB
 * |
 * +-> [ asanAbortBlock: ]
 * |   |  call @LogFunc(...)
 * |   |  br label %safeBlock
 * |
 * +-> [ afterMemcpymvBB: ]
 * |  %call = call @asan_memcpymv(...)
 * |  ...
 *
 * @param call The `asan_memcpymv` call instruction (as a CallBase).
 */
static void instrumentAsanMemcpymv(CallBase *call) {
    if (!call) return;
    // errs()<<*call<<"\n";
    BasicBlock *origBB = call->getParent();

    if (!origBB) return;

    // errs()<<*(dyn_cast<Instruction>(call))<<"\n";

    // Get the destination (arg 0) and source (arg 1) pointers.
    Value *dest = call->getArgOperand(0);
    Value *src = call->getArgOperand(1);
    Type *VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(call->getContext()));
    Type *Int32Ty = Type::getInt32Ty(call->getContext());

    // --- Part 1: Find a Safe Continuation Point ---
    // This is where execution will resume if a null pointer is detected
    // (i.e., where asanAbortBlock will eventually branch to).
    unsigned int faultline = call->getDebugLoc().getLine();
    Instruction *curr = PassUtils::findNextSafeInstruction(dyn_cast<Instruction>(call));

    BasicBlock *safeBlock = nullptr;
    if (curr == nullptr) {
        // No safe instruction in this block; find the next viable block.
        safeBlock = PassUtils::findNextSafeBlock(origBB, faultline);
    } else {
        // Found a safe instruction; split the block there to create the safe target.
        safeBlock = origBB->splitBasicBlock(curr, "MemcpymvSafeBB");
    }

    // --- Part 2: Split the Basic Block ---
    // Split *before* the call. The call instruction will be the first
    // instruction in the new 'afterMemcpymvBB'.
    BasicBlock *afterMemcpymvBB = origBB->splitBasicBlock(call, "afterMemcpymv");

    // Remove the unconditional branch that splitBasicBlock added to origBB.
    origBB->getTerminator()->eraseFromParent();

    // We will now insert a new terminator at the end of origBB.
    IRBuilder<> BrInst(origBB);

    // --- Fallback Case ---
    if (!safeBlock) {
        // If no safe block was found, we cannot safely bypass the call.
        // Re-insert the original unconditional branch to the 'after' block.
        // This means the null check won't happen, but the code remains valid.
        BrInst.CreateBr(afterMemcpymvBB);
        // errs() << "[instrumentAsanMemcpymv] Warning: no safe continuation found from " << origBB->getName()
        //        << "; inserted unconditional branch to " << afterMemcpymvBB->getName() << "\n";
        return;
    }

    // --- Part 3: Create the Error Logging Block ---
    // This block will be executed if a null pointer is found.
    Module *M = origBB->getModule();
    Function *F = origBB->getParent();
    LLVMContext &Fctx = F->getContext();

    std::string sourcefile = getBaseName(M->getSourceFileName());
    llvm::FunctionCallee LogFunc = PassUtils::declareIllegalAccessLogger(M);
    std::string logMsg = PassUtils::createLogMsg(dyn_cast<Instruction>(call));

    BasicBlock *asanAbortBlock = BasicBlock::Create(Fctx, "MemcpymvAsanBlock", F);
    IRBuilder<> builder(asanAbortBlock);

    // Populate the block with a call to the logger...
    Value *msgPtr = builder.CreateGlobalStringPtr(logMsg);
    Value *srcPtr = builder.CreateGlobalStringPtr(sourcefile);
    builder.CreateCall(LogFunc, {msgPtr, srcPtr});

    // ...and a branch to the safe continuation point.
    builder.CreateBr(safeBlock);

    // // Update PHI nodes in safeBlock for new incoming edge from AsanLogBlock
    // for (PHINode &PN : safeBlock->phis()) {
    //     PN.addIncoming(UndefValue::get(PN.getType()), asanAbortBlock);
    // }

    // --- Part 4: Create the Conditional Branch ---
    // Now, insert the new terminator into origBB.
    

    Value* voiddestPtr = BrInst.CreateBitCast(dest,VoidPtrTy);
    Value* voidsrcPtr = BrInst.CreateBitCast(src,VoidPtrTy);
    
    Value* isInvalidsest = BrInst.CreateCall(PassUtils::getIsValidBasePtr(M),{voiddestPtr});

    Value* isInvalidsrc = BrInst.CreateCall(PassUtils::getIsValidBasePtr(M),{voidsrcPtr});
    
    // Create the null-check comparison.
    Value *destcond = BrInst.CreateICmpEQ(isInvalidsest, ConstantInt::get(Int32Ty, 0));
    Value *srccond = BrInst.CreateICmpEQ(isInvalidsrc, ConstantInt::get(Int32Ty, 0));
    Value *anycond = BrInst.CreateOr(destcond, srccond);
    // If either is null, go to the abort/log block.
    // Otherwise, proceed to the block containing the memcpymv.
    BrInst.CreateCondBr(anycond, asanAbortBlock, afterMemcpymvBB);

    // errs() << "[instrumentAsanMemcpymv] inserted condbr from " << origBB->getName()
    //        << " to " << asanAbortBlock->getName() << " and " << afterMemcpymvBB->getName() << "\n";
}

/**
 * @struct Memcpymv
 * @brief An LLVM ModulePass to instrument `asan_memcpymv` calls.
 *
 * Implements the main pass logic, including collecting calls
 * and dispatching them for instrumentation.
 */

/**
 * @brief Main entry point for the Memcpymv pass.
 * @param M The Module being processed.
 * @param AM The ModuleAnalysisManager.
 * @return PreservedAnalyses::none() as the CFG is modified.
 */
PreservedAnalyses Memcpymv::run(Module &M, ModuleAnalysisManager &AM) {
    SmallVector<CallBase *, 64> AsanMemcpymvs;

    // Pass 1: Collection
    // We collect all calls first to avoid invalidating iterators
    // while modifying the CFG in the transformation pass.
    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (auto *C = dyn_cast<CallBase>(&I)) {
                    Function *called = C->getCalledFunction();
                    if (called && (called->getName().starts_with("__asan_memcpy") || called->getName().starts_with("__asan_memmove"))) {
                        AsanMemcpymvs.push_back(C);
                    }
                }
            }
        }
    }

    // Pass 2: Transformation
    // Now, iterate over the collected calls and instrument them.
    for (CallBase *C : AsanMemcpymvs) {
        instrumentAsanMemcpymv(C);
    }

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
    
    // We have modified the CFG, so no analyses are preserved.
    return PreservedAnalyses::none();
}


/**
 * @brief Plugin registration function.
 *
 * Registers the Memcpymv pass with the LLVM pass manager, allowing it to
 * be loaded via tools like `opt`.
 * @return PassPluginLibraryInfo
 */
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<Memcpymv>("Memcpymv", "v0.1");
}