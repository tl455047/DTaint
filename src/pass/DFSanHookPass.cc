
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SpecialCaseList.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

using namespace llvm;


static cl::opt<bool> ClHookInst(
    "memlog-hook-inst",
    cl::desc("Insert calls to hook critical memory instructions, calls."),
    cl::Hidden, cl::init(false));

// for hook API list
static cl::list<std::string> ClABIListFiles(
    "memlog-hook-abilist",
    cl::desc("File listing native ABI functions and how the pass hooks them"),
    cl::Hidden);


static StringRef GetGlobalTypeString(const GlobalValue &G) {
  // Types of GlobalVariables are always pointer types.
  Type *GType = G.getValueType();
  // For now we support excluding struct types only.
  if (StructType *SGType = dyn_cast<StructType>(GType)) {
    if (!SGType->isLiteral())
      return SGType->getName();
  }
  return "<unknown type>";
}

namespace {


class HookABIList {
  std::unique_ptr<SpecialCaseList> SCL;

 public:
  HookABIList() = default;

  void set(std::unique_ptr<SpecialCaseList> List) { SCL = std::move(List); }

  /// Returns whether either this function or its source file are listed in the
  /// given category.
  bool isIn(const Function &F, StringRef Category) const {
    return isIn(*F.getParent(), Category) ||
           SCL->inSection("hook", "fun", F.getName(), Category);
  }

  /// Returns whether this global alias is listed in the given category.
  ///
  /// If GA aliases a function, the alias's name is matched as a function name
  /// would be.  Similarly, aliases of globals are matched like globals.
  bool isIn(const GlobalAlias &GA, StringRef Category) const {
    if (isIn(*GA.getParent(), Category))
      return true;

    if (isa<FunctionType>(GA.getValueType()))
      return SCL->inSection("hook", "fun", GA.getName(), Category);

    return SCL->inSection("hook", "global", GA.getName(), Category) ||
           SCL->inSection("hook", "type", GetGlobalTypeString(GA),
                          Category);
  }

  /// Returns whether this module is listed in the given category.
  bool isIn(const Module &M, StringRef Category) const {
    return SCL->inSection("hook", "src", M.getModuleIdentifier(), Category);
  }
};

class DFSanHookPass: public ModulePass, public InstVisitor<DFSanHookPass> {
    
    
    const DataLayout *TaintDataLayout;
    
    HookABIList __HookABIList; 

    Type *LabelTy;
    Type *Int64PtrTy;
    Type *SizeTy;
    Type *Int32Ty;

    FunctionType *DebugFuncTy;
    FunctionType *MemlogHook1FuncTy;
    FunctionType *MemlogHook2FuncTy;
    FunctionType *MemlogHook38FuncTy;
    FunctionType *MemlogHook4FuncTy;
    FunctionType *MemlogHook56FuncTy;
    //FunctionType *MemlogHook6FuncTy;
    FunctionType *MemlogHook7FuncTy;
    FunctionType *MemlogHook8FuncTy;

    FunctionCallee DebugFunc;
    FunctionCallee MemlogHook1Func;
    FunctionCallee MemlogHook2Func;
    FunctionCallee MemlogHook3Func;
    FunctionCallee MemlogHook4Func;
    FunctionCallee MemlogHook5Func;
    FunctionCallee MemlogHook6Func;
    FunctionCallee MemlogHook7Func;
    FunctionCallee MemlogHook8Func;

    public:
        static char ID;
        
        DFSanHookPass(): ModulePass(ID) { 
            __HookABIList.set(
            SpecialCaseList::createOrDie(ClABIListFiles, *vfs::getRealFileSystem()));
        }

        StringRef getPassName() const override {
            return StringRef("DFSanHookPass");
        }

        bool doInitialization(Module &M) override;
        bool runOnModule(Module &M) override;

        bool shouldHook(const Function *F);

        void visitLoadInst(LoadInst &LI);
        void visitStoreInst(StoreInst &SI);
        void visitCallBase(CallBase &CB);
        //void visitCallInst(CallInst &I);
        void visitInvokeInst(InvokeInst &I);
        void visitGetElementPtrInst(GetElementPtrInst &I);
     
        void visitMemSetInst(MemSetInst &I);
        void visitMemCpyInst(MemCpyInst &I);
        void visitMemCpyInlineInst(MemCpyInlineInst &I);
        void visitMemMoveInst(MemMoveInst &I);
        //void visitMemTransferInst(MemTransferInst &I);

        void visitAllocaInst(AllocaInst &I);
        void visitExtractElementInst(ExtractElementInst &I);
        void visitInsertElementInst(InsertElementInst &I);  
        void visitExtractValueInst(ExtractValueInst &I);
        void visitInsertValueInst(InsertValueInst &I);
        void visitShuffleVectorInst(ShuffleVectorInst &I);

        //void visitAtomicRMWInst(AtomicRMWInst &I);
        //void visitSelectInst(SelectInst &I);
        //void visitPHINode(PHINode &PN);
     

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
    LabelTy = Type::getInt32Ty(M.getContext());
    Int64PtrTy = Type::getInt64PtrTy(M.getContext());
    SizeTy = Type::getInt64Ty(M.getContext());
    Int32Ty = Type::getInt32Ty(M.getContext());
    Type *VoidTy = Type::getVoidTy(M.getContext());

    DebugFuncTy = FunctionType::get(VoidTy, {LabelTy, SizeTy}, false);
    MemlogHook1FuncTy = FunctionType::get(VoidTy, {SizeTy}, false);
    MemlogHook2FuncTy = FunctionType::get(VoidTy, {Int64PtrTy}, false);
    MemlogHook38FuncTy = FunctionType::get(VoidTy, {Int64PtrTy, SizeTy}, false);
    MemlogHook4FuncTy = FunctionType::get(VoidTy, {Int64PtrTy, Int64PtrTy, SizeTy}, false);
    MemlogHook56FuncTy = FunctionType::get(VoidTy, {Int64PtrTy, SizeTy, SizeTy}, false);
    //MemlogHook6FuncTy = FunctionType::get(VoidTy, {Int64PtrTy, SizeTy, SizeTy}, false);
    MemlogHook7FuncTy = FunctionType::get(VoidTy, {SizeTy, SizeTy}, false);
    //Builder = std::make_unique<IRBuilder<>>(M.getContext());
    return true;
}

bool DFSanHookPass::runOnModule(Module &M) {

    
    /**
     * Seems that object FunctionCallee will be released after doInitialization,
     * insert the function in runOnModule may be the better choice.
     */
    DebugFunc = M.getOrInsertFunction("__hook_debug", DebugFuncTy);
    MemlogHook1Func = M.getOrInsertFunction("__memlog_hook1", MemlogHook1FuncTy);
    MemlogHook2Func = M.getOrInsertFunction("__memlog_hook2", MemlogHook2FuncTy);
    MemlogHook3Func = M.getOrInsertFunction("__memlog_hook3", MemlogHook38FuncTy);
    MemlogHook4Func = M.getOrInsertFunction("__memlog_hook4", MemlogHook4FuncTy);
    MemlogHook5Func = M.getOrInsertFunction("__memlog_hook5", MemlogHook56FuncTy);
    MemlogHook6Func = M.getOrInsertFunction("__memlog_hook6", MemlogHook56FuncTy);
    MemlogHook7Func = M.getOrInsertFunction("__memlog_hook7", MemlogHook7FuncTy);
    MemlogHook8Func = M.getOrInsertFunction("__memlog_hook8", MemlogHook38FuncTy);
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
    for (Function &F : M) {
    
      
           
        visit(F);   
        
        
    }

    return true;
}


bool DFSanHookPass::shouldHook(const Function *F) {
  return __HookABIList.isIn(*F, "hook");
}

void DFSanHookPass::visitLoadInst(LoadInst &LI) {

    size_t Bytes = TaintDataLayout->getTypeStoreSize(LI.getType());
    // check access bytes
    if (!ClHookInst || Bytes == 0) 
        return;
    
    IRBuilder <> IRB(&LI);
    IRB.CreateCall(MemlogHook3Func, {LI.getPointerOperand(), 
      ConstantInt::get(SizeTy, Bytes)});
    
}

void DFSanHookPass::visitStoreInst(StoreInst &SI) {
    
    size_t Bytes = TaintDataLayout->getTypeStoreSize(SI.getValueOperand()->getType());
   
    if (!ClHookInst) return;
    
    IRBuilder <> IRB(&SI);
    IRB.CreateCall(MemlogHook6Func, {SI.getPointerOperand(), 
      SI.getValueOperand(), ConstantInt::get(SizeTy, Bytes)});
  
}

void DFSanHookPass::visitCallBase(CallBase &CB) {
    auto F = CB.getCalledFunction();
    
    if(ClHookInst && F && !F->isIntrinsic() && shouldHook(F)) {
        
        errs() << "visit Call Base inst "<< F->getName() << "\n"; 
        //if(F->getName() == "malloc") {

            //Value *Arg = CB.getArgOperand(0);
            //errs() << "visit Call Base inst "<< F->getName()  << "\n"; 
            //IRBuilder <> IRB(&CB);
            //IRB.SetInsertPoint(&CB);
            //IRB.CreateCall(DebugFunc, {Arg});
        //}
    }

}
/*void DFSanHookPass::visitCallInst(CallInst &I) {
    auto F = I.getCalledFunction();
    
    errs() << "visit Call inst "<< F->getName()  << "\n"; 
}*/

void DFSanHookPass::visitInvokeInst(InvokeInst &I) {
    errs() << "visitInvokeInst\n";

    if (!ClHookInst) return;

}

void DFSanHookPass::visitGetElementPtrInst(GetElementPtrInst &I) {
    
    size_t Bytes = TaintDataLayout->getTypeStoreSize(I.getSourceElementType());
    
    if (!ClHookInst) return;
    
    // get index
    Value *Idx;
    for (User::op_iterator it = I.idx_begin(); it != I.idx_end(); it++) {
        Idx = *it;
    }

    IRBuilder <> IRB(&I);
    IRB.CreateCall(MemlogHook5Func, {I.getPointerOperand(), Idx, 
      ConstantInt::get(SizeTy, Bytes)});
    
}

void DFSanHookPass::visitMemSetInst(MemSetInst &I) {
   
    if (!ClHookInst) return;
    IRBuilder <> IRB(&I);
    IRB.CreateCall(MemlogHook6Func, {I.getArgOperand(0),
      I.getArgOperand(1), I.getArgOperand(2)});

}

void DFSanHookPass::visitMemCpyInst(MemCpyInst &I) {
    
    if (!ClHookInst) return;
    IRBuilder <> IRB(&I);
    IRB.CreateCall(MemlogHook4Func, {I.getArgOperand(0),
      I.getArgOperand(1), I.getArgOperand(2)});

}

void DFSanHookPass::visitMemCpyInlineInst(MemCpyInlineInst &I) {

    if (!ClHookInst) return;
    IRBuilder <> IRB(&I);
    IRB.CreateCall(MemlogHook4Func, {I.getArgOperand(0),
      I.getArgOperand(1), I.getArgOperand(2)}); 

}

void DFSanHookPass::visitMemMoveInst(MemMoveInst &I) {
    
    if (!ClHookInst) return;
    IRBuilder <> IRB(&I);
    IRB.CreateCall(MemlogHook4Func, {I.getArgOperand(0),
      I.getArgOperand(1), I.getArgOperand(2)});

}

// Use for vector 

void DFSanHookPass::visitAllocaInst(AllocaInst &I) {
    errs() << "visitAllocaInst\n"; 

    if (!ClHookInst) return;

    size_t Bytes = TaintDataLayout->getTypeStoreSize(I.getAllocatedType());

    //IRBuilder <> IRB(&I);
    //IRB.CreateCall(MemlogHook7Func, {ConstantInt::get(SizeTy, Bytes),
    //  I.getArraySize()});
}

void DFSanHookPass::visitExtractElementInst(ExtractElementInst &I) {
    errs() << "visitExtractElementInst\n";  

    if (!ClHookInst) return;
    
    //IRBuilder <> IRB(&I);
    //IRB.CreateCall(MemlogHook8Func, {I.getVectorOperand(),
    //  I.getIndexOperand()});
}

void DFSanHookPass::visitInsertElementInst(InsertElementInst &I) {
    errs() << "visitInsertElementInst\n"; 

    if (!ClHookInst) return;

}

void DFSanHookPass::visitExtractValueInst(ExtractValueInst &I) {
    errs() << "visitExtractValueInst\n"; 

    if (!ClHookInst) return;

}

void DFSanHookPass::visitInsertValueInst(InsertValueInst &I) {
    errs() << "visitInsertValueInst\n"; 

    if (!ClHookInst) return;

}

void DFSanHookPass::visitShuffleVectorInst(ShuffleVectorInst &I) {
    errs() << "visitShuffleVectorInst\n";
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
