#pragma once
#include "llvm/IR/PassManager.h"
#include "PassUtils.h"

struct FreePass : llvm::PassInfoMixin<FreePass> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};