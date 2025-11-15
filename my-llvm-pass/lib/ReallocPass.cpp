
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
        errs() << "[asan-recover] running on module: " << M.getName() << "\n";

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
                errs() << "[asan-recover] Replaced malloc with tracked_malloc\n";
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

        // Collect instructions => illegal accesses
        std::vector<Instruction*> worklist;
        for (Function &F : M) {
            if (F.isDeclaration()) continue;
            for (BasicBlock &BB : F)
                for (Instruction &I : BB)
                    if (isa<StoreInst>(&I))  
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
                    errs() << "[asan-recover] SKIPPING (no heap base found): " << *SI << "\n";
                    continue;
                }

                errs() << "[asan-recover] instrumenting Store: " << *SI << "\n";
                errs() << "value: " << *val << "\n";
                errs() << "pointer: " << *origPtr << "\n";
                errs() << "Heap base: " << *heapBase << "\n";

                // Find the variable that holds the heap pointer (for updating after realloc)
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

                uint64_t bytes = DL.getTypeStoreSize(val->getType());
                Value *sizeVal = ConstantInt::get(i64Ty, bytes);

                IRBuilder<> builder(I);
                Value *origPtrI8 = builder.CreateBitCast(origPtr, i8PtrTy, "store.ptr.i8");

                BasicBlock *origBB = I->getParent();
                BasicBlock *contBB = origBB->splitBasicBlock(I, origBB->getName() + ".cont");
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
                Value *newPtrI8 = Berr.CreateCall(fn_handle, {base_ptr, origPtrI8, sizeVal}, "rep.ptr");
                
                // Update the original pointer variable 
                if (ptrVariable) {

                    Value *newBaseTyped = Berr.CreateCall(fn_get_new_base, {base_ptr});
                    Berr.CreateStore(newBaseTyped, ptrVariable);

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