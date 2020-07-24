//===-- HookFunc.h - Hook Functions definition for instrumenter ----------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_HOOKFUNC_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_HOOKFUNC_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"


using namespace llvm;
using namespace std;

class HookFunc
{
public:
    virtual inline Constant *geHookFunction(Module *M) = 0;
    virtual inline Constant *geInitFunction(Module *M) = 0;
    virtual inline Constant *geExitFunction(Module *M) = 0;
};


class TraceFunc:public HookFunc
{
public:

    inline Constant *geHookFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void TRC_track (char *Data) */      
        Type *ArgTypes[] = {Type::getInt8PtrTy(M->getContext()), };
        FunctionType *TRC_track = FunctionType::get(Type::getVoidTy(context), ArgTypes, true);
            
        return M->getOrInsertFunction("TRC_track", TRC_track);
    }

 
    inline Constant *geInitFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void IBS_init () */
        FunctionType *TRC_init = FunctionType::get(Type::getInt32Ty(context), false);
            
        return M->getOrInsertFunction("TRC_init", TRC_init);
    }


    inline Constant *geExitFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void IBS_exit () */
        FunctionType *TRC_exit = FunctionType::get(Type::getVoidTy(context), false);
            
        return M->getOrInsertFunction("TRC_exit", TRC_exit);
    }

};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_HOOKFUNC_H