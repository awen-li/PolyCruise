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
    virtual inline Constant *getTraceFunction(Module *M) = 0;
    virtual inline Constant *getInitFunction(Module *M) = 0;
    virtual inline Constant *getExitFunction(Module *M) = 0;
};


class TraceFunc:public HookFunc
{
public:

    inline Constant *getTraceFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void TRC_track (unsigned long EventId, char *Data) */      
        Type *ArgTypes[] = {Type::getInt64Ty (M->getContext()), 
                            Type::getInt8PtrTy(M->getContext()), };
        //Type *ArgTypes[] = {Type::getInt8PtrTy(M->getContext()), };
        FunctionType *TRC_trace = FunctionType::get(Type::getVoidTy(context), ArgTypes, true);
            
        return M->getOrInsertFunction("TRC_trace", TRC_trace);
    }

 
    inline Constant *getInitFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void TRC_init () */
        FunctionType *TRC_init = FunctionType::get(Type::getVoidTy(context), false);
            
        return M->getOrInsertFunction("TRC_init", TRC_init);
    }


    inline Constant *getExitFunction(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void TRC_exit () */
        FunctionType *TRC_exit = FunctionType::get(Type::getVoidTy(context), false);
            
        return M->getOrInsertFunction("TRC_exit", TRC_exit);
    }

    inline Constant *getThreadTrace(Module *M) 
    {
        LLVMContext &context = M->getContext();

        /* void TRC_thread (ULONG EventId, char* ThreadEntry, DWORD *ThrId) */
        Type *ArgTypes[] = {Type::getInt64Ty (M->getContext()), 
                            Type::getInt8PtrTy(M->getContext()),
                            Type::getInt64PtrTy(M->getContext()),};

        
        FunctionType *Threadtr = FunctionType::get(Type::getVoidTy(context), ArgTypes, false);
            
        return M->getOrInsertFunction("TRC_thread", Threadtr);
    }

};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_HOOKFUNC_H