#pragma once
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "logger.h"
#include <queue>
#include <set>
#include <string>

using namespace llvm;

struct PassUtils{
    static bool isAsanAbortBlock(const BasicBlock *BB) {
        for (const Instruction &I : *BB) {
            if (isa<UnreachableInst>(&I))
                return true;
            if (const CallBase *CB = dyn_cast<CallBase>(&I)) {
                if (const Function *Callee = CB->getCalledFunction()) {
                    StringRef name = Callee->getName();
                    if (name.starts_with("__asan_report_") || name.starts_with("__asan_log_"))
                        return true;
                }
            }
        }
        return false;
    }

    static BasicBlock * findNextSafeBlock(BasicBlock *start,unsigned faultLine){
        std::queue<BasicBlock *> Q;
        std::set<BasicBlock *> visited;
        Q.push(start);
        visited.insert(start);
        errs()<<faultLine<<"\n";
        while (!Q.empty()) {
            BasicBlock *cur = Q.front();
            Q.pop();
            
            // Iterate successors of the current block
            for (succ_iterator SI = succ_begin(cur), SE = succ_end(cur); SI != SE; ++SI) {
                BasicBlock *nextSucc = *SI;
                errs()<<*nextSucc<<"\n";
                if (!visited.insert(nextSucc).second) continue; // already visited
                if (isAsanAbortBlock(nextSucc)) continue;       // skip ASan abort blocks

                // Check if nextSucc contains the same fault line
                bool relatedToFault = false;
                for (Instruction &NI : *nextSucc) {
                    if (NI.getDebugLoc()) {
                        
                        unsigned line = NI.getDebugLoc().getLine();
                        errs()<<NI<<" "<<line<<"\n";
                        if (line == faultLine) {
                            relatedToFault = true;
                            break;
                        }
                    }
                    else{
                        errs()<<NI<<"\n";
                    }
                }

                if (!relatedToFault) {
                    // errs() << "BFS found unrelated block: " << nextSucc->getName() << "\n";
                    return nextSucc; // Found nearest unrelated block
                }

                // Otherwise, keep exploring
                Q.push(nextSucc);
            }
        }

        return (BasicBlock *)nullptr; // No unrelated block found
    }

    static BasicBlock * findNextAsanBlock(BasicBlock *sourceBB) {
        if (!sourceBB) return nullptr;

        std::queue<BasicBlock*> Q;
        std::set<BasicBlock*> visited;

        Q.push(sourceBB);
        visited.insert(sourceBB);
        int ct = 10;

        while (!Q.empty() && ct--) {
            BasicBlock *curr = Q.front();
            Q.pop();

            for (succ_iterator SI = succ_begin(curr), SE = succ_end(curr); SI != SE; ++SI) {
                BasicBlock *nextSucc = *SI;

                if (visited.count(nextSucc)) continue;
                if (isAsanAbortBlock(nextSucc)) return nextSucc;

                Q.push(nextSucc);
                visited.insert(nextSucc);
            }
        }

        return nullptr;
    }

    static Instruction* findNextSafeInstruction(Instruction* Instr){

        Instruction* curr = Instr->getNextNode();
        if(!curr) return curr;

        unsigned int faultLine = Instr->getDebugLoc().getLine();

        while(curr){
            
            // there might be case that instruction can be 
            // phi node so 
            if(isa<PHINode>(curr)) continue;
            if(curr->getDebugLoc()){
                if(curr->getDebugLoc().getLine() != faultLine){
                    break;
                }
            }else break;

            curr = curr->getNextNode();
        }

        return curr;
    }


    static FunctionCallee declareIllegalAccessLogger(Module* M){
        LLVMContext &Ctx = M->getContext();
        Type *Int8PtrTy = PointerType::getUnqual(Type::getInt8Ty(Ctx));
        FunctionType *LogFuncTy =
            FunctionType::get(Type::getVoidTy(Ctx),
                                    {Int8PtrTy, Int8PtrTy}, false);
        FunctionCallee LogFunc =
            M->getOrInsertFunction("__asan_log_violation", LogFuncTy);
        
        return LogFunc;
    }

    static std::string createLogMsg(Instruction* I){
        std::string logMsg = "ASAN violation in function: " + I->getFunction()->getName().str();
        unsigned faultLine = 0;
        if (I->getDebugLoc()) {
            DebugLoc DL = I->getDebugLoc();
            StringRef fname = DL->getFilename();
            unsigned line = DL.getLine();
            logMsg += " at " + fname.str() + ":" + std::to_string(line);
            faultLine = line;
        } else {
            logMsg += " (no debug location)";
        }

        return logMsg;
    }

    /**
     * In New Pass Manager Passes are not inherited they use
     * CRTP template, PassInfoMixin<> is not a common base
     * so can't be use as polymorphic base class , hence 
     * here template will be used to accept any pass
     */
    template<typename PassT>
    static PassPluginLibraryInfo buildPassPluginInfo(const char* passName, const char* versionNum) {
        
        static std::string NM(passName);     // must persist after function returns
        // static std::string VER(versionNum);  // same for version

        return {
            LLVM_PLUGIN_API_VERSION,
            NM.c_str(),
            versionNum,
            [](llvm::PassBuilder &PB) {

                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef Name,
                    llvm::ModulePassManager &MPM,
                    llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {

                        if (Name == NM) {               // now safe
                            MPM.addPass(PassT{});       // requires default constructor
                            return true;
                        }
                        return false;
                    }
                );
            }
        };
    }

    static BasicBlock* createCanonicalSafeExit(Function &F) {
        LLVMContext &Ctx = F.getContext();
        Type *RetTy = F.getReturnType();

        BasicBlock *SafeExit = BasicBlock::Create(Ctx, "SafeExit", &F);
        IRBuilder<> B(SafeExit);

        if (RetTy->isVoidTy()) {
            B.CreateRetVoid();
        } else if (RetTy->isIntegerTy()) {
            B.CreateRet(ConstantInt::get(RetTy, 0));
        } else if (RetTy->isPointerTy()) {
            B.CreateRet(ConstantPointerNull::get(cast<PointerType>(RetTy)));
        } else if (RetTy->isFloatingPointTy()) {
            B.CreateRet(ConstantFP::get(RetTy, 0.0));
        } else {
            // structs, vectors, arrays → return undef
            B.CreateRet(UndefValue::get(RetTy));
        }

        return SafeExit;
    }



};