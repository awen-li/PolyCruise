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
#include "ModuleSet.h"

using namespace std;
using namespace llvm;

    
class Fts
{
private:
    map<Type*, unsigned> m_Type2Id;
    map<Function*, string> m_Func2Typs;
    
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
                FuncType += to_string (GetTypeId (VType));
            }

            errs()<<Func->getName ()<<" -> "<<FuncType<<"\r\n";  
        }

        errs ()<<"\r\n================= Type List =================\r\n";
        for (auto It = m_Type2Id.begin(); It != m_Type2Id.end(); It++)
        {
            errs()<<*(It->first)<<" -> "<<It->second<<"\r\n";
        }
        
        return;
    }
    
};


#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_FTS_H