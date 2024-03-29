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
#include "SdaBin.h"
#include "Event.h"


using namespace llvm;
using namespace std;

#define IS_TAINTED(TaintBit, BitNo) (TaintBit & (1 << (32-BitNo)))


struct ParaFt
{
    string m_Format;
    vector<Value *>m_ArgBuf;
    unsigned m_ArgNum;

    inline void AppendFormat (string Ft)
    {
        unsigned Length = m_Format.length();
        if (Length > 0)
        {
            char LastChar   = m_Format.at(Length -1);
            if (Ft == "," && LastChar == ',')
            {
                return;
            }

            char *Last = &m_Format[Length-1];
            if (*Last == ',' && Ft == "}")
            {
                *Last = '}';
                return;
            }
        }

        m_Format += Ft; 

        return;
    }

    inline bool IsNameExist (string Name)
    {
        string::size_type Position = 0;
        unsigned Num = 0;
        while((Position = m_Format.find(Name, Position)) != string::npos)
        {
            Position++;
            Num++;
        }

        return (bool)(Num >= 2);
    }

    inline bool IsArgExist (Value *Arg)
    {
        unsigned Num = 0;
        for (unsigned Index = 0; Index < m_ArgNum; Index++)
        {
            if (m_ArgBuf [Index] == Arg)
            {
                Num++;
            }
        }

        if (Num >= 2)
        {
            return true;
        }

        return false;
    }

    inline void AddArg (Value *Arg)
    {
        m_ArgBuf.push_back (Arg);
        m_ArgNum++;
        
        return;
    }

    inline void Reset ()
    {
        m_Format = "";
        m_ArgNum = 0;
        m_ArgBuf.clear ();
    }
};

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
typedef map <unsigned, Instruction*>::iterator instid_iterator;
typedef set <unsigned>::iterator thr_iterator;

class FSda
{
private:
    string m_FuncName;
    unsigned m_Id;
    unsigned m_InstNum;
    unsigned m_BlockNum;

    map <unsigned long, CSTaint> m_CsID2Cst;
    map <unsigned, unsigned long> m_TaintInstID2EventID;
    map <unsigned, Instruction*> m_InstID2Inst;
    map <BasicBlock*, Instruction*> m_BB2Inst;
    
    set <unsigned> m_ThrCInstIDs;

public:
    FSda (string FuncName, unsigned Id, unsigned InstNum, unsigned BlockNum)
    {
        m_FuncName = FuncName;
        m_Id       = Id;
        m_InstNum  = InstNum;
        m_BlockNum = BlockNum;
    }

    ~FSda ()
    {

    }

    inline void InitBlocks (Function *F)
    {
        for (Function::iterator BB = F->begin(); BB != F->end(); ++BB) 
        {
            BasicBlock *CurBlock = &*BB;
            m_BB2Inst [CurBlock] = CurBlock->getFirstNonPHI();
        }
    }

    inline Instruction* GetFirstNonPHI (BasicBlock *BBlock)
    {
        auto It = m_BB2Inst.find (BBlock);
        assert (It != NULL);

        return It->second;
    }

    inline unsigned GetId ()
    {
        return m_Id;
    }

    inline unsigned GetInstNum ()
    {
        return m_InstNum;
    }

    inline unsigned GetBBNum ()
    {
        return m_BlockNum;
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

    inline unsigned long GetEventID (unsigned      InstId)
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

    inline void AddThrcInstId (unsigned InstID)
    {
        m_ThrCInstIDs.insert (InstID);

        return;
    }

    inline void AddInstMap (unsigned InstID, Instruction *Inst)
    {
        m_InstID2Inst [InstID] = Inst;

        return;
    }

    inline Instruction* GetInstById (unsigned InstID)
    {
        auto It = m_InstID2Inst.find (InstID);
        if (It == m_InstID2Inst.end())
        {
            return NULL;
        }

        return It->second;
    }

    inline instid_iterator tt_begin ()
    {
        return m_InstID2Inst.begin ();
    }

    inline instid_iterator tt_end ()
    {
        return m_InstID2Inst.end ();
    }

    inline thr_iterator thr_begin ()
    {
        return m_ThrCInstIDs.begin ();
    }

    inline thr_iterator thr_end ()
    {
        return m_ThrCInstIDs.end ();
    }
};

class Instrumenter
{
private:
    Module *m_Module;

    Constant *m_TaceFunc;
    Constant *m_InitFunc;
    Constant *m_ExitFunc;
    Constant *m_ThreadTc;

    Value *m_ThreadId;

    map <string, FSda> m_Fname2Flda;

    map <Value*, unsigned> m_Value2Id;

    set<Instruction *> m_ExitInsts;
public:
    
    Instrumenter(Module *M)
    {
        m_Module = M;


        TraceFunc Trb;
        m_TaceFunc = Trb.getTraceFunction (M);
        assert (m_TaceFunc != NULL);
        
        m_InitFunc = Trb.getInitFunction (M);
        assert (m_InitFunc != NULL);
        
        m_ExitFunc = Trb.getExitFunction (M);
        assert (m_ExitFunc != NULL);

        m_ThreadTc = Trb.getThreadTrace (M);
        assert (m_ThreadTc != NULL);

        m_ThreadId = NULL;
    } 

    unsigned RunInstrm (bool CallTrace=false)
    {
        unsigned InstNum = 0;

        if (CallTrace == false)
        {        
            LoadLdaBin ();
            InstNum += VisitFunction ();
        }
        else
        {
            InstNum += InstrmCallTrace ();
        }

        InstNum += InitInstrumenter ();
        return InstNum;
    }
    
private:

    inline FSda* GetFlda (string Fname)
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

    inline FSda* AddFlda (string Fname, unsigned Id, unsigned InstNum, unsigned BlockNum)
    {
        FSda* Fd = GetFlda (Fname);
        if (Fd == NULL)
        {
            auto Pit = m_Fname2Flda.insert (make_pair(Fname, FSda (Fname, Id, InstNum, BlockNum)));
            assert (Pit.second == true);

            Fd = &(Pit.first->second);
        }

        return Fd;
    }

    inline void LoadLdaBin ()
    {
        size_t N;
        FILE *Bf = fopen ("/tmp/difg/LdaBin.bin", "rb");
        assert (Bf != NULL);
        
        LdaBin Lb;
        N = fread (&Lb, sizeof(Lb), 1, Bf);
        if (N == 0)
        {
            printf ("LoadLdaBin: read ldabin error...\r\n");
            exit (0);
        }   

        char NameBuf[1024];
        for (unsigned Fid = 0; Fid < Lb.FuncNum; Fid++)
        {
            //printf ("Load Flda number = %u/%u \r", Fid, Lb.FuncNum);
            FldaBin Fdb = {0};
            N = fread (&Fdb, sizeof(Fdb), 1, Bf);
            assert (N == 1);
            N = fread (NameBuf, Fdb.NameLen, 1, Bf);
            NameBuf[Fdb.NameLen] = 0;
            assert (N == 1);
            FSda *Fd = AddFlda (NameBuf, Fdb.FuncId, Fdb.TotalInstNum, Fdb.BlockNum);

            //printf ("Load Function: %s, FID = %u, TaintInstNum:%u\r\n", Fdb.FuncName, Fdb.FuncId, Fdb.TaintInstNum);
            for (unsigned Iid = 0; Iid < Fdb.TaintInstNum; Iid++)
            {
                unsigned long Id = 0;
                N = fread (&Id, sizeof(Id), 1, Bf);
                assert (N == 1);
                
                Fd->AddInstID (Id);
                //printf ("\tTaintInst:[%u] %lx\r\n", R_EID2IID (Id), Id);
            }        

            for (unsigned Cid = 0; Cid < Fdb.TaintCINum; Cid++)
            {
                CSTaintBin Cstb;
                N = fread (&Cstb, sizeof(Cstb), 1, Bf);
                assert (N == 1);

                CSTaint *Cst = Fd->AddCst (Cstb.InstID);
                Cst->m_InTaintBits  = Cstb.InTaintBits;
                Cst->m_OutTaintBits = Cstb.OutTaintBits;

                unsigned NameLen = 0;
                for (unsigned Fid = 0; Fid < Cstb.CalleeNum; Fid++)
                {
                    N = fread (&NameLen, sizeof(NameLen), 1, Bf);
                    assert (N == 1 && NameLen != 0 && NameLen < sizeof (NameBuf));
                    
                    N = fread (NameBuf, NameLen, 1, Bf);
                    assert (N == 1);
                    NameBuf [NameLen] = 0;
                    Cst->m_Callees.insert (NameBuf);
                }
            }
        }

        printf ("Load Flda number = %u \r\n", Lb.FuncNum);

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

    inline bool IsGlv (Value *Val)
    {
        GlobalValue *GVal = dyn_cast<GlobalValue>(Val);

        return (GVal != NULL);
    }

    inline unsigned GetValueType (Value *Val)
    {
        return Val->getType ()->getTypeID ();
    }

    inline bool AddVarFormat (ParaFt *Pf, Value *Val)
    {
        if (Pf->IsArgExist(Val))
        {
            return false;
        }
        
        unsigned VType = GetValueType (Val);
        bool Glv = IsGlv (Val);
        
        switch (VType)
        {
            case Type::FloatTyID:
            case Type::DoubleTyID:
            case Type::IntegerTyID:
            {
                string Name;
                
                if (Glv)
                {
                    Name = GetValueName (Val) + ":" + "G";
                }
                else
                {
                    Name = GetValueName (Val) + ":" + "U";
                }

                if (!Pf->IsNameExist (Name))
                {           
                    Pf->AppendFormat (Name);
                }
                return false;
            }
            case Type::PointerTyID:
            {
                if (Glv)
                {
                    Pf->AppendFormat ("%lX:G");
                }
                else
                {
                    Pf->AppendFormat ("%lX:P");
                }
                
                return true;
            }
            case Type::VectorTyID:
            {
                break;
            }
            case Type::VoidTyID:
            case Type::HalfTyID:
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
            default:
            {
                //printf ("Type=>%u, Not support\r\n", VType);
                //assert (0);
            }
        }

        return false;        
        
    }


    inline unsigned SetEventType (FSda *Fd, unsigned InstId, LLVMInst *LI)
    {
        unsigned EventType = EVENT_NR;
        
        if (LI->IsGep ())
        {
            EventType = EVENT_GEP;            
        }
        else if (LI->IsAdd ())
        {
            EventType = EVENT_ADD;            
        }
        else if (LI->IsMul())
        {
            EventType = EVENT_MUL;            
        }
        else if (LI->IsDiv())
        {
            EventType = EVENT_DIV;            
        }
        else if (LI->IsRet ())
        {
            EventType = EVENT_RET;  
        }
        else if (LI->IsCall ())
        {
            EventType = EVENT_CALL;
        }
        else if (LI->IsBR())
        {
            EventType = EVENT_BR;  
        }
        else
        {
            EventType = EVENT_NR;
        }

        Fd->SetEventType (InstId, EventType);
        return EventType;    
    }


    inline void GetInstrPara (FSda *Fd, unsigned InstId, Instruction* Inst, ParaFt *Pf, set <Value*> *DefSets)
    { 
        Value *Def = NULL;

        LLVMInst LI (Inst);

        unsigned EventType = SetEventType (Fd, InstId, &LI);

        /*
        def:value=use:value,use:value.....
        func(x,y),def:use=use:value,use:value.....
        ret=use:value
        */
        
        Pf->AppendFormat ("{");
        if (EventType == EVENT_CALL)
        {
            string Callee = LI.GetCallName ();
            if (Callee != "")
            {
                Pf->AppendFormat (Callee);
            }
            else
            {
                Pf->AppendFormat ("PtsCall");
            }
            
            CSTaint *Cst = Fd->GetCsTaint (InstId);
            if (Cst != NULL)
            {
                Function *CallFunc = LI.GetCallee ();
                unsigned OutArg = (~Cst->m_InTaintBits) & Cst->m_OutTaintBits;
                if (CallFunc == NULL || CallFunc->isDeclaration ()) //if (OutArg == 0)
                {
                    OutArg = Cst->m_OutTaintBits;
                }

                //printf ("[in:out] = [%x, %x], OutArg = %x \r\n", 
                //        Cst->m_InTaintBits, Cst->m_OutTaintBits, OutArg);

                /* In parameters, get formal parameters */ 
                if (CallFunc != NULL && !CallFunc->isDeclaration () && Cst->m_InTaintBits != 0)
                {
                    Pf->AppendFormat ("(");
                    unsigned BitNo = 2; /* ARG0_NO */
                    unsigned ParaNum = 0;
                    for (auto Ita = CallFunc->arg_begin(); Ita != CallFunc->arg_end(); Ita++) 
                    {
                        Argument *Formal = &(*Ita);
                        if (!IS_TAINTED (Cst->m_InTaintBits, BitNo))
                        {
                            continue;
                        }

                        if (GetValueType (Formal) == Type::PointerTyID)
                        {
                            continue;
                        }

                        if (ParaNum > 0)
                        {
                            Pf->AppendFormat (",");
                        }

                        if (AddVarFormat (Pf, Formal))
                        {
                            
                            Pf->AddArg (Formal);
                            ParaNum++;
                        }
                        
                        BitNo++;
                    }
                    Pf->AppendFormat ("),");
                }
                else
                {
                    Pf->AppendFormat (",");
                }

                /* ret tatinted */
                Def = LI.GetDef ();
                if ((OutArg & (1<<31)) && (Def != NULL))
                {    
                    if (AddVarFormat (Pf, Def)) 
                    {
                        Pf->AddArg (Def);
                    }

                    OutArg = OutArg << 1;
                    if (OutArg == 0)
                    {
                        Pf->AppendFormat ("=");
                    }
                    else
                    {
                        Pf->AppendFormat (",");
                    }

                    //printf ("Ret tainted, Now OutArg = %x \r\n", OutArg);
                }
                else
                {
                    // skip the ret bit
                    OutArg = OutArg << 1;
                }

                /* Output parameters */
                unsigned No = 1;
                unsigned DefNo = 0;
                bool ArgFlg = false;
                while (OutArg != 0)
                { 
                    if (OutArg & (1<<31))
                    {
                        Def = LI.GetUse (No-1);
                        if (Def == NULL)
                        {
                            break;
                        }

                        if (DefNo > 0)
                        {
                            Pf->AppendFormat(",");
                        }

                        if (AddVarFormat (Pf, Def))
                        {
                            Pf->AddArg (Def);
                        }
                        
                        ArgFlg = true;
                        DefNo++;
                    }

                    OutArg = OutArg << 1;
                    No++;
                    if (ArgFlg && OutArg == 0)
                    {
                        Pf->AppendFormat("=");
                    }
                }   
            }
            else
            {
                Pf->AppendFormat (",");
                Def = LI.GetDef ();
                if (Def != NULL)
                {
                    if (AddVarFormat (Pf, Def))
                    {
                        Pf->AddArg (Def); 
                    }
                    Pf->AppendFormat ("=");
                }
            }
        }
        else
        {
            Def = LI.GetDef ();
            if (Def != NULL)
            {
                if (AddVarFormat (Pf, Def))
                {
                    Pf->AddArg (Def); 
                }
                Pf->AppendFormat ("=");
            }
        }

        for (auto It = LI.begin (); It != LI.end(); It++)
        {
            Value *Val = *It;
            if (LI.IsConst (Val))
            {
                continue;
            }

            //if (EventType != EVENT_CALL && DefSets->find (Val) == DefSets->end ())
            //{
            //    continue;
            //}

            //errs()<<"visit use:" <<Val->getName ()<<"\r\n";
            if (AddVarFormat (Pf, Val))
            {
                Pf->AddArg (Val); 
            }

            if ((It+1) != LI.end())
            {
                Pf->AppendFormat (",");
            }
        }
        Pf->AppendFormat ("}");

        //errs()<<"\tTrg: "<<Pf->m_Format<<", ArgNum="<<Pf->m_ArgNum<<"\r\n";
  
        return;
    }

    inline void AddTrack (unsigned long EventId, Instruction* Inst, ParaFt *Pf)
    { 
        IRBuilder<> Builder(Inst);

        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, EventId, false);
        Value *TFormat = Builder.CreateGlobalStringPtr(Pf->m_Format);

        std::vector<Value *> ValueAry;
        unsigned Index= 0;
        
        ValueAry.push_back (Ev);
        ValueAry.push_back (TFormat);
        while (Index < Pf->m_ArgNum)
        {
            ValueAry.push_back (Pf->m_ArgBuf[Index]);
            Index++;
        }
        Builder.CreateCall(m_TaceFunc, ValueAry);

        return;
    }


    inline void AddCallTrack (Function* F, FSda *Fd)
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

    inline Function* GetCallee(Instruction *Inst) 
    {
        if (!isa<CallInst>(Inst) && !isa<InvokeInst>(Inst))
        {
            return NULL;
        }
        
        CallSite Cs(const_cast<Instruction*>(Inst));
        
        return dyn_cast<Function>(Cs.getCalledValue()->stripPointerCasts());
    }


    inline bool IsThreadEntry (Instruction *Inst)
    {
        Function *Callee = GetCallee (Inst);
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
    
    inline void InstrumThrc (FSda *Fd, unsigned ThrInstId, Instruction* ThrcInst, Instruction *Site)
    {
        IRBuilder<> Builder(Site);

        /* pthread_create(&ID1, NULL, ...., .....) */
        Value *Ef = ThrcInst->getOperand (2);
        assert (llvm::isa<llvm::Function>(Ef));
        string ThrEntry = Ef->getName ().data();
        Value *ValThrEntry = Builder.CreateGlobalStringPtr(ThrEntry);

        Value *ThrID   = ThrcInst->getOperand (0);
        Value *ThrData = ThrcInst->getOperand (3);

        unsigned long FID = Fd->GetId (); 
        unsigned long EventId = F_LANG2EID (CLANG_TY) | 
                                F_ETY2EID (EVENT_THRC) |
                                F_FID2EID (FID) |
                                F_IID2EID (ThrInstId);
        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, EventId, false);
       
        Builder.CreateCall(m_ThreadTc, {Ev, ValThrEntry, ThrID, ThrData});
            
        return;
    }


    inline void VisitInst (Function *F, FSda *Fd)
    {
        unsigned InstId = 1;
        for (auto ItI = inst_begin(*F), Ed = inst_end(*F); ItI != Ed; ++ItI, InstId++) 
        {
            Instruction *Inst = &*ItI;
            Fd->AddInstMap (InstId, Inst);

            //errs ()<<"[*VisitInst*]-["<<InstId<<"] "<<*Inst<<"\r\n";

            if (IsThreadEntry (Inst))
            {
                Fd->AddThrcInstId (InstId);
            }

            GetProgramExitInsts (Inst);
        }

        Fd->InitBlocks(F);

        return;
    }

    inline bool ValidCheck (FSda *Fd, Function *Func)
    {   
        unsigned InstNum = 0;
        for (auto ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI, InstNum++){}
        unsigned BBNum = 0;
        for (Function::iterator BB = Func->begin(); BB != Func->end(); ++BB, BBNum++){} 

        if (Fd->GetInstNum() != InstNum || Fd->GetBBNum() != BBNum)
        {
            printf ("Warning - Function[%s] mismatch Inst (%u, %u), BB (%u, %u) \r\n",
                    Func->getName().data(),
                    Fd->GetInstNum(), InstNum, Fd->GetBBNum(), BBNum);
            return false;
        }
                
        return true;
    }

    inline void AddCallTrace (Function* F, BasicBlock *BBlock, string Tag)
    {
        Instruction *Site = BBlock->getFirstNonPHI();

        //errs()<<"@@@@@@@@@@@@@@@@@@ CallTrace: "<<Tag+string (F->getName())<<" ----- "<< *Site<<"\r\n";
        
        IRBuilder<> Builder(Site);
        string Format = Tag + string (F->getName().data());
        Value *TFormat = Builder.CreateGlobalStringPtr(Format);

        Type *I64ype = IntegerType::getInt64Ty(m_Module->getContext());
        Value *Ev = ConstantInt::get(I64ype, 0, false);

        Builder.CreateCall(m_TaceFunc, {Ev, TFormat});

        return;
    }

    inline unsigned InstrmCallTrace    ()
    {
        unsigned InstrumNum = 0;
        ParaFt Pf;
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *Func = &*it;
            if (Func->isIntrinsic() || Func->isDeclaration ())
            {
                continue;
            }

            for (auto ItI = inst_begin(*Func), Ed = inst_end(*Func); ItI != Ed; ++ItI) 
            {
                Instruction *Inst = &*ItI;
                LLVMInst LI (Inst);

                if (LI.IsInstrinsicDbgInst() || LI.IsUnReachable() || LI.IsAlloca ())
                {
                    continue;
                }

                if (LI.IsCall())
                {
                    Pf.Reset ();
                    
                    Pf.AppendFormat ("[C][CS]:");
                    string Callee = LI.GetCallName ();
                    if (Callee != "")
                    {
                        Pf.AppendFormat (Callee);
                    }
                    else
                    {
                        Pf.AppendFormat ("PtsCall");
                    }

                    AddTrack (0, Inst, &Pf);
                    //errs()<<"@@@@@@@@@@@@@@@@@@ CallInst: "<<Pf.m_ArgBuf<<" ----- "<< *Inst<<"\r\n";
                }
            }   

            AddCallTrace (Func, &Func->front(), "[C][IN]:");
            AddCallTrace (Func, &Func->back(), "[C][OUT]:");
            InstrumNum += 2;
        }  

        //errs ()<<"@@ InstrmCallTrace ....................END "<<"\r\n";
        return InstrumNum;
    }
    
    inline unsigned VisitFunction ()
    {
        unsigned InstrumNum = 0;
        set <Value*> DefSets;
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *Func = &*it;
            if (Func->isIntrinsic() || Func->isDeclaration ())
            {
                continue;
            }

            FSda *Fd = GetFlda (Func->getName ().data());
            if (Fd == NULL || !ValidCheck (Fd, Func))
            {
                continue;
            }

            m_Value2Id.clear ();
            //errs()<<"Process Function: "<<Func->getName ()<<"\r\n";

            VisitInst (Func, Fd);
            InstrumNum++;

            /* basic instrumentation */
            ParaFt Pf;
            DefSets.clear ();            
            for (auto IIt = Fd->tt_begin(), IEd = Fd->tt_end(); IIt != IEd; ++IIt) 
            {
                unsigned InstID = IIt->first;
                Instruction *CurInst = IIt->second;

                if (CurInst->getOpcode() == Instruction::Invoke)
                {
                    continue;
                }
                
                DefSets.insert (CurInst);
                unsigned long EventId = Fd->GetEventID (InstID);
                if (EventId == 0)
                {
                    //errs ()<<"INST: "<<*CurInst<<" ---- EventId==0, pass...\r\n";  
                    continue;
                }

                Pf.Reset ();
                GetInstrPara (Fd, InstID, CurInst, &Pf, &DefSets);
                if (Pf.m_Format != "")
                {
                    Instruction *InstrumentSite = Fd->GetInstById (InstID+1);
                    if (InstrumentSite == NULL || CurInst->getOpcode() == Instruction::Ret)
                    {
                        InstrumentSite = CurInst;
                    }
                    else
                    {
                        if (InstrumentSite->getOpcode() == Instruction::PHI)
                        {
                            InstrumentSite = Fd->GetFirstNonPHI(CurInst->getParent ());
                        }

                        if (isa<LandingPadInst>(InstrumentSite))
                        {
                            continue;
                        }
                    }
    
                    //errs ()<<"\r\nINST: "<<*CurInst<<"\r\n";            
                    //errs ()<<"\t=>InstrumentSite: "<<*InstrumentSite<<"\r\n";

                    EventId = Fd->GetEventID (InstID);
                    AddTrack (EventId, InstrumentSite, &Pf);
                }
            }

            /* pthread_create instrumentation */
            for (auto ThrIt = Fd->thr_begin (), ThrEnd = Fd->thr_end (); ThrIt != ThrEnd; ThrIt++)
            {
                unsigned InstId       = *ThrIt;
                Instruction *ThrcInst = Fd->GetInstById (InstId);
                Instruction *InstrumentSite = Fd->GetInstById (InstId+1);
                assert (InstrumentSite != NULL);

                InstrumThrc (Fd, InstId, ThrcInst, InstrumentSite);
            }

            AddCallTrack (Func, Fd);
        }

        //DumpInsts ();
        printf ("@@@ Instrument function num: %u \r\n", InstrumNum);
        return InstrumNum;
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
    
    inline unsigned InitInstrumenter ()
    {
        Function *mainFunc = m_Module->getFunction("main");
        if (NULL == mainFunc)
        {
            return 0;
        }
                
        BasicBlock *entryBlock = &mainFunc->front();
        CallInst::Create(m_InitFunc, "", entryBlock->getFirstNonPHI());
        //errs()<<"@@@@@ Instrument Init Function...\r\n";

        GetTermInstofFunction(mainFunc);
        for (auto it = m_ExitInsts.begin(); it != m_ExitInsts.end(); ++it) 
        {
            Instruction *Inst = *it;
            CallInst::Create(m_ExitFunc, "", Inst);
        }
        
        return 1;
    }


    
    inline void DumpInsts ()
    {
        for (Module::iterator it = m_Module->begin(), eit = m_Module->end(); it != eit; ++it) 
        {
            Function *F = &*it;  
            for (auto ItI = inst_begin(*F), Ed = inst_end(*F); ItI != Ed; ++ItI) 
            {
                Instruction *Inst = &*ItI;
                errs ()<<"[*DumpInsts*] -> "<<*Inst<<"\r\n";
            }
        }

        return;
    }
};



#endif // LLVM_LIB_TRANSFORMS_INSTRUMENTATION_INSTRUMENTER_H
