
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;

namespace {

class TaintPass: public ModulePass, public InstVisitor<TaintPass> {
    
    
    const DataLayout *TaintDataLayout;
    //Taint Visitor;
    //std::unique_ptr<IRBuilder<>> Builder;
    FunctionType *DebugFuncTy;
    FunctionType *LoadFuncCallbackTy;
    FunctionType *StoreFuncCallbackTy;
    FunctionType *MemTransferCallbackTy;

    FunctionCallee DebugFunc;
    FunctionCallee LoadFuncCallback;
    FunctionCallee StoreFuncCallback;
    FunctionCallee MemTransferCallback;
    public:
        static char ID;
        
        TaintPass(): ModulePass(ID) { 

        }

        StringRef getPassName() const override {
            return StringRef("TaintPass");
        }

        bool doInitialization(Module &M) override;
        bool runOnModule(Module &M) override;
       
        void visitLoadInst(LoadInst &LI);
        void visitStoreInst(StoreInst &SI);
        void visitMemSetInst(MemSetInst &I);
        void visitCmpInst(CmpInst &CI);
        void visitBinaryOperator(BinaryOperator &BO);
       
        
};

}

char TaintPass::ID = 0;
bool TaintPass::doInitialization(Module &M) {

    errs() << getPassName() << " init\n";

    TaintDataLayout = &M.getDataLayout();
    // get type
    /*Type *Int8PtrTy = Type::getInt8PtrTy(M.getContext());
    Type *Int32PtrTy = Type::getInt32PtrTy(M.getContext());
    Type *Int32Ty = Type::getInt32Ty(M.getContext());
    Type *Int64Ty = Type::getInt64Ty(M.getContext());
    
    SizeTy = IntegerType::getInt64Ty(M.getContext());
    VoidPtrTy = Type::getInt8PtrTy(M.getContext());
    ShadowTy = IntegerType::getInt8Ty(M.getContext());*/

    /*Type *DFSanSetFuncArgTy[3] = {Int32Ty, Int8PtrTy, SizeTy};
    DFSanSetFuncTy = FunctionType::get(VoidTy, DFSanSetFuncArgTy, false);*/

    Type *VoidTy = Type::getVoidTy(M.getContext());
    Type *Int16Ty = Type::getInt16Ty(M.getContext());
    Type *Int16PtrTy = Type::getInt16PtrTy(M.getContext());
    Type *SizetTy= Type::getInt64Ty(M.getContext());

    DebugFuncTy = FunctionType::get(VoidTy, false);
    LoadFuncCallbackTy = FunctionType::get(VoidTy, {Int16Ty}, false);
    StoreFuncCallbackTy = FunctionType::get(VoidTy, {Int16Ty}, false);
    MemTransferCallbackTy = FunctionType::get(VoidTy, {Int16PtrTy, SizetTy}, false);
    
    //Builder = std::make_unique<IRBuilder<>>(M.getContext());
    return true;
}

bool TaintPass::runOnModule(Module &M) {

    
    /**
     * Seems that object FunctionCallee will be released after doInitialization,
     * insert the function in runOnModule may be the better choice.
     */
    // getorinsertfunction
    DebugFunc = M.getOrInsertFunction("__dfsw_debug_func", DebugFuncTy);
    /*LoadFuncCallback = M.getOrInsertFunction("__taint_load_callback", LoadFuncCallbackTy);
    StoreFuncCallback = M.getOrInserFunction("__taint_store_callback", StoreFuncCallbackTy);
    MemTransferCallback = M.getOrInsertFunction("__taint_mem_transfer_callback", MemTransferCallbackTy);*/
    for (Function &i : M) {
        if(i.isIntrinsic())
            errs() << i.getName() << "\n";
        if(!i.isIntrinsic() && &i != DebugFunc.getCallee()->stripPointerCasts() ) {
            //errs() << i.getName()<<"\n";
            visit(i);
        }
    }

    return true;
}


void TaintPass::visitLoadInst(LoadInst &LI) {

    //errs() << "visit load inst \n"; 
   
    size_t Bytes = TaintDataLayout->getTypeStoreSize(LI.getType());
    // check access bytes
    if (Bytes == 0) 
        return;
    
    //Builder->SetInsertPoint(&LI);
    //Builder->CreateCall(DebugFunc, {});
    IRBuilder <> IRB(&LI);
    //IRB.CreateCall(LoadFuncCallbackTy, {});
    //IRB.CreateCall(DebugFunc, {});
    
}

void TaintPass::visitStoreInst(StoreInst &SI) {
    
    //errs() << "visit store inst \n"; 
    
    size_t Bytes = TaintDataLayout->getTypeStoreSize(SI.getValueOperand()->getType());
    // check access bytes
    if (Bytes == 0) 
       return;
    
    IRBuilder <> IRB(&SI);
    //IRB.CreateCall(DebugFunc, {});
}


void TaintPass::visitBinaryOperator(BinaryOperator &BO) {

}

void TaintPass::visitMemSetInst(MemSetInst &I) {
    //errs() << "visit memset inst \n";
}

void TaintPass::visitCmpInst(CmpInst &CI) {
    //errs() << "visit cmp inst \n"; 
   
}

static RegisterPass<TaintPass> X("taint", "TaintPass", false, false);

static void registerTaint(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {

  PM.add(new TaintPass());
}

/**
 * enum ExtensionPointTy {
 *      EP_EarlyAsPossible, EP_ModuleOptimizerEarly, EP_LoopOptimizerEnd, EP_ScalarOptimizerLate,
 *      EP_OptimizerLast, EP_VectorizerStart, EP_EnabledOnOptLevel0, EP_Peephole,
 *      EP_LateLoopOptimizations, EP_CGSCCOptimizerLate, EP_FullLinkTimeOptimizationEarly, EP_FullLinkTimeOptimizationLast
 * } 
 */

/**
 * Bind to optimization level 0, is it a good choise ?
 */
static RegisterStandardPasses
    RegisterTaint(PassManagerBuilder::EP_OptimizerLast,
                   registerTaint);

static RegisterStandardPasses
    RegisterTaint0(PassManagerBuilder::EP_EnabledOnOptLevel0,
                   registerTaint);
