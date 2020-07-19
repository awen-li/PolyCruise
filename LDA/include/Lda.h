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
#include "common/WorkList.h"
#include "ExternalLib.h"
#include "StField.h"


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
    Fts *m_Fts;
    map <Function*, unsigned> m_FTaintBits;
    
    set<Value*> m_GlobalLexSet;
    
    set <Instruction*> m_InstSet;

    ModuleManage *m_Ms;
    ComQueue <Function *> m_FuncList;

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

    inline void ExeFunction (LLVMInst *LI, Function *Callee, unsigned TaintBits, set<Value*> *LexSet)
    {
        unsigned FTaintBits;
        
        if (Callee->isDeclaration ())
        {
            FTaintBits = ExtLib->ComputeTaintBits (Callee->getName ().data(), TaintBits);
            //printf ("[CALL Library] %s -> TaintBits = %#x \r\n", Callee->getName ().data(), FTaintBits);
        }
        else
        {
            if (!IsInStack (Callee))
            {
                FTaintBits = Flda (Callee, TaintBits);
            }
            else
            {
                FTaintBits = TaintBits;
            }
        }      

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

    inline void ProcessCall (LLVMInst *LI, unsigned TaintBits, set<Value*> *LexSet)
    {
        Function *Callee = LI->GetCallee();
        if  (Callee != NULL)
        {
            ExeFunction (LI, Callee, TaintBits, LexSet);
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
                ExeFunction (LI, Callee, TaintBits, LexSet);
            }
        }

        return;
    }

    inline unsigned Flda (Function *Func, unsigned FTaintBits)
    {
        set<Value*> LocalLexSet;

        m_RunStack.insert (Func);

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

                ProcessCall (&LI, TaintedBits, &LocalLexSet);
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
                        ProcessCall (&LI, TaintedBits, &LocalLexSet); 
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

        Flda (Entry, TAINT_NONE);
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