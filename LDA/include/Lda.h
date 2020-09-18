//===-- Lda.h - lexical dependence analysis -------------------------------===//
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
#include "Source.h"
#include "ExternalLib.h"
#include "StField.h"
#include "LdaBin.h"
#include "Event.h"


using namespace llvm;
using namespace std;


typedef set<Instruction*>::iterator sii_iterator;
typedef map <Instruction*, unsigned long>::iterator miu_iterator;

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


typedef map <Instruction*, CSTaint>::iterator mic_iterator;


class Flda
{
private:
    unsigned m_FuncId;
    Function *m_CurFunc;
    map <Instruction*, CSTaint> m_CallSite2Cst;
    map <Instruction*, unsigned long> m_TaintInsts2ID;
    map <BasicBlock*, unsigned> m_BB2Id;

public:
    Flda (unsigned FuncId, Function *Func)
    {
        m_CurFunc = Func;
        m_FuncId  = FuncId;

        unsigned Id = 1;
        for (auto Bit = Func->begin(), Bend = Func->end(); Bit != Bend; ++Bit) 
        {
            m_BB2Id [&*Bit] = Id++;
        }
    }

    ~Flda ()
    {

    }

    inline char* GetName ()
    {
        return (char*)m_CurFunc->getName ().data();
    }

    
    inline unsigned GetFID ()
    {
        return m_FuncId;
    }

    inline unsigned long GetInstID (Instruction *Inst)
    {
        return m_TaintInsts2ID[Inst];
    }

    inline unsigned GetCINum ()
    {
        return (unsigned)m_CallSite2Cst.size();
    }

    inline unsigned GetTaintInstNum ()
    {
        return (unsigned)m_TaintInsts2ID.size();
    }

    inline miu_iterator inst_begin ()
    {
        return m_TaintInsts2ID.begin();
    }

    inline miu_iterator inst_end ()
    {
        return m_TaintInsts2ID.end();
    }

    inline void InsertInst (Instruction* TaintInst, unsigned InstID, unsigned long SSD)
    {
        unsigned long EventId = GetEventId (TaintInst, InstID, SSD);
        //printf ("---> %lx \r\n", EventId);
        m_TaintInsts2ID[TaintInst] = EventId;

        return;
    }

    inline mic_iterator ic_begin ()
    {
        return m_CallSite2Cst.begin();
    }

    inline mic_iterator ic_end ()
    {
        return m_CallSite2Cst.end();
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

private:
    inline unsigned GetBBId (Instruction* Inst)
    {
        auto It = m_BB2Id.find (Inst->getParent ());
        assert (It != m_BB2Id.end());

        return It->second;
    }

    inline unsigned long GetEventId(Instruction* Inst, unsigned InstID, unsigned long SSD)
    {
        /*
         Event Id definition:
         |4b language|4b type|2b soure/sink|18b FunctionId|12b Blockid|24b Instructionid|
        */
        
        unsigned long EventId = 0;

        unsigned long FID = m_FuncId;
        unsigned long BID = GetBBId (Inst);

        EventId = F_LANG2EID (CLANG_TY) | F_ETY2EID (EVENT_DF) | F_SSD2EID (SSD) |
                  F_FID2EID (FID) | F_BID2EID (BID) | F_IID2EID (InstID);
        
        return EventId;
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

    ComQueue<Function*> m_EntryFQ;

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

        Dump ();
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
    inline bool IsEntryFunc (Function *Callee)
    {
        if (Callee == NULL)
        {
            return false;
        }
        if (strcmp (Callee->getName().data(), "pthread_create") == 0)
        {
            return true;
        }

        return false;
    }

    inline Flda* GetFlda (Function *Func)
    {
        auto It = m_Func2Flda.find (Func);
        if (It == m_Func2Flda.end())
        {
            unsigned FuncId = m_Func2Flda.size () + 1;
            auto Pit = m_Func2Flda.insert (make_pair(Func, Flda (FuncId, Func)));
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


    inline void GetEntryFunction (Instruction *CallInst)
    {
        Value *Ef = CallInst->getOperand (2);
        assert (llvm::isa<llvm::Function>(Ef));
        //errs()<<"Type = "<<*Ef->getType ()<<", Name = "<<Ef->getName ()<<"\r\n";

        m_EntryFQ.InQueue ((Function*)Ef);
        
        return;
    }

    inline void ProcessCall (LLVMInst *LI, CSTaint *Cst, set<Value*> *LexSet)
    {
        Function *Callee = LI->GetCallee();
        if (IsEntryFunc (Callee))
        {
            GetEntryFunction (LI->GetInst ());
            return;
        }
        
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

        unsigned InstID = 1;
        for (inst_iterator ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI, InstID++) 
        {
            Instruction *Inst = &*ItI;
            
            LLVMInst LI (Inst);          
            if (LI.IsIntrinsic() || LI.IsUnReachable())
            {
                continue;
            }

            if (m_Source->IsSrcInst (Inst))
            {
                errs ()<<"Add Source: "<<*Inst<<"\r\n";
                m_InstSet.insert (Inst);
                fd->InsertInst (Inst, InstID, SOURCE_TY);
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
                fd->InsertInst (Inst, InstID, 0);
                errs ()<<"\t["<<InstID<<"]Tainted Inst: "<<*Inst<<"\r\n";
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

        m_EntryFQ.InQueue (Entry);
        while (!m_EntryFQ.IsEmpty ())
        {
            Entry = m_EntryFQ.OutQueue ();
            errs()<<"=====================> Process Entery Function: "<<Entry->getName ()<<" <====================\r\n";         
            
            ComputeFlda (Entry, TAINT_NONE);
        }     
    }


    /////////////////////////////////////////////////////////////////////
    //// LdaBin.bin
    /////////////////////////////////////////////////////////////////////
    ///  unsigned FuncNum;
	//   FldaBin[]
	//            char FuncName[F_NAME_LEN];
	//            unsigned TaintInstNum;
	//            unsigned TaintCINum;
	//            unsigned InstID[]
	//            unsigned TaintCI[] 
	//                              unsigned InstID;
	//                              unsigned InTaintBits;
	//                              unsigned OutTaintBits;
	//                              unsigned CalleeNum;
	//                              char FuncName[F_NAME_LEN][]
	/////////////////////////////////////////////////////////////////////

    inline void Dump ()
    {
        FILE *Bf = fopen ("LdaBin.bin", "wb");
        assert (Bf != NULL);

        LdaBin Lb;
        Lb.Version = 1;
        Lb.FuncNum = m_Func2Flda.size();
        printf ("FldaNum = %u \r\n", Lb.FuncNum);
        fwrite (&Lb, sizeof(Lb), 1, Bf);

        for (auto It = m_Func2Flda.begin (); It != m_Func2Flda.end(); It++)
        {
            Flda *Fd = &(It->second);
            
            FldaBin Fdb = {0};
            strcpy (Fdb.FuncName, Fd->GetName());
            Fdb.FuncId       = Fd->GetFID ();
            Fdb.TaintCINum   = Fd->GetCINum ();
            Fdb.TaintInstNum = Fd->GetTaintInstNum ();
            fwrite (&Fdb, sizeof(Fdb), 1, Bf);

            unsigned long *IID = new unsigned long [Fdb.TaintInstNum];
            assert (IID != NULL);
            unsigned Index = 0;
            for (auto Iit = Fd->inst_begin (); Iit != Fd->inst_end (); Iit++)
            {
                IID [Index++] = Iit->second;
            }
            fwrite (IID, sizeof(unsigned long), Index, Bf);
            delete IID;

            for (auto Cit = Fd->ic_begin (); Cit != Fd->ic_end (); Cit++)
            {
                CSTaint *Cst = &(Cit->second);

                CSTaintBin Cstb;
                Cstb.InstID = Fd->GetInstID (Cit->first);
                Cstb.InTaintBits  = Cst->m_InTaintBits;
                Cstb.OutTaintBits = Cst->m_OutTaintBits;
                Cstb.CalleeNum    = Cst->m_Callees.size();
                fwrite (&Cstb, sizeof(Cstb), 1, Bf);

                for (auto Fit = Cst->m_Callees.begin(); Fit != Cst->m_Callees.end(); Fit++)
                {
                    char CalleeName[F_NAME_LEN] = {0};
                    strcpy (CalleeName, (*Fit)->getName().data());
                    fwrite (CalleeName, sizeof(CalleeName), 1, Bf);
                }
            }
        }

        fclose (Bf);
    }  
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H