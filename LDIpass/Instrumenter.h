//===-- Instrumenter.h - instruction level instrumentation ----------------===//
//
// Copyright (C) <2019-2024>  <Wen Li>
//
//                      The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
#define LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
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
#include "HookFunc.h"
#include "LLVMInst.h"
#include "LdaBin.h"
#include "Event.h"


using namespace llvm;
using namespace std;

struct CSTaint
{
    CSTaint ()
    {
        m_InTaintBits  = 0;
        m_OutTaintBits = 0;
    }
    
    set <string> m_Callees;
    unsigned m_InTaintBits;
    unsigned m_OutTaintBits;

    inline string GetName ()
    {
        return *(m_Callees.begin());
    }
};

typedef map <unsigned, CSTaint>::iterator mcs_iterator;
class Flda
{
private:
    string m_FuncName;
    unsigned m_Id;
    map <unsigned long, CSTaint> m_CsID2Cst;
    map <unsigned, unsigned long> m_TaintInstID2EventID;
    set <unsigned long> m_TaintInstIDs;

public:
    Flda (string FuncName)
    {
        m_FuncName = FuncName;
    }

    ~Flda ()
    {

    }

    inline unsigned GetId ()
    {
        return m_Id;
    }

    inline void SetId (unsigned Id)
    {
        m_Id = Id;
    }

    inline string GetName ()
    {
        return m_FuncName;
    }

    inline CSTaint* GetCsTaint (unsigned InstId)
    {
        auto It = m_CsID2Cst.find (InstId);
        if (It == m_CsID2Cst.end ())
        {
            return NULL;
        }
        else
        {
            return &(It->second);
        }
    }

    inline CSTaint* AddCst (unsigned long EventId)
    {
        unsigned InstId = R_EID2IID(EventId);
        auto It = m_CsID2Cst.find (InstId);
        if (It == m_CsID2Cst.end())
        {
            auto Pit = m_CsID2Cst.insert (make_pair(InstId, CSTaint ()));
            assert (Pit.second == true);

            return &(Pit.first->second);
        }
        else
        {
            return &(It->second);
        }
    }

    inline bool IsInstTainted (unsigned InstId)
    {
        if (GetEventID (InstId) == 0)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    inline void AddInstID (unsigned long EventId)
    {
        unsigned InstId = R_EID2IID(EventId);
        m_TaintInstID2EventID[InstId] = EventId;

        return;
    }

    inline unsigned long GetEventID (unsigned    InstId)
    {
        auto It = m_TaintInstID2EventID.find (InstId);
        if (It == m_TaintInstID2EventID.end())
        {
            return 0;
        }
        else
        {
            return It->second;
        }
    }

    inline void SetEventType (unsigned    InstId, unsigned long Type)
    {
        unsigned long EId = GetEventID (InstId);
        assert (EId != 0);

        m_TaintInstID2EventID[InstId] = EId | F_ETY2EID (Type);

        return;
    }
};

class Instrumenter
{
private:
    Module *m_Module;

    Constant *m_TaceFunc;
    Constant *m_InitFunc;
    Constant *m_ExitFunc;

    map <string, Flda> m_Fname2Flda;

    map<StringRef, Value*> m_GlobalPtrMap;

    map <Value*, unsigned> m_Value2Id;

public:
    
    Instrumenter(Module *M)
    {
        m_Module = M;


        TraceFunc Trb;
        m_TaceFunc = Trb.geTraceFunction (M);
        assert (m_TaceFunc != NULL);
        m_InitFunc = Trb.geInitFunction (M);
        m_ExitFunc = Trb.geExitFunction (M);
    } 

    void RunInstrm ()
    {
        LoadLdaBin ();
 
        VisitFunction ();

        InitInstrumenter ();
    }
    
private:

    Value* GetGlobalPtr(StringRef strRef, IRBuilder<> *IRB)
    {
        auto It = m_GlobalPtrMap.find(strRef);
        if(It != m_GlobalPtrMap.end())
        {
            return It->second;
        }
        else
        {
            Value* VStr = IRB->CreateGlobalStringPtr(strRef);
            m_GlobalPtrMap[strRef] = VStr;
            return VStr;
        }
  }

    inline Flda* GetFlda (string Fname)
    {
        auto It = m_Fname2Flda.find (Fname);
        if (It == m_Fname2Flda.end())
        {
            return NULL;
        }
        else
        {
            return &(It->second);
        }
    }

    inline Flda* AddFlda (string Fname, unsigned Id)
    {
        Flda* Fd = GetFlda (Fname);
        if (Fd == NULL)
        {
            auto Pit = m_Fname2Flda.insert (make_pair(Fname, Flda (Fname)));
            assert (Pit.second == true);

            Fd = &(Pit.first->second);

            Fd->SetId (Id);
        }

        return Fd;
    }

    inline void LoadLdaBin ()
    {
        size_t N;
        FILE *Bf = fopen ("LdaBin.bin", "rb");
        assert (Bf != NULL);
        
        LdaBin Lb;
        N = fread (&Lb, sizeof(Lb), 1, Bf);
        assert (N == 1);
        printf ("FldaNum = %u \r\n", Lb.FuncNum);

        for (unsigned Fid = 0; Fid < Lb.FuncNum; Fid++)
        {
            FldaBin Fdb = {0};
            N = fread (&Fdb, sizeof(Fdb), 1, Bf);
            assert (N == 1);
            Flda *Fd = AddFlda (Fdb.FuncName, Fdb.FuncId);

            printf ("Load Function: %s\r\n", Fdb.FuncName);
            for (unsigned Iid = 0; Iid < Fdb.TaintInstNum; Iid++)
            {
                unsigned long Id = 0;
                N = fread (&Id, sizeof(Id), 1, Bf);
                assert (N == 1);
                Fd->AddInstID (Id);
                printf ("\tTaintInst:[%u] %lu\r\n", R_EID2IID (Id), Id);
            }        

            for (unsigned Cid = 0; Cid < Fdb.TaintCINum; Cid++)
            {
                CSTaintBin Cstb;
                N = fread (&Cstb, sizeof(Cstb), 1, Bf);
                assert (N == 1);

                CSTaint *Cst = Fd->AddCst (Cstb.InstID);
                Cst->m_InTaintBits  = Cstb.InTaintBits;
                Cst->m_OutTaintBits = Cstb.OutTaintBits;
                
                for (unsigned Fid = 0; Fid < Cstb.CalleeNum; Fid++)
                {
                    char CalleeName[F_NAME_LEN] = {0};
                    N = fread (CalleeName, sizeof(CalleeName), 1, Bf);
                    assert (N == 1);
                    Cst->m_Callees.insert (CalleeName);
                }
            }
        }

        fclose (Bf);
    }

    inline string GetValueType (Instruction* Inst, Value *Val)
    {
        unsigned VType = Val->getType ()->getTypeID ();
        switch (VType)
        {
            case Type::VoidTyID:
            {
                return "";
            }
            case Type::HalfTyID:
            {
                printf ("Type=> [HalfTyID]:%u \r\n", VType);
                break;
            }
            case Type::FloatTyID:
            {
                printf ("Type=> [FloatTyID]:%u \r\n", VType);
                break;
            }
            case Type::DoubleTyID:
            {
                printf ("Type=> [DoubleTyID]:%u \r\n", VType);
                break;
            }
            case Type::X86_FP80TyID:
            {
                printf ("Type=> [X86_FP80TyID]:%u \r\n", VType);
                break;
            }
            case Type::FP128TyID:
            {
                printf ("Type=> [FP128TyID]:%u \r\n", VType);
                break;
            }
            case Type::PPC_FP128TyID:
            {
                printf ("Type=> [PPC_FP128TyID]:%u \r\n", VType);
                break;
            }
            case Type::LabelTyID:
            {
                printf ("Type=> [LabelTyID]:%u \r\n", VType);
                break;
            }
            case Type::MetadataTyID:
            {
                printf ("Type=> [MetadataTyID]:%u \r\n", VType);
                break;
            }
            case Type::X86_MMXTyID:
            {
                printf ("Type=> [X86_MMXTyID]:%u \r\n", VType);
                break;
            }
            case Type::TokenTyID:
            {
                printf ("Type=> [TokenTyID]:%u \r\n", VType);
                break;
            }
            case Type::IntegerTyID:
            {
                return "U";
            }
            case Type::FunctionTyID:
            {
                printf ("Type=> [FunctionTyID]:%u \r\n", VType);
                break;
            }
            case Type::StructTyID:
            {
                printf ("Type=> [StructTyID]:%u \r\n", VType);
                break;
            }
            case Type::ArrayTyID:
            {
                printf ("Type=> [ArrayTyID]:%u \r\n", VType);
                break;
            }
            case Type::PointerTyID:
            {
                return "P";
            }
            case Type::VectorTyID:
            {
                printf ("Type=> [VectorTyID]:%u \r\n", VType);
                break;
            }
            default:
            {
                assert (0);
            }
        }

        return "";
    }

    inline string GetValueName (Value *Val)
    {
        if (Val->hasName ())
        {
            return Val->getName ().data();
        }
        
        unsigned Id;
        auto It = m_Value2Id.find (Val);
        if (It != m_Value2Id.end ())
        {
            Id = It->second;
        }
        else
        {
            Id = m_Value2Id.size()+1;
            m_Value2Id[Val] = Id;
        }

        return  "Val" + to_string(Id);
    }


    inline unsigned GetArgNo (unsigned TaintBit)
    {
        unsigned No = 1;
        while (TaintBit != 0)
        {
            TaintBit = TaintBit << 1;
            if (TaintBit & (1<<31))
            {
                return No;
            }

            No++;
        }

        return 0;
    }

    

    inline void GetInstrPara (Flda *Fd, unsigned InstId, Instruction* Inst, 
                                  string &Format, Value **ArgBuf, unsigned *ArgNum)
    { 
        Value *Def = NULL;
        unsigned ArgIndex = 0;
        
        LLVMInst LI (Inst);

        /*
        [Fid:Iid:0]def:value=use:value,use:value.....
        [Fid:Iid:1]func=def:use=use:value,use:value.....
        [Fid:Iid:2]ret=use:value
        */

        errs()<<*Inst<<"\r\n";
        
        Format = "{";
        if (LI.IsRet ())
        {
            Fd->SetEventType (InstId, EVENT_RET);
            Format += "R=";
        }
        else if (LI.IsCall ())
        {
            Fd->SetEventType (InstId, EVENT_CALL);
            CSTaint *Cst = Fd->GetCsTaint (InstId);
            if (Cst != NULL)
            {          
                Format += Cst->GetName () + ",";
                
                unsigned Outbits = (~Cst->m_InTaintBits) & Cst->m_OutTaintBits;
                unsigned OutArg  =  GetArgNo(Outbits);
                if (OutArg != 0)
                {
                    Def = LI.GetUse (OutArg-1);
                    assert (Def != NULL);
                        
                    ArgBuf [ArgIndex++] = Def;            
                    Format += GetValueName (Def) + ":" + GetValueType (Inst, Def) + "=";
                }
            }
            else
            {
                Format += LI.GetCallName () + ",";
                Def = LI.GetDef ();
                if (Def != NULL)
                {
                    ArgBuf [ArgIndex++] = Def;            
                    Format += GetValueName (Def) + ":" + GetValueType (Inst, Def) + "=";
                }
            }
        }
        else
        {
            Fd->SetEventType (InstId, EVENT_NR);
            
            Def = LI.GetDef ();
            if (Def != NULL)
            {
                ArgBuf [ArgIndex++] = Def;            
                Format += GetValueName (Def) + ":" + GetValueType (Inst, Def) + "=";
            }
        }

        for (auto It = LI.begin (); It != LI.end(); It++)
        {
            Value *Val = *It;
            if (Val == Def)
            {
                //continue;
            }
            
            ArgBuf [ArgIndex++] = Val;
            Format += GetValueName (Val) + ":" + GetValueType (Inst, Val);

            if ((It+1) != LI.end())
            {
                Format += ",";
            }
        }
        Format += "}";
        *ArgNum = ArgIndex;

        errs()<<"\tTrg: "<<Format<<", ArgNum="<<*ArgNum<<"\r\n";
  
        return;
    }

    inline void AddTrack (unsigned long EventId, Instruction* Inst, 
                             string Format, Value **ArgBuf, unsigned ArgNum)
    { 
        IRBuilder<> Builder(Inst);

        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, EventId, false);

        Value *TFormat = GetGlobalPtr(Format, &Builder);
        switch (ArgNum)
        {
            case 1:
            {
                Builder.CreateCall(m_TaceFunc, {Ev, TFormat, ArgBuf[0]});
                break;
            }
            case 2:
            {
                Builder.CreateCall(m_TaceFunc, {Ev, TFormat, ArgBuf[0], ArgBuf[1]});
                break;
            }
            case 3:
            {
                Builder.CreateCall(m_TaceFunc, {Ev, TFormat, ArgBuf[0], ArgBuf[1], ArgBuf[2]});
                break;
            }
            default:
            {
                errs()<<*Inst;
                printf ("ArgNum = %u\r\n", ArgNum);
                assert (0);
            }
        }

        return;
    }


    inline void AddCallTrack (Function* F, Flda *Fd)
    {
        BasicBlock *entryBlock = &F->front();
        Instruction *Site = entryBlock->getFirstNonPHI();
        
        IRBuilder<> Builder(Site);

        unsigned long EventId = F_LANG2EID (CLANG_TY) | F_ETY2EID (EVENT_FENTRY);
        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, EventId, false);

        /*
        FunctionName
        */
        string Format = "{" + Fd->GetName () + "}";
        Value *TFormat = GetGlobalPtr(Format, &Builder);

        Builder.CreateCall(m_TaceFunc, {Ev, TFormat});

        return;
    }

    inline void VisitFunction ()
    {
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *F = &*it;  
            if (F->isIntrinsic() || F->isDeclaration ())
            {
                continue;
            }

            Flda *Fd = GetFlda (F->getName ().data());
            if (Fd == NULL)
            {
                continue;
            }

            m_Value2Id.clear ();
            errs()<<"Process Function: "<<F->getName ()<<"\r\n";

            string Format = "";
            Value *ArgBuf[4];
            unsigned ArgNum = 0;
            unsigned InstId = 1;
            for (auto ItI = inst_begin(*F), Ed = inst_end(*F); ItI != Ed; ++ItI, InstId++) 
            {
                Instruction *Inst = &*ItI;

                if (Format != "")
                {
                    unsigned long EventId = Fd->GetEventID (InstId-1);
                    AddTrack (EventId, Inst, Format, ArgBuf, ArgNum);
                    Format = "";
                }
                
                if (Fd->IsInstTainted (InstId))
                {
                    GetInstrPara (Fd, InstId, Inst, Format, ArgBuf, &ArgNum);
                }
            }

            AddCallTrack (F, Fd);
        }

        return;
    }
    
    inline void InitInstrumenter ()
    {
        Function *mainFunc     = m_Module->getFunction("main");
        if (NULL == mainFunc)
        {
            return;
        }
                
        BasicBlock *entryBlock = &mainFunc->front();
        CallInst::Create(m_InitFunc, "", entryBlock->getFirstNonPHI());
        return;
    }
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
