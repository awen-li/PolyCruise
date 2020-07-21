//===-- Source.h - compute source set with input --------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SOURCE_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SOURCE_H
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
#include "llvm/IR/InstIterator.h"

using namespace llvm;
using namespace std;


typedef set<Value*>::iterator svi_iterator;

#define IS_TAINTED(TaintBit, BitNo) (TaintBit & (1 << (32-BitNo)))
#define SET_TAINTED(TaintBit, BitNo) (TaintBit |= (1 << (32-BitNo)))


class Source
{
private:
    Function *m_ScCaller;
    set<Value *> m_Criterion;
    set<Instruction*> m_SInsts;
    

    ModuleManage *m_Ms;

public:
    Source (ModuleManage *Ms,
              string ScFName, string ApiName, unsigned TaintBit)
    {
        m_Ms = Ms;

        Function* ScF = GetScFunc (ScFName);
        assert ((ScF != NULL) && "Source Caller should exists!");
        
        Compute (ScF, ApiName, TaintBit);
        m_ScCaller = ScF;

        assert (m_Criterion.size() != 0);
    }

public:
    inline Function* GetSrcCaller ()
    {
        return m_ScCaller;
    }

    inline svi_iterator begin ()
    {
        return m_Criterion.begin();
    }

    inline svi_iterator end ()
    {
        return m_Criterion.end();
    }

    inline bool IsSrcInst (Instruction *Inst)
    {
        auto It = m_SInsts.find (Inst);
        if (It == m_SInsts.end ())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

private:
    inline Function* GetScFunc (string ScFName)
    {
        for (auto ItF = m_Ms->func_begin(), End = m_Ms->func_end(); ItF != End; ++ItF)
        {
            Function *F = *ItF;  
            if (F->isIntrinsic())
            {
                continue;
            }

            if (!ScFName.compare(F->getName ().data()))
            {
                return F;
            }
        }

        return NULL;
    }
    
    
    inline void Compute (Function* ScF, string ApiName, unsigned TaintBit)
    {
        for (auto ItI = inst_begin(*ScF), Ed = inst_end(*ScF); ItI != Ed; ++ItI) 
        {
            Function *Callee = NULL;
            Instruction *Inst = &*ItI;

            LLVMInst LI (Inst);
            if (!LI.IsCall () || ((Callee = LI.GetCallee ()) == NULL))
            {
                continue;
            }

            if (ApiName.compare(Callee->getName ().data()))
            {
                continue;
            }

            errs()<<*Inst<<"=========> ";

            unsigned BitNo = RET_NO;
            if (IS_TAINTED (TaintBit, BitNo))
            {
                m_Criterion.insert (LI.GetDef ());
                m_SInsts.insert (Inst);
                errs()<<" "<<BitNo;
            }

            BitNo++;
            for (auto It = LI.begin (); It != LI.end(); It++)
            {
                Value *U = *It;
                if (IS_TAINTED (TaintBit, BitNo))
                {
                    errs()<<" "<<BitNo;
                    m_Criterion.insert(U);
                    m_SInsts.insert (Inst);
                }
       
                BitNo++;                        
            }

            errs()<<"\r\n";
        }
    }
};

#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H