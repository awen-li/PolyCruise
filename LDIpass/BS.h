//===-- BS.h - definition for block summary ------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_BS_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_BS_H
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

typedef vector<Instruction*>::iterator ci_iterator;
typedef set<Value*>::iterator si_iterator;


class BS
{
private:
    BasicBlock *m_CurBB;
    
    set <Value*> m_Def; 
    map <Value*, Value*> m_Def2Use;

    set <Value*> m_Input;
    set <Value*> m_Output;
    
    vector<Instruction*> m_CallInst;
    Instruction* m_RetInst; 

public:
    
    BS(BasicBlock *BB)
    {
        m_CurBB = BB;
        Summarize ();
    }
    
    inline ci_iterator begin ()
    {
        return m_CallInst.begin();
    }

    inline ci_iterator end ()
    {
        return m_CallInst.end();
    }

    inline si_iterator in_begin ()
    {
        return m_Input.begin();
    }

    inline si_iterator in_end ()
    {
        return m_Input.end();
    }

    inline si_iterator out_begin ()
    {
        return m_Output.begin();
    }

    inline si_iterator out_end ()
    {
        return m_Output.end();
    }

    inline Instruction* GetBsSite ()
    {
        return m_RetInst;
    }
    
private:

    inline void Summarize ()
    {
        for (auto It = m_CurBB->begin (); It != m_CurBB->end (); It++)
        {
            Instruction *Inst = &(*It);
            errs ()<<"Instruction: "<<*Inst<<"\r\n";
            
            LLVMInst LI (Inst);
            if (LI.IsRet())
            {
                Value* Ret = LI.GetRetValue();
                if (Ret && !LI.IsConst (Ret))
                {
                    m_Output.insert (Ret);
                }

                m_RetInst = Inst;
                continue;
            }
           
            Value *Def = LI.GetDef ();
            if (Def != NULL)
            {
                m_Def.insert (Def);
                m_Output.insert (Def);
                errs ()<<"Define: "<<Def<<"\r\n";

                Function *Callee;
                if (LI.IsCall (&Callee))
                {
                    m_CallInst.push_back (Inst);
                }
            }
               
            for (auto It = LI.begin (); It != LI.end(); It++)
            {
                Value *U = *It;
                errs ()<<"Use: "<<U<<"\r\n";
                
                if (m_Def.find (U) == m_Def.end())
                {
                    m_Input.insert (U);
                    errs ()<<"AddInput: "<<U<<"\r\n";
                }
                else
                {
                    m_Output.erase (U);
                    errs ()<<"RemoveOupt: "<<U<<"\r\n"; 
                }
            }

            errs ()<<"\r\n\r\n"; 
        }


        errs()<<"============ Input =====================\r\n";
        Print (m_Input);
        errs()<<"============ Output =====================\r\n";
        Print (m_Output);
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
