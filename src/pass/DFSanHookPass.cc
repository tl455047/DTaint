
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

class DFSanHookPass: public ModulePass, public InstVisitor<DFSanHookPass> {
    
    
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
        
        DFSanHookPass(): ModulePass(ID) { 

        }

        StringRef getPassName() const override {
            return StringRef("DFSanHookPass");
        }

        bool doInitialization(Module &M) override;
        bool runOnModule(Module &M) override;
       
        void visitLoadInst(LoadInst &LI);
        void visitStoreInst(StoreInst &SI);
        void visitMemSetInst(MemSetInst &I);
        void visitCmpInst(CmpInst &CI);
        void visitBinaryOperator(BinaryOperator &BO);
        void visitCallBase(CallBase &CB);
        
};

}

char DFSanHookPass::ID = 0;
bool DFSanHookPass::doInitialization(Module &M) {

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

bool DFSanHookPass::runOnModule(Module &M) {

    
    /**
     * Seems that object FunctionCallee will be released after doInitialization,
     * insert the function in runOnModule may be the better choice.
     */
    /**
     * Replace argc with global variable.
     */
    /*GlobalVariable *Pipe_argc =
      new GlobalVariable(M, Type::getInt32Ty(M.getContext()), false,
                         GlobalValue::ExternalLinkage, ConstantInt::get(Type::getInt32Ty(M.getContext()), 10000), "__pipe_argc");
    for (Function &F : M) {
    
        if(F.getName() == "main") {
            errs() << F.getName()<<"\n"; 
            auto &BB = F.getEntryBlock();
            BasicBlock::iterator IP = BB.getFirstInsertionPt();
            IRBuilder<> IRB(&(*IP));
            Value *Argc = IRB.CreateLoad(Type::getInt32Ty(M.getContext()), Pipe_argc);
            F.getArg(0)->replaceAllUsesWith(Argc);
        }
        
    }*/

    return true;
}


void DFSanHookPass::visitLoadInst(LoadInst &LI) {

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

void DFSanHookPass::visitStoreInst(StoreInst &SI) {
    
    //errs() << "visit store inst \n"; 
    
    size_t Bytes = TaintDataLayout->getTypeStoreSize(SI.getValueOperand()->getType());
    // check access bytes
    if (Bytes == 0) 
       return;
    
    IRBuilder <> IRB(&SI);
    //IRB.CreateCall(DebugFunc, {});
}


void DFSanHookPass::visitBinaryOperator(BinaryOperator &BO) {

}

void DFSanHookPass::visitMemSetInst(MemSetInst &I) {
    //errs() << "visit memset inst \n";
}

void DFSanHookPass::visitCmpInst(CmpInst &CI) {
    //errs() << "visit cmp inst \n"; 
   
}

void DFSanHookPass::visitCallBase(CallBase &CB) {
}

static RegisterPass<DFSanHookPass> X("DFSan hook", "DFSanHookPass", false, false);

static void registerDFSanHook(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {

  PM.add(new DFSanHookPass());
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
                   registerDFSanHook);

static RegisterStandardPasses
    RegisterTaint0(PassManagerBuilder::EP_EnabledOnOptLevel0,
                   registerDFSanHook);
