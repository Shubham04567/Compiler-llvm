#include "../include/GEP.h"
#include "llvm/IR/IRBuilder.h"
#include "../include/logger.h"

using namespace llvm;

void instrumentGEP(GetElementPtrInst *GEP) {
    if (!GEP) return;

    BasicBlock *origBB = GEP->getParent();
    if (!origBB) return;

    LLVMContext &Ctx = GEP->getContext();

    Value *basePtr = GEP->getPointerOperand();
    Value *nullPtr = Constant::getNullValue(basePtr->getType());

    // Find asan abort block in successors BEFORE splitting. Use origBB as start.
    BasicBlock *asanAbortBlock = PassUtils::findNextAsanBlock(origBB);

    BasicBlock* safeBlock = nullptr;

    if(!asanAbortBlock){
        // find safe instruction for split or safe block and inisert asan block and then 
        // jump on asan if illegal or continue;

        // find safe block:
        Instruction* curr = PassUtils::findNextSafeInstruction(dyn_cast<Instruction>(GEP));
        
        if(!curr){
            safeBlock = PassUtils::findNextSafeBlock(origBB, dyn_cast<Instruction>(GEP)->getDebugLoc()->getLine());
        }else{
            safeBlock = origBB->splitBasicBlock(curr,"SafeBlockGEP");
        }

        // create a block and add a function call
        // void asan_violation_log(char* char*) 
        // then add a unconditional jump to a safeblock
        
        Module* M = origBB->getModule();
        std::string sourcefile = getBaseName(M->getSourceFileName());
        llvm::FunctionCallee LogFunc = PassUtils::declareIllegalAccessLogger(M);
        
        Instruction* I = dyn_cast<Instruction>(GEP);
        Function* F = I->getFunction();
        std::string logMsg = PassUtils::createLogMsg(I);

        LLVMContext& Fctx = F->getContext();
        asanAbortBlock = BasicBlock::Create(Fctx,"GEPAsanBlock",F);
        IRBuilder<> builder(asanAbortBlock);

        Value *msgPtr = builder.CreateGlobalStringPtr(logMsg);
        Value *srcPtr = builder.CreateGlobalStringPtr(sourcefile);
        builder.CreateCall(LogFunc, {msgPtr, srcPtr});

        builder.CreateBr(safeBlock);
    
    }

    // Create continuation block by splitting at the GEP instruction.
    BasicBlock *contBB = origBB->splitBasicBlock(GEP, "gepCont");

    // Remove the auto-added unconditional branch from origBB and replace it with a conditional branch
    // Insert the condition and branch at the end of origBB.
    origBB->getTerminator()->eraseFromParent();
    IRBuilder<> BrInst(origBB);

    Value *isNullInOrig = BrInst.CreateICmpEQ(basePtr, Constant::getNullValue(basePtr->getType()), "isnull");

    BrInst.CreateCondBr(isNullInOrig, asanAbortBlock, contBB);

    // errs() << "[instrumentGEP] inserted condbr from " << origBB->getName()
    //        << " to " << asanAbortBlock->getName() << " and " << contBB->getName() << "\n";
}


PreservedAnalyses GEP::run(Module &M, ModuleAnalysisManager &AM) {

    SmallVector<GetElementPtrInst*, 64> GEPs;
    SmallVector<CallBase*, 64> AsanMemcpys;

    // Pass 1: collection (safe; does not mutate blocks)
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

    // Pass 2: transformation
    for (GetElementPtrInst *G : GEPs) {
        instrumentGEP(G);
    }

    return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<GEP>("GEP","v0.1");
}