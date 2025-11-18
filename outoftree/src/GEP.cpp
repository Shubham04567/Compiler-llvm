/**
 * @file GEP.cpp
 * @brief Implementation of the GEP (GetElementPtr) instrumentation pass.
 *
 * This pass instruments GetElementPtr (GEP) instructions to perform a
 * runtime null-check on the base pointer. If the base pointer is null,
 * it redirects control flow to a logging block instead of allowing a
 * potential crash (e.g., segmentation fault) to occur.
 *
 * The pass operates in two phases:
 * 1.  **Collection**: Iterates over the module and collects all GEP instructions.
 * 2.  **Transformation**: For each collected GEP, the `instrumentGEP`
 * function is called to modify the Control Flow Graph (CFG).
 */

#include "../include/GEP.h"
#include "../include/PassUtils.h" // For PassUtils helper functions
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

/**
 * @brief Instruments a single GetElementPtr (GEP) instruction.
 *
 * This function modifies the CFG around the GEP instruction. It splits the
 * basic block containing the GEP and inserts a conditional branch.
 *
 * Before:
 * [ ... ; %ptr = getelementptr ... ; ... ]
 *
 * After:
 * [ ... ]
 * |
 * |  br i1 (%base == null), label %asanOrLogBlock, label %contBB
 * |
 * +-> [ asanOrLogBlock: ... (logs error) ... ]
 * |
 * +-> [ contBB: %ptr = getelementptr ... ; ... ]
 *
 * @param GEP The GetElementPtrInst to instrument.
 */
void instrumentGEP(GetElementPtrInst *GEP) {
    if (!GEP) return;

    BasicBlock *origBB = GEP->getParent();
    if (!origBB) return;
    Module *M = origBB->getModule();
    LLVMContext &Ctx = GEP->getContext();
    Value *basePtr = GEP->getPointerOperand();
    Value *nullPtr = Constant::getNullValue(basePtr->getType());

    Type *VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(Ctx));
    Type *Int32Ty = Type::getInt32Ty(Ctx);

    // --- Part 1: Find or Create the Error-Handling Block ---

    // First, check if an ASan block *already* exists as a successor.
    // This might happen if ASan has already instrumented this GEP.
    // BasicBlock *asanAbortBlock = nullptr;
    BasicBlock *asanAbortBlock = PassUtils::findNextAsanBlock(origBB);

    BasicBlock *safeBlock = nullptr;

    if (!asanAbortBlock) {
        // Asan block not found. We must create our own block to log the
        // null pointer violation and then continue to a safe place.

        // Step 1.A: Find a safe continuation point.
        // This is where execution will resume *after* we log the error.
        Instruction *curr = PassUtils::findNextSafeInstruction(dyn_cast<Instruction>(GEP));

        if (!curr) {
            // No safe instruction in this block. Find the next safe block.
            safeBlock = PassUtils::findNextSafeBlock(origBB, GEP->getDebugLoc()->getLine());
        } else {
            // Found a safe instruction. Split the block there.
            safeBlock = origBB->splitBasicBlock(curr, "SafeBlockGEP");
        }

        
        // Step 1.B: Create the new "asan" block (which we'll call asanLogBlock).
        
        Function *F = origBB->getParent();
        LLVMContext &Fctx = F->getContext();

        BasicBlock *asanLogBlock = BasicBlock::Create(Fctx, "GEPAsanBlock", F);
        IRBuilder<> builder(asanLogBlock);

        // Step 1.C: Populate asanLogBlock with logging code.
        std::string sourcefile = getBaseName(M->getSourceFileName());
        llvm::FunctionCallee LogFunc = PassUtils::declareIllegalAccessLogger(M);
        std::string logMsg = PassUtils::createLogMsg(dyn_cast<Instruction>(GEP));

        Value *msgPtr = builder.CreateGlobalStringPtr(logMsg);
        Value *srcPtr = builder.CreateGlobalStringPtr(sourcefile);
        builder.CreateCall(LogFunc, {msgPtr, srcPtr});

        // Step 1.D: Add terminator to branch from log block to safe block.
        builder.CreateBr(safeBlock);

        // // Update PHI nodes in safeBlock for new incoming edge from AsanLogBlock
        // for (PHINode &PN : safeBlock->phis()) {
        //     PN.addIncoming(UndefValue::get(PN.getType()), asanLogBlock);
        // }
        // This newly created block is now our target for the error path.
        asanAbortBlock = asanLogBlock;
    }

    // errs()<<*asanAbortBlock<<"\n";

    // --- Part 2: Restructure the CFG ---

    // Create the continuation block by splitting *at* the GEP instruction.
    // The GEP instruction will be the first instruction in `contBB`.
    BasicBlock *contBB = origBB->splitBasicBlock(GEP, "gepCont");

    // Remove the unconditional branch that splitBasicBlock added to origBB.
    origBB->getTerminator()->eraseFromParent();

    // Insert the new conditional branch at the end of origBB.
    IRBuilder<> BrInst(origBB);

    Value* voidBasePtr = BrInst.CreateBitCast(basePtr,VoidPtrTy);
    
    Value* isInvalid = BrInst.CreateCall(PassUtils::getIsValidBasePtr(M),{voidBasePtr});
    
    // Create the null-check comparison.
    Value* cond = BrInst.CreateICmpEQ(isInvalid, ConstantInt::get(Int32Ty, 0));

    // If null, go to asan/log block. Otherwise, go to the continuation.
    BrInst.CreateCondBr(cond, asanAbortBlock, contBB);

    // errs() << "[instrumentGEP] inserted condbr from " << origBB->getName()
    //        << " to " << asanAbortBlock->getName() << " and " << contBB->getName() << "\n";
    // errs()<<*origBB<<"\n"<<*asanAbortBlock<<"\n"<<*contBB<<"\n";
    // errs()<<"SAfe: \n"<<*safeBlock<<"\n";
}

/**
 * @struct GEP
 * @brief An LLVM ModulePass to instrument GEP instructions.
 *
 * Implements the main pass logic, including collecting GEPs and
 * dispatching them for instrumentation.
 */
/**
 * @brief Main entry point for the GEP pass.
 * @param M The Module being processed.
 * @param AM The ModuleAnalysisManager.
 * @return PreservedAnalyses::none() as the CFG is modified.
 */
PreservedAnalyses GEP::run(Module &M, ModuleAnalysisManager &AM) {
    SmallVector<GetElementPtrInst *, 64> GEPs;
    // This is unused in the provided code, but kept for completeness
    // SmallVector<CallBase*, 64> AsanMemcpys;

    // Pass 1: Collection
    // We collect all GEPs first to avoid iterator invalidation issues
    // while we modify the CFG in Pass 2.
    for (Function &F : M) {
        if (F.isDeclaration()) continue;

        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (auto *G = dyn_cast<GetElementPtrInst>(&I)) {
                    GEPs.push_back(G);
                }
            }
        }
    }

    // Pass 2: Transformation
    // Now, iterate over the collected GEPs and instrument them.
    for (GetElementPtrInst *G : GEPs) {
        instrumentGEP(G);
    }

    FunctionAnalysisManager &FAM = AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();

    for (Function &F : M) {
        if (F.isDeclaration()) continue;

        DominatorTree &DT = FAM.getResult<DominatorTreeAnalysis>(F);
        BasicBlock *SafeExit = PassUtils::createCanonicalSafeExit(F);

        for (BasicBlock &BB : F) {
            // errs()<<BB<<"\n";
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
 * Registers the GEP pass with the LLVM pass manager, allowing it to
 * be loaded via tools like `opt`.
 * @return PassPluginLibraryInfo
 */
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<GEP>("GEP", "v0.1");
}