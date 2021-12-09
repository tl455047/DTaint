
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
#include "llvm/Transforms/Utils/Local.h"
#include <fstream>

using namespace llvm;

static cl::opt<bool> ClHookInst(
    "memlog-hook-inst",
    cl::desc("Insert calls to hook critical memory instructions, calls."),
    cl::Hidden, cl::init(true));

// for hook API list
static cl::list<std::string> ClHookABIListFiles(
    "memlog-hook-abilist",
    cl::desc("File listing native ABI functions and how the pass hooks them."),
    cl::Hidden);

static cl::list<std::string> ClABIListFiles(
    "memlog-dfsan-abilist",
    cl::desc("File listing native ABI functions and how the pass treats them"),
    cl::Hidden);
// for debug
static cl::opt<bool> ClDebug(
    "memlog-debug",
    cl::desc("Output debug message."),
    cl::Hidden, cl::init(false));

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

class DFSanABIList {
  std::unique_ptr<SpecialCaseList> SCL;

 public:
  DFSanABIList() = default;

  void set(std::unique_ptr<SpecialCaseList> List) { SCL = std::move(List); }

  /// Returns whether either this function or its source file are listed in the
  /// given category.
  bool isIn(const Function &F, StringRef Category) const {
    return isIn(*F.getParent(), Category) ||
           SCL->inSection("dataflow", "fun", F.getName(), Category);
  }

  /// Returns whether this global alias is listed in the given category.
  ///
  /// If GA aliases a function, the alias's name is matched as a function name
  /// would be.  Similarly, aliases of globals are matched like globals.
  bool isIn(const GlobalAlias &GA, StringRef Category) const {
    if (isIn(*GA.getParent(), Category))
      return true;

    if (isa<FunctionType>(GA.getValueType()))
      return SCL->inSection("dataflow", "fun", GA.getName(), Category);

    return SCL->inSection("dataflow", "global", GA.getName(), Category) ||
           SCL->inSection("dataflow", "type", GetGlobalTypeString(GA),
                          Category);
  }

  /// Returns whether this module is listed in the given category.
  bool isIn(const Module &M, StringRef Category) const {
    return SCL->inSection("dataflow", "src", M.getModuleIdentifier(), Category);
  }
};

class MemlogPass: public ModulePass, public InstVisitor<MemlogPass> {

    // Decide which type of hook functions the call belongs to. 
    enum HookType {
        
        HT_UNKNOWN,
        // __memlog_hook1 (unsigned id, void* ptr, unsigned src_type, unsigned rst_type);
        // ex. load inst.
        HT_HOOK1,
        // __memlog_hook2 (unsigned id, size_t value, unsigned src_type, void* ptr, unsigned rst_type);
        // ex. store inst.
        HT_HOOK2,
        // __memlog_hook2_int128 (unsigned id, uint128_t value, unsigned src_type, void* ptr, unsigned rst_type);
        // deal with 16byte float point value
        HT_HOOK2_INT128,
        // __memlog_hook3 (unsigned id, void* ptr, int c, size_t size);
        // ex. memset
        HT_HOOK3,
        // __memlog_hook4 (unsigned id, void* dst, void* src, size_t size);
        // ex. memcpy
        HT_HOOK4,
        // __memlog_hook5 (unsigned id, size_t size);
        // ex. malloc
        HT_HOOK5,
        // __memlog_hook6 (unsigned id, void* ptr);
        // ex. free
        HT_HOOK6,
        // __memlog_hook7 (unsigned id, void* ptr, size_t size);
        // ex. realloc
        HT_HOOK7,
        // __memlog_va_arg_hook1 (unsigned id, void* ptr, unsigned src_type, 
        // unsigned rst_type, unsigned num_of_idx, ...);
        // ex. get_element_ptr
        HT_VARARG_HOOK1
        
    };

    /// How should calls to uninstrumented functions be handled?
    enum WrapperKind {
        /// This function is present in an uninstrumented form but we don't know
        /// how it should be handled.  Print a warning and call the function anyway.
        /// Don't label the return value.
        WK_Warning,

        /// This function does not write to (user-accessible) memory, and its return
        /// value is unlabelled.
        WK_Discard,

        /// This function does not write to (user-accessible) memory, and the label
        /// of its return value is the union of the label of its arguments.
        WK_Functional,

        /// Instead of calling the function, a custom wrapper __dfsw_F is called,
        /// where F is the name of the function.  This function may wrap the
        /// original function or provide its own implementation.  This is similar to
        /// the IA_Args ABI, except that IA_Args uses a struct return type to
        /// pass the return value shadow in a register, while WK_Custom uses an
        /// extra pointer argument to return the shadow.  This allows the wrapped
        /// form of the function type to be expressed in C.
        WK_Custom
    };
    const DataLayout *TaintDataLayout;
    
    HookABIList __HookABIList; 
    DFSanABIList ABIList;

    Type *Int64PtrTy;
    Type *Int8Ty;
    Type *Int32Ty;
    Type *Int128Ty;
    Type *FP128Ty;
    Type *SizeTy;
    
    FunctionType *MemlogHookDebugFuncTy;
    FunctionType *MemlogHook1FuncTy;
    FunctionType *MemlogHook2FuncTy;
    FunctionType *MemlogHook2Int128FuncTy;
    FunctionType *MemlogHook3FuncTy;
    FunctionType *MemlogHook4FuncTy;
    FunctionType *MemlogHook5FuncTy;
    FunctionType *MemlogHook6FuncTy;
    FunctionType *MemlogHook7FuncTy;
    FunctionType *MemlogVaArgHook1FuncTy;

    FunctionCallee MemlogHookDebugFunc;
    FunctionCallee MemlogHook1Func;
    FunctionCallee MemlogHook2Func;
    FunctionCallee MemlogHook2Int128Func;
    FunctionCallee MemlogHook3Func;
    FunctionCallee MemlogHook4Func;
    FunctionCallee MemlogHook5Func;
    FunctionCallee MemlogHook6Func;
    FunctionCallee MemlogHook7Func;
    FunctionCallee MemlogVaArgHook1Func;

    static unsigned HookID;
    static const std::string HookIDFileName;
    static const unsigned int MemlogMapW;
    public:
        static char ID;
        
        MemlogPass(): ModulePass(ID) { 
            
            ABIList.set(
            SpecialCaseList::createOrDie(ClABIListFiles, *vfs::getRealFileSystem()));
            
            __HookABIList.set(
            SpecialCaseList::createOrDie(ClHookABIListFiles, *vfs::getRealFileSystem()));

            std::fstream InFile;
            InFile.open(HookIDFileName, std::ios::in | std::ios::out);
            if (InFile.eof())
                HookID = 0;
            else 
                InFile >> HookID;
            InFile.close();

        }

        StringRef getPassName() const override {
            return StringRef("MemlogPass");
        }

        bool doInitialization(Module &M) override;
        bool runOnModule(Module &M) override;
        
        WrapperKind getWrapperKind(Function *F);
        bool isInstrumented(const Function *F);
        bool isInstrumented(const GlobalAlias *GA);
        bool shouldHook(const Function *F);
        HookType getHookType(const Function *F);
        void whichType(Type *T);
        // visitor override
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

        void visitAtomicCmpXchgInst(AtomicCmpXchgInst &I);
        void visitAtomicRMWInst(AtomicRMWInst &I);
        //void visitAtomicRMWInst(AtomicRMWInst &I);
        //void visitSelectInst(SelectInst &I);
        //void visitPHINode(PHINode &PN);
     

};

}

unsigned MemlogPass::HookID = 0;
const std::string MemlogPass::HookIDFileName = "/tmp/.MemlogHookID.txt";
const unsigned int MemlogPass::MemlogMapW = 65536 * 2;

char MemlogPass::ID = 0;

bool MemlogPass::doInitialization(Module &M) {

    errs() << getPassName() << " " << HookID << " init\n";

    TaintDataLayout = &M.getDataLayout();
    // get type
    Int64PtrTy = Type::getInt64PtrTy(M.getContext());
    Int8Ty = Type::getInt8Ty(M.getContext());
    Int32Ty = Type::getInt32Ty(M.getContext());
    Int128Ty = Type::getInt128Ty(M.getContext());
    SizeTy = Type::getInt64Ty(M.getContext());
    
    Type *VoidTy = Type::getVoidTy(M.getContext());
    FP128Ty = Type::getDoubleTy(M.getContext());
    MemlogHookDebugFuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy, SizeTy, FP128Ty}, false);
    MemlogHook1FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy, Int32Ty, Int32Ty}, false);
    MemlogHook2FuncTy = FunctionType::get(VoidTy, {Int32Ty, SizeTy, Int32Ty, Int64PtrTy, Int32Ty}, false);
    MemlogHook2Int128FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int128Ty, Int32Ty, Int64PtrTy, Int32Ty}, false);
    MemlogHook3FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy, Int32Ty, SizeTy}, false);
    MemlogHook4FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy, Int64PtrTy, SizeTy}, false);
    MemlogHook5FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int32Ty, SizeTy}, false);
    MemlogHook6FuncTy = FunctionType::get(VoidTy, {Int32Ty, SizeTy}, false);
    MemlogHook7FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy}, false);
    MemlogVaArgHook1FuncTy = FunctionType::get(VoidTy, {Int32Ty, Int64PtrTy, Int32Ty, Int32Ty, Int32Ty}, true);
    
    return true;
}

bool MemlogPass::runOnModule(Module &M) {

    
    /**
     * Seems that object FunctionCallee will be released after doInitialization,
     * insert the function in runOnModule may be the better choice.
     */
    MemlogHookDebugFunc = M.getOrInsertFunction("__memlog_hook_debug", MemlogHookDebugFuncTy);
    MemlogHook1Func = M.getOrInsertFunction("__memlog_hook1", MemlogHook1FuncTy);
    MemlogHook2Func = M.getOrInsertFunction("__memlog_hook2", MemlogHook2FuncTy);
    MemlogHook2Int128Func = M.getOrInsertFunction("__memlog_hook2_int128", MemlogHook2Int128FuncTy);
    MemlogHook3Func = M.getOrInsertFunction("__memlog_hook3", MemlogHook3FuncTy);
    MemlogHook4Func = M.getOrInsertFunction("__memlog_hook4", MemlogHook4FuncTy);
    MemlogHook5Func = M.getOrInsertFunction("__memlog_hook5", MemlogHook5FuncTy);
    MemlogHook6Func = M.getOrInsertFunction("__memlog_hook6", MemlogHook6FuncTy);
    MemlogHook7Func = M.getOrInsertFunction("__memlog_hook7", MemlogHook7FuncTy);
    MemlogVaArgHook1Func = M.getOrInsertFunction("__memlog_va_arg_hook1", MemlogVaArgHook1FuncTy);
   
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

    std::vector<Function *> FnsToInstrument;
    for (Function &F : M) {
    
        if (!F.isIntrinsic() && 
                &F != MemlogHookDebugFunc.getCallee()->stripPointerCasts() &&
                &F != MemlogHook1Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook2Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook2Int128Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook3Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook4Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook5Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook6Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook7Func.getCallee()->stripPointerCasts() &&
                &F != MemlogVaArgHook1Func.getCallee()->stripPointerCasts()) {
            //errs() << F.getName() << "\n";
            //visit(F);
            FnsToInstrument.push_back(&F);
        }
        
    }
    
    // Give function aliases prefixes when necessary, and build wrappers where the
    // instrumentedness is inconsistent.
    for (Module::alias_iterator i = M.alias_begin(), e = M.alias_end(); i != e;) {
        GlobalAlias *GA = &*i;
        ++i;
        // Don't stop on weak.  We assume people aren't playing games with the
        // instrumentedness of overridden weak aliases.
        if (auto F = dyn_cast<Function>(GA->getBaseObject())) {
            bool GAInst = isInstrumented(GA), FInst = isInstrumented(F);
            if (GAInst && FInst) {
            } else if (GAInst != FInst) {
            }
        }
    }
    // First, change the ABI of every function in the module.  ABI-listed
    // functions keep their original ABI and get a wrapper function.
    for (std::vector<Function *>::iterator i = FnsToInstrument.begin(),
                                            e = FnsToInstrument.end();
        i != e; ++i) {
        Function &F = **i;
        FunctionType *FT = F.getFunctionType();
        
        bool IsZeroArgsVoidRet = (FT->getNumParams() == 0 && !FT->isVarArg() &&
                                FT->getReturnType()->isVoidTy());

        if (isInstrumented(&F)) {
      
        } else if (!IsZeroArgsVoidRet || getWrapperKind(&F) == WK_Custom) {

            *i = nullptr;
            if (!F.isDeclaration()) {
                // This function is probably defining an interposition of an
                // uninstrumented function and hence needs to keep the original ABI.
                // But any functions it may call need to use the instrumented ABI, so
                // we instrument it in a mode which preserves the original ABI.
                // This code needs to rebuild the iterators, as they may be invalidated
                // by the push_back, taking care that the new range does not include
                // any functions added by this code.
                size_t N = i - FnsToInstrument.begin(),
                    Count = e - FnsToInstrument.begin();
                FnsToInstrument.push_back(&F);
                i = FnsToInstrument.begin() + N;
                e = FnsToInstrument.begin() + Count;
            }
                // Hopefully, nobody will try to indirectly call a vararg
                // function... yet.
        } else if (FT->isVarArg()) {
            *i = nullptr;
        }
    }

    for (Function *i : FnsToInstrument) {
        if (!i || i->isDeclaration())
            continue;

        removeUnreachableBlocks(*i);
        //errs() << i->getName() << "\n";
        SmallVector<BasicBlock *, 4> BBList(depth_first(&i->getEntryBlock()));
        for (BasicBlock *i : BBList) {
            Instruction *Inst = &i->front();
            while (true) {
                // DFSanVisitor may split the current basic block, changing the current
                // instruction's next pointer and moving the next instruction to the
                // tail block from which we should continue.
                Instruction *Next = Inst->getNextNode();
                // DFSanVisitor may delete Inst, so keep track of whether it was a
                // terminator.
                bool IsTerminator = Inst->isTerminator();
                visit(Inst);
                if (IsTerminator)
                break;
                Inst = Next;
            }
        }
        //visit(i);

    }

    /*for (Function &F : M) {
    
        if (!F.isIntrinsic() && 
                &F != MemlogHookDebugFunc.getCallee()->stripPointerCasts() &&
                &F != MemlogHook1Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook2Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook2Int128Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook3Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook4Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook5Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook6Func.getCallee()->stripPointerCasts() &&
                &F != MemlogHook7Func.getCallee()->stripPointerCasts() &&
                &F != MemlogVaArgHook1Func.getCallee()->stripPointerCasts()) {
            
            if (F.isDeclaration())
                continue;
            //errs() << F.getName() << "\n";
            visit(F);
        }
        
    }*/

    std::fstream OutFile;
    OutFile.open(HookIDFileName, std::ios::out | std::ios::trunc);
    OutFile << HookID % MemlogMapW;
    OutFile.close();

    return true;
}


bool MemlogPass::isInstrumented(const Function *F) {
    return !ABIList.isIn(*F, "uninstrumented");
}

bool MemlogPass::isInstrumented(const GlobalAlias *GA) {
  return !ABIList.isIn(*GA, "uninstrumented");
}

MemlogPass::WrapperKind MemlogPass::getWrapperKind(Function *F) {
  if (ABIList.isIn(*F, "functional"))
    return WK_Functional;
  if (ABIList.isIn(*F, "discard"))
    return WK_Discard;
  if (ABIList.isIn(*F, "custom"))
    return WK_Custom;

  return WK_Warning;
}
bool MemlogPass::shouldHook(const Function *F) {
    return __HookABIList.isIn(*F, "hook");
}

MemlogPass::HookType MemlogPass::getHookType(const Function *F) {

    if(__HookABIList.isIn(*F, "hook1")) 
        return HT_HOOK1;
    else if(__HookABIList.isIn(*F, "hook2"))
        return HT_HOOK2;
    else if(__HookABIList.isIn(*F, "hook3"))
        return HT_HOOK3;
    else if(__HookABIList.isIn(*F, "hook4"))
        return HT_HOOK4;
    else if(__HookABIList.isIn(*F, "hook5"))
        return HT_HOOK5;
    else if(__HookABIList.isIn(*F, "hook6"))
        return HT_HOOK6;
    else if(__HookABIList.isIn(*F, "hook7"))
        return HT_HOOK7;
    else if(__HookABIList.isIn(*F, "vararghook1"))
        return HT_VARARG_HOOK1;
    return HT_UNKNOWN;

}

void MemlogPass::whichType(Type *T) {

    if (T->isFloatingPointTy())
        errs() << "FloatingPoint\n";
    else if (T->isIntegerTy())
        errs() << "Integer\n";
    else if (T->isPointerTy())
        errs() << "Pointer\n";
    else if (T->isStructTy())
        errs() << "Struct\n";
    else if (T->isArrayTy()) 
        errs() << "Array\n";
    else if (T->isDoubleTy())
        errs() << "Double\n";
    else if (T->isVectorTy())
        errs() << "Vector\n";
}

void MemlogPass::visitLoadInst(LoadInst &LI) {
    
    /*if (ClHookInst) { 
        
        size_t SourceType = TaintDataLayout->getPointerTypeSize(LI.getPointerOperandType());
        size_t ResultType = TaintDataLayout->getTypeStoreSize(LI.getType());
        
        IRBuilder <> IRB(&LI);
        
        IRB.CreateCall(MemlogHook1Func, {ConstantInt::get(Int32Ty, HookID++), LI.getPointerOperand(),
            ConstantInt::get(Int32Ty, SourceType), ConstantInt::get(Int32Ty, ResultType)});
    
    }*/

}

void MemlogPass::visitStoreInst(StoreInst &SI) {
    
    /*if (ClHookInst) {
        
        size_t SourceType = TaintDataLayout->getTypeStoreSize(SI.getValueOperand()->getType());
        size_t ResultType = TaintDataLayout->getPointerTypeSize(SI.getPointerOperandType());

        IRBuilder <> IRB(&SI);
        
        if (SourceType == 16) {

            Value *Int128Val = IRB.CreateBitCast(SI.getValueOperand(), Int128Ty);
            IRB.CreateCall(MemlogHook2Int128Func,{ConstantInt::get(Int32Ty, HookID++), Int128Val, 
                ConstantInt::get(Int32Ty, SourceType), SI.getPointerOperand(),
                ConstantInt::get(Int32Ty, ResultType)});

        }
        else {
          
            IRB.CreateCall(MemlogHook2Func, {ConstantInt::get(Int32Ty, HookID++), SI.getValueOperand(), 
                ConstantInt::get(Int32Ty, SourceType), SI.getPointerOperand(),
                ConstantInt::get(Int32Ty, ResultType)});

        }
        
    }*/

}

void MemlogPass::visitCallBase(CallBase &CB) {
    auto F = CB.getCalledFunction();
    
    if (ClHookInst && F && !F->isIntrinsic() && shouldHook(F)) {
        
        if (ClDebug)
          errs() << CB.getFunction()->getName() << " hook "<< F->getName() << " " << HookID << "\n"; 
        
        IRBuilder <> IRB(&CB);
        std::vector<Value *> ArgArray;
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        for (User::op_iterator it = CB.arg_begin(); it != CB.arg_end(); it++) {
            ArgArray.push_back(*it);
        }

        switch(getHookType(F)) {
            case HT_HOOK1:
                IRB.CreateCall(MemlogHook1Func, ArgArray);
                break;
            case HT_HOOK2:
                IRB.CreateCall(MemlogHook2Func, ArgArray);
                break;
            case HT_HOOK3:
                IRB.CreateCall(MemlogHook3Func, ArgArray);
                break;
            case HT_HOOK4:
                IRB.CreateCall(MemlogHook4Func, ArgArray);
                break;
            case HT_HOOK5:
                IRB.CreateCall(MemlogHook5Func, ArgArray);
                break;
            case HT_HOOK6:
                IRB.CreateCall(MemlogHook6Func, ArgArray);
                break;
            case HT_HOOK7:
                IRB.CreateCall(MemlogHook7Func, ArgArray);
                break;
            default:
                break;
        }
        
    }

}
/*void MemlogPass::visitCallInst(CallInst &I) {
    auto F = I.getCalledFunction();
    
    errs() << "visit Call inst "<< F->getName()  << "\n"; 
}*/

void MemlogPass::visitInvokeInst(InvokeInst &I) {
    //errs() << "visitInvokeInst\n";

    if (ClHookInst) {

    }

}

void MemlogPass::visitGetElementPtrInst(GetElementPtrInst &I) {
    
    if (ClHookInst) {
        
        if (ClDebug) 
            errs() << I.getFunction()->getName() << " hook GetElementPtr id: " << HookID << "\n";
        
        size_t SourceType = TaintDataLayout->getTypeStoreSize(I.getSourceElementType());
        size_t ResultType = TaintDataLayout->getTypeStoreSize(I.getResultElementType());
        
        std::vector<Value *> ArgArray;  
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        ArgArray.push_back(I.getPointerOperand());
        ArgArray.push_back(ConstantInt::get(Int32Ty, SourceType));
        ArgArray.push_back(ConstantInt::get(Int32Ty, ResultType));
        // need to pass type and idx both
        ArgArray.push_back(ConstantInt::get(Int32Ty, I.getNumIndices()));
        //ArgArray.push_back(ConstantInt::get(Int32Ty, 9527));
        // get idx and type
        for (User::op_iterator it = I.idx_begin(); it != I.idx_end(); it++) {
            ArgArray.push_back(ConstantInt::get(Int32Ty, 
                TaintDataLayout->getTypeStoreSize((*it)->getType())));
            ArgArray.push_back(*it); 
        }

        IRBuilder <> IRB(&I);
        IRB.CreateCall(MemlogVaArgHook1Func, ArgArray);

    }

}

void MemlogPass::visitMemSetInst(MemSetInst &I) {
   
    if (ClHookInst) {
        
        if (ClDebug)
            errs() << I.getFunction()->getName() << " hook MemSetInst id: " << HookID << "\n";

        IRBuilder <> IRB(&I);
        std::vector<Value *> ArgArray;
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        for (User::op_iterator it = I.arg_begin(); it != I.arg_end(); it++) {
            ArgArray.push_back(*it);
        }
        IRB.CreateCall(MemlogHook3Func, ArgArray);

    }

}

void MemlogPass::visitMemCpyInst(MemCpyInst &I) {
    
    if (ClHookInst) {
        
        if (ClDebug)
            errs() << I.getFunction()->getName() << " hook MemCpyInst id: " << HookID << "\n";

        IRBuilder <> IRB(&I);
        std::vector<Value *> ArgArray;
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        for (User::op_iterator it = I.arg_begin(); it != I.arg_end(); it++) {
            ArgArray.push_back(*it);
        }
        IRB.CreateCall(MemlogHook4Func, ArgArray);
    }

}

void MemlogPass::visitMemCpyInlineInst(MemCpyInlineInst &I) {

    if (ClHookInst) {
        
        if (ClDebug)
            errs() << I.getFunction()->getName() << " hook MemCpyInlineInst id: " << HookID << "\n";
        
        IRBuilder <> IRB(&I);
        std::vector<Value *> ArgArray;
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        for (User::op_iterator it = I.arg_begin(); it != I.arg_end(); it++) {
            ArgArray.push_back(*it);
        }
        IRB.CreateCall(MemlogHook4Func, ArgArray);

    }

}

void MemlogPass::visitMemMoveInst(MemMoveInst &I) {
    
    if (ClHookInst) {
        
        if (ClDebug)
            errs() << I.getFunction()->getName() << " hook MemMoveInst id: " << HookID << "\n";

        IRBuilder <> IRB(&I);
        std::vector<Value *> ArgArray;
        ArgArray.push_back(ConstantInt::get(Int32Ty, HookID++));
        for (User::op_iterator it = I.arg_begin(); it != I.arg_end(); it++) {
            ArgArray.push_back(*it);
        }
        IRB.CreateCall(MemlogHook4Func, ArgArray);
    }

}

// C++ allocator
void MemlogPass::visitAllocaInst(AllocaInst &I) {
    
    /*if (ClHookInst) {

        size_t AllocatedType = TaintDataLayout->getTypeAllocSize(I.getAllocatedType());

        IRBuilder <> IRB(&I);
        IRB.CreateCall(MemlogHook5Func, {ConstantInt::get(Int32Ty, HookID++), ConstantInt::get(Int32Ty, AllocatedType),
            I.getArraySize()});

    }*/

}
/**
 * According to llvm manupage, extractValue/insertValue are used for register structure(?),
 * extractElement/insterElement are used for vector operations.
 * I don't know how to invode these instructions yet.
 * Since these instructions also contains memory read/write operations, better to cover it
 * in the future. 
 */
void MemlogPass::visitExtractElementInst(ExtractElementInst &I) {
   
    //if (ClDebug)
        //errs() << "hook ExtractElementInst id: " << HookID << "\n";
    /*if (ClHookInst) {
        
        size_t VectorType = TaintDataLayout->getPointerTypeSize(I.getVectorOperandType());
        IRBuilder <> IRB(&I);
        IRB.CreateCall(MemlogHook5Func, {I.getVectorOperand(),
        I.getIndexOperand(), ConstantInt::get(SizeTy, VectorType)});

    }*/

}

void MemlogPass::visitInsertElementInst(InsertElementInst &I) {

    //if (ClDebug)
        //errs() << "hook InsertElementInst id: " << HookID << "\n";
    /*if (ClHookInst) {
        
        size_t VectorType = TaintDataLayout->getTypeStoreSize(
            I.getType()->getElementType());
        IRBuilder <> IRB(&I);
        IRB.CreateCall(MemlogHook1Func, {ConstantInt::get(SizeTy, VectorType)});
        
    }*/

}

void MemlogPass::visitExtractValueInst(ExtractValueInst &I) {
    
    /*if (ClDebug)
        errs() << "hook ExtractValueInst id: " << HookID << "\n";*/
    /*if (ClHookInst) {

        errs() << I.getNumIndices() << " " << I.getAggregateOperand() << "\n";
        
        Value *Idx;
        for(ExtractValueInst::idx_iterator it 
            = I.idx_begin(); it != I.idx_end(); it++) {
            //Idx = ConstantInt::get(SizeTy, *it);
        }

        IRBuilder <> IRB(&I);
        IRB.CreateCall(DebugFunc, {I.getAggregateOperand(), I.getAggregateOperand()});
    
    }*/

}

void MemlogPass::visitInsertValueInst(InsertValueInst &I) {
   
    //if (ClDebug)
        //errs() << "hook InsertValueInst id: " << HookID << "\n";
    /*if (ClHookInst) {

        std::vector<Value *> ArgArray;
        ArgArray.push_back(I.getInsertedValueOperand());
        // value type
        ArgArray.push_back(ConstantInt::get(Int32Ty, TaintDataLayout->
            getTypeStoreSize(I.getInsertedValueOperand()->getType())));
        ArgArray.push_back(I.getAggregateOperand());
        // aggregate type
        ArgArray.push_back(ConstantInt::get(Int32Ty, TaintDataLayout->
            getTypeStoreSize(I.getAggregateOperand()->getType())));
        Value *Idx;
        for(ExtractValueInst::idx_iterator it 
            = I.idx_begin(); it != I.idx_end(); it++) {
            Idx = ConstantInt::get(Int32Ty, *it);
        }

        IRBuilder <> IRB(&I);
        IRB.CreateCall(DebugFunc, ArgArray);

    }*/

}

void MemlogPass::visitShuffleVectorInst(ShuffleVectorInst &I) {
    //errs() << "visitShuffleVectorInst\n";
}

void MemlogPass::visitAtomicCmpXchgInst(AtomicCmpXchgInst &I) {
    //errs() << "visitAtomicCmpXchgInst\n"; 
}

void MemlogPass::visitAtomicRMWInst(AtomicRMWInst &I) {
    //errs() << "visitAtomicRMWInst\n"; 
}

static RegisterPass<MemlogPass> X("Memloghook", "MemlogPass", false, false);

static void registerMemlog(const PassManagerBuilder &,
                                 legacy::PassManagerBase &PM) {

  PM.add(new MemlogPass());
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
    RegisterMemlog(PassManagerBuilder::EP_OptimizerLast,
                   registerMemlog);

static RegisterStandardPasses
    RegisterMemlog0(PassManagerBuilder::EP_EnabledOnOptLevel0,
                   registerMemlog);
