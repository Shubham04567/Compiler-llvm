#include "../include/Memcpy.h"
#include "llvm/IR/IRBuilder.h"
#include "../include/logger.h"

using namespace llvm;

// Instrument an asan_memcpy call: if any operand is null -> branch to safeBlock, else continue.
static void instrumentAsanMemcpy(CallBase *call) {
    if (!call) return;
    // errs()<<*call<<"\n";
    BasicBlock *origBB = call->getParent();

    if (!origBB) return;

    // arg0 -> dest, arg1 -> src (according to signature of asan_memcpy)
    Value *dest = call->getArgOperand(0);
    Value *src  = call->getArgOperand(1);

    // find the safe instruction in block itself
    unsigned int faultline = call->getDebugLoc().getLine();
    Instruction* curr = PassUtils::findNextSafeInstruction(dyn_cast<Instruction>(call));

    // Find a safe continuation block BEFORE splitting.
    BasicBlock *start = call->getParent();
    BasicBlock *safeBlock = nullptr;
    if(curr == nullptr)
        safeBlock = PassUtils::findNextSafeBlock(start, faultline);
    else{
        safeBlock = origBB->splitBasicBlock(curr,"MemcpySafeBB");
    }

    // Split block before the call so the call and following instructions move to 'afterMemcpyBB'
    BasicBlock *afterMemcpyBB = origBB->splitBasicBlock(call, "afterMemcpy");

    // Remove the auto-inserted unconditional branch
    origBB->getTerminator()->eraseFromParent();

    IRBuilder<> BrInst(origBB);

    if (!safeBlock) {
        // No safeBlock found: fall back to unconditional branch to afterMemcpyBB
        BrInst.CreateBr(afterMemcpyBB);
        // errs() << "[instrumentAsanMemcpy] Warning: no safe continuation found from " << origBB->getName()
        //        << "; inserted unconditional branch to " << afterMemcpyBB->getName() << "\n";
        return;
    }

    Module* M = origBB->getModule();
        
    std::string sourcefile = getBaseName(M->getSourceFileName());
    // errs()<<sourcefile<<"\n";
    llvm::FunctionCallee LogFunc = PassUtils::declareIllegalAccessLogger(M);
    
    Instruction* I = dyn_cast<Instruction>(call);
    Function* F = I->getFunction();
    std::string logMsg = PassUtils::createLogMsg(I);

    LLVMContext& Fctx = F->getContext();
    BasicBlock* asanAbortBlock = BasicBlock::Create(Fctx,"MemcpyAsanBlock",F);
    IRBuilder<> builder(asanAbortBlock);

    Value *msgPtr = builder.CreateGlobalStringPtr(logMsg);
    Value *srcPtr = builder.CreateGlobalStringPtr(sourcefile);
    builder.CreateCall(LogFunc, {msgPtr, srcPtr});

    builder.CreateBr(safeBlock);

    Value *destNull = BrInst.CreateICmpEQ(dest, Constant::getNullValue(dest->getType()), "destNull");
    Value *srcNull  = BrInst.CreateICmpEQ(src,  Constant::getNullValue(src->getType()),  "srcNull");
    Value *anyNull = BrInst.CreateOr(destNull, srcNull, "anyNull");

    BrInst.CreateCondBr(anyNull, asanAbortBlock, afterMemcpyBB);

    // errs() << "[instrumentAsanMemcpy] inserted condbr from " << origBB->getName()
    //        << " to " << safeBlock->getName() << " and " << afterMemcpyBB->getName() << "\n";
}

PreservedAnalyses Memcpy::run(Module &M, ModuleAnalysisManager &AM) {
    SmallVector<CallBase*, 64> AsanMemcpys;
    for (Function &F : M) {
        if (F.isDeclaration()) continue;
        for (BasicBlock &BB : F) {
            for (Instruction &I : BB) {
                if (auto *C = dyn_cast<CallBase>(&I)) {
                    if (Function *called = C->getCalledFunction()) {
                        if (called->getName().contains("asan_memcpy")) {
                            AsanMemcpys.push_back(C);
                        }
                    }
                }
            }
        }
    }

    for (CallBase *C : AsanMemcpys) {
        instrumentAsanMemcpy(C);
    }

    return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<Memcpy>("Memcpy","v0.1");
}