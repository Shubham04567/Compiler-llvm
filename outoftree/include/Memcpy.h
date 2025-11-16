#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/IR/DebugLoc.h"
#include "PassUtils.h"

struct Memcpy : llvm::PassInfoMixin<Memcpy> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};