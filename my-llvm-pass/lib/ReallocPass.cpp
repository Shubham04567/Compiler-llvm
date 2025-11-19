
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/Analysis/ValueTracking.h"
#include <vector>

using namespace llvm;

namespace {

struct ReallocPass : public PassInfoMixin<ReallocPass> {

    // actual heap allocation 
    Value* findHeapBase(Value *ptr, std::set<Value*> &visited) {
        if (visited.count(ptr)) return nullptr;
        visited.insert(ptr);

        if (auto *gep = dyn_cast<GetElementPtrInst>(ptr)) {
            return findHeapBase(gep->getPointerOperand(), visited);
        }
        if (auto *bc = dyn_cast<BitCastInst>(ptr)) {
            return findHeapBase(bc->getOperand(0), visited);
        }

        if (auto *load = dyn_cast<LoadInst>(ptr)) {
            Value *loadPtr = load->getPointerOperand();
            
            // Check all stores to this location
            for (User *U : loadPtr->users()) {
                if (auto *store = dyn_cast<StoreInst>(U)) {
                    if (store->getPointerOperand() == loadPtr) {
                        Value *storedVal = store->getValueOperand();
                        
                        
                        if (auto *call = dyn_cast<CallInst>(storedVal)) {
                            if (Function *f = call->getCalledFunction()) {
                                StringRef name = f->getName();
                                if (name == "malloc" || name == "calloc" || name == "realloc" ||
                                    name == "tracked_malloc") {
                                    return call;  
                                }
                            }
                        }
                    
                        // Recursively 
                        Value *result = findHeapBase(storedVal, visited);
                        if (result) return result;
                    }
                }
            }
            return nullptr;
        }

        if (auto *call = dyn_cast<CallInst>(ptr)) {
            if (Function *f = call->getCalledFunction()) {
                StringRef name = f->getName();
                if (name == "malloc" || name == "calloc" || name == "realloc" || 
                    name == "tracked_malloc") {
                    return call;
                }
            }
        }

        if (auto *phi = dyn_cast<PHINode>(ptr)) {
            for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
                Value *result = findHeapBase(phi->getIncomingValue(i), visited);
                if (result) return result;
            }
        }

        return nullptr;
    }
    
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &MAM) {
        LLVMContext &Ctx = M.getContext();
        const DataLayout &DL = M.getDataLayout();
        // errs() << "[asan-recover] running on module: " << M.getName() << "\n";

        // Types
        IntegerType *i8Ty = IntegerType::get(Ctx, 8);
        IntegerType *i64Ty = IntegerType::get(Ctx, 64);
        PointerType *i8PtrTy = PointerType::get(i8Ty, 0);
        IntegerType *i32Ty = IntegerType::get(Ctx, 32);

        // replace malloc calls with tracked_malloc
        FunctionCallee tracked_malloc_fn = M.getOrInsertFunction(
            "tracked_malloc",
            i8PtrTy,
            i64Ty
        );

        for (Function &F : M) {
            if (F.isDeclaration()) continue;
            std::vector<CallInst*> malloc_calls;
            
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                    if (auto *CI = dyn_cast<CallInst>(&I)) {
                        if (Function *callee = CI->getCalledFunction()) {
                            if (callee->getName() == "malloc") {
                                malloc_calls.push_back(CI);
                            }
                        }
                    }
                }
            }
            
            for (CallInst *CI : malloc_calls) {
                IRBuilder<> builder(CI);
                Value *size = CI->getArgOperand(0);
                Value *newCall = builder.CreateCall(tracked_malloc_fn, {size});
                CI->replaceAllUsesWith(newCall);
                CI->eraseFromParent();
                // errs() << "[asan-recover] Replaced malloc with tracked_malloc\n";
            }
        }

        // Runtime function prototypes
        FunctionCallee fn_isValid = M.getOrInsertFunction(
            "isAddressValid",
            i32Ty,
            i8PtrTy,
            i64Ty
        );

        FunctionCallee fn_handle = M.getOrInsertFunction(
            "my_handle_illegal_access",
            i8PtrTy,
            i8PtrTy,
            i8PtrTy,
            i64Ty
        );

        FunctionCallee fn_update_ptr = M.getOrInsertFunction(
            "update_pointer_after_realloc",
            Type::getVoidTy(Ctx),
            i8PtrTy,  
            i8PtrTy   
        );

        FunctionCallee fn_get_new_base = M.getOrInsertFunction(
            "get_new_base",
            i8PtrTy,
            i8PtrTy
        );

        // Calculate offset between two pointers at runtime
        FunctionCallee fn_calc_offset = M.getOrInsertFunction(
            "calculate_pointer_offset",
            i64Ty,     // returns offset
            i8PtrTy,   // pointer
            i8PtrTy    // base
        );

        // Apply offset to a new base
        FunctionCallee fn_apply_offset = M.getOrInsertFunction(
            "apply_offset_to_base",
            i8PtrTy,   // returns adjusted pointer
            i8PtrTy,   // new base
            i64Ty      // offset
        );

        // Collect instructions => illegal accesses
        std::vector<Instruction*> worklist;
        for (Function &F : M) {
            if (F.isDeclaration()) continue;
            for (BasicBlock &BB : F)
                for (Instruction &I : BB)
                    if (isa<StoreInst>(&I))  
                        worklist.push_back(&I);
                    else if (isa<LoadInst>(&I))
                        worklist.push_back(&I);
        }

        for (Instruction *I : worklist) {
            if (!I->getFunction()) continue;

            if (StoreInst *SI = dyn_cast<StoreInst>(I)) {
                Value *val = SI->getValueOperand();
                Value *origPtr = SI->getPointerOperand();

                // heap base
                std::set<Value*> visited;
                Value *heapBase = findHeapBase(origPtr, visited);

                if (!heapBase) {
                    // errs() << "[asan-recover] SKIPPING (no heap base found): " << *SI << "\n";
                    continue;
                }

                // errs() << "[asan-recover] instrumenting Store: " << *SI << "\n";
                // errs() << "value: " << *val << "\n";
                // errs() << "pointer: " << *origPtr << "\n";
                // errs() << "Heap base: " << *heapBase << "\n";

                // variable that holds the heap pointer (for updating after realloc)
                Value *ptrVariable = nullptr;

                if (auto *gep = dyn_cast<GetElementPtrInst>(origPtr)) {
                    Value *base = gep->getPointerOperand();

                    // Trace back through nested GEPs
                    while (auto *innerGep = dyn_cast<GetElementPtrInst>(base)) {
                        base = innerGep->getPointerOperand();
                    }
                    if (auto *load = dyn_cast<LoadInst>(base)) {
                        ptrVariable = load->getPointerOperand();
                    }
                }

                uint64_t bytes = DL.getTypeStoreSize(val->getType());
                Value *sizeVal = ConstantInt::get(i64Ty, bytes);

                IRBuilder<> builder(I);
                Value *origPtrI8 = builder.CreateBitCast(origPtr, i8PtrTy, "store.ptr.i8");

                //split basic block
                BasicBlock *origBB = I->getParent();
                BasicBlock *contBB = origBB->splitBasicBlock(I, origBB->getName() + ".cont");

                //removing the unconditional branch added by splitBasicBlock
                origBB->getTerminator()->eraseFromParent();

                Function *Fparent = origBB->getParent();
                BasicBlock *okBB = BasicBlock::Create(Ctx, origBB->getName() + ".ok", Fparent, contBB);
                BasicBlock *errBB = BasicBlock::Create(Ctx, origBB->getName() + ".err", Fparent, contBB);
                
                // Insert check for valid address
                IRBuilder<> Borig(origBB);
                Value *isok_i32 = Borig.CreateCall(fn_isValid, {origPtrI8, sizeVal});
                Value *isValid = Borig.CreateICmpNE(isok_i32, ConstantInt::get(i32Ty, 0));
                Borig.CreateCondBr(isValid, okBB, errBB);

                // okBB -> no problem, perform original store
                IRBuilder<> Bok(okBB);
                Bok.CreateBr(contBB);

                // errBB -> handle illegal access
                IRBuilder<> Berr(errBB);
                Value *base_ptr = Berr.CreateBitCast(heapBase, i8PtrTy);
                

                //check whether base_ptr id replaced or not
                Value *updated_ptr = Berr.CreateCall(fn_get_new_base, {base_ptr}, "updated.base.ptr");

                // Call the handler to get a new valid pointer
                Value *newPtrI8 = Berr.CreateCall(fn_handle, {updated_ptr, origPtrI8, sizeVal}, "rep.ptr");

                // Update the original pointer variable
                // if their any pointer variable associated (with offset calculation)
                // first find the offset of current pointer from base
                // then apply the same offset to the new base pointer
                if (ptrVariable) {
                    // errs() << "Load Found pointer variable to update: " << *ptrVariable << "\n";
                    
                    Value *currentPtrVal = Berr.CreateLoad(i8PtrTy, ptrVariable, "current.ptr.val");
                    Value *currentPtrI8 = Berr.CreateBitCast(currentPtrVal, i8PtrTy);
                    
                    // Calculate offset: current - base
                    Value *offset = Berr.CreateCall(fn_calc_offset, {currentPtrI8, updated_ptr}, "ptr.offset");
                    Value *newBase = Berr.CreateCall(fn_get_new_base, {updated_ptr}, "new.base");
                    Value *newPtrWithOffset = Berr.CreateCall(fn_apply_offset, {newBase, offset}, "new.ptr.with.offset");
                    
                    // Store back
                    Berr.CreateStore(newPtrWithOffset, ptrVariable);
                    // errs() << "  Will update pointer variable with offset: " << *ptrVariable << "\n";
                }

                Berr.CreateBr(contBB);

                // merge in contBB
                IRBuilder<> Bcont(&*contBB->getFirstInsertionPt());
                PHINode *phiPtr = Bcont.CreatePHI(i8PtrTy, 2, "merged.ptr");
                phiPtr->addIncoming(origPtrI8, okBB);
                phiPtr->addIncoming(newPtrI8, errBB);

                // update store pointer
                Value *finalPtr = Bcont.CreateBitCast(phiPtr, origPtr->getType());
                SI->setOperand(1, finalPtr);
            }
            // Handle LoadInst
            else if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
                Value *origPtr = LI->getPointerOperand();

                // heap base
                std::set<Value*> visited;
                Value *heapBase = findHeapBase(origPtr, visited);

                if (!heapBase) {
                    // errs() << "[asan-recover] SKIPPING LOAD (no heap base found): " << *LI << "\n";
                    continue;
                }

                bool isHeap = false;
                if (auto *call = dyn_cast<CallInst>(heapBase)) {
                    if (Function *f = call->getCalledFunction()) {
                        StringRef name = f->getName();
                        if (name == "malloc" || name == "calloc" || name == "realloc" ||
                            name == "tracked_malloc") {
                            isHeap = true;
                        }
                    }
                }

                if (!isHeap) {
                    // errs() << "[asan-recover] SKIPPING LOAD (not heap malloc): " << *LI << "\n";
                    continue;
                }

                // errs() << "[asan-recover] instrumenting Load: " << *LI << "\n";
                // errs() << "  pointer: " << *origPtr << "\n";
                // errs() << "  Heap base: " << *heapBase << "\n";

                // variable that holds the heap pointer
                Value *ptrVariable = nullptr;
                if (auto *gep = dyn_cast<GetElementPtrInst>(origPtr)) {
                    Value *base = gep->getPointerOperand();
                    while (auto *innerGep = dyn_cast<GetElementPtrInst>(base)) {
                        base = innerGep->getPointerOperand();
                    }
                    if (auto *load = dyn_cast<LoadInst>(base)) {
                        ptrVariable = load->getPointerOperand();
                    }
                }

                uint64_t bytes = DL.getTypeStoreSize(LI->getType());
                Value *sizeVal = ConstantInt::get(i64Ty, bytes);

                IRBuilder<> builder(LI);
                Value *origPtrI8 = builder.CreateBitCast(origPtr, i8PtrTy, "load.ptr.i8");

                BasicBlock *origBB = LI->getParent();
                BasicBlock *contBB = origBB->splitBasicBlock(LI, origBB->getName() + ".load.cont");
                origBB->getTerminator()->eraseFromParent();

                Function *Fparent = origBB->getParent();
                BasicBlock *okBB = BasicBlock::Create(Ctx, origBB->getName() + ".load.ok", Fparent, contBB);
                BasicBlock *errBB = BasicBlock::Create(Ctx, origBB->getName() + ".load.err", Fparent, contBB);
                
                // Insert check for valid address
                IRBuilder<> Borig(origBB);
                Value *isok_i32 = Borig.CreateCall(fn_isValid, {origPtrI8, sizeVal});
                Value *isValid = Borig.CreateICmpNE(isok_i32, ConstantInt::get(i32Ty, 0));
                Borig.CreateCondBr(isValid, okBB, errBB);

                // okBB -> no problem
                IRBuilder<> Bok(okBB);
                Bok.CreateBr(contBB);

                // errBB -> handle illegal access
                IRBuilder<> Berr(errBB);
                Value *base_ptr = Berr.CreateBitCast(heapBase, i8PtrTy);
                
                //check whether base_ptr id replaced or not
                // if replaced, use the updated base pointer
                Value *updated_ptr = Berr.CreateCall(fn_get_new_base, {base_ptr}, "updated.base.ptr");
                Value *newPtrI8 = Berr.CreateCall(fn_handle, {updated_ptr, origPtrI8, sizeVal}, "rep.ptr.load");

                // Update the base pointer variable if found
                // if their any pointer variable associated (with offset calculation)
                // first find the offset of current pointer from base
                // then apply the same offset to the new base pointer
                if (ptrVariable) {
                    // errs() << "Store Found pointer variable to update: " << *ptrVariable << "\n";
                    Value *currentPtrVal = Berr.CreateLoad(i8PtrTy, ptrVariable, "current.ptr.val");
                    Value *currentPtrI8 = Berr.CreateBitCast(currentPtrVal, i8PtrTy);
                    
                    //Calculate offset: current - base
                    Value *offset = Berr.CreateCall(fn_calc_offset, {currentPtrI8, updated_ptr}, "ptr.offset");
                    Value *newBase = Berr.CreateCall(fn_get_new_base, {updated_ptr}, "new.base");
                    Value *newPtrWithOffset = Berr.CreateCall(fn_apply_offset, {newBase, offset}, "new.ptr.with.offset");

                    // Store back
                    Berr.CreateStore(newPtrWithOffset, ptrVariable);
                    
                    // errs() << "  Will update pointer variable with offset: " << *ptrVariable << "\n";
                }
                
                Berr.CreateBr(contBB);

                // merge in contBB
                IRBuilder<> Bcont(&*contBB->getFirstInsertionPt());
                PHINode *phiPtr = Bcont.CreatePHI(i8PtrTy, 2, "merged.ptr.load");
                phiPtr->addIncoming(origPtrI8, okBB);
                phiPtr->addIncoming(newPtrI8, errBB);

                // update load pointer
                Value *finalPtr = Bcont.CreateBitCast(phiPtr, origPtr->getType());
                LI->setOperand(0, finalPtr);
            }
        }

        return PreservedAnalyses::none();
    }
};

} 

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "ASanRecoverPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM, ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "asan-pass") {
                        MPM.addPass(ReallocPass());
                        return true;
                    }
                    return false;
                }
            );
        }
    };
}