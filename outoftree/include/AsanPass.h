#pragma once
#include "llvm/IR/PassManager.h"

struct AsanPass : llvm::PassInfoMixin<AsanPass> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};