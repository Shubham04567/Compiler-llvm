#pragma once
#include "llvm/IR/PassManager.h"
#include "llvm/IR/DebugLoc.h"
#include "PassUtils.h"

struct GEP : llvm::PassInfoMixin<GEP> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &AM);
};