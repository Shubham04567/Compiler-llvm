#pragma once
#include "llvm/IR/PassManager.h"
#include "PassUtils.h"

struct InitPtrPass : llvm::PassInfoMixin<InitPtrPass> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};