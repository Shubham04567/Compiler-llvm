#include "llvm/IR/PassManager.h"


struct CountLGvars : llvm::PassInfoMixin<CountLGvars> {
    llvm::PreservedAnalyses run(llvm::Module &M,llvm::ModuleAnalysisManager &MA);
};

