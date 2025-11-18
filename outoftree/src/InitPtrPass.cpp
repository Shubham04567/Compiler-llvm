#include "../include/InitPtrPass.h"
#include "../include/PassUtils.h"
#include "llvm/IR/InstIterator.h"

PreservedAnalyses InitPtrPass::run(Module &M, ModuleAnalysisManager &AM) {
    bool changed = false;

    for (Function &F : M) {
        if (F.isDeclaration()) continue;

        std::vector<AllocaInst*> ptrAllocas;

        // pointers those are allocated
        for (Instruction &I : instructions(F)) {
            if (auto *AI = dyn_cast<AllocaInst>(&I)) {
                Type *allocTy = AI->getAllocatedType();

                if (allocTy->isPointerTy()) {
                    ptrAllocas.push_back(AI);
                }
            }
        }

        // how much got initialised
        std::set<AllocaInst*> initialized;

        for (Instruction &I : instructions(F)) {
            if (auto *SI = dyn_cast<StoreInst>(&I)) {
                if (auto *AI = dyn_cast<AllocaInst>(SI->getPointerOperand())) {
                    initialized.insert(AI);
                }
            }
        }

        // initialised which are not initialised
        for (AllocaInst *AI : ptrAllocas) {
            if (initialized.count(AI) == 0) {
                Instruction *insertPt = AI->getNextNode();
                IRBuilder<> builder(insertPt);

                Value *nullPtr =
                    ConstantPointerNull::get(cast<PointerType>(AI->getAllocatedType()));

                builder.CreateStore(nullPtr, AI);
                // errs() << "[InitPtrPass] Initialized uninit pointer: "
                //         << AI->getName() << "\n";
            }
        }
    }

    return PreservedAnalyses::none();
}

extern "C" ::llvm::PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return PassUtils::buildPassPluginInfo<InitPtrPass>("InitPtrPass", "v0.1");
}