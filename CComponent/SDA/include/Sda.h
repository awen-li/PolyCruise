//===-- Sda.h - lexical dependence analysis -------------------------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SDA_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_SDA_H
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
#include "SdaBin.h"
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


class FSda
{
private:
    unsigned m_InstNum;
    unsigned m_FuncId;
    Function *m_CurFunc;
    map <Instruction*, CSTaint> m_CallSite2Cst;
    map <Instruction*, unsigned long> m_TaintInsts2ID;
    map <BasicBlock*, unsigned> m_BB2Id;

public:
    FSda (unsigned FuncId, Function *Func)
    {
        m_CurFunc = Func;
        m_FuncId  = FuncId;

        unsigned Id = 1;
        for (auto Bit = Func->begin(), Bend = Func->end(); Bit != Bend; ++Bit) 
        {
            m_BB2Id [&*Bit] = Id++;
        }
    }

    ~FSda ()
    {

    }

    inline void SetInstNum (unsigned InstNum)
    {
        m_InstNum = InstNum;
    }

    inline unsigned GetInstNum ()
    {
        return m_InstNum;
    }

    inline char* GetName ()
    {
        return (char*)m_CurFunc->getName ().data();
    }

    inline Function* GetFunc ()
    {
        return m_CurFunc;
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
            CSTaint *Cst = &(It->second);
            Cst->m_InTaintBits |= InTaintBits;
            return Cst;
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

class Sda
{
private:
    Fts *m_Fts;

    map <Function*, FSda> m_Func2Fsda;    
    set <Instruction*> m_InstSet;

    ModuleManage *m_Ms;

    set<Source *> *m_Source;

    ExternalLib *ExtLib;

    set<Function *> m_RunStack;
    Function *m_CurEntry;

    map<Function*, unsigned> m_EntryExeNum;
    map<Function*, set<Value*>> m_Entry2GlvUse;
    map<Value *, set<Function*>> m_GlvUse2Entry;

    map<Value*, Value*> m_EqualVal;

    StField *m_Sf;

    ComQueue<Function*> m_EntryFQ;
    map<Function*, unsigned> m_EntryTaintBits;

    set<Value *> m_GlvLexset;
    map<Value *, Value *> m_GlvAlias;

public:
    
    Sda(ModuleManage *Ms, set<Source *> *SS, StField *Sf)
    {
        m_Ms  = Ms;
        m_Sf  = Sf;
        
        m_Source = SS;

        m_Fts = new Fts (Ms);
        assert (m_Fts != NULL);

        ExtLib = new ExternalLib ();
        assert (ExtLib != NULL);

        m_CurEntry = NULL;
        InitGlv ();
    }

    ~Sda ()
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

    inline void AddEntry (Function *Entry)
    {
        m_EntryFQ.InQueue (Entry);
        unsigned ArgNum = Entry->arg_size ();
        unsigned TaintBits = 0;
        for (unsigned bit = 2; bit <= ArgNum+1; bit++) 
        {
            TaintBits |= TAINT_BIT (bit);
        }
        m_EntryTaintBits[Entry] = TaintBits;
        return;
    }

    inline void Compute ()
    {      
        Function *MainEntry = m_Ms->GetEntryFunction ();
        if (MainEntry != NULL)
        {
            m_EntryFQ.InQueue (MainEntry);
        }
        
        while (!m_EntryFQ.IsEmpty ())
        {
            m_CurEntry = m_EntryFQ.OutQueue ();
            if (GetEntryExeNum (m_CurEntry) > m_Entry2GlvUse[m_CurEntry].size ()+1)
            {
                continue;
            }
            
            
            errs()<<"=====================> Process Entery Function: "<<m_CurEntry->getName ()<<" <====================\r\n";         
            unsigned TaintBits = GetEntryTaintbits (m_CurEntry);
            printf ("IN TaintBits = %x\r\n", TaintBits);
            
            /* update entry tainted bits */
            TaintBits = ComputeFlda (m_CurEntry, TaintBits);
            m_EntryTaintBits [m_CurEntry] = TaintBits; 

            UpdateEntryExeNum (m_CurEntry);
            printf ("OUT TaintBits = %x\r\n", TaintBits);
        }

        Dump ();
        printf ("\r\n#m_InstSet = %u \r\n", (unsigned)m_InstSet.size());
    }
    
private:
    inline unsigned GetEntryTaintbits (Function *Entry)
    {
        /* from the taint bits map */
        auto It = m_EntryTaintBits.find (Entry);
        if (It != m_EntryTaintBits.end ())
        {
            return It->second;
        }

        /* check the para whether it is a tainted share variable */
        for (auto fItr = Entry->arg_begin(); fItr != Entry->arg_end() ; ++fItr) 
        {
            Argument *Formal = &*fItr;
            Value *Glv = IsInGlvSet (Formal);
            if (Glv == NULL)
            {
                continue;
            }

            auto It = m_GlvLexset.find (Glv);
            if (It != m_GlvLexset.end ())
            {
                return TAINT_ARG0;
            }
        }

        return TAINT_NONE;
    }
    
    inline unsigned GetEntryExeNum (Function *Entry)
    {
        auto It = m_EntryExeNum.find (Entry);
        if (It == m_EntryExeNum.end ())
        {
            return 0;
        }
        else
        {
            return It->second;
        }
    }

    inline void UpdateEntryExeNum (Function *Entry)
    {
        auto It = m_EntryExeNum.find (Entry);
        if (It == m_EntryExeNum.end ())
        {
            m_EntryExeNum [Entry] = 1;
        }
        else
        {
            It->second += 1;
        }
    }
    
    inline bool IsGlobalValue (Value *Val)
    {
        GlobalValue *GVal = dyn_cast<GlobalValue>(Val);

        return (GVal != NULL);
    }

    inline bool IsConstStr (Value* Glv)
    {
        #define CONST_STR (".str")
        const char *ValueName = Glv->getName ().data();
        if (strncmp (CONST_STR, ValueName, sizeof(CONST_STR)-1) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    inline Value* IsInGlvSet (Value *Val)
    {
        auto It = m_GlvAlias.find (Val);
        if (It != m_GlvAlias.end ())
        {
            return It->second;
        }
        else
        {
            return NULL;
        }
    }

    inline void InitGlv ()
    {
        /* global variables */
        Value *Glv;
        for (auto It = m_Ms->global_begin (); It != m_Ms->global_end (); It++) 
        {
            Glv = *It;
            if (IsConstStr (Glv))
            {
                continue;
            }

            m_GlvAlias[Glv] = Glv;
        }


        /* share variables */
        for (auto It = m_Ms->func_begin (); It != m_Ms->func_end (); It++)
        {
            Function *Func  = *It;
            if (Func->isDeclaration() || Func->isIntrinsic())
            {
                continue;
            }

            for (inst_iterator itr = inst_begin(*Func), ite = inst_end(*Func); itr != ite; ++itr) 
            {
                Instruction *Inst = &*itr.getInstructionIterator();
                LLVMInst LI (Inst);
                if (!IsEntryFunc (LI.GetCallee()))
                {
                    continue;
                }
        
                errs ()<<*Inst<<"\r\n";

                /* relate the actual and formal parameter, treat them as global variables */
                Function *Entry = (Function *)Inst->getOperand (2);
                Value *InPara = Inst->getOperand (3);
                m_GlvAlias[InPara] = InPara;
                errs()<<"add actual para => "<<InPara<<"\r\n";

                /* get formal para */
                for (auto fItr = Entry->arg_begin(); fItr != Entry->arg_end() ; ++fItr) 
                {
                    Argument *Formal = &*fItr;
                    m_GlvAlias[Formal] = InPara;
                    errs()<<"add formal para => "<<Formal<<" mapping to "<<InPara<<"\r\n";
                }
        
            }
        }

        errs ()<<"Global Value Num: "<<m_GlvAlias.size()<<"\r\n";
    }

    inline void AddGlvUseEntry (Value *Glv)
    {
        auto GlvIt = m_GlvUse2Entry.find (Glv);
        if (GlvIt == m_GlvUse2Entry.end ())
        {
            return;
        }

        set<Function *> *EntrySet = &(GlvIt->second);
        for (auto It = EntrySet->begin (); It != EntrySet->end (); It++)
        {
            Function *Entry = *It;
            if (Entry == m_CurEntry)
            {
                continue;
            }

            DWORD ExeNum = GetEntryExeNum (Entry);       
            if (ExeNum > m_Entry2GlvUse[Entry].size ())
            {  
                //errs ()<<"****** Entry: "<<Entry->getName()<<" execute number: "<<GetEntryExeNum (Entry)<<"\r\n";
                continue;
            }

            if (m_EntryFQ.InQueue (Entry))
            {
                errs ()<<"===> Add Entry: "<<Entry->getName()<<" Use Glv: "<<Glv->getName ()<<"\r\n";
            }
        }

        return;
    }
    
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

    inline FSda* GetFlda (Function *Func)
    {
        auto It = m_Func2Fsda.find (Func);
        if (It == m_Func2Fsda.end())
        {
            unsigned FuncId = m_Func2Fsda.size () + 1;
            auto Pit = m_Func2Fsda.insert (make_pair(Func, FSda (FuncId, Func)));
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

        if (Inst->GetInst()->getNumOperands() <= 2)
        {
            return true;
        }

        Value *Ival = Inst->GetValue (2);
        //errs()<<*(Inst->GetInst())<<" -> OpNum:"<<<<" -> Val: "<<Ival<<"\r\n";
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
        if (m_Source != NULL)
        {
            for (auto ItS = m_Source->begin(), EndS = m_Source->end(); ItS != EndS; ItS++)
            {
                Source *S = *ItS;
                if (Func == S->GetSrcCaller ())
                {
                    for (auto It = S->begin(), End = S->end(); It != End; It++)
                    {
                        LexSet->insert (*It);
                    }
                }
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

            /* 1. search local lex set */
            if (LexSet->find (U) != LexSet->end())
            {
                SET_TAINTED (TaintBit, BitNo);
            }
            else
            {
                /* 2. search global lex set */
                if (m_GlvLexset.find (U) != m_GlvLexset.end())
                {
                    SET_TAINTED (TaintBit, BitNo);
                }

                /* 3. search equal lex set */
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

    inline unsigned DefaultTaints (LLVMInst *LI, Function *Func, unsigned InTaintBits)
    {
        unsigned TaintBits = 0;

        if (InTaintBits == 0)
        {
            return TAINT_NONE;
        }

        if (Func != NULL && !Func->getReturnType ()->isVoidTy())
        {
            TaintBits |= TAINT_RET;
        }
        
        unsigned BitNo = ARG0_NO;
        for (auto It = LI->begin (); It != LI->end(); It++, BitNo++)
        {
            Value *U = *It;
            if (LI->IsConst (U))
            {
                continue;
            }

            TaintBits |= TAINT_BIT (BitNo);
        }
        return TaintBits;
    }

    inline void ExeFunction (LLVMInst *LI, Function *Callee, CSTaint *Cst, set<Value*> *LexSet)
    {
        unsigned FTaintBits;

        if (Callee == NULL)
        {
            FTaintBits = DefaultTaints (LI, NULL, Cst->m_InTaintBits);
        }
        else if (Callee->isDeclaration ())
        {
            FTaintBits = ExtLib->ComputeTaintBits (Callee->getName ().data(), Cst->m_InTaintBits);
            if (FTaintBits == TAINT_UNKNOWN)
            {
                FTaintBits = DefaultTaints (LI, Callee, Cst->m_InTaintBits);
                printf ("[CALL Library] %s -> IN:%#x, TaintBits unknown, GetDefault: %#x\r\n", 
                        Callee->getName ().data(), Cst->m_InTaintBits, FTaintBits);
            }
            else
            {
                Cst->m_InTaintBits &= ~FTaintBits;
                FTaintBits |= Cst->m_InTaintBits; 
                printf ("[CALL Library] %s -> IN:%#x, TaintBits = %#x \r\n", 
                        Callee->getName ().data(), Cst->m_InTaintBits, FTaintBits);
            }
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


    inline VOID ProcEntry (Instruction *CallInst, CSTaint *Cst, set<Value*> *LexSet)
    {
        Value *Ef = CallInst->getOperand (2);
        assert (llvm::isa<llvm::Function>(Ef));
        
        Function *Entry = (Function *)Ef;
        m_EntryFQ.InQueue (Entry);
        if (Cst->m_InTaintBits != 0)
        {
            m_EntryTaintBits[Entry] = TAINT_ARG0;
            LexSet->insert (CallInst->getOperand (3));
        }
        else
        {
            unsigned EntryTaintBits = GetEntryTaintbits (Entry);
            if (EntryTaintBits != 0)
            {
                LexSet->insert (CallInst->getOperand (3));
            }
        }

        errs()<<"ProcEntry ===> Type = "<<*Ef->getType ()<<", Name = "<<Ef->getName ()
              <<" TaintBits = "<<m_EntryTaintBits[Entry]<<"\r\n";   

        return;
    }

    inline void ProcessCall (LLVMInst *LI, CSTaint *Cst, set<Value*> *LexSet)
    {
        Function *Callee = LI->GetCallee();
        if (IsEntryFunc (Callee))
        {
            ProcEntry (LI->GetInst (), Cst, LexSet);
            return;            
        }
        
        if  (Callee != NULL)
        {  
            ExeFunction (LI, Callee, Cst, LexSet);
            Cst->m_Callees.insert(Callee);
        }
        else
        {
            FUNC_SET *Fset = m_Fts->GetCalleeFuncs (LI);
            //assert (Fset != NULL);
            if (Fset == NULL)
            {
                ExeFunction (LI, NULL, Cst, LexSet);
                return;
            }

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


    inline bool IsSourceInst (Instruction *Inst)
    {
        if (m_Source == NULL)
        {
            return false;
        }
        
        for (auto ItS = m_Source->begin(), EndS = m_Source->end(); ItS != EndS; ItS++)
        {
            Source *S = *ItS;
            if (S->IsSrcInst (Inst))
            {
                return true;
            }
        }

        return false;
    }


    inline unsigned BackwardDeduce (FSda *Fd, unsigned FTaintBits, set<Value*> *LocalLexSet)
    {
        //printf ("=>BackwardDeduce %s: FTaintBits = %#x, InstNum = %u\r\n",
        //        Fd->GetName (), FTaintBits, Fd->GetInstNum ());
        unsigned InstID = Fd->GetInstNum ();
        Function *Func  = Fd->GetFunc ();
        inst_iterator ItI = inst_end(*Func);
        inst_iterator St  = inst_begin(*Func);
        for (ItI--; ItI != St; --ItI, InstID--) 
        {
            Instruction *Inst = &*ItI;

            LLVMInst LI (Inst);          
            if (LI.IsIntrinsic() || LI.IsUnReachable())
            {
                continue;
            }

            Value *Def = LI.GetDef ();
            auto It = LocalLexSet->find (Def);
            if (It == LocalLexSet->end ())
            {
                continue;
            }

            
            for (auto It = LI.begin (); It != LI.end (); It++)
            {
                Value *Val = *It;
                
                LocalLexSet->insert (Val);
                
                FTaintBits |= CheckOutArgTaint (Func, Val);        
            }

            auto ItTI = m_InstSet.find (Inst);
            if (ItTI  == m_InstSet.end ())
            {
                m_InstSet.insert (Inst);
                Fd->InsertInst (Inst, InstID, 0);
                //errs ()<<"["<<InstID<<"]BackwardDeduce -> "<<*Inst<<"\r\n";
            }
        }

        //printf ("=>BackwardDeduce %s exit\r\n", Fd->GetName ());

        return FTaintBits;
    }


    inline unsigned ForwardDeduce (FSda *Fd, unsigned FTaintBits, set<Value*> *LocalLexSet)
    {
        //printf ("=>ForwardDeduce Entry %s : FTaintBits = %#x\r\n", Fd->GetName (), FTaintBits);
        Function *Func = Fd->GetFunc ();
        InitCriterions (Func, FTaintBits, LocalLexSet);

        unsigned InstID = 1;
        for (inst_iterator ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI, InstID++) 
        {
            Instruction *Inst = &*ItI;

            LLVMInst LI (Inst);          
            if (LI.IsIntrinsic() || LI.IsUnReachable())
            {
                continue;
            }

            /* check all use */
            for (auto It = LI.begin (); It != LI.end (); It++)
            {
                Value *LV = IsInGlvSet (*It);
                if (LV  != NULL)
                {
                    m_GlvAlias [Inst] = LV;
                    m_GlvUse2Entry[LV].insert (m_CurEntry);
                    m_Entry2GlvUse[m_CurEntry].insert (LV);
                    //errs ()<<"Entry Function: "<<m_CurEntry->getName()
                    //       <<" Use Glv: "<<LV<<" - "<<LV->getName ()<<"\r\n";
                }
            }

            if (IsSourceInst (Inst))
            {
                errs ()<<"Add Source: "<<*Inst<<"\r\n";
                m_InstSet.insert (Inst);
                LocalLexSet->insert (Inst);
                Fd->InsertInst (Inst, InstID, SOURCE_TY);
                continue;
            }

            unsigned TaintedBits = GetTaintedBits (&LI, LocalLexSet);
            if (LI.IsBitCast())
            {
                Value *Def = LI.GetDef ();
                Value *Use = LI.GetValue (0);
                m_EqualVal[Def] = Use;

                Value *LV = IsInGlvSet (Use);
                if (LV != NULL)
                {
                    m_GlvAlias [Def] = LV;
                    //errs()<<*Inst<<" => "<<Def<<" to "<<LV<<"\r\n";
                }
                
                continue;
            }
            
            if (TaintedBits == 0)
            {
                if (!LI.IsCall ())
                {
                    continue;
                }

                CSTaint *Cst = Fd->InsertCst (Inst, TaintedBits);
                assert (Cst != NULL);

                unsigned TaintInstNum = m_InstSet.size ();
                ProcessCall (&LI, Cst, LocalLexSet);
                if (m_InstSet.size () > TaintInstNum &&
                    !IsEntryFunc (LI.GetCallee ()))
                {
                    m_InstSet.insert (Inst);
                    Fd->InsertInst (Inst, InstID, 0);
                }
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
                        CSTaint *Cst = Fd->InsertCst (Inst, TaintedBits);
                        assert (Cst != NULL);

                        ProcessCall (&LI, Cst, LocalLexSet); 
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

                        AddTaintValue (LocalLexSet, Val);

                        /* global taints */
                        Value *Glv = IsInGlvSet (Val);
                        if (Glv != NULL)
                        {
                            m_GlvLexset.insert (Glv);
                            AddGlvUseEntry (Glv);
                        }

                        if (LI.IsPHI ())
                        {
                            continue;
                        }

                        break;
                    }
                }

                if (IsEntryFunc (LI.GetCallee ()))
                {
                    continue;
                }

                m_InstSet.insert (Inst);
                Fd->InsertInst (Inst, InstID, 0);
                //errs ()<<"\t["<<InstID<<"]Tainted Inst: "<<*Inst<<"\r\n";
            }
        }

        Fd->SetInstNum (InstID-1);
        //printf ("=>ForwardDeduce Exit %s: FTaintBits = %#x\r\n", Fd->GetName (), FTaintBits);

        return FTaintBits;
    }


    inline unsigned ComputeFlda (Function *Func, unsigned FTaintBits)
    {
        unsigned Count = 0;
        set<Value*> LocalLexSet;
        FSda *Fd = GetFlda (Func);

        m_RunStack.insert (Func);

        while (1)
        {            
            /* forward execution */
            FTaintBits = ForwardDeduce (Fd, FTaintBits, &LocalLexSet); 
            unsigned FWTaintedNum = m_InstSet.size ();
                    
            /* deduce backward information */
            FTaintBits = BackwardDeduce (Fd, FTaintBits, &LocalLexSet);
            if (m_InstSet.size () == FWTaintedNum)
            {
                break;                
            }
       
            Count++;
        }
        
        m_RunStack.erase (Func);
        return FTaintBits;
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
        printf ("\r\n");
        printf ("************************************************************************\r\n");
        printf ("Start dump tainted instructions ...... \r\n");
        printf ("************************************************************************\r\n");
        FILE *Bf = fopen ("/tmp/difg/LdaBin.bin", "wb");
        FILE *BfTxt = fopen ("/tmp/difg/LdaBin.txt", "wb");
        assert (Bf != NULL);
        assert (BfTxt != NULL);

        LdaBin Lb;
        Lb.Version = 1;
        Lb.FuncNum = m_Func2Fsda.size();
        fprintf (BfTxt, "FldaNum = %u \r\n", Lb.FuncNum);
        fwrite (&Lb, sizeof(Lb), 1, Bf);

        unsigned TotalInstNum = 0;
        unsigned TaintInstNum = 0;
        for (auto It = m_Func2Fsda.begin (); It != m_Func2Fsda.end(); It++)
        {
            FSda *Fd = &(It->second);

            TotalInstNum += Fd->GetInstNum ();
            TaintInstNum += Fd->GetTaintInstNum ();
            
            FldaBin Fdb = {0};
            strcpy (Fdb.FuncName, Fd->GetName());
            Fdb.FuncId       = Fd->GetFID ();
            Fdb.TaintCINum   = Fd->GetCINum ();
            Fdb.TaintInstNum = Fd->GetTaintInstNum ();
            fwrite (&Fdb, sizeof(Fdb), 1, Bf);
            fprintf (BfTxt, "Function[%u, %s]: TaintInstNum:%u \r\n", 
                     Fd->GetFID (), Fd->GetName (), Fd->GetTaintInstNum ());
            printf ("Function[%u, %s]: TaintInstNum:%u \r\n", 
                     Fd->GetFID (), Fd->GetName (), Fd->GetTaintInstNum ());

            unsigned long *IID = new unsigned long [Fdb.TaintInstNum];
            assert (IID != NULL);
            unsigned Index = 0;
            for (auto Iit = Fd->inst_begin (); Iit != Fd->inst_end (); Iit++)
            {
                IID [Index++] = Iit->second;
                //errs ()<<"["<<(unsigned)R_EID2IID(Iit->second)<<"]"<<*Iit->first<<"\r\n";
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
                    fprintf (BfTxt, "\tCall %s: TaintBits[in, out]=[%x, %x] \r\n", 
                             CalleeName, Cst->m_InTaintBits, Cst->m_OutTaintBits);
                }
            }
        }

        fclose (Bf);
        fclose (BfTxt);
        printf ("@@@@@@@@@ Instrumentation rate: %0.2f [%u/%u]\r\n", 
                TaintInstNum*1.0/TotalInstNum, TaintInstNum, TotalInstNum);
    }  
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_LDA_H
