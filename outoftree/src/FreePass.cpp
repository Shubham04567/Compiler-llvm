//===- FreePass.cpp - Pass to Instrument 'free' calls ---===//
//
// This pass instruments calls to 'free' to check for memory errors
// like double-free, invalid-free, or use-after-free.
//
// It uses a custom runtime function, '__my_is_valid_free_ptr',
// (which should be linked in) to check the validity of the pointer
// at runtime, just before 'free' is called.
//
// If the pointer is invalid, control is redirected to a logging block
// that reports the error (using '__asan_log_violation') and then
// skips the 'free' call to prevent a crash.
//
// If the pointer is valid, control flows to the original 'free' call.
//
// --- Block Structure Transformation ---
//
// Before:
//   [ ... code ... ]
//   [ call void @free(i8* %ptr) ]
//   [ ... code after free ... ]
//   [ terminator ]
//
// After:
//   [ ... code ... ]
//   [ %is_invalid = call i32 @__my_is_valid_free_ptr(i8* %ptr) ]
//   [ %cond = icmp eq i32 %is_invalid, 0 ]
//   [ br i1 %cond, label %AsanLogBlock, label %WAfterFreeBlock ]
//
//   AsanLogBlock:
//   [ call void @__asan_log_violation(...) ]
//   [ br label %SafeBlock ]
//
//   WAfterFreeBlock:
//   [ call void @free(i8* %ptr) ]
//   [ br label %SafeBlock ]
//
//   SafeBlock:
//   [ ... code after free ... ]
//   [ terminator ]
//
//===----------------------------------------------------------------------===//

#include "../include/FreePass.h"
#include "../include/PassUtils.h" // For PassUtils helper functions
#include "llvm/IR/IRBuilder.h"

using namespace llvm;

///
/// Main run method for the FreePass.
///
PreservedAnalyses FreePass::run(Module &M, ModuleAnalysisManager &AM) {

  // Get the LLVM context and define key types
  LLVMContext &Context = M.getContext();
  Type *VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(Context));
  Type *Int32Ty = Type::getInt32Ty(Context);

  // 1. Declare the runtime check function: i32 @__my_is_valid_free_ptr(i8*)
  // This function is expected to be provided by a linked runtime library.
  // It returns 0 if the pointer is INVALID for free, 1 if valid.
  FunctionType *CheckFnTy = FunctionType::get(Int32Ty, {VoidPtrTy}, false);
  FunctionCallee MemCheckFn =
      M.getOrInsertFunction("__my_is_valid_free_ptr", CheckFnTy);

  // 2. Declare the ASan logging function
  FunctionCallee LogFunc = PassUtils::declareIllegalAccessLogger(&M);
  std::string SrcFile = getBaseName(M.getSourceFileName());

  // Iterate over all functions in the module
  for (auto &F : M) {
    if (F.isDeclaration())
      continue; // Skip external function declarations

    // Collect all direct free call sites in this function first
    SmallVector<CallInst*, 16> frees;
    for (BasicBlock &BB : F) {
        for (Instruction &I : BB) {
            if (CallInst *CI = dyn_cast<CallInst>(&I)) {
                if (Function *CF = CI->getCalledFunction()) {
                    if (CF->getName().starts_with("free")) {
                        frees.push_back(CI);
                    }
                }
            }
        }
    }

    for(CallInst* Call : frees){

      BasicBlock* BB = Call->getParent();
      Instruction* I = dyn_cast<Instruction>(Call);

      // --- 1. Find the continuation block ("SafeBlock") ---
      // This is where execution will continue AFTER the check/free.

      Instruction *SplitPoint = PassUtils::findNextSafeInstruction(I);
      BasicBlock *SafeBlock = nullptr;

      if (!SplitPoint) {
        // 'free' is the last instruction, so the "safe" block
        // is the original successor block.
        SafeBlock = PassUtils::findNextSafeBlock(BB, I->getDebugLoc()->getLine());
      } else {
        // Split the block AFTER the 'free' call.
        // 'SafeBlock' will contain the instructions that
        // originally came after 'free'.
        SafeBlock = BB->splitBasicBlock(SplitPoint, "FreeSafeBlock");
      }

      // --- 2. Create the "WAfterFree" Block ---
      // This block will contain the original 'free' call.
      // We split at &I (the 'free' call itself).
      // BB becomes the "PreFree" block.
      BasicBlock *WAfterFreeBlock = BB->splitBasicBlock(I, "WAfterFree");

      // --- 3. Create the "AsanLogBlock" ---
      // This block is executed if the pointer check fails.
      std::string LogMsg = PassUtils::createLogMsg(I);
      BasicBlock *AsanLogBlock =
          BasicBlock::Create(Context, "FreeLogBlock", &F);

      // Populate the AsanLogBlock
      IRBuilder<> LogBuilder(AsanLogBlock);
      Value *SrcFilePtr = LogBuilder.CreateGlobalStringPtr(SrcFile);
      Value *LogMsgPtr = LogBuilder.CreateGlobalStringPtr(LogMsg);
      LogBuilder.CreateCall(LogFunc, {LogMsgPtr, SrcFilePtr});
      LogBuilder.CreateBr(SafeBlock); // Jump to safety

      // --- 4. Instrument the "PreFree" block (BB) ---
      // Insert the check and conditional branch at the
      // end of the original block (which is now BB).

      // Get the old terminator (which jumps to WAfterFreeBlock)
      Instruction *OldTerminator = BB->getTerminator();
      IRBuilder<> CheckBuilder(OldTerminator);

      // Get the pointer being freed
      Value *MemPtr = Call->getArgOperand(0);
      Value *VoidPtr = MemPtr;

      // Ensure the pointer is cast to i8* for the check function
      if (VoidPtr->getType() != VoidPtrTy) {
        VoidPtr = CheckBuilder.CreateBitCast(MemPtr, VoidPtrTy);
      }

      // Insert the call to our check function
      Value *IsInvalid = CheckBuilder.CreateCall(MemCheckFn, {VoidPtr});

      // Check if the result is 0 (invalid)
      Value *Cond =
          CheckBuilder.CreateICmpEQ(IsInvalid, ConstantInt::get(Int32Ty, 0));

      // Create the conditional branch
      // if (invalid) -> AsanLogBlock
      // else (valid) -> WAfterFreeBlock
      CheckBuilder.CreateCondBr(Cond, AsanLogBlock, WAfterFreeBlock);

      // Remove the old terminator
      OldTerminator->eraseFromParent();

      // We have fundamentally changed the block structure,
      // so we must stop iterating instructions in this block.
    }
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
  // The pass modified the IR, so don't preserve any analyses.
  return PreservedAnalyses::none();
}

///
/// Plugin registration boilerplate.
///
/// Tells LLVM how to create a new instance of this pass.
///
extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
  return PassUtils::buildPassPluginInfo<FreePass>("FreePass", "v0.1");
}