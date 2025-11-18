#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/IR/DebugLoc.h"
#include "PassUtils.h"

struct Memcpymv : llvm::PassInfoMixin<Memcpymv> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};