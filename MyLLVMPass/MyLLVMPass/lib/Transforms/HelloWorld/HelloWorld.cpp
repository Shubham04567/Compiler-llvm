#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// Define the pass: operates on each function
struct HelloWorldPass : public PassInfoMixin<HelloWorldPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        errs() << "Hello from: " << F.getName() << "\n";
        return PreservedAnalyses::all();
    }
};

// Register the pass with opt
extern "C" PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "HelloWorld", "v0.1",
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager &FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "hello-world") {
                        FPM.addPass(HelloWorldPass());
                        return true;
                    }
                    return false;
                });
        }
    };
}
