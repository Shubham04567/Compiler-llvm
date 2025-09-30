#pragma once
#include "llvm/IR/PassManager.h"

struct HelloWorldPass : llvm::PassInfoMixin<HelloWorldPass> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};