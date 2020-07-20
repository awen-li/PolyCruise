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
#include "Source.h"
#include "ExternalLib.h"
#include "StField.h"


using namespace llvm;
using namespace std;

typedef set<Instruction*>::iterator sii_iterator;

struct CSTaint
{
    CSTaint (unsigned InTaintBits)
    {
        m_InTaintBits = InTaintBits;
    }

    ~CSTaint ()
    {
    }
    
    set <Function*> m_Callees;
    unsigned m_InTaintBits;
    unsigned m_OutTaintBits;
};

class Flda
{
private:
    Function *m_CurFunc;
    map <Instruction*, CSTaint> m_CallSite2Cst;
    set <Instruction*> m_TaintInsts;

public:
    Flda (Function *Func)
    {
        m_CurFunc = Func;
    }

    ~Flda ()
    {

    }

    inline void InsertInst (Instruction* TaintInst)
    {
        m_TaintInsts.insert (TaintInst);
    }

    inline CSTaint* InsertCst (Instruction* CI, unsigned InTaintBits)
    {
        auto It = m_CallSite2Cst.find (CI);
        if (It == m_CallSite2Cst.end())
        {
            auto Pit = m_CallSite2Cst.insert (make_pair(CI, CSTaint (InTaintBits)));
            assert (Pit.second == true);

            return &(Pit.first->second);
        }
        else
        {
            return &(It->second);
        }
    }
};

class Lda
{
private:
    Fts *m_Fts;

    map <Function*, Flda> m_Func2Flda;
    
    set<Value*> m_GlobalLexSet;
    
    set <Instruction*> m_InstSet;

    ModuleManage *m_Ms;

    Source *m_Source;

    ExternalLib *ExtLib;

    set <Function *> m_RunStack;

    map<Value*, Value*> m_EqualVal;

    StField *m_Sf;

public:
    
    Lda(ModuleManage *Ms, Source *S, StField *Sf)
    {
        m_Ms  = Ms;
        m_Sf  = Sf;
        m_Source = S;

        m_Fts = new Fts (Ms);
        assert (m_Fts != NULL);

        ExtLib = new ExternalLib ();
        assert (ExtLib != NULL);

        Compute ();

        printf ("\r\n#m_InstSet = %u \r\n", (unsigned)m_InstSet.size());
    }

    ~Lda ()
    {
        if (m_Fts)
        {
            delete m_Fts;
            m_Fts = NULL;
        }

        if (ExtLib)
        {
            delete ExtLib;
            ExtLib = NULL;
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

    inline Flda* GetFlda (Function *Func)
    {
        auto It = m_Func2Flda.find (Func);
        if (It == m_Func2Flda.end())
        {
            auto Pit = m_Func2Flda.insert (make_pair(Func, Flda (Func)));
            assert (Pit.second == true);

            return &(Pit.first->second);
        }
        else
        {
            return &(It->second);
        }
    }

    inline bool IsActiveFields (LLVMInst *Inst)
    {
        unsigned Index = 0;
        Value *Val = Inst->GetValue (0);
        
        Type *Ty = Val->getType ();
        if (!Ty->isPointerTy ())
        {
            return true;            
        }

        Type *Rty = cast<PointerType>(Ty)->getElementType();
        if (!Rty->isStructTy ())
        {
            return true;
        }

        Value *Ival = Inst->GetValue (2);
        if (ConstantInt* CI = dyn_cast<ConstantInt>(Ival)) 
        {
            Index = (unsigned)CI->getSExtValue();
        }
        else
        {
            return true;
        }

        return m_Sf->IsActiveFields (Rty->getStructName ().data(), Index);
    }

    inline void InitCriterions (Function *Func, unsigned TaintBit, set<Value*> *LexSet)
    {
        if (Func == m_Source->GetSrcCaller ())
        {
            for (auto It = m_Source->begin(), End = m_Source->end(); It != End; It++)
            {
                LexSet->insert (*It);
            }
        }
      
        unsigned BitNo = ARG0_NO;
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

    inline unsigned CheckOutArgTaint (Function *Func, Value *Val)
    {
        unsigned BitNo = ARG0_NO;
        for (auto Ita = Func->arg_begin(); Ita != Func->arg_end(); Ita++, BitNo++) 
        {
            Argument *Formal = &(*Ita);

            if (Formal == Val)
            {
                return TAINT_BIT(BitNo);
            }
        }

        return TAINT_NONE;
    }

    inline void AddTaintValue (set<Value*> *LexSet, Value *Val)
    {
        if (Val == NULL)
        {
            return;
        }

        auto It = m_EqualVal.find (Val);
        if (It != m_EqualVal.end ())
        {
            LexSet->insert (It->second);
        }
        else
        {
            LexSet->insert (Val);
        }

        return;
    }

    inline unsigned GetTaintedBits (LLVMInst *Inst, set<Value*> *LexSet)
    {
        unsigned TaintBit = 0;
        unsigned BitNo = ARG0_NO;
        for (auto It = Inst->begin (); It != Inst->end(); It++, BitNo++)
        {
            Value *U = *It;

            if (LexSet->find (U) != LexSet->end())
            {
                SET_TAINTED (TaintBit, BitNo);
            }
            else
            {
                auto  Eq = m_EqualVal.find (U);
                if ((Eq  != m_EqualVal.end ()) && (LexSet->find (Eq ->second) != LexSet->end()))
                {
                    SET_TAINTED (TaintBit, BitNo);
                }
            }
        }

        return TaintBit;
    }

    inline bool IsInStack (Function *Func)
    {
        auto It = m_RunStack.find (Func);
        if (It == m_RunStack.end())
        {
            return false;
        }

        return true;
    }

    inline void ExeFunction (LLVMInst *LI, Function *Callee, CSTaint *Cst, set<Value*> *LexSet)
    {
        unsigned FTaintBits;
        
        if (Callee->isDeclaration ())
        {
            FTaintBits = ExtLib->ComputeTaintBits (Callee->getName ().data(), Cst->m_InTaintBits);
            //printf ("[CALL Library] %s -> TaintBits = %#x \r\n", Callee->getName ().data(), FTaintBits);
        }
        else
        {
            if (!IsInStack (Callee))
            {
                FTaintBits = ComputeFlda (Callee, Cst->m_InTaintBits);
            }
            else
            {
                FTaintBits = Cst->m_InTaintBits;
            }
        }
        Cst->m_OutTaintBits = FTaintBits;

        /* actual arguments */       
        unsigned BitNo = ARG0_NO;
        for (auto It = LI->begin (); It != LI->end(); It++, BitNo++)
        {
            Value *U = *It;

            if (IS_TAINTED(FTaintBits, BitNo))
            {
                LexSet->insert (U);
            }
        }

        /* return value */
        if (IS_TAINTED(FTaintBits, RET_NO))
        {
            AddTaintValue (LexSet, LI->GetDef ());
        }

        return;
    }

    inline void ProcessCall (LLVMInst *LI, CSTaint *Cst, set<Value*> *LexSet)
    {
        Function *Callee = LI->GetCallee();
        if  (Callee != NULL)
        {   
            ExeFunction (LI, Callee, Cst, LexSet);
            Cst->m_Callees.insert(Callee);
        }
        else
        {
            //errs()<<"Indirect: "<<*(LI->GetInst ())<<"\r\n";
            FUNC_SET *Fset = m_Fts->GetCalleeFuncs (LI);
            assert (Fset != NULL);

            for (auto Fit = Fset->begin(), End = Fset->end(); Fit != End; Fit++)
            {
                Callee = *Fit;
                errs()<<"Indirect Function: "<<Callee->getName ()<<"\r\n";
                ExeFunction (LI, Callee, Cst, LexSet);
                Cst->m_Callees.insert(Callee);
            }
        }

        return;
    }

    inline unsigned ComputeFlda (Function *Func, unsigned FTaintBits)
    {
        set<Value*> LocalLexSet;

        m_RunStack.insert (Func);
        Flda *fd = GetFlda (Func);

        printf ("=>Entry %s : FTaintBits = %#x\r\n", Func->getName ().data(), FTaintBits);
        InitCriterions (Func, FTaintBits, &LocalLexSet);
        
        for (inst_iterator ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI) 
        {
            Instruction *Inst = &*ItI;
            
            LLVMInst LI (Inst);          
            if (LI.IsIntrinsic() || LI.IsUnReachable())
            {
                continue;
            }

            unsigned TaintedBits = GetTaintedBits (&LI, &LocalLexSet);
            if (LI.IsBitCast())
            {
                Value *Def = LI.GetDef ();
                Value *Use = LI.GetValue (0);
                m_EqualVal[Def] = Use;
                continue;
            }
            
            if (TaintedBits == 0)
            {
                if (!LI.IsCall ())
                {
                    continue;
                }

                CSTaint *Cst = fd->InsertCst (Inst, TaintedBits);
                assert (Cst != NULL);
                
                ProcessCall (&LI, Cst, &LocalLexSet);
            }
            else
            {
                switch (Inst->getOpcode ())
                {
                    case Instruction::Ret:
                    {
                        FTaintBits |= TAINT_BIT (RET_NO);
                        break;
                    }
                    case Instruction::Call:
                    case Instruction::Invoke:
                    {
                        CSTaint *Cst = fd->InsertCst (Inst, TaintedBits);
                        assert (Cst != NULL);
                        
                        ProcessCall (&LI, Cst, &LocalLexSet); 
                        break;
                    }
                    case Instruction::ICmp:
                    {
                        continue;
                    }
                    case Instruction::GetElementPtr:
                    {
                        if (!IsActiveFields (&LI))
                        {
                            continue;                            
                        }                 
                    }
                    default:
                    {
                        Value *Val = LI.GetDef ();
                        FTaintBits |= CheckOutArgTaint (Func, Val);

                        AddTaintValue (&LocalLexSet, Val);

                        if (LI.IsPHI ())
                        {
                            continue;
                        }

                        break;
                    }
                }

                m_InstSet.insert (Inst);
                fd->InsertInst (Inst);
                errs ()<<"\tTainted Inst: "<<*Inst<<"\r\n";
            }
        }

        printf ("=>Exit %s: FTaintBits = %#x\r\n", Func->getName ().data(), FTaintBits);

        m_RunStack.erase (Func);
        return FTaintBits;
    }

    inline void Compute ()
    {       
        Function *Entry = m_Ms->GetEntryFunction ();
        assert (Entry != NULL);

        ComputeFlda (Entry, TAINT_NONE);
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