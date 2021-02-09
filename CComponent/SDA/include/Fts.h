//===-- Fts.h - Function Type summarize -----------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_FTS_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_FTS_H
#include <utility>
#include <vector>
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "ModuleSet.h"
#include "LLVMInst.h"

using namespace std;
using namespace llvm;

typedef set <Function*> FUNC_SET;
    
class Fts
{
private:
    map<Type*, unsigned> m_Type2Id;
    map<Function*, string> m_Func2Typs;
    map<string, FUNC_SET>  m_Type2Funcs;
    
    ModuleManage *m_Ms;
    
public:
    Fts (ModuleManage* Ms)
    {
        m_Ms = Ms;
        Summarize ();
    }

    ~Fts()
    {
    }

public:
    FUNC_SET* GetCalleeFuncs (LLVMInst *CI)
    {
        Value *val = CI->GetDef ();
        string FuncType = to_string (GetTypeId (val->getType ()));

        for (auto It = CI->begin (); It != CI->end(); It++) 
        {
            val = *It;
            FuncType += "." + to_string (GetTypeId (val->getType ()));
        }

        //errs()<<*(CI->GetInst())<<" ---> Type = "<<FuncType<<"\r\n";
        auto Fit = m_Type2Funcs.find (FuncType);
        if (Fit != m_Type2Funcs.end ())
        {
            return &(Fit->second);
        }
        
        return NULL;
    }
    

private:
    inline unsigned GetTypeId (Type *T)
    {
        unsigned No = m_Type2Id.size()+1;
        
        auto It = m_Type2Id.find (T);
        if (It == m_Type2Id.end ())
        {
            m_Type2Id [T] = No;
            return No;
        }
        else
        {
            return It->second;
        }
    }

    inline void Show ()
    {
        errs ()<<"\r\n================= Type List =================\r\n";
        for (auto It = m_Type2Id.begin(); It != m_Type2Id.end(); It++)
        {
            Type *VType = It->first;

            if (VType->isPointerTy ())
            {
                Type *PType = cast<PointerType>(VType)->getElementType();
                if (PType->isStructTy ())
                {
                    errs()<<"[Structure "<<PType->getStructName ()<<"]";
                }
            }
            
            errs()<<*VType<<" -> "<<It->second<<"\r\n";
        }
        errs ()<<"=============================================\r\n";
    }
    
    inline void Summarize ()
    {
        for (auto ItF = m_Ms->func_begin(), End = m_Ms->func_end(); ItF != End; ++ItF) 
        {   
            string FuncType = "";
            Function *Func = *ItF;
    
            Type *VType = Func->getReturnType ();
            FuncType += to_string (GetTypeId (VType));

            for (auto Ita = Func->arg_begin(); Ita != Func->arg_end(); Ita++) 
            {
                Argument *Formal = &(*Ita);

                Type *VType = Formal->getType();
                FuncType += "." + to_string (GetTypeId (VType));
            }

            m_Func2Typs[Func]  = FuncType;
            m_Type2Funcs[FuncType].insert (Func);

            //errs()<<Func->getName ()<<" -> "<<FuncType<<"\r\n";  
        }

        //Show ();
        
        return;
    }
    
};


#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_FTS_H