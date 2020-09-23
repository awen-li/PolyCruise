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

    map <Value*, unsigned> m_Value2Id;

    set<Instruction *> m_ExitInsts;

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

            printf ("Load Function: %s, FID = %u\r\n", Fdb.FuncName, Fdb.FuncId);
            for (unsigned Iid = 0; Iid < Fdb.TaintInstNum; Iid++)
            {
                unsigned long Id = 0;
                N = fread (&Id, sizeof(Id), 1, Bf);
                assert (N == 1);
                Fd->AddInstID (Id);
                printf ("\tTaintInst:[%u] %lx\r\n", R_EID2IID (Id), Id);
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

    inline bool AddVarFormat (string &Format, Value *Val)
    {
        unsigned VType = Val->getType ()->getTypeID ();
        switch (VType)
        {
            case Type::IntegerTyID:
            {
                Format += GetValueName (Val) + ":" + "U";
                return false;
            }
            case Type::PointerTyID:
            {
                Format += "%lX:P";
                return true;
            }
            case Type::VoidTyID:
            case Type::HalfTyID:
            case Type::FloatTyID:
            case Type::DoubleTyID:
            case Type::X86_FP80TyID:
            case Type::FP128TyID:
            case Type::PPC_FP128TyID:
            case Type::LabelTyID:
            case Type::MetadataTyID:
            case Type::X86_MMXTyID:
            case Type::TokenTyID:           
            case Type::FunctionTyID:
            case Type::StructTyID:
            case Type::ArrayTyID:
            case Type::VectorTyID:
            default:
            {
                printf ("Type=>%u, Not support\r\n", VType);
                assert (0);
            }
        }

        return false;        
        
    }
   

    inline void GetInstrPara (Flda *Fd, unsigned InstId, Instruction* Inst, 
                                  string &Format, Value **ArgBuf, unsigned *ArgNum)
    { 
        Value *Def = NULL;
        unsigned ArgIndex = 0;
        
        LLVMInst LI (Inst);

        /*
        def:value=use:value,use:value.....
        func,def:use=use:value,use:value.....
        ret=use:value
        */

        errs()<<*Inst<<"\r\n";
        
        Format = "{";
        if (LI.IsRet ())
        {
            Fd->SetEventType (InstId, EVENT_RET);
        }
        else if (LI.IsCall ())
        {
            string Callee = LI.GetCallName ();
            if (Callee != "")
            {
                Format += Callee + ",";
            }
            
            Fd->SetEventType (InstId, EVENT_CALL);
            CSTaint *Cst = Fd->GetCsTaint (InstId);
            if (Cst != NULL)
            {
                unsigned OutArg = (~Cst->m_InTaintBits) & Cst->m_OutTaintBits;

                printf ("[in:out] = [%x, %x], OutArg = %x \r\n", 
                        Cst->m_InTaintBits, Cst->m_OutTaintBits, OutArg);

                // ret tatinted
                if (OutArg & (1<<31))
                {
                    Def = LI.GetDef ();             
                    if (AddVarFormat (Format, Def))
                    {
                        ArgBuf [ArgIndex++] = Def;
                    }

                    OutArg = OutArg << 1;
                    if (OutArg == 0)
                    {
                        Format += "=";
                    }
                    else
                    {
                        Format += ",";
                    }

                    printf ("Ret tainted, Now OutArg = %x \r\n", OutArg);
                }
                else
                {
                    // skip the ret bit
                    OutArg = OutArg << 1;
                }
                
                unsigned No = 1;
                bool ArgFlg = false;
                while (OutArg != 0)
                { 
                    if (OutArg & (1<<31))
                    {
                        printf ("OutArg = %x, No = %u \r\n", OutArg, No);
                        Def = LI.GetUse (No-1);
                        assert (Def != NULL);
                            
                        if (AddVarFormat (Format, Def))
                        {
                            ArgBuf [ArgIndex++] = Def; 
                        }

                        ArgFlg = true;
                    }

                    OutArg = OutArg << 1;
                    No++;
                    if (ArgFlg)
                    {
                        if (OutArg != 0)
                        {
                            Format += ",";
                        }
                        else
                        {
                            Format += "=";
                        }
                    }
                }   
            }
            else
            {
                Def = LI.GetDef ();
                if (Def != NULL)
                {
                    if (AddVarFormat (Format, Def))
                    {
                        ArgBuf [ArgIndex++] = Def; 
                    }
                    Format += "=";
                }
            }
        }
        else
        {
            Fd->SetEventType (InstId, EVENT_NR);
            
            Def = LI.GetDef ();
            if (Def != NULL)
            {
                if (AddVarFormat (Format, Def))
                {
                    ArgBuf [ArgIndex++] = Def; 
                }
                Format += "=";
            }
        }

        for (auto It = LI.begin (); It != LI.end(); It++)
        {
            Value *Val = *It;
            if (LI.IsConst (Val))
            {
                continue;
            }

            //errs()<<"visit use:" <<Val->getName ()<<"\r\n";
            if (AddVarFormat (Format, Val))
            {
                ArgBuf [ArgIndex++] = Val; 
            }

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

        Value *TFormat = Builder.CreateGlobalStringPtr(Format);
        switch (ArgNum)
        {
            case 0:
            {
                Builder.CreateCall(m_TaceFunc, {Ev, TFormat});
                break;
            }
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
            }
        }

        return;
    }


    inline void AddCallTrack (Function* F, Flda *Fd)
    {
        BasicBlock *entryBlock = &F->front();
        Instruction *Site = entryBlock->getFirstNonPHI();
        
        IRBuilder<> Builder(Site);

        unsigned long FID = Fd->GetId (); 
        unsigned long EventId = F_LANG2EID (CLANG_TY) | 
                                F_ETY2EID (EVENT_FENTRY) |
                                F_FID2EID (FID);
        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, EventId, false);

        /*
        FunctionName
        */
        string Format = "{" + Fd->GetName () + "}";
        Value *TFormat = Builder.CreateGlobalStringPtr(Format);

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

                GetProgramExitInsts (Inst);

                if (Format != "")
                {
                    unsigned long EventId = Fd->GetEventID (InstId-1);
                    AddTrack (EventId, Inst, Format, ArgBuf, ArgNum);
                    Format = "";
                    memset (ArgBuf, 0, sizeof (ArgBuf));
                    ArgNum = 0;
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

    inline void GetTermInstofFunction(Function *Func) 
    {
        BasicBlock &termbBlock = Func->back();
        Instruction *retInst   = termbBlock.getTerminator();

        if (!isa<ReturnInst>(retInst) &&
            !isa<UnreachableInst>(retInst))
        {
            return;
        }

        m_ExitInsts.insert(retInst);
        return;
    }

    void GetProgramExitInsts(Instruction *Inst) 
    {
        CallInst *Ci = dyn_cast<CallInst>(Inst);
        if (Ci == NULL)
        {
            return;
        }
            
        Function *CalledFunc = Ci->getCalledFunction();
        if (CalledFunc == NULL || !CalledFunc->hasName())
        {
            return;
        }
        
        if (CalledFunc->getName().str() == "exit") 
        {
            m_ExitInsts.insert(Inst);
        }     

        return;
    }
    
    inline void InitInstrumenter ()
    {
        Function *mainFunc = m_Module->getFunction("main");
        if (NULL == mainFunc)
        {
            return;
        }
                
        BasicBlock *entryBlock = &mainFunc->front();
        CallInst::Create(m_InitFunc, "", entryBlock->getFirstNonPHI());

        GetTermInstofFunction(mainFunc);
        for (auto it = m_ExitInsts.begin(); it != m_ExitInsts.end(); ++it) 
        {
            Instruction *Inst = *it;
            CallInst::Create(m_ExitFunc, "", Inst);
        }
        return;
    }
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
