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

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H
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
#include "LLVMInst.h"
#include "ModuleSet.h"
#include "common/WorkList.h"
#include "ExternalLib.h"


using namespace llvm;
using namespace std;


typedef set<Instruction*>::iterator sii_iterator;
typedef set<Value*>::iterator svi_iterator;

#define IS_TAINTED(TaintBit, BitNo) (TaintBit & (1 << (32-BitNo)))
#define SET_TAINTED(TaintBit, BitNo) (TaintBit |= (1 << (32-BitNo)))


class Source
{
private:
    Function *m_ScCaller;
    set<Value *> m_Criterion;

    ModuleManage *m_Ms;

public:
    Source (ModuleManage *Ms, string ScFName, 
              string ApiName, unsigned TaintBit)
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

            unsigned BitNo = 1;
            if (IS_TAINTED (TaintBit, BitNo))
            {
                m_Criterion.insert (LI.GetDef ());
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
                }
       
                BitNo++;                        
            }

            errs()<<"\r\n";
        }
    }
};


class Lda
{
private:
    Fts *m_Fs;
    map <Function*, unsigned> m_FTaintBits;
    
    set<Value*> m_GlobalLexSet;
    
    set <Instruction*> m_InstSet;

    ModuleManage *m_Ms;
    ComQueue <Function *> m_FuncList;

    Source *m_Source;

public:
    
    Lda(ModuleManage *Ms, Source *S)
    {
        m_Ms = Ms;
        m_Fs = new Fts(Ms);
        assert (m_Fs != NULL);

        m_Source = S;

        Compute ();
    }

    ~Lda ()
    {
        if (m_Fs)
        {
            delete m_Fs;
            m_Fs = NULL;
        }
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

    inline void InitCriterions (Function *Func, set<Value*> *LexSet)
    {
        if (Func == m_Source->GetSrcCaller ())
        {
            for (auto It = m_Source->begin(), End = m_Source->end(); It != End; It++)
            {
                LexSet->insert (*It);
            }
        }
        
        auto It = m_FTaintBits.find (Func);
        if (It == m_FTaintBits.end())
        {
            return;
        }

        unsigned TaintBit = It->second;        
        unsigned BitNo = 2;
        for (auto Ita = Func->arg_begin(); Ita != Func->arg_end(); Ita++) 
        {
            Argument *Formal = &(*Ita);

            if (IS_TAINTED (TaintBit, BitNo))
            {
                LexSet->insert (Formal);
            }

            BitNo++;
        }

        return;
    }

    inline unsigned GetFuncTaintBits (Function *F)
    {
        unsigned TaintBit;
        
        auto It = m_FTaintBits.find (F);
        if (It == m_FTaintBits.end())
        {
            TaintBit = 0;
        }
        else
        {
            TaintBit = It->second;
        }

        return TaintBit;
    }

    inline void UpdateFuncTaintBit (Function *F, unsigned Bits)
    {
        unsigned TaintBit = GetFuncTaintBits (F);

        TaintBit |= Bits;
        m_FTaintBits[F] = TaintBit;

        return;
    }

    inline bool IsRetTainted (Function *F)
    {
        unsigned TaintBit;
        
        auto It = m_FTaintBits.find (F);
        if (It == m_FTaintBits.end())
        {
            return false;
        }
        else
        {
            TaintBit = It->second;
            return IS_TAINTED (TaintBit, 1);
        }
    }

    inline void CheckOutArgTainted (Function *Func, Value *Val)
    {
        unsigned BitNo = 2;
        for (auto Ita = Func->arg_begin(); Ita != Func->arg_end(); Ita++, BitNo++) 
        {
            Argument *Formal = &(*Ita);

            if (Formal == Val)
            {
                UpdateFuncTaintBit (Func, (1 << (32-BitNo)));
                return;
            }
        }

        return;
    }

    inline void PrintFuncTaintBit (string Tag, Function *F)
    {
        unsigned TaintBit;
        
        auto It = m_FTaintBits.find (F);
        if (It == m_FTaintBits.end())
        {
            TaintBit = 0;
        }
        else
        {
            TaintBit = It->second;
        }

        printf ("[%s] %s TaintBits = %#x \r\n", Tag.c_str(), F->getName ().data(), TaintBit);
    }

    inline void AddTaintValue (set<Value*> *LexSet, Value *Val)
    {
        if (Val != NULL)
        {
            errs ()<<"\t Insert Lex: "<<Val<<"\r\n";
            LexSet->insert (Val);
        }
    }

    inline unsigned GetTaintedBits (LLVMInst *Inst, set<Value*> *LexSet)
    {
        unsigned TaintBit = 0;
        unsigned BitNo = 2;
        for (auto It = Inst->begin (); It != Inst->end(); It++, BitNo++)
        {
            Value *U = *It;

            if (LexSet->find (U) == LexSet->end())
            {
                continue;
            }

            SET_TAINTED (TaintBit, BitNo);
        }

        return TaintBit;
    }

    inline void ProcessCall (LLVMInst *LI, unsigned TaintBits, set<Value*> *LexSet)
    {
        Function *Callee = LI->GetCallee();
        if  (Callee != NULL)
        {
            UpdateFuncTaintBit (Callee, TaintBits);
            
            PrintFuncTaintBit ("Call_Begin", Callee);

            Flda (Callee);

            /* actual arguments */
            unsigned FTaintBits = GetFuncTaintBits (Callee);
            unsigned BitNo = 2;
            for (auto It = LI->begin (); It != LI->end(); It++, BitNo++)
            {
                Value *U = *It;

                if (IS_TAINTED(FTaintBits, BitNo))
                {
                    LexSet->insert (U);
                }
            }

            /* return value */
            if (IsRetTainted (Callee))
            {
                AddTaintValue (LexSet, LI->GetDef ());
            }

            PrintFuncTaintBit ("Call_End", Callee);
        }
        else
        {
        }
    }

    inline void Flda (Function *Func)
    {
        set<Value*> LocalLexSet;

        errs()<<"Process "<<Func->getName ()<<"\r\n";
        InitCriterions (Func, &LocalLexSet);
        
        for (inst_iterator ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI) 
        {
            Instruction *Inst = &*ItI;
            
            LLVMInst LI (Inst);
            if (LI.IsIntrinsic())
            {
                continue;
            }

            unsigned TaintedBits = GetTaintedBits (&LI, &LocalLexSet);
            if (TaintedBits == 0)
            {
                if (!LI.IsCall ())
                {
                    continue;
                }

                ProcessCall (&LI, TaintedBits, &LocalLexSet);
            }
            else
            {

                errs ()<<"LDA: "<<*Inst<<"\r\n";
                if (LI.IsRet ())
                {
                    UpdateFuncTaintBit (Func, 1<<31);
                }
                else if (LI.IsCall ()) 
                {
                    ProcessCall (&LI, TaintedBits, &LocalLexSet);                    
                }
                else
                {
                    Value *Val = LI.GetDef ();
                    CheckOutArgTainted (Func, Val);

                    AddTaintValue (&LocalLexSet, Val);
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

    inline void Compute ()
    {       
        Function *Entry = m_Ms->GetEntryFunction ();
        assert (Entry != NULL);

        Flda (Entry);
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



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H