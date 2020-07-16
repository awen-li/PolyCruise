//===-- SymDependence.h - symbol dependence computation --------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SYMDEPENDENCE_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SYMDEPENDENCE_H
#include <set>
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "LLVMInst.h"

using namespace llvm;
using namespace std;


typedef set<Instruction*>::iterator sii_iterator;


class SymDependence
{
private:
    Function *m_Func;

    set<Value*> m_SymSet;

    set <Instruction*> m_InstSet;

public:
    
    SymDependence(Function *F, set<Value *> &Criterion)
    {
        m_Func = F;
        for (auto It = Criterion.begin(); It != Criterion.end(); It++)
        {
            m_SymSet.insert (*It);
        }
        Compute ();
    }

    sii_iterator begin ()
    {
        return m_InstSet.begin();
    }

    sii_iterator end ()
    {
        return m_InstSet.end();
    }
    
private:

    inline void Compute ()
    {
        if (m_SymSet.size() == 0)
        {
            return;
        }
        
        for (inst_iterator ItI = inst_begin(*m_Func), Ed = inst_end(*m_Func); ItI != Ed; ++ItI) 
        {
            Instruction *Inst = &*ItI;
            
            LLVMInst LI (Inst);
            if (LI.IsIntrinsic())
            {
                continue;
            }

            int No = 0;
            for (auto It = LI.begin (); It != LI.end(); It++)
            {
                Value *U = *It;

                if (m_SymSet.find (U) == m_SymSet.end())
                {
                    continue;
                }

                if (LI.IsLoad () && (Inst->getType ()->getTypeID () == Type::IntegerTyID))
                {
                    errs ()<<"Load Type = "<<*(Inst->getType ())<<"\r\n";
                    continue;
                }

                errs ()<<"SD: "<<*Inst<<"\r\n";

                Value *Def = LI.GetDef ();
                if (Def != NULL)
                {
                    errs ()<<"\t Insert Sym: "<<Def<<"\r\n";
                    m_SymSet.insert (Def);
                }

                Function *CallFunc;
                if (LI.IsCall (&CallFunc) && CallFunc != NULL && !CallFunc->isDeclaration ())
                {
                    errs()<<"===> Call "<<CallFunc->getName ()<<", Tainted Argument: "<<U<<" No = "<<No<<"\r\n";
                }

                if (LI.IsPHI ())
                {
                    continue;
                }

                m_InstSet.insert (Inst);
                errs ()<<"\tTainted Inst: "<<Inst<<"\r\n";
            }
        }
    }

    inline void Print (set <Value*>& S)
    {
        for (auto It = S.begin(); It != S.end(); It++)
        {
            Value *V = *It;
            errs () <<*V<<"  --->  "<<V<<"\r\n";
        }
    }
   
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
