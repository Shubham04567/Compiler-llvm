/**
 * @file PassUtils.h
 * @brief A collection of static utility functions for LLVM passes.
 *
 * This header provides a `PassUtils` struct that acts as a namespace for
 * various helper functions. These utilities are used across different
 * instrumentation passes for tasks like:
 * - Identifying AddressSanitizer (ASan) blocks.
 * - Traversing the Control Flow Graph (CFG) to find "safe" blocks.
 * - Declaring logger functions within LLVM modules.
 * - Registering new Pass Manager (NPM) plugins.
 */

#pragma once

// LLVM Headers
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Dominators.h"

// Project-specific Headers
#include "logger.h"

// Standard Library Headers
#include <queue>
#include <set>
#include <string>

using namespace llvm;

/**
 * @struct PassUtils
 * @brief A utility class (struct) providing static helper functions.
 *
 * This struct is used as a namespace for common utilities needed across
 * different LLVM passes. All members are static.
 */
struct PassUtils {
    /**
     * @brief Checks if a BasicBlock is an AddressSanitizer (ASan) abort block.
     *
     * An ASan abort block is identified by the presence of an
     * `UnreachableInst` or a call to an `__asan_report_` or `__asan_log_`
     * function.
     *
     * @param BB The BasicBlock to check.
     * @return true if the block is an ASan abort block, false otherwise.
     */
    static bool isAsanAbortBlock(const BasicBlock *BB) {
        if (!BB) return false;
        for (const Instruction &I : *BB) {
            // ASan blocks often end in 'unreachable' after reporting.
            if (isa<UnreachableInst>(&I)) {
                return true;
            }
            // Check for calls to ASan report or log functions.
            if (const CallBase *CB = dyn_cast<CallBase>(&I)) {
                if (const Function *Callee = CB->getCalledFunction()) {
                    StringRef name = Callee->getName();
                    if (name.starts_with("__asan_report_") || name.starts_with("__asan_log_")) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    static bool isReachable(BasicBlock *src, BasicBlock *target) {
        std::queue<BasicBlock*> Q;
        std::set<BasicBlock*> visited;

        Q.push(src);
        visited.insert(src);

        while (!Q.empty()) {
            BasicBlock *cur = Q.front();
            Q.pop();

            if (cur == target)
                return true;

            for (BasicBlock *n : successors(cur)) {
                if (!visited.insert(n).second) continue;
                Q.push(n);
            }
        }
        return false;
    }

    /**
     * @brief Finds the nearest "safe" successor block using BFS.
     *
     * A "safe" block is defined as a block that:
     * 1. Is not an ASan abort block.
     * 2. Does not contain any instructions originating from the `faultLine`.
     *
     * This is used to find a suitable place to redirect control flow after
     * logging an error, avoiding re-execution of the faulting code.
     *
     * @param start The BasicBlock to start the BFS from.
     * @param faultLine The source code line number of the faulting instruction.
     * @return A pointer to the nearest safe BasicBlock. If none is found,
     * returns a canonical exit block for the function.
     */
    static BasicBlock *findNextSafeBlock(BasicBlock *start, unsigned faultLine) {
        std::queue<BasicBlock *> Q;
        std::set<BasicBlock *> visited;
        Q.push(start);
        visited.insert(start);
        // errs() << "findNextSafeBlock starting from: " << start->getName() << " for line " << faultLine << "\n";

        while (!Q.empty()) {
            BasicBlock *cur = Q.front();
            Q.pop();

            // Iterate successors of the current block
            for (BasicBlock *nextSucc : successors(cur)) {
                // errs() << "  Visiting successor: " << nextSucc->getName() << "\n";
                
                // Skip already visited blocks
                if (!visited.insert(nextSucc).second) continue;
                
                // Skip ASan abort blocks
                if (isAsanAbortBlock(nextSucc)) continue;

                // Check if nextSucc contains instructions related to the fault line
                bool relatedToFault = false;
                for (Instruction &NI : *nextSucc) {
                    if (NI.getDebugLoc()) {
                        unsigned line = NI.getDebugLoc().getLine();
                        // errs() << "    Inst line: " << line << "\n";
                        if (line == faultLine) {
                            relatedToFault = true;
                            break;
                        }
                    }
                }

                if (!relatedToFault) {
                    // Found a block unrelated to the fault; this is our safe target.
                    // errs() << "BFS found unrelated block: " << nextSucc->getName() << "\n";
                    if(isReachable(nextSucc,start)){
                        Function *F = start->getParent();
                        return createCanonicalSafeExit(*F);
                    }
                    return nextSucc;
                }

                // This block is related, so keep exploring from it.
                Q.push(nextSucc);
            }
        }

        // BFS failed to find a safe successor in the reachable CFG.
        // As a fallback, create or get a canonical exit block for the function.
        Function *F = start->getParent();
        return createCanonicalSafeExit(*F);
    }

    /**
     * @brief Finds the nearest ASan abort block in the successors.
     *
     * Performs a limited-depth (default 10 levels) BFS from the `sourceBB`
     * to find the first successor block that is an ASan abort block.
     *
     * @param sourceBB The BasicBlock to start the search from.
     * @return A pointer to the ASan abort block if found, nullptr otherwise.
     */
    static BasicBlock *findNextAsanBlock(BasicBlock *sourceBB) {
        if (!sourceBB) return nullptr;

        // errs()<<"finding"<<"\n";
        std::queue<BasicBlock *> Q;
        std::set<BasicBlock *> visited;

        Q.push(sourceBB);
        visited.insert(sourceBB);
        
        // Limit the search depth to 5 levels to avoid runaway BFS
        // in complex CFGs.
        int ct = 5; 

        while (!Q.empty() && ct) {

            int n = Q.size();
            while(n--){
                BasicBlock *curr = Q.front();
                // errs()<<*curr<<"\n";
                Q.pop();

                for (BasicBlock *nextSucc : successors(curr)) {
                    if (visited.count(nextSucc)) continue;

                    // Found it.
                    if (isAsanAbortBlock(nextSucc)) {
                        return nextSucc;
                    }

                    Q.push(nextSucc);
                    visited.insert(nextSucc);
                }
            }
            // errs()<<"-----\n";
            --ct;
        }

        // errs()<<"unable\n";

        // Not found within the depth limit.
        return nullptr;
    }

    /**
     * @brief Finds the next "safe" instruction within the same BasicBlock.
     *
     * A "safe" instruction is the next instruction that is not a PHI node
     * and does not originate from the same source line as the input `Instr`.
     * This is used to find a split point within a basic block *after*
     * the faulting instruction.
     *
     * @param Instr The instruction to start searching from (exclusive).
     * @return A pointer to the next safe instruction, or nullptr if none
     * is found in the block.
     */
    static Instruction *findNextSafeInstruction(Instruction *Instr) {

        // first i want to check that is this block is part of a loop
        // if yes then it will be always better to go out of loop
        // that is jump to safeExit so first check is this block reachable 
        // from any of its successor
        BasicBlock* src = Instr->getParent();
        // errs()<<"Inst\n"<<*Instr<<"\n";
        // errs()<<*src<<"\n";
        
        for(BasicBlock* succ : successors(src)){
            // errs()<<*succ<<"\n";
            if(isReachable(succ,src)) return nullptr;
        }

        Instruction *curr = Instr->getNextNode();
        if (!curr) return nullptr;

        unsigned int faultLine = Instr->getDebugLoc().getLine();

        while(curr){
            
            // there might be case that instruction can be 
            // phi node so 
            if(isa<PHINode>(curr) || isa<BranchInst>(curr)){
                curr = curr->getNextNode();
                continue;
            }

            if(curr->getDebugLoc()){
                if(curr->getDebugLoc().getLine() != faultLine){
                    break;
                }
            } else {
                // No debug info, assume it's part of a "safe" epilogue
                // or non-debuggable code.
                break;
            }

            // This instruction is still on the same faulting line, keep searching.
            curr = curr->getNextNode();
        }

        return curr;
    }

    /**
     * @brief Declares the external `__asan_log_violation` function in the module.
     *
     * This function (or retrieves it if it already exists) inserts the
     * function declaration for the custom logger.
     * The expected signature is `void __asan_log_violation(i8* msg, i8* file)`.
     *
     * @param M The Module to insert the declaration into.
     * @return A FunctionCallee wrapper for the logging function.
     */
    static FunctionCallee declareIllegalAccessLogger(Module *M) {
        LLVMContext &Ctx = M->getContext();
        
        // Create type for `i8*` (char*)
        Type *Int8PtrTy = PointerType::getUnqual(Type::getInt8Ty(Ctx));
        
        // Create function type: void(i8*, i8*)
        FunctionType *LogFuncTy =
            FunctionType::get(Type::getVoidTy(Ctx), {Int8PtrTy, Int8PtrTy}, false);
        
        // Get or insert the function declaration
        FunctionCallee LogFunc =
            M->getOrInsertFunction("__asan_log_violation", LogFuncTy);

        return LogFunc;
    }

    /**
     * @brief Creates a formatted log message string for an ASan violation.
     *
     * The message includes the function name, filename, and line number
     * extracted from the instruction's debug information.
     *
     * @param I The faulting instruction.
     * @return A std::string containing the formatted log message.
     */
    static std::string createLogMsg(Instruction *I) {
        std::string logMsg = "ASAN violation in function: " + I->getFunction()->getName().str();
        
        if (I->getDebugLoc()) {
            DebugLoc DL = I->getDebugLoc();
            StringRef fname = DL->getFilename();
            unsigned line = DL.getLine();
            logMsg += " at " + fname.str() + ":" + std::to_string(line);
        } else {
            logMsg += " (no debug location)";
        }

        return logMsg;
    }

    /**
     * @brief A template helper to build the PassPluginLibraryInfo for a new PM pass.
     *
     * This function abstracts the boilerplate required to register a pass
     * (that uses `PassInfoMixin`) with the new Pass Manager. It sets up
     * the `registerPipelineParsingCallback` to instantiate the pass
     * when its name is invoked (e.g., via `opt -passes=passName`).
     *
     * @tparam PassT The type of the pass to register (must derive from
     * PassInfoMixin<PassT> and have a default constructor).
     * @param passName The string name used to invoke the pass.
     * @param versionNum The version string for the plugin.
     * @return A fully populated PassPluginLibraryInfo struct.
     */
    template <typename PassT>
    static PassPluginLibraryInfo buildPassPluginInfo(const char *passName, const char *versionNum) {
        
        // The string must persist for the lifetime of the plugin,
        // so we make it static.
        static std::string NM(passName);

        return {
            LLVM_PLUGIN_API_VERSION,
            NM.c_str(),
            versionNum,
            [](llvm::PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](llvm::StringRef Name,
                       llvm::ModulePassManager &MPM,
                       llvm::ArrayRef<llvm::PassBuilder::PipelineElement>) {
                        
                        // Check if this callback is for our pass.
                        if (Name == NM) {
                            // Add an instance of the pass.
                            MPM.addPass(PassT{});
                            return true;
                        }
                        return false;
                    });
            }};
    }

    /**
     * @brief Creates or retrieves a canonical "SafeExit" block for a function.
     *
     * This block serves as a universal "safe" continuation point, especially
     * when no other safe successor can be found. It inserts a `Ret`
     * instruction with a sensible default value (0, null, void, undef)
     * based on the function's return type.
     *
     * If a block named "SafeExit" already exists, it is returned.
     *
     * @param F The Function to add the exit block to.
     * @return A pointer to the existing or newly created "SafeExit" block.
     */
    static BasicBlock *createCanonicalSafeExit(Function &F) {
        // Check if the block already exists to avoid duplicates.
        for (auto &BB : F) {
            if (BB.getName() == "SafeExit") {
                return &BB;
            }
        }

        // Create the new block.
        LLVMContext &Ctx = F.getContext();
        BasicBlock *SafeExit = BasicBlock::Create(Ctx, "SafeExit", &F);
        IRBuilder<> B(SafeExit);

        // Populate with a default return instruction.
        Type *RetTy = F.getReturnType();
        if (RetTy->isVoidTy()) {
            B.CreateRetVoid();
        } else if (RetTy->isIntegerTy()) {
            B.CreateRet(ConstantInt::get(RetTy, 0));
        } else if (RetTy->isPointerTy()) {
            B.CreateRet(ConstantPointerNull::get(cast<PointerType>(RetTy)));
        } else if (RetTy->isFloatingPointTy()) {
            B.CreateRet(ConstantFP::get(RetTy, 0.0));
        } else {
            // Fallback for complex types (structs, vectors, etc.)
            B.CreateRet(UndefValue::get(RetTy));
        }

        return SafeExit;
    }

    static FunctionCallee getIsValidBasePtr(Module* M){
        // Get the LLVM context and define key types
        LLVMContext &Context = M->getContext();
        Type *VoidPtrTy = PointerType::getUnqual(Type::getInt8Ty(Context));
        Type *Int32Ty = Type::getInt32Ty(Context);

        // 1. Declare the runtime check function: i32 @__my_is_valid_free_ptr(i8*)
        // This function is expected to be provided by a linked runtime library.
        // It returns 0 if the pointer is INVALID for free, 1 if valid.
        FunctionType *CheckFnTy = FunctionType::get(Int32Ty, {VoidPtrTy}, false);
        FunctionCallee MemCheckFn =
        M->getOrInsertFunction("__my_is_valid_base_ptr", CheckFnTy);

        return MemCheckFn;
    }

    static void fixDominanceForInstruction(Instruction *I, DominatorTree &DT, BasicBlock *SafeExitBB)  {
        BasicBlock *CurrBB = I->getParent();

        // For each operand used by the instruction
        for (Value *Op : I->operands()) {

            
            Instruction *DefI = dyn_cast<Instruction>(Op);
            if (!DefI) continue;  // Operand is a constant or argument → skip

            BasicBlock *DefBB = DefI->getParent();

            if(DefBB == CurrBB) continue;

            // first  collect all the predecessors 
            std::queue<std::pair<BasicBlock*,BasicBlock*>> predq;
            std::set<BasicBlock*> predvisited;
            // errs()<<"predecessors\n";
            // Iterate over all predecessors of the block containing I
            for (BasicBlock *PredBB : predecessors(CurrBB)) {

                // Dominance condition:
                // DefBB must dominate PredBB for the operand to be valid on all paths
                if (!DT.dominates(DefBB, PredBB)) {
                    // errs()<<*PredBB<<"\n";
                    predq.push({PredBB,CurrBB});
                    predvisited.insert(PredBB);
                    
                }
            }

            if(!predq.empty()){
                // precompute the reachability of blocks from DefBB

                std::set<BasicBlock*> visited;
                std::queue<BasicBlock*> q;
                q.push(DefBB);

                visited.insert(DefBB);

                while(!q.empty()){
                    BasicBlock* curr = q.front();q.pop();
                    
                    for(auto succ : successors(curr)){
                        if(!visited.count(succ)){
                            visited.insert(succ);
                            q.push(succ);
                        }
                    }
                }
                // errs()<<"Use block"<<*CurrBB<<"\n";

                // errs()<<"Reachable from def"<<*DefBB<<"\n";

                // for(auto& bb : visited){
                //     errs()<<*bb<<"\n";
                // }

                // errs()<<"end\n";

                // errs()<<"backward bfs\n";

                while(!predq.empty()) {
                    
                    auto [curr,par] = predq.front();predq.pop();
                    // errs()<<*curr<<"\n";

                    if(visited.count(curr)){
                        
                        if(curr != DefBB){
                            for(auto pred : predecessors(curr)){
                                if(!predvisited.count(pred)){
                                    predq.push({pred,curr});
                                    predvisited.insert(pred);
                                }
                            }
                        }

                    }else{
                        // get the divergence
                        Instruction *TI = curr->getTerminator();
                        for (unsigned i = 0; i < TI->getNumSuccessors(); i++) {
                            if (TI->getSuccessor(i) == par) {
                                TI->setSuccessor(i, SafeExitBB);
                                // errs()<< *curr << "\n";
                                // errs() << "[DominanceFix] Redirected edge "
                                //     << curr->getName()
                                //     << " → SafeExitBB (" 
                                //     << SafeExitBB->getName() << ")\n";
                            }
                        }
                    }

                }
                // errs()<<"backward bfs end\n";
            }
        }
    }
};